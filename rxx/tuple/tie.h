// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/tuple.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr tuple<Args&...> tie(Args&... args RXX_LIFETIMEBOUND) noexcept {
    return {args...};
}

RXX_DEFAULT_NAMESPACE_END
