// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <compare>

RXX_DEFAULT_NAMESPACE_BEGIN

struct __RXX_ABI_PUBLIC monostate {
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
    constexpr bool operator==(monostate const&) const noexcept { return true; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
    constexpr auto operator<=>(monostate const&) const noexcept {
        return std::strong_ordering::equal;
    }
};

RXX_DEFAULT_NAMESPACE_END
