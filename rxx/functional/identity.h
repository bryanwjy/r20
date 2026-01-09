// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/utility.h"

RXX_DEFAULT_NAMESPACE_BEGIN
struct identity {
    using is_transparent = void;

    template <typename T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr T&& operator()(T&& arg) RXX_CONST_CALL noexcept {
        return __RXX forward<T>(arg);
    }
};
RXX_DEFAULT_NAMESPACE_END
