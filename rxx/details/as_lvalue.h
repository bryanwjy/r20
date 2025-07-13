// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T& as_lvalue(T&& val RXX_LIFETIMEBOUND) noexcept {
    return static_cast<T&>(val);
}

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
