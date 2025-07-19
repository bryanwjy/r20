// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/functional/equal_to.h"
#include "rxx/functional/is_transparent.h"

#include <concepts>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
struct not_equal_to : private equal_to {
    using is_transparent = details::is_transparent;

    template <typename L, typename R>
    requires std::equality_comparable_with<L, R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(L&& left, R&& right) RXX_CONST_CALL
        noexcept(noexcept(std::declval<L>() == std::declval<R>())) {
        return !equal_to::operator()(
            std::forward<L>(left), std::forward<R>(right));
    }
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
