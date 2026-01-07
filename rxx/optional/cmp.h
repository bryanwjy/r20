// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h"

#include "rxx/concepts/core_convertible_to.h"
#include "rxx/optional/nullopt.h"

#include <compare>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <core_convertible_to<bool> T>
using optional_cmp_result_t = bool;
template <typename L, typename R>
using optional_eq_result_t =
    optional_cmp_result_t<decltype(std::declval<L const&>() ==
        std::declval<R const&>())>;
template <typename L, typename R>
using optional_ne_result_t =
    optional_cmp_result_t<decltype(std::declval<L const&>() !=
        std::declval<R const&>())>;
template <typename L, typename R>
using optional_lt_result_t =
    optional_cmp_result_t<decltype(std::declval<L const&>() <
        std::declval<R const&>())>;
template <typename L, typename R>
using optional_gt_result_t =
    optional_cmp_result_t<decltype(std::declval<L const&>() >
        std::declval<R const&>())>;
template <typename L, typename R>
using optional_le_result_t =
    optional_cmp_result_t<decltype(std::declval<L const&>() <=
        std::declval<R const&>())>;
template <typename L, typename R>
using optional_ge_result_t =
    optional_cmp_result_t<decltype(std::declval<L const&>() >=
        std::declval<R const&>())>;
template <typename T>
concept not_derived_from_optional =
    !requires(T const& arg) { []<typename U>(optional<U> const&) {}(arg); };
} // namespace details
__RXX_INLINE_IF_NUA_ABI
namespace nua {
template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator==(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs == *rhs))
    -> details::optional_eq_result_t<L, R> {
    return lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || static_cast<bool>(*lhs == *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator!=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs != *rhs))
    -> details::optional_ne_result_t<L, R> {
    return lhs.has_value() != rhs.has_value() ||
        (lhs.has_value() && static_cast<bool>(*lhs != *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs < *rhs))
    -> details::optional_lt_result_t<L, R> {
    return lhs.has_value() < rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() &&
            static_cast<bool>(*lhs < *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs > *rhs))
    -> details::optional_gt_result_t<L, R> {
    return lhs.has_value() > rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() &&
            static_cast<bool>(*lhs > *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <= *rhs))
    -> details::optional_le_result_t<L, R> {
    return lhs.has_value() < rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || static_cast<bool>(*lhs <= *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs >= *rhs))
    -> details::optional_ge_result_t<L, R> {
    return lhs.has_value() > rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || static_cast<bool>(*lhs >= *rhs));
}

template <typename L, std::three_way_comparable_with<L> R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<L, R> operator<=>(
    optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <=> *rhs)) {
    return lhs && rhs ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator==(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs == rhs)) -> details::optional_eq_result_t<L, R> {
    return lhs.has_value() && static_cast<bool>(*lhs == rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator!=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs != rhs)) -> details::optional_ne_result_t<L, R> {
    return !lhs.has_value() || static_cast<bool>(*lhs != rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs < rhs)) -> details::optional_lt_result_t<L, R> {
    return !lhs.has_value() || static_cast<bool>(*lhs < rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs > rhs)) -> details::optional_gt_result_t<L, R> {
    return lhs.has_value() && static_cast<bool>(*lhs > rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs <= rhs)) -> details::optional_le_result_t<L, R> {
    return !lhs.has_value() || static_cast<bool>(*lhs <= rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs >= rhs)) -> details::optional_ge_result_t<L, R> {
    return lhs.has_value() && static_cast<bool>(*lhs >= rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator==(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs == *rhs)) -> details::optional_eq_result_t<L, R> {
    return rhs.has_value() && static_cast<bool>(lhs == *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator!=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs != *rhs)) -> details::optional_ne_result_t<L, R> {
    return !rhs.has_value() || static_cast<bool>(lhs != *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs < *rhs)) -> details::optional_lt_result_t<L, R> {
    return rhs.has_value() && static_cast<bool>(lhs < *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs > *rhs)) -> details::optional_gt_result_t<L, R> {
    return !rhs.has_value() || static_cast<bool>(lhs > *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs <= *rhs)) -> details::optional_le_result_t<L, R> {
    return rhs.has_value() && static_cast<bool>(lhs <= *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs >= *rhs)) -> details::optional_ge_result_t<L, R> {
    return !rhs.has_value() || static_cast<bool>(lhs >= *rhs);
}

template <typename L, details::not_derived_from_optional R>
requires std::three_way_comparable_with<R, L>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<L, R> operator<=>(
    optional<L> const& lhs, R const& rhs) noexcept(noexcept(*lhs <=> rhs)) {
    return lhs.has_value() ? *lhs <=> rhs : std::strong_ordering::less;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return !opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return !opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(optional<T> const&, __RXX nullopt_t) noexcept {
    return false;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return false;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return !opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(__RXX nullopt_t, optional<T> const&) noexcept {
    return true;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return true;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return !opt.has_value();
}

template <typename L>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=>(optional<L> const& opt, __RXX nullopt_t) noexcept {
    return opt.has_value() <=> false;
}
} // namespace nua

__RXX_INLINE_IF_GCC_ABI
namespace gcc {
template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator==(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs == *rhs))
    -> details::optional_eq_result_t<L, R> {
    return lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || static_cast<bool>(*lhs == *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator!=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs != *rhs))
    -> details::optional_ne_result_t<L, R> {
    return lhs.has_value() != rhs.has_value() ||
        (lhs.has_value() && static_cast<bool>(*lhs != *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs < *rhs))
    -> details::optional_lt_result_t<L, R> {
    return lhs.has_value() < rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() &&
            static_cast<bool>(*lhs < *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs > *rhs))
    -> details::optional_gt_result_t<L, R> {
    return lhs.has_value() > rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() &&
            static_cast<bool>(*lhs > *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <= *rhs))
    -> details::optional_le_result_t<L, R> {
    return lhs.has_value() < rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || static_cast<bool>(*lhs <= *rhs));
}

template <typename L, typename R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs >= *rhs))
    -> details::optional_ge_result_t<L, R> {
    return lhs.has_value() > rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || static_cast<bool>(*lhs >= *rhs));
}

