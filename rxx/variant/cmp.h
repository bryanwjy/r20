// Copyright 2025-2026 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/concepts/core_convertible_to.h"
#include "rxx/variant/get.h"
#include "rxx/variant/visit.h"

#include <compare>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <core_convertible_to<bool>... Ts>
using variant_cmp_result_t = bool;

template <typename... Ts>
using variant_eq_result_t =
    variant_cmp_result_t<decltype(std::declval<Ts const&>() ==
        std::declval<Ts const&>())...>;

template <typename... Ts>
using variant_ne_result_t =
    variant_cmp_result_t<decltype(std::declval<Ts const&>() !=
        std::declval<Ts const&>())...>;

template <typename... Ts>
using variant_lt_result_t =
    variant_cmp_result_t<decltype(std::declval<Ts const&>() <
        std::declval<Ts const&>())...>;

template <typename... Ts>
using variant_gt_result_t =
    variant_cmp_result_t<decltype(std::declval<Ts const&>() >
        std::declval<Ts const&>())...>;

template <typename... Ts>
using variant_le_result_t =
    variant_cmp_result_t<decltype(std::declval<Ts const&>() <=
        std::declval<Ts const&>())...>;

template <typename... Ts>
using variant_ge_result_t =
    variant_cmp_result_t<decltype(std::declval<Ts const&>() >=
        std::declval<Ts const&>())...>;
} // namespace details

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr details::variant_eq_result_t<Ts...> operator==(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() == std::declval<Ts const&>()))) {
    return left.index() == right.index() &&
        details::visit_table_for<variant<Ts...>>(
            [&]<size_t I>(std::integral_constant<size_t, I>) -> bool {
                if constexpr (I == variant_npos) {
                    return true;
                } else {
                    return *get_if<I>(RXX_BUILTIN_addressof(left)) ==
                        *get_if<I>(RXX_BUILTIN_addressof(right));
                }
            },
            left.index());
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr details::variant_ne_result_t<Ts...> operator!=(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() != std::declval<Ts const&>()))) {
    return left.index() != right.index() ||
        details::visit_table_for<variant<Ts...>>(
            [&]<size_t I>(std::integral_constant<size_t, I>) -> bool {
                if constexpr (I == variant_npos) {
                    return false;
                } else {
                    return *get_if<I>(RXX_BUILTIN_addressof(left)) !=
                        *get_if<I>(RXX_BUILTIN_addressof(right));
                }
            },
            left.index());
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr details::variant_lt_result_t<Ts...> operator<(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() < std::declval<Ts const&>()))) {
    using ssize_t = std::make_signed_t<size_t>;
    return ((ssize_t)left.index()) < ((ssize_t)right.index()) ||
        (left.index() == right.index() &&
            details::visit_table_for<variant<Ts...>>(
                [&]<size_t I>(std::integral_constant<size_t, I>) -> bool {
                    if constexpr (I == variant_npos) {
                        return false;
                    } else {
                        return *get_if<I>(RXX_BUILTIN_addressof(left)) <
                            *get_if<I>(RXX_BUILTIN_addressof(right));
                    }
                },
                left.index()));
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr details::variant_gt_result_t<Ts...> operator>(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() > std::declval<Ts const&>()))) {
    using ssize_t = std::make_signed_t<size_t>;
    return ((ssize_t)left.index()) > ((ssize_t)right.index()) ||
        (left.index() == right.index() &&
            details::visit_table_for<variant<Ts...>>(
                [&]<size_t I>(std::integral_constant<size_t, I>) -> bool {
                    if constexpr (I == variant_npos) {
                        return false;
                    } else {
                        return *get_if<I>(RXX_BUILTIN_addressof(left)) >
                            *get_if<I>(RXX_BUILTIN_addressof(right));
                    }
                },
                left.index()));
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr details::variant_le_result_t<Ts...> operator<=(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() < std::declval<Ts const&>()))) {
    using ssize_t = std::make_signed_t<size_t>;
    return ((ssize_t)left.index()) < ((ssize_t)right.index()) ||
        (left.index() == right.index() &&
            details::visit_table_for<variant<Ts...>>(
                [&]<size_t I>(std::integral_constant<size_t, I>) -> bool {
                    if constexpr (I == variant_npos) {
                        return true;
                    } else {
                        return *get_if<I>(RXX_BUILTIN_addressof(left)) <=
                            *get_if<I>(RXX_BUILTIN_addressof(right));
                    }
                },
                left.index()));
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr details::variant_ge_result_t<Ts...> operator>=(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() >= std::declval<Ts const&>()))) {
    using ssize_t = std::make_signed_t<size_t>;
    return ((ssize_t)left.index()) > ((ssize_t)right.index()) ||
        (left.index() == right.index() &&
            details::visit_table_for<variant<Ts...>>(
                [&]<size_t I>(std::integral_constant<size_t, I>) -> bool {
                    if constexpr (I == variant_npos) {
                        return true;
                    } else {
                        return *get_if<I>(RXX_BUILTIN_addressof(left)) >=
                            *get_if<I>(RXX_BUILTIN_addressof(right));
                    }
                },
                left.index()));
}

template <typename... Ts>
requires (... && std::three_way_comparable<Ts>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=>(
    variant<Ts...> const& left, variant<Ts...> const& right) noexcept((... &&
    noexcept(std::declval<Ts const&>() <=> std::declval<Ts const&>()))) {
    using ssize_t = std::make_signed_t<size_t>;
    using result = std::common_comparison_category_t<std::strong_ordering,
        std::compare_three_way_result_t<ssize_t>,
        std::compare_three_way_result_t<Ts>...>;

    if (auto cmp = (ssize_t)left.index() <=> (ssize_t)right.index(); cmp != 0) {
        return [&]() -> result { return cmp; }();
    }

    return details::visit_table_for<variant<Ts...>>(
        [&]<size_t I>(std::integral_constant<size_t, I>) -> result {
            if constexpr (I == variant_npos) {
                return 0 <=> 0;
            } else {
                return *get_if<I>(RXX_BUILTIN_addressof(left)) <=>
                    *get_if<I>(RXX_BUILTIN_addressof(right));
            }
        },
        left.index());
}

RXX_DEFAULT_NAMESPACE_END
