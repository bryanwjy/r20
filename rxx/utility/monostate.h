// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <compare>
#if RXX_LIBSTDCXX
#  if __has_include(<bits/functional_hash.h>)
#    include <bits/functional_hash.h>
#  else
RXX_STD_NAMESPACE_BEGIN
template <typename>
struct hash;
RXX_STD_NAMESPACE_END
#  endif
#elif RXX_LIBCXX

#  if __has_include(<__functional/hash.h>)
#    include <__functional/hash.h>
#  else
RXX_STD_NAMESPACE_BEGIN
template <typename>
struct hash;
RXX_STD_NAMESPACE_END
#  endif
#else
RXX_STD_NAMESPACE_BEGIN
template <typename>
struct hash;
RXX_STD_NAMESPACE_END
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

struct __RXX_ABI_PUBLIC monostate {};

RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr bool operator==(monostate, monostate) noexcept {
    return true;
}

RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto operator<=>(monostate, monostate) noexcept {
    return std::strong_ordering::equal;
}

RXX_DEFAULT_NAMESPACE_END

template <>
struct std::hash<__RXX monostate> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX monostate) RXX_CONST_CALL noexcept {
        return 66740831;
    }
};
