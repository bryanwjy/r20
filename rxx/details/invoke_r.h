// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <functional>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename R, typename F, typename... Args>
requires std::is_invocable_r_v<R, F, Args...>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE) inline constexpr R invoke_r(F&& f,
    Args&&... args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>) {
    if constexpr (std::is_void_v<R>) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    } else {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
}

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
