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

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

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
    operator in_value_result<I2, T2>() const& {
        return {in, value};
    }

    template <typename I2, typename T2>
    requires std::convertible_to<I, I2> && std::convertible_to<T, T2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr
    operator in_value_result<I2, T2>() && {
        return {std::move(in), std::move(value)};
    }
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
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END

#ifndef RXX_DISABLE_RANGES_ALGO_STD_DEDUCTION
#  define RXX_DISABLE_RANGES_ALGO_STD_DEDUCTION 0
#endif

#if RXX_CXX23 & !RXX_DISABLE_RANGES_ALGO_STD_DEDUCTION
RXX_STD_NAMESPACE_BEGIN
namespace ranges {
// TODO
} // namespace ranges
RXX_STD_NAMESPACE_END
#endif
