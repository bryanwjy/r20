// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <concepts>
#if __has_include(<__algorithm/in_fun_result.h>)
#  include <__algorithm/in_fun_result.h>
#endif
#if __has_include(<__algorithm/in_in_result.h>)
#  include <__algorithm/in_in_result.h>
#endif
#if __has_include(<__algorithm/in_out_result.h>)
#  include <__algorithm/in_out_result.h>
#endif
#if __has_include(<__algorithm/in_in_out_result.h>)
#  include <__algorithm/in_in_out_result.h>
#endif
#if __has_include(<__algorithm/in_out_out_result.h>)
#  include <__algorithm/in_out_out_result.h>
#endif
#if __has_include(<__algorithm/min_max_result.h>)
#  include <__algorithm/min_max_result.h>
#endif
#if __has_include(<__algorithm/in_found_result.h>)
#  include <__algorithm/in_found_result.h>
#endif

#if !RXX_LIBCXX
#  include <algorithm>
#endif

#ifndef RXX_DISABLE_RANGES_ALGO_STD_INTEROP
#  define RXX_DISABLE_RANGES_ALGO_STD_INTEROP 0
#endif

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
namespace details {
template <template <typename...> class To, template <typename...> class From,
    typename... Ts>
To<Ts...> rebind_as(From<Ts...>&&) noexcept;

template <template <typename...> class To, typename From>
requires requires { rebind_as(std::declval<From>()); } &&
    (!std::same_as<From, decltype(rebind_as(std::declval<From>()))>)
using rebind_as_t = decltype(rebind_as(std::declval<From>()));
} // namespace details

using std::ranges::in_fun_result;

using std::ranges::in_in_result;

using std::ranges::in_out_result;

using std::ranges::in_in_out_result;

using std::ranges::in_out_out_result;

using std::ranges::min_max_result;

using std::ranges::in_found_result;

template <typename I, typename T>
struct in_value_result {
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) I in;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T value;

    template <typename I2, typename T2>
    requires std::convertible_to<I const&, I2> &&
        std::convertible_to<T const&, T2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator in_value_result<I2, T2>() const& noexcept(
        std::is_nothrow_copy_constructible_v<I> &&
        std::is_nothrow_copy_constructible_v<T>) {
        return {in, value};
    }

    template <typename I2, typename T2>
    requires std::convertible_to<I, I2> && std::convertible_to<T, T2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator in_value_result<I2, T2>() && noexcept(
        std::is_nothrow_move_constructible_v<I> &&
        std::is_nothrow_move_constructible_v<T>) {
        return {std::move(in), std::move(value)};
    }

#if RXX_CXX23 & !RXX_DISABLE_RANGES_ALGO_STD_INTEROP
    template <template <typename, typename> class Other>
    requires requires(in_value_result self, void (*func)(Other<I, T>)) {
        requires !std::same_as<Other<I, T>, in_value_result>;
        Other<I, T>{.in = self.in, .value = self.value};
        func({self.in, self.value});
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other<I, T>() const& noexcept(
        std::is_nothrow_copy_constructible_v<I> &&
        std::is_nothrow_copy_constructible_v<T>) {
        return {in, value};
    }

    template <template <typename, typename> class Other>
    requires requires(in_value_result self, void (*func)(Other<I, T>)) {
        requires !std::same_as<Other<I, T>, in_value_result>;
        Other<I, T>{.in = std::move(self.in), .value = std::move(self.value)};
        func({std::move(self.in), std::move(self.value)});
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other<I, T>() && noexcept(
        std::is_nothrow_move_constructible_v<I> &&
        std::is_nothrow_move_constructible_v<T>) {
        return {std::move(in), std::move(value)};
    }

    template <typename Other>
    requires requires {
        typename details::rebind_as_t<in_value_result, Other>;
        requires std::convertible_to<in_value_result const&,
            details::rebind_as_t<in_value_result, Other>>;
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other() const& noexcept(
        std::is_nothrow_convertible_v<in_value_result const&,
            details::rebind_as_t<in_value_result, Other>>) {
        return {in, value};
    }

    template <typename Other>
    requires requires {
        typename details::rebind_as_t<in_value_result, Other>;
        requires std::convertible_to<in_value_result,
            details::rebind_as_t<in_value_result, Other>>;
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other() && noexcept(std::is_nothrow_convertible_v<in_value_result,
        details::rebind_as_t<in_value_result, Other>>) {
        return {std::move(in), std::move(value)};
    }
#endif
};

template <typename O, typename T>
struct out_value_result {
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) O out;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T value;

    template <typename O2, typename T2>
    requires std::convertible_to<O const&, O2> &&
        std::convertible_to<T const&, T2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator out_value_result<O2, T2>() const& {
        return {out, value};
    }

    template <typename O2, typename T2>
    requires std::convertible_to<O, O2> && std::convertible_to<T, T2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator out_value_result<O2, T2>() && {
        return {std::move(out), std::move(value)};
    }

#if RXX_CXX23 & !RXX_DISABLE_RANGES_ALGO_STD_INTEROP
    template <template <typename, typename> class Other>
    requires requires(out_value_result self, void (*func)(Other<O, T>)) {
        requires !std::same_as<Other<O, T>, out_value_result>;
        Other<O, T>{.out = self.out, .value = self.value};
        func({self.out, self.value});
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other<O, T>() const& noexcept(
        std::is_nothrow_copy_constructible_v<O> &&
        std::is_nothrow_copy_constructible_v<T>) {
        return {out, value};
    }

    template <template <typename, typename> class Other>
    requires requires(out_value_result self, void (*func)(Other<O, T>)) {
        requires !std::same_as<Other<O, T>, out_value_result>;
        Other<O, T>{.out = std::move(self.out), .value = std::move(self.value)};
        func({std::move(self.out), std::move(self.value)});
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other<O, T>() && noexcept(
        std::is_nothrow_move_constructible_v<O> &&
        std::is_nothrow_move_constructible_v<T>) {
        return {std::move(out), std::move(value)};
    }

    template <typename Other>
    requires requires {
        typename details::rebind_as_t<out_value_result, Other>;
        requires std::convertible_to<out_value_result const&,
            details::rebind_as_t<out_value_result, Other>>;
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other() const& noexcept(
        std::is_nothrow_convertible_v<out_value_result const&,
            details::rebind_as_t<out_value_result, Other>>) {
        return {out, value};
    }

    template <typename Other>
    requires requires {
        typename details::rebind_as_t<out_value_result, Other>;
        requires std::convertible_to<out_value_result,
            details::rebind_as_t<out_value_result, Other>>;
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator Other() && noexcept(std::is_nothrow_convertible_v<out_value_result,
        details::rebind_as_t<out_value_result, Other>>) {
        return {std::move(out), std::move(value)};
    }
#endif
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END

#if RXX_CXX23 & !RXX_DISABLE_RANGES_ALGO_STD_INTEROP
RXX_STD_NAMESPACE_BEGIN
namespace ranges {
template <typename I>
in_value_result(__RXX ranges::in_value_result<I> const&)
    -> in_value_result<I>;
template <typename I>
in_value_result(__RXX ranges::in_value_result<I>&&) -> in_value_result<I>;

template <typename I>
out_value_result(__RXX ranges::out_value_result<I> const&)
    -> out_value_result<I>;
template <typename I>
out_value_result(__RXX ranges::out_value_result<I>&&) -> out_value_result<I>;
} // namespace ranges
RXX_STD_NAMESPACE_END
#endif
