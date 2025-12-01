// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/tuple.h"
#include "rxx/utility/forward.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr tuple<Args&&...> forward_as_tuple(
    Args&&... args RXX_LIFETIMEBOUND) noexcept {
    return {__RXX forward<Args>(args)...};
}

template <template <typename...> class Tuple, typename... Args>
requires requires { typename Tuple<Args&&...>; } &&
    tuple_like<Tuple<Args&&...>> &&
    std::constructible_from<Tuple<Args&&...>, Args&&...>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr Tuple<Args&&...> forward_as(
    Args&&... args RXX_LIFETIMEBOUND) noexcept(std::
        is_nothrow_constructible_v<Tuple<Args&&...>, Args&&...>) {
    return Tuple<Args&&...>(__RXX forward<Args>(args)...);
}

RXX_DEFAULT_NAMESPACE_END
