// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/ceil_div.h"
#include "rxx/details/const_if.h"
#include "rxx/details/iterator_category_of.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"

#include <cassert>
#include <compare>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <input_range V>
requires view<V>
class stride_view : public view_interface<stride_view<V>> {
    template <bool>
    class iterator;

public:
    __RXX_HIDE_FROM_ABI explicit constexpr stride_view(V base,
        range_difference_t<V>
            stride) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_move_constructible_v<range_difference_t<V>>)
        : base_{std::move(base)}
        , stride_{stride} {
        assert(stride > 0);
    }

    __RXX_HIDE_FROM_ABI constexpr auto stride() const noexcept {
        return stride_;
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
    requires (!details::simple_view<V>)
    {
        return iterator<false>{*this, ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires range<V const>
    {
        return iterator<true>{*this, ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        if constexpr (common_range<V> && sized_range<V> && forward_range<V>) {
            auto const missing =
                (stride_ - ranges::distance(base_) % stride_) % stride_;
            return iterator<false>{*this, ranges::end(base_), missing};
        } else if constexpr (common_range<V> && !bidirectional_range<V>) {
            return iterator<false>{*this, ranges::end(base_)};
        } else {
            return std::default_sentinel;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires range<V const>
    {
        if constexpr (common_range<V const> && sized_range<V const> &&
            forward_range<V const>) {
            auto const missing =
                (stride_ - ranges::distance(base_) % stride_) % stride_;
            return iterator<true>{*this, ranges::end(base_), missing};
        } else if constexpr (common_range<V const> &&
            !bidirectional_range<V const>) {
            return iterator<true>{*this, ranges::end(base_)};
        } else {
            return std::default_sentinel;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return details::to_unsigned_like(
            details::ceil_div(ranges::distance(base_), stride_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return details::to_unsigned_like(
            details::ceil_div(ranges::distance(base_), stride_));
    }

private:
    V base_;
    range_difference_t<V> stride_;
};

template <typename R>
stride_view(R&&, range_difference_t<R>) -> stride_view<views::all_t<R>>;

namespace details {
template <bool Const, input_range V>
requires view<V>
struct stride_view_iterator_category {};

template <bool Const, forward_range V>
requires view<V>
struct stride_view_iterator_category<Const, V> {
    using iterator_category = decltype([]() {
        if constexpr (std::derived_from<iterator_category_of<Const, V>,
                          std::random_access_iterator_tag>) {
            return std::random_access_iterator_tag{};
        } else {
            return iterator_category_of<Const, V>{};
        }
    }());
};
} // namespace details

template <input_range V>
requires view<V>
template <bool Const>
class stride_view<V>::iterator :
    public details::stride_view_iterator_category<Const, V> {
    using Parent RXX_NODEBUG = details::const_if<Const, stride_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    friend stride_view;

public:
    using difference_type = range_difference_t<Base>;
    using value_type = range_value_t<Base>;
    using iterator_concept = decltype([]() {
        if constexpr (random_access_range<Base>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (bidirectional_range<Base>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (forward_range<Base>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());

    __RXX_HIDE_FROM_ABI iterator() noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>> &&
        std::is_nothrow_default_constructible_v<sentinel_t<Base>>)
    requires std::default_initializable<iterator_t<Base>> &&
        std::default_initializable<sentinel_t<Base>>
    = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>> &&
                 std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : current_{std::move(other.current_)}
        , end_{std::move(other.end_)}
        , stride_{other.stride_}
        , missing_{other.missing_} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> base() && noexcept(
        std::is_nothrow_move_constructible_v<iterator_t<Base>>) {
        return std::move(current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> const& base() const& noexcept {
        return current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const noexcept(
        noexcept(*std::declval<iterator_t<Base> const&>())) -> decltype(auto) {
        return *current_;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        assert(current_ != end_);
        missing_ = ranges::advance(current_, stride_, end_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires forward_range<Base>
    {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires bidirectional_range<Base>
    {
        ranges::advance(current_, missing_ - stride_);
        missing_ = 0;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires bidirectional_range<Base>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires random_access_range<Base>
    {
        if (offset > 0) {
            assert(ranges::distance(current_, end_) > stride_ * (offset - 1));
            missing_ = ranges::advance(current_, stride_ * offset, end_);
        } else if (offset < 0) {
            ranges::advance(current_, stride_ * offset + missing_);
            missing_ = 0;
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires random_access_range<Base>
    {
        return *this += -offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type offset) const -> decltype(auto)
    requires random_access_range<Base>
    {
        return *(*this + offset);
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
        return iter + offset;
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
        auto const diff = left.current_ - right.current_;
        if constexpr (forward_range<Base>) {
            return (diff + left.missing_ - right.missing_) / left.stride_;
        } else if (diff < 0) {
            return -details::ceil_div(-diff, left.stride_);
        } else {
            return details::ceil_div(diff, left.stride_);
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        std::default_sentinel_t, iterator const& iter)
    requires std::sized_sentinel_for<sentinel_t<Base>, iterator_t<Base>>
    {
        return details::ceil_div(iter.end_ - iter.current_, iter.stride_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& iter, std::default_sentinel_t end)
    requires std::sized_sentinel_for<sentinel_t<Base>, iterator_t<Base>>
    {
        return -(end - iter);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& iter, std::default_sentinel_t) {
        return iter.current_ == iter.end_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires std::equality_comparable<iterator_t<Base>>
    {
        return left.current_ == right.current_;
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
        return right.current_ < left.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<=(
        iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return !(right.current_ < left.current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>=(
        iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return !(left.current_ < right.current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires random_access_range<Base> &&
        std::three_way_comparable<iterator_t<Base>>
    {
        return left.current_ <=> right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_rvalue_reference_t<Base>
    iter_move(iterator const& iter) noexcept(
        noexcept(ranges::iter_move(iter.current_))) {
        return ranges::iter_move(iter.current_);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(iterator const& left, iterator const& right) noexcept(
        noexcept(ranges::iter_swap(left.current_, right.current_)))
    requires std::indirectly_swappable<iterator_t<Base>>
    {
        ranges::iter_swap(left.current_, right.current_);
    }

private:
    __RXX_HIDE_FROM_ABI constexpr iterator(Parent& parent,
        iterator_t<Base> current, range_difference_t<Base> missing = 0)
        : current_{std::move(current)}
        , end_{ranges::end(parent.base_)}
        , stride_{parent.stride_}
        , missing_{missing} {}

    iterator_t<Base> current_{};
    sentinel_t<Base> end_{};
    range_difference_t<Base> stride_ = 0;
    range_difference_t<Base> missing_ = 0;
};

namespace views {
namespace details {
struct stride_t : ranges::details::adaptor_non_closure<stride_t> {

    template <viewable_range R, typename D = range_difference_t<R>>
    requires requires { stride_view(std::declval<R>(), std::declval<D>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& arg, std::type_identity_t<D> size) RXX_CONST_CALL
        noexcept(noexcept(stride_view(std::declval<R>(), std::declval<D>()))) {
        return stride_view(std::forward<R>(arg), size);
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<stride_t>::operator();
    static constexpr bool _S_has_simple_extra_args = true;
    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename D>
    requires std::constructible_from<std::decay_t<D>, D>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(D&& size) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<D>, D>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(*this),
            std::forward<D>(size));
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::stride_t stride{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename V>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::stride_view<V>> =
        std::ranges::enable_borrowed_range<V>;
