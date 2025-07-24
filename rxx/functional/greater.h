// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/functional/less.h"

#include <concepts>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

struct greater : private less {
    using is_transparent = void;

    template <typename L, typename R>
    requires std::totally_ordered_with<L, R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(L&& left, R&& right) RXX_CONST_CALL
        noexcept(noexcept(std::declval<R>() < std::declval<L>())) {
        return less::operator()(std::forward<R>(right), std::forward<L>(left));
    }
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
