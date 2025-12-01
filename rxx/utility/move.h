// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/utility/as_const.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T>
RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
constexpr std::remove_reference_t<T>&& move(
    T&& ref RXX_LIFETIMEBOUND) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(ref);
}

template <typename T>
RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
constexpr decltype(auto)
    move_if_noexcept(T& ref RXX_LIFETIMEBOUND) noexcept {
    if constexpr (!std::is_nothrow_move_constructible_v<T> &&
        std::is_copy_constructible_v<T>) {
        return __RXX as_const(ref);
    } else {
        return __RXX move(ref);
    }
}

RXX_DEFAULT_NAMESPACE_END
