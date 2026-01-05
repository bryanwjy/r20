// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/comparable.h"
#include "rxx/optional/nullopt.h"
#include "rxx/optional/optional_abi.h"

#include <compare>

RXX_DEFAULT_NAMESPACE_BEGIN
__RXX_INLINE_IF_NUA_ABI
namespace nua {
// nua: no_unique_address

template <typename>
class optional;

template <typename L, typename R>
requires details::supports_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs == *rhs)) {
    return lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || *lhs == *rhs);
}

template <typename L, typename R>
requires details::supports_inequality_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs != *rhs)) {
    return lhs.has_value() != rhs.has_value() ||
        (lhs.has_value() && *lhs != *rhs);
}

template <typename L, typename R>
requires details::supports_less_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs < *rhs)) {
    return lhs.has_value() < rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() && *lhs < *rhs);
}

template <typename L, typename R>
requires details::supports_greater_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs > *rhs)) {
    return lhs.has_value() > rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() && *lhs > *rhs);
}

template <typename L, typename R>
requires details::supports_less_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <= *rhs)) {
    return lhs.has_value() < rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || *lhs <= *rhs);
}

template <typename L, typename R>
requires details::supports_greater_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs >= *rhs)) {
    return lhs.has_value() > rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || *lhs >= *rhs);
}

template <typename L, std::three_way_comparable_with<L> R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<L, R> operator<=>(
    optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <=> *rhs)) {
    return lhs && rhs ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
}

template <typename L, typename R>
requires details::supports_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs == rhs)) {
    return lhs.has_value() && *lhs == rhs;
}

template <typename L, typename R>
requires details::supports_inequality_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs != rhs)) {
    return !lhs.has_value() || *lhs != rhs;
}

template <typename L, typename R>
requires details::supports_less_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs < rhs)) {
    return !lhs.has_value() || *lhs < rhs;
}

template <typename L, typename R>
requires details::supports_greater_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs > rhs)) {
    return lhs.has_value() && *lhs > rhs;
}

template <typename L, typename R>
requires details::supports_less_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs <= rhs)) {
    return !lhs.has_value() || *lhs <= rhs;
}

template <typename L, typename R>
requires details::supports_greater_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs >= rhs)) {
    return lhs.has_value() && *lhs >= rhs;
}

template <typename L, typename R>
requires details::supports_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs == *rhs)) {
    return rhs.has_value() && lhs == *rhs;
}

template <typename L, typename R>
requires details::supports_inequality_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs != *rhs)) {
    return !rhs.has_value() || lhs != *rhs;
}

template <typename L, typename R>
requires details::supports_less_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs < *rhs)) {
    return rhs.has_value() && lhs < *rhs;
}

template <typename L, typename R>
requires details::supports_greater_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs > *rhs)) {
    return !rhs.has_value() || lhs > *rhs;
}

template <typename L, typename R>
requires details::supports_less_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs <= *rhs)) {
    return rhs.has_value() && lhs <= *rhs;
}

template <typename L, typename R>
requires details::supports_greater_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs >= *rhs)) {
    return !rhs.has_value() || lhs >= *rhs;
}

template <typename L, typename R>
requires (!requires(R const& arg) {
    []<typename T>(optional<T> const&) {}(arg);
}) && std::three_way_comparable_with<L, R>
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
template <typename>
class optional;

template <typename L, typename R>
requires details::supports_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs == *rhs)) {
    return lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || *lhs == *rhs);
}

template <typename L, typename R>
requires details::supports_inequality_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs != *rhs)) {
    return lhs.has_value() != rhs.has_value() ||
        (lhs.has_value() && *lhs != *rhs);
}

template <typename L, typename R>
requires details::supports_less_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs < *rhs)) {
    return lhs.has_value() < rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() && *lhs < *rhs);
}

template <typename L, typename R>
requires details::supports_greater_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs > *rhs)) {
    return lhs.has_value() > rhs.has_value() ||
        (lhs.has_value() == rhs.has_value() && lhs.has_value() && *lhs > *rhs);
}

template <typename L, typename R>
requires details::supports_less_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <= *rhs)) {
    return lhs.has_value() < rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || *lhs <= *rhs);
}

template <typename L, typename R>
requires details::supports_greater_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs >= *rhs)) {
    return lhs.has_value() > rhs.has_value() ||
        lhs.has_value() == rhs.has_value() &&
        (!lhs.has_value() || *lhs >= *rhs);
}

template <typename L, std::three_way_comparable_with<L> R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<L, R> operator<=>(
    optional<L> const& lhs,
    optional<R> const& rhs) noexcept(noexcept(*lhs <=> *rhs)) {
    return lhs && rhs ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
}

template <typename L, typename R>
requires details::supports_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs == rhs)) {
    return lhs.has_value() && *lhs == rhs;
}

template <typename L, typename R>
requires details::supports_inequality_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs != rhs)) {
    return !lhs.has_value() || *lhs != rhs;
}

template <typename L, typename R>
requires details::supports_less_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs < rhs)) {
    return !lhs.has_value() || *lhs < rhs;
}

template <typename L, typename R>
requires details::supports_greater_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs > rhs)) {
    return lhs.has_value() && *lhs > rhs;
}

template <typename L, typename R>
requires details::supports_less_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs <= rhs)) {
    return !lhs.has_value() || *lhs <= rhs;
}

template <typename L, typename R>
requires details::supports_greater_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(optional<L> const& lhs, R const& rhs) noexcept(
    noexcept(*lhs >= rhs)) {
    return lhs.has_value() && *lhs >= rhs;
}

template <typename L, typename R>
requires details::supports_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs == *rhs)) {
    return rhs.has_value() && lhs == *rhs;
}

template <typename L, typename R>
requires details::supports_inequality_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator!=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs != *rhs)) {
    return !rhs.has_value() || lhs != *rhs;
}

template <typename L, typename R>
requires details::supports_less_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs < *rhs)) {
    return rhs.has_value() && lhs < *rhs;
}

template <typename L, typename R>
requires details::supports_greater_than_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs > *rhs)) {
    return !rhs.has_value() || lhs > *rhs;
}

template <typename L, typename R>
requires details::supports_less_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs <= *rhs)) {
    return rhs.has_value() && lhs <= *rhs;
}

template <typename L, typename R>
requires details::supports_greater_equal_with<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(L const& lhs, optional<R> const& rhs) noexcept(
    noexcept(lhs >= *rhs)) {
    return !rhs.has_value() || lhs >= *rhs;
}

template <typename L, typename R>
requires (!requires(R const& arg) {
    []<typename T>(optional<T> const&) {}(arg);
}) && std::three_way_comparable_with<L, R>
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
