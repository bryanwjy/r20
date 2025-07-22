// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

#if RXX_LIBCXX
template <typename T>
requires requires(T arg) { std::__to_unsigned_like(arg); }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN,
    NODISCARD) inline constexpr auto to_unsigned_like(T arg) noexcept {
    return std::__to_unsigned_like(arg);
}
#elif RXX_LIBSTDCXX
template <typename T>
requires requires(T arg) { std::ranges::__detail::__to_unsigned_like(arg); }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN,
    NODISCARD) inline constexpr auto to_unsigned_like(T arg) noexcept {
    return std::ranges::__detail::__to_unsigned_like(arg);
}
#elif RXX_MSVC_STL
template <typename T>
requires requires(T arg) { std::_To_unsigned_like(arg); }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto to_unsigned_like(
    T arg) noexcept {
    return std::_To_unsigned_like(arg);
}
#else
#  error "Unsupported"
#endif
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
