// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/totally_ordered_with.h"
#include "rxx/functional/less.h"

#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

struct greater_equal : private less {
    using is_transparent = void;

    template <typename L, typename R>
    requires totally_ordered_with<L, R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(L&& left, R&& right) RXX_CONST_CALL
        noexcept(noexcept(std::declval<L>() < std::declval<R>())) {
        return !less::operator()(std::forward<L>(left), std::forward<R>(right));
    }
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
