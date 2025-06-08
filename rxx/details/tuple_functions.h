// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/get_element.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <typename F, typename TupleLike>
requires tuple_like<std::remove_cvref_t<TupleLike>>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto transform(
    F&& callable, TupleLike&& tuple) {
    return std::apply(
        [&]<typename... Ts>(Ts&&... elements) {
            return std::tuple<std::invoke_result_t<F&, Ts>...>(
                std::invoke(callable, std::forward<Ts>(elements))...);
        },
        std::forward<TupleLike>(tuple));
}

template <typename F, typename TupleLike>
requires tuple_like<std::remove_cvref_t<TupleLike>>
__RXX_HIDE_FROM_ABI constexpr void for_each(F&& callable, TupleLike&& tuple) {
    std::apply(
        [&]<typename... Ts>(Ts&&... elements) {
            (static_cast<void>(
                 std::invoke(callable, std::forward<Ts>(elements))),
                ...);
        },
        std::forward<TupleLike>(tuple));
}

} // namespace details

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
