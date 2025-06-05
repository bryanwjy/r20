// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/view_match.h"
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

template <std::ranges::view View>
requires std::ranges::input_range<View>
class as_const_view : public std::ranges::view_interface<as_const_view<View>> {
public:
    __RXX_HIDE_FROM_ABI constexpr as_const_view() noexcept
    requires std::default_initializable<View>
    = default;

    __RXX_HIDE_FROM_ABI constexpr explicit as_const_view(View base) noexcept(
        std::is_nothrow_move_constructible_v<View>)
        : base_{std::move(base)} {}

    __RXX_HIDE_FROM_ABI constexpr View base() const& noexcept(
        std::is_nothrow_copy_constructible_v<View>)
    requires std::copy_constructible<View>
    {
        return base_;
    }

    __RXX_HIDE_FROM_ABI constexpr View base() && noexcept(
        std::is_nothrow_move_constructible_v<View>) {
        return std::move(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto begin()
    requires (!details::simple_view<View>)
    {
        return std::ranges::cbegin(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto begin() const
    requires std::ranges::range<View const>
    {
        return std::ranges::cbegin(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto end()
    requires (!details::simple_view<View>)
    {
        return std::ranges::cend(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto end() const
    requires std::ranges::range<View const>
    {
        return std::ranges::cend(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto size()
    requires std::ranges::sized_range<View>
    {
        return std::ranges::size(base_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto size() const
    requires std::ranges::sized_range<View const>
    {
        return std::ranges::size(base_);
    }

private:
    View base_{};
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
    template <std::ranges::viewable_range R>
    requires requires { as_const_view(std::declval<R>()); }
    __RXX_HIDE_FROM_ABI constexpr auto operator()(R&& arg) const
        noexcept(noexcept(as_const_view(std::declval<R>()))) {
        using Type = std::remove_cvref_t<R>;
        using Element = std::remove_reference_t<range_reference_t<R>>;
        if constexpr (constant_range<std::views::all_t<R>>) {
            return std::views::all(std::forward<R>(arg));
        } else if constexpr (__RXX ranges::details::is_empty_view<R>) {
            return std::views::empty<Element const>;
        } else if constexpr (__RXX ranges::details::span_type<Type>) {
            return std::span<Element const, Type::extent>(std::forward<R>(arg));
        } else if constexpr (is_constable_ref_view<R>) {
            return std::ranges::ref_view(
                std::as_const(std::forward<R>(arg).base()));
        } else if constexpr (std::is_lvalue_reference_v<R> &&
            constant_range<R const> && !std::ranges::view<R>) {
            return std::ranges::ref_view(static_cast<R const&>(arg));
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
