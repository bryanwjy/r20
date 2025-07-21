// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/const_if.h"
#include "rxx/details/iterator_category_of.h"
#include "rxx/details/non_propagating_cache.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/details/variant_base.h"
#include "rxx/functional.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/single_view.h"
#include "rxx/ranges/subrange.h"
#include "rxx/ranges/view_interface.h"

#include <cassert>
#include <compare>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {

template <auto> // do we need to constraint?
struct constant_value;

template <typename T>
concept tiny_range = sized_range<T> && requires {
    typename constant_value<std::remove_reference_t<T>::size()>;
} && (std::remove_reference_t<T>::size() <= 1);
} // namespace details

template <input_range V, forward_range P>
requires view<V> && view<P> &&
    std::indirectly_comparable<iterator_t<V>, iterator_t<P>,
        ranges::equal_to> &&
    (forward_range<V> || details::tiny_range<P>)
class lazy_split_view : public view_interface<lazy_split_view<V, P>> {

    template <bool>
    class outer_iterator;
    template <bool>
    class inner_iterator;

public:
    __RXX_HIDE_FROM_ABI constexpr lazy_split_view() noexcept(
        std::is_nothrow_default_constructible_v<V> &&
        std::is_nothrow_default_constructible_v<P>)
    requires std::default_initializable<V> && std::default_initializable<P>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr lazy_split_view(
        V base, P pattern) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_move_constructible_v<P>)
        : base_{std::move(base)}
        , pattern_{std::move(pattern)} {}

    template <input_range R>
    requires std::constructible_from<V, views::all_t<R>> &&
                 std::constructible_from<P, single_view<range_value_t<R>>>
    __RXX_HIDE_FROM_ABI explicit constexpr lazy_split_view(
        R&& range, range_value_t<R> pattern)
        : base_{views::all(std::forward<R>(range))}
        , pattern_{views::single(std::move(pattern))} {}

    __RXX_HIDE_FROM_ABI constexpr V base() const& noexcept(
        std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return base_;
    }

    __RXX_HIDE_FROM_ABI constexpr V base() && noexcept(
        std::is_nothrow_move_constructible_v<V>) {
        return std::move(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto begin() {
        if constexpr (forward_range<V>) {
            using IteratorType = outer_iterator<details::simple_view<V> &&
                details::simple_view<P>>;
            return IteratorType{*this, __RXX ranges::begin(base_)};
        } else {
            current_ = __RXX ranges::begin(base_);
            return outer_iterator<false>{*this};
        }
    }

    __RXX_HIDE_FROM_ABI constexpr auto begin() const
    requires forward_range<V> && forward_range<V const>
    {
        return outer_iterator<true>{*this, __RXX ranges::begin(base_)};
    }

    __RXX_HIDE_FROM_ABI constexpr auto end()
    requires forward_range<V> && common_range<V>
    {
        using IteratorType =
            outer_iterator<details::simple_view<V> && details::simple_view<P>>;
        return IteratorType{*this, __RXX ranges::end(base_)};
    }

    __RXX_HIDE_FROM_ABI constexpr auto end() const {
        if constexpr (forward_range<V> && forward_range<V const> &&
            common_range<V const>) {
            return outer_iterator<true>{*this, __RXX ranges::end(base_)};
        } else {
            return std::default_sentinel;
        }
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_{};
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) P pattern_{};
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    std::conditional_t<!forward_range<V>,
        details::non_propagating_cache<iterator_t<V>>, details::empty_cache>
        current_;
};

namespace details {
template <bool, typename>
struct lazy_split_view_outer_iterator_category {};

template <bool Const, typename V>
requires forward_range<details::const_if<Const, V>>
struct lazy_split_view_outer_iterator_category<Const, V> {
    using iterator_category = std::input_iterator_tag;
};
} // namespace details

template <input_range V, forward_range P>
requires view<V> && view<P> &&
    std::indirectly_comparable<iterator_t<V>, iterator_t<P>,
        ranges::equal_to> &&
    (forward_range<V> || details::tiny_range<P>)
template <bool Const>
class lazy_split_view<V, P>::outer_iterator :
    public details::lazy_split_view_outer_iterator_category<Const, V> {
    friend lazy_split_view;
    using Parent RXX_NODEBUG = details::const_if<Const, lazy_split_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;

public:
    using iterator_concept = std::conditional_t<forward_range<Base>,
        std::forward_iterator_tag, std::input_iterator_tag>;
    using difference_type = range_difference_t<Base>;
    struct value_type : view_interface<value_type> {
    public:
        friend outer_iterator;

        __RXX_HIDE_FROM_ABI explicit constexpr value_type(
            outer_iterator iter) noexcept(std::
                is_nothrow_move_constructible_v<outer_iterator>)
            : iter_{std::move(iter)} {}

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        constexpr inner_iterator<Const> begin() const
            noexcept(std::is_nothrow_constructible_v<inner_iterator<Const>,
                outer_iterator const&>) {
            return inner_iterator<Const>{iter_};
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        constexpr std::default_sentinel_t end() const noexcept {
            return std::default_sentinel;
        }

    private:
        outer_iterator iter_{};
    };

    __RXX_HIDE_FROM_ABI constexpr outer_iterator() noexcept(!forward_range<V> ||
        std::is_nothrow_default_constructible_v<iterator_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI explicit constexpr outer_iterator(
        Parent& parent) noexcept
    requires (!forward_range<Base>)
        : parent_{RXX_BUILTIN_addressof(parent)} {}

    __RXX_HIDE_FROM_ABI constexpr outer_iterator(
        Parent& parent, iterator_t<Base> current) noexcept(std::
            is_nothrow_move_constructible_v<iterator_t<Base>>)
    requires forward_range<Base>
        : parent_{RXX_BUILTIN_addressof(parent)}
        , current_{std::move(current)} {}

    __RXX_HIDE_FROM_ABI constexpr outer_iterator(
        outer_iterator<!Const> other) noexcept(!forward_range<Base> ||
        std::is_nothrow_constructible_v<iterator_t<Base>, iterator_t<V>>)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : parent_{other.parent_}
        , current_{std::move(other.current_)}
        , trailing_empty_{other.trailing_empty_} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr value_type operator*() const noexcept(
        std::is_nothrow_constructible_v<value_type, outer_iterator const&>) {
        return value_type{*this};
    }

    __RXX_HIDE_FROM_ABI
    constexpr outer_iterator& operator++() {
        auto const end = __RXX ranges::end(parent_->base_);
        if (cur() == end) {
            trailing_empty_ = false;
            return *this;
        }
        auto const [pbegin, pend] = subrange{parent_->pattern_};
        if (pbegin == pend) {
            ++cur();
        } else if constexpr (details::tiny_range<P>) {
            cur() = ranges::find(std::move(cur()), end, *pbegin);
            if (cur() != end) {
                ++cur();
                if (cur() == end) {
                    trailing_empty_ = true;
                }
            }
        } else {
            do {
                auto const [b, p] = ranges::mismatch(cur(), end, pbegin, pend);
                if (p == pend) {
                    cur() = b;
                    if (cur() == end) {
                        trailing_empty_ = true;
                    }
                    break; // The pattern matched; skip it
                }
            } while (++cur() != end);
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr auto operator++(int) -> decltype(auto) {
        if constexpr (forward_range<Base>) {
            auto prev = *this;
            ++*this;
            return prev;
        } else {
            ++*this;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(outer_iterator const& left,
        outer_iterator const&
            right) noexcept(noexcept(std::declval<iterator_t<Base>>() ==
        std::declval<iterator_t<Base>>()))
    requires forward_range<Base>
    {
        return left.current_ == right.current_ &&
            left.trailing_empty_ == right.trailing_empty_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator==(outer_iterator const& iter, std::default_sentinel_t) noexcept(
        noexcept(std::declval<iterator_t<Base>>() ==
            std::declval<sentinel_t<Base>>())) {
        // function member defined since we cannot access parent_->base_ in gcc
        // from here
        return iter.at_end();
    }

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool at_end() const noexcept(noexcept(
        std::declval<iterator_t<Base>>() == std::declval<sentinel_t<Base>>())) {

        return cur() == __RXX ranges::end(parent_->base_) && !trailing_empty_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto& cur() noexcept {
        if constexpr (forward_range<V>) {
            return current_;
        } else {
            return *parent_->current_;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto& cur() const noexcept {
        if constexpr (forward_range<V>) {
            return current_;
        } else {
            return *parent_->current_;
        }
    }

    Parent* parent_ = nullptr;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    std::conditional_t<forward_range<V>, iterator_t<Base>, details::empty_cache>
        current_{};
    bool trailing_empty_ = false;
};

namespace details {
template <bool, typename>
struct lazy_split_view_inner_iterator_category {};

template <bool Const, typename V>
requires forward_range<details::const_if<Const, V>>
struct lazy_split_view_inner_iterator_category<Const, V> {
    using iterator_category = std::conditional_t<
        std::derived_from<details::iterator_category_of<Const, V>,
            std::forward_iterator_tag>,
        std::forward_iterator_tag, details::iterator_category_of<Const, V>>;
};
} // namespace details

template <input_range V, forward_range P>
requires view<V> && view<P> &&
    std::indirectly_comparable<iterator_t<V>, iterator_t<P>,
        ranges::equal_to> &&
    (forward_range<V> || details::tiny_range<P>)
template <bool Const>
class lazy_split_view<V, P>::inner_iterator :
    public details::lazy_split_view_inner_iterator_category<Const, V> {
    using Base RXX_NODEBUG = details::const_if<Const, V>;

public:
    using iterator_concept = std::conditional_t<forward_range<Base>,
        std::forward_iterator_tag, std::input_iterator_tag>;
    using value_type = range_value_t<Base>;
    using difference_type = range_difference_t<Base>;

    __RXX_HIDE_FROM_ABI constexpr inner_iterator() noexcept(
        std::is_nothrow_default_constructible_v<outer_iterator<Const>>) =
        default;
    __RXX_HIDE_FROM_ABI explicit constexpr inner_iterator(
        outer_iterator<Const> iter) noexcept(std::
            is_nothrow_move_constructible_v<outer_iterator<Const>>)
        : iter_{std::move(iter)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> const& base() const& noexcept {
        return iter_.cur();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> base() && noexcept(
        std::is_nothrow_move_constructible_v<iterator_t<Base>>)
    requires forward_range<V>
    {
        return std::move(iter_.cur());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const
        noexcept(noexcept(*std::declval<outer_iterator<Const> const&>().cur()))
            -> decltype(auto) {
        return *iter_.cur();
    }

    __RXX_HIDE_FROM_ABI
    constexpr inner_iterator& operator++() {
        incremented_ = true;
        if constexpr (!forward_range<Base>) {
            if constexpr (P::size() == 0) {
                return *this;
            }
        }
        ++iter_.cur();
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr auto operator++(int) -> decltype(auto) {
        if constexpr (forward_range<Base>) {
            auto prev = *this;
            ++*this;
            return prev;
        } else {
            ++*this;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(inner_iterator const& left,
        inner_iterator const&
            right) noexcept(noexcept(std::declval<iterator_t<Base>>() ==
        std::declval<iterator_t<Base>>()))
    requires forward_range<Base>
    {
        return left.iter_.cur() == right.iter_.cur();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        inner_iterator const& iter, std::default_sentinel_t) {
        return iter.at_end();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto iter_move(inner_iterator const& iter) noexcept(
        noexcept(ranges::iter_move(std::declval<iterator_t<Base>>())))
        -> decltype(auto) {
        return ranges::iter_move(iter.outer_current());
    }

    __RXX_HIDE_FROM_ABI
    friend constexpr void
    iter_swap(inner_iterator const& left, inner_iterator const& right) noexcept(
        noexcept(
            ranges::iter_swap(left.outer_current(), right.outer_current())))
    requires std::indirectly_swappable<iterator_t<Base>>
    {
        ranges::iter_swap(left.outer_current(), right.outer_current());
    }

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto const& outer_current() const noexcept { return iter_.cur(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool at_end() const {
        auto [pcur, pend] = subrange{iter_.parent_->pattern_};
        auto end = __RXX ranges::end(iter_.parent_->base_);
        if constexpr (details::tiny_range<P>) {
            auto const& cur = iter_.cur();
            if (cur == end) {
                return true;
            }
            if (pcur == pend) {
                return incremented_;
            }

            return *cur == *pcur;
        } else {
            auto cur = iter_.cur();
            if (cur == end) {
                return true;
            }
            if (pcur == pend) {
                return incremented_;
            }
            do {
                if (*cur != *pcur) {
                    return false;
                }
                if (++pcur == pend) {
                    return true;
                }
            } while (++cur != end);
            return false;
        }
    }

    outer_iterator<Const> iter_{};
    bool incremented_ = false;
};

template <typename R, typename P>
lazy_split_view(R&&, P&&) -> lazy_split_view<views::all_t<R>, views::all_t<P>>;

template <input_range R>
lazy_split_view(R&&, range_value_t<R>)
    -> lazy_split_view<views::all_t<R>, single_view<range_value_t<R>>>;

namespace views {
namespace details {

struct lazy_split_t : ranges::details::adaptor_non_closure<lazy_split_t> {

    template <typename R, typename P>
    requires requires { lazy_split_view(std::declval<R>(), std::declval<P>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& range, P&& pattern) RXX_CONST_CALL noexcept(
        noexcept(lazy_split_view(std::declval<R>(), std::declval<P>()))) {
        return lazy_split_view(
            std::forward<R>(range), std::forward<P>(pattern));
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<lazy_split_t>::operator();
    template <typename P>
    static constexpr bool _S_has_simple_extra_args =
        std::is_scalar_v<P> || (view<P> && std::copy_constructible<P>);
    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename P>
    requires std::constructible_from<std::decay_t<P>, P>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(P&& pattern) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<P>, P>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(*this),
            std::forward<P>(pattern));
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::lazy_split_t lazy_split{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
