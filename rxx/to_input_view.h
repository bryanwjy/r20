// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/all.h"
#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/iterator_category_of.h"
#include "rxx/details/non_propagating_cache.h"
#include "rxx/details/simple_view.h"
#include "rxx/primitives.h"
#include "rxx/view_interface.h"

#include <cassert>
#include <compare>
#include <iterator>
#include <ranges>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
template <input_range V>
requires view<V>
class to_input_view : public view_interface<to_input_view<V>> {

    template <bool>
    class iterator;

public:
    __RXX_HIDE_FROM_ABI constexpr to_input_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr to_input_view(V base) noexcept(
        std::is_nothrow_move_constructible_v<V>)
        : base_{std::move(base)} {}

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
        return iterator<false>(ranges::begin(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires range<V const>
    {
        return iterator<true>(ranges::begin(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        return ranges::end(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires range<V const>
    {
        return ranges::end(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return ranges::size(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return ranges::size(base_);
    }

private:
    V base_{};
};

template <typename R>
to_input_view(R&&) -> to_input_view<views::all_t<R>>;

template <input_range V>
requires view<V>
template <bool Const>
class to_input_view<V>::iterator {

    friend to_input_view;
    using Base RXX_NODEBUG = details::const_if<Const, V>;

    __RXX_HIDE_FROM_ABI explicit constexpr iterator(iterator_t<Base> current)
        : current_(std::move(current)) {}

public:
    using difference_type = range_difference_t<Base>;
    using value_type = range_value_t<Base>;
    using iterator_concept = std::input_iterator_tag;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>>) = default;
    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : current_{std::move(other.current_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> base() && { return std::move(current_); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> const& base() const& noexcept { return current_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const
        noexcept(requires(iterator_t<Base> current) {
            { *current } noexcept;
        }) -> decltype(auto) {
        return *current_;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() noexcept(
        requires(iterator_t<Base> current) {
            { ++current } noexcept;
        }) {
        ++current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) noexcept(
        requires(iterator_t<Base> current) {
            { ++current } noexcept;
        }) {
        ++*this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(iterator const& left,
        sentinel_t<Base> const& right) noexcept(noexcept(left.current_ ==
        right)) {
        return left.current_ == right;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(sentinel_t<Base> const& left,
        iterator const& right) noexcept(noexcept(left - right.current_))
    requires std::sized_sentinel_for<sentinel_t<Base>, iterator_t<Base>>
    {
        return left - right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(iterator const& left,
        sentinel_t<Base> const& right) noexcept(noexcept(left.current_ - right))
    requires std::sized_sentinel_for<sentinel_t<Base>, iterator_t<Base>>
    {
        return left.current_ - right;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_rvalue_reference_t<Base>
    iter_move(iterator const& iter) noexcept(
        noexcept(std::ranges::iter_move(iter.current_))) {
        return std::ranges::iter_move(iter.current_);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(iterator const& left, iterator const& right) noexcept(
        noexcept(std::ranges::iter_swap(left.current_, right.current_)))
    requires std::indirectly_swappable<iterator_t<Base>>
    {
        std::ranges::iter_swap(left.current_, right.current_);
    }

private:
    iterator_t<Base> current_{};
};

namespace views {
namespace details {
struct to_input_t : __RXX ranges::details::adaptor_closure<to_input_t> {

    template <viewable_range R>
    requires requires { to_input_view(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(to_input_view(std::declval<R>()))) {
        if constexpr (input_range<R> && !common_range<R> && !forward_range<R>) {
            return std::views::all(std::forward<R>(arg));
        } else {
            return to_input_view(std::forward<R>(arg));
        }
    }

#if RXX_LIBSTDCXX
    static constexpr bool _S_has_simple_call_op = true;
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::to_input_t to_input{};
} // namespace cpo
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename T>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::to_input_view<T>> =
        std::ranges::enable_borrowed_range<T>;
