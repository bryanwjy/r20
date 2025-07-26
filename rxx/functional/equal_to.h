// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/equality_comparable_with.h"
#include "rxx/utility.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

struct equal_to {

    using is_transparent = void;

    template <typename L, typename R>
    requires equality_comparable_with<L, R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(L&& left, R&& right) RXX_CONST_CALL noexcept(
        noexcept(static_cast<bool>(std::declval<L>() == std::declval<R>()))) {
        return static_cast<bool>(
            std::forward<L>(left) == std::forward<R>(right));
    }
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
