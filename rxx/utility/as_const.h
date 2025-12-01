// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T>
void as_const(T&&) noexcept = delete;

template <typename T>
RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
constexpr T const& as_const(T const& value RXX_LIFETIMEBOUND) noexcept {
    return value;
}

template <typename T>
RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
constexpr T const& as_const(T& value RXX_LIFETIMEBOUND) noexcept {
    return value;
}

RXX_DEFAULT_NAMESPACE_END
