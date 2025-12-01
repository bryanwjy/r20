// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

RXX_ATTRIBUTES(_HIDE_FROM_ABI, NORETURN) inline void unreachable() noexcept {
    RXX_BUILTIN_unreachable();
}

RXX_DEFAULT_NAMESPACE_END
