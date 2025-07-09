// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/view_traits.h"
#include "rxx/get_element.h"
#include "rxx/primitives.h"

#include <compare>
#include <functional>
#include <iterator>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <view V>
requires input_range<V>
class as_const_view : public std::ranges::view_interface<as_const_view<V>> {
public:
    __RXX_HIDE_FROM_ABI constexpr as_const_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr as_const_view(V base) noexcept(
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
        return __RXX ranges::cbegin(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires range<V const>
    {
        return __RXX ranges::cbegin(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        return __RXX ranges::cend(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires range<V const>
    {
        return __RXX ranges::cend(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires std::ranges::sized_range<V>
    {
        return std::ranges::size(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires std::ranges::sized_range<V const>
    {
        return std::ranges::size(base_);
    }

private:
    V base_{};
};

template <typename R>
as_const_view(R&&) -> as_const_view<std::views::all_t<R>>;

namespace views {
namespace details {
template <typename T>
inline constexpr bool is_constable_ref_view = false;

template <typename R>
inline constexpr bool is_constable_ref_view<std::ranges::ref_view<R>> =
    constant_range<R const>;

struct as_const_t : __RXX ranges::details::adaptor_closure<as_const_t> {
    template <viewable_range R>
    requires requires { as_const_view(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        R&& arg) const noexcept(noexcept(as_const_view(std::declval<R>()))) {
        using Type = std::remove_cvref_t<R>;
        using Element = std::remove_reference_t<range_reference_t<R>>;
        if constexpr (constant_range<std::views::all_t<R>>) {
            return std::views::all(std::forward<R>(arg));
        } else if constexpr (__RXX ranges::details::is_empty_view<Type>) {
            return std::views::empty<Element const>;
        } else if constexpr (__RXX ranges::details::is_span<Type>) {
            return std::span<Element const, Type::extent>(std::forward<R>(arg));
        } else if constexpr (is_constable_ref_view<R>) {
            return std::ranges::ref_view(
                std::as_const(std::forward<R>(arg).base()));
        } else if constexpr (std::is_lvalue_reference_v<R> &&
            constant_range<Type const> && !view<Type>) {
            return std::ranges::ref_view(static_cast<Type const&>(arg));
        } else {
            return as_const_view(std::forward<R>(arg));
        }
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::as_const_t as_const{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename R>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::as_const_view<R>> =
        std::ranges::enable_borrowed_range<R>;
