// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr auto&& forward_like(U&& x) noexcept {
    constexpr bool is_adding_const =
        std::is_const_v<std::remove_reference_t<T>>;
    if constexpr (std::is_lvalue_reference_v<T&&>) {
        if constexpr (is_adding_const)
            return std::as_const(x);
        else
            return static_cast<U&>(x);
    } else if constexpr (is_adding_const)
        return std::move(std::as_const(x));
    else
        return std::move(x);
}

RXX_DEFAULT_NAMESPACE_END