template <typename L, std::three_way_comparable_with<L> R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<L, R> operator<=>(
    optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <=> *rhs)) {
    return lhs && rhs ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator==(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs == rhs)) -> details::optional_eq_result_t<L, R> {
    return lhs.has_value() && static_cast<bool>(*lhs == rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator!=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs != rhs)) -> details::optional_ne_result_t<L, R> {
    return !lhs.has_value() || static_cast<bool>(*lhs != rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs < rhs)) -> details::optional_lt_result_t<L, R> {
    return !lhs.has_value() || static_cast<bool>(*lhs < rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs > rhs)) -> details::optional_gt_result_t<L, R> {
    return lhs.has_value() && static_cast<bool>(*lhs > rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs <= rhs)) -> details::optional_le_result_t<L, R> {
    return !lhs.has_value() || static_cast<bool>(*lhs <= rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<R>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs >= rhs)) -> details::optional_ge_result_t<L, R> {
    return lhs.has_value() && static_cast<bool>(*lhs >= rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator==(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs == *rhs)) -> details::optional_eq_result_t<L, R> {
    return rhs.has_value() && static_cast<bool>(lhs == *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator!=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs != *rhs)) -> details::optional_ne_result_t<L, R> {
    return !rhs.has_value() || static_cast<bool>(lhs != *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs < *rhs)) -> details::optional_lt_result_t<L, R> {
    return rhs.has_value() && static_cast<bool>(lhs < *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs > *rhs)) -> details::optional_gt_result_t<L, R> {
    return !rhs.has_value() || static_cast<bool>(lhs > *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs <= *rhs)) -> details::optional_le_result_t<L, R> {
    return rhs.has_value() && static_cast<bool>(lhs <= *rhs);
}

template <typename L, typename R>
requires (!details::is_optional_v<L>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator>=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs >= *rhs)) -> details::optional_ge_result_t<L, R> {
    return !rhs.has_value() || static_cast<bool>(lhs >= *rhs);
}

template <typename L, details::not_derived_from_optional R>
requires std::three_way_comparable_with<R, L>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<L, R> operator<=>(
    optional<L> const& lhs, R const& rhs) noexcept(noexcept(*lhs <=> rhs)) {
    return lhs.has_value() ? *lhs <=> rhs : std::strong_ordering::less;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return !opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return !opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(optional<T> const&, __RXX nullopt_t) noexcept {
    return false;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return false;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return !opt.has_value();
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(__RXX nullopt_t, optional<T> const&) noexcept {
    return true;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(optional<T> const& opt, __RXX nullopt_t) noexcept {
    return true;
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(__RXX nullopt_t, optional<T> const& opt) noexcept {
    return !opt.has_value();
}

template <typename L>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=>(optional<L> const& opt, __RXX nullopt_t) noexcept {
    return opt.has_value() <=> false;
}
} // namespace gcc

RXX_DEFAULT_NAMESPACE_END
