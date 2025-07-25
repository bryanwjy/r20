// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/compare/three_way_synthesizer.h"

#include <utility>

#if RXX_LIBSTDCXX && !RXX_LIBSTDCXX_AFTER(2024, 04, 16)

RXX_STD_NAMESPACE_BEGIN

template <typename L1, typename L2, typename R1, typename R2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool
operator==(pair<L1, L2> const& left, pair<R1, R2> const& right) noexcept(
    noexcept(left.first == right.first && left.second == right.second)) {
    return left.first == right.first && left.second == right.second;
}

template <typename L1, typename L2, typename R1, typename R2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::common_comparison_category_t<
    __RXX details::three_way_result_t<L1, R1>,
    __RXX details::three_way_result_t<L2, R2>>
operator<=>(pair<L1, L2> const& left, pair<R1, R2> const& right) noexcept(
    std::is_nothrow_invocable_v<__RXX details::three_way_synthesizer_t, L1,
        R1> &&
    std::is_nothrow_invocable_v<__RXX details::three_way_synthesizer_t, L2,
        R2>) {
    if (auto const cmp =
            __RXX details::three_way_synthesizer(left.first, right.first);
        cmp != 0) {
        return cmp;
    }

    return __RXX details::three_way_synthesizer(left.second, right.second);
}

RXX_STD_NAMESPACE_END

#endif
