// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/swap.h"
#include "rxx/details/class_or_enum.h"
#include "rxx/iter_traits.h"
#include "rxx/iterator/iter_move.h"

#include <iterator>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {
template <typename I1, typename I2>
void iter_swap(I1, I2) = delete;

template <typename T1, typename T2>
concept unqualified_iter_swap = (class_or_enum<std::remove_cvref_t<T1>> ||
    class_or_enum<std::remove_cvref_t<T2>>)&&requires {
    iter_swap(std::declval<T1>(), std::declval<T2>());
};

template <typename T1, typename T2>
concept readable_swappable =
    std::indirectly_readable<T1> && std::indirectly_readable<T2> &&
    std::swappable_with<iter_reference_t<T1>, iter_reference_t<T2>>;

struct iter_swap_t {
    template <typename T1, typename T2>
    requires unqualified_iter_swap<T1, T2>
    __RXX_HIDE_FROM_ABI constexpr void operator()(T1&& left, T2&& right) const
        noexcept(noexcept(
            iter_swap(std::forward<T1>(left), std::forward<T2>(right)))) {
        (void)iter_swap(std::forward<T1>(left), std::forward<T2>(right));
    }

    template <typename T1, typename T2>
    requires (!unqualified_iter_swap<T1, T2>) && readable_swappable<T1, T2>
    __RXX_HIDE_FROM_ABI constexpr void operator()(T1&& left, T2&& right) const
        noexcept(noexcept(
            ranges::swap(*std::forward<T1>(left), *std::forward<T2>(right)))) {
        ranges::swap(*std::forward<T1>(left), *std::forward<T2>(right));
    }

    template <typename T1, typename T2>
    requires (!unqualified_iter_swap<T1, T2> &&     //
                 !readable_swappable<T1, T2>) &&    //
        std::indirectly_movable_storable<T1, T2> && //
        std::indirectly_movable_storable<T2, T1>
    __RXX_HIDE_FROM_ABI constexpr void operator()(T1&& left, T2&& right) const
        noexcept(noexcept(iter_value_t<T2>(ranges::iter_move(right))) && //
            noexcept(*right = ranges::iter_move(left)) &&                //
            noexcept(
                *std::forward<T1>(left) = std::declval<iter_value_t<T2>>())) {
        iter_value_t<T2> old(ranges::iter_move(right));
        *right = ranges::iter_move(left);
        *std::forward<T1>(left) = std::move(old);
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::iter_swap_t iter_swap{};
} // namespace cpo
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
