// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T>
RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
constexpr T&& forward(
    std::remove_reference_t<T>& ref RXX_LIFETIMEBOUND) noexcept {
    return static_cast<T&&>(ref);
}

template <typename T>
RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
constexpr T&& forward(
    std::remove_reference_t<T>&& ref RXX_LIFETIMEBOUND) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value,
        "cannot forward an rvalue as an lvalue");
    return static_cast<T&&>(ref);
}

RXX_DEFAULT_NAMESPACE_END
