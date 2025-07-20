// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/cached_position.h"
#include "rxx/details/const_if.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"

#include <cassert>
#include <compare>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {
template <typename V>
concept slide_caches_nothing = random_access_range<V> && sized_range<V>;

template <typename V>
concept slide_caches_last =
    !slide_caches_nothing<V> && bidirectional_range<V> && common_range<V>;

template <typename V>
concept slide_caches_first = !slide_caches_nothing<V> && !slide_caches_last<V>;

} // namespace details

template <forward_range V>
requires view<V>
class slide_view : public view_interface<slide_view<V>> {
    template <bool>
    class iterator;
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI explicit constexpr slide_view(V base,
        range_difference_t<V>
            num) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_copy_constructible_v<range_difference_t<V>>)
        : base_{std::move(base)}
        , num_{num} {
        assert(num > 0);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const& noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return base_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() && noexcept(std::is_nothrow_move_constructible_v<V>) {
        return std::move(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (
        !(details::simple_view<V> && details::slide_caches_nothing<V const>))
    {
        if constexpr (details::slide_caches_first<V>) {
            if (!cache_begin_) {
                cache_begin_.set(base_,
                    std::ranges::next(__RXX ranges::begin(base_), num_ - 1,
                        __RXX ranges::end(base_)));
            }

            return iterator<false>(
                __RXX ranges::begin(base_), cache_begin_.get(base_), num_);
        } else {
            return iterator<false>(__RXX ranges::begin(base_), num_);
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires details::slide_caches_nothing<V const>
    {
        return iterator<true>(__RXX ranges::begin(base_), num_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (
        !(details::simple_view<V> && details::slide_caches_nothing<V const>))
    {
        if constexpr (details::slide_caches_nothing<V>)
            return iterator<false>(
                __RXX ranges::begin(base_) + range_difference_t<V>(size()),
                num_);
        else if constexpr (details::slide_caches_last<V>) {
            if (!cache_end_) {
                cache_end_.set(base_,
                    std::ranges::prev(__RXX ranges::end(base_), num_ - 1,
                        __RXX ranges::begin(base_)));
            }

            return iterator<false>(cache_end_.get(base_), num_);
        } else if constexpr (common_range<V>) {
            return iterator<false>(
                __RXX ranges::end(base_), __RXX ranges::end(base_), num_);
        } else {
            return sentinel(__RXX ranges::end(base_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires details::slide_caches_nothing<V const>
    {
        return begin() + range_difference_t<V const>(size());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        if (auto const value = std::ranges::distance(base_) - num_ + 1;
            value >= 0) {
            return details::to_unsigned_like(value);
        } else {
            return details::to_unsigned_like(static_cast<decltype(value)>(0));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        if (auto const value = std::ranges::distance(base_) - num_ + 1;
            value >= 0) {
            return details::to_unsigned_like(value);
        } else {
            return details::to_unsigned_like(static_cast<decltype(value)>(0));
        }
    }

private:
    using CacheBegin RXX_NODEBUG =
        std::conditional_t<details::slide_caches_first<V>,
            details::cached_position<V>, details::empty_cache>;
    using CacheEnd RXX_NODEBUG =
        std::conditional_t<details::slide_caches_last<V>,
            details::cached_position<V>, details::empty_cache>;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_;
    range_difference_t<V> num_;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) CacheBegin cache_begin_;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) CacheEnd cache_end_;
};

template <typename R>
slide_view(R&&, range_difference_t<R>) -> slide_view<views::all_t<R>>;

template <forward_range V>
requires view<V>
template <bool Const>
class slide_view<V>::iterator {

    friend slide_view;
    friend slide_view::sentinel;

    using Base RXX_NODEBUG = details::const_if<Const, V>;

    __RXX_HIDE_FROM_ABI constexpr iterator(
        iterator_t<Base> current, range_difference_t<Base> num)
    requires (!details::slide_caches_first<Base>)
        : current_{std::move(current)}
        , num_{num} {}

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator_t<Base> current,
        iterator_t<Base> last_ele, range_difference_t<Base> num)
    requires details::slide_caches_first<Base>
        : current_{std::move(current)}
        , last_ele_{std::move(last_ele)}
        , num_{num} {}

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype([]() {
        if constexpr (random_access_range<V>)
            return std::random_access_iterator_tag{};
        else if constexpr (bidirectional_range<V>)
            return std::bidirectional_iterator_tag{};
        else
            return std::forward_iterator_tag{};
    }());
    using value_type =
        decltype(std::views::counted(std::declval<iterator_t<Base> const&>(),
            std::declval<range_difference_t<Base> const&>()));
    using difference_type = range_difference_t<Base>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_constructible_v<iterator_t<Base>, iterator_t<V>>)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : current_{std::move(other.current)}
        , num_{other.num_} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const noexcept(
        noexcept(std::views::counted(std::declval<iterator_t<Base> const&>(),
            std::declval<range_difference_t<Base> const&>()))) {
        return std::views::counted(current_, num_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type pos) const noexcept(noexcept(
        std::views::counted(std::declval<iterator_t<Base> const&>() + pos,
            std::declval<range_difference_t<Base> const&>())))
    requires random_access_range<Base>
    {
        return std::views::counted(current_ + pos, num_);
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        ++current_;
        if constexpr (details::slide_caches_first<Base>) {
            ++last_ele_;
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int) {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires bidirectional_range<Base>
    {
        --current_;
        if constexpr (details::slide_caches_first<Base>) {
            --last_ele_;
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires bidirectional_range<Base>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type val)
    requires random_access_range<Base>
    {
        current_ += val;
        if constexpr (details::slide_caches_first<Base>) {
            last_ele_ += val;
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type val)
    requires random_access_range<Base>
    {
        current_ -= val;
        if constexpr (details::slide_caches_first<Base>) {
            last_ele_ -= val;
        }
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& iter, difference_type offset)
    requires random_access_range<Base>
    {
        auto copy = iter;
        copy += offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type offset, iterator const& iter)
    requires random_access_range<Base>
    {
        auto copy = iter;
        copy += offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& iter, difference_type offset)
    requires random_access_range<Base>
    {
        auto copy = iter;
        copy -= offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires std::sized_sentinel_for<iterator_t<Base>, iterator_t<Base>>
    {
        if constexpr (details::slide_caches_first<Base>) {
            return left.last_ele_ - right.last_ele_;
        } else {
            return left.current_ - right.current_;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right) {
        if constexpr (details::slide_caches_first<Base>) {
            return left.last_ele_ == right.last_ele_;
        } else {
            return left.current_ == right.current_;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<(iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return left.current_ < right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>(iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return right < left;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<=(
        iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return !(right < left);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>=(
        iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return !(left < right);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires random_access_range<Base> &&
        std::three_way_comparable<iterator_t<Base>>
    {
        return left.current_ <=> right.current_;
    }

private:
    using Last = std::conditional_t<details::slide_caches_first<Base>,
        iterator_t<Base>, details::empty_cache>;

    iterator_t<Base> current_{};
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) Last last_ele_{};
    range_difference_t<Base> num_ = 0;
};

template <forward_range V>
requires view<V>
class slide_view<V>::sentinel {
    friend slide_view;

    __RXX_HIDE_FROM_ABI explicit constexpr sentinel(sentinel_t<V> end) noexcept(
        std::is_nothrow_move_constructible_v<sentinel_t<V>>)
        : end_(std::move(end)) {}

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<sentinel_t<V>>) = default;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator<false> const& iter, sentinel const& sen) {
        return iter.last_ele_ == sen.end_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_difference_t<V> operator-(
        iterator<false> const& iter, sentinel const& sen)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return iter.last_ele_ - sen.end_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_difference_t<V> operator-(
        sentinel const& sen, iterator<false> const& iter)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return sen.end_ - iter.last_ele_;
    }

private:
    sentinel_t<V> end_{};
};

namespace views {
namespace details {
struct slide_t : ranges::details::adaptor_non_closure<slide_t> {
    template <viewable_range R, typename D = range_difference_t<R>>
    requires requires { slide_view(std::declval<R>(), std::declval<D>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& range, std::type_identity_t<D> num) RXX_CONST_CALL
        noexcept(noexcept(slide_view(std::declval<R>(), std::declval<D>()))) {
        return slide_view(std::forward<R>(range), num);
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<slide_t>::operator();
    static constexpr int _S_arity = 2;
    static constexpr bool _S_has_simple_extra_args = true;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename D>
    requires std::constructible_from<std::decay_t<D>, D>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(D num) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<D>, D>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(*this), num);
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::slide_t slide{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename V>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::slide_view<V>> =
        std::ranges::enable_borrowed_range<V>;
