// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/tuple.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
namespace tuple {

template <tuple_like T, tuple_like U, size_t... Is>
__RXX_HIDE_FROM_ABI auto three_way_result(
    T const& l, U const& r, std::index_sequence<Is...>) noexcept
    -> std::common_comparison_category_t<decltype(ranges::get_element<Is>(l) <=>
        ranges::get_element<Is>(r))...>;

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>)
using three_way_result_t RXX_NODEBUG =
    decltype(three_way_result(std::declval<L>(), std::declval<R>(),
        sequence_for<std::remove_cvref_t<L>>));

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>)
inline constexpr bool is_nothrow_three_way_v = requires {
    typename three_way_result_t<L, R>;
    requires ([]<size_t... Is>(std::index_sequence<Is...>) {
        return (...&& noexcept(
            decl_element<Is, L const&>() <=> decl_element<Is, R const&>()));
    }(sequence_for<std::remove_cvref_t<L>>));
} && is_nothrow_accessible_v<L> && is_nothrow_accessible_v<R>;

template <tuple_like L, tuple_like R>
inline constexpr bool is_equality_comparable_v =
    []<size_t... Is>(std::index_sequence<Is...>) {
        return (... && requires {
            bool(decl_element<Is, L const&>() == decl_element<Is, R const&>());
        });
    }(sequence_for<std::remove_cvref_t<L>>);

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>)
inline constexpr bool is_nothrow_equality_v = requires {
    requires is_equality_comparable_v<L, R>;
    requires ([]<size_t... Is>(std::index_sequence<Is...>) {
        return (...&& noexcept(
            decl_element<Is, L const&>() == decl_element<Is, R const&>()));
    }(sequence_for<std::remove_cvref_t<L>>));
} && is_nothrow_accessible_v<L> && is_nothrow_accessible_v<R>;

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>)
inline constexpr bool is_less_comparable_v = []<size_t... Is>(
                                                 std::index_sequence<Is...>) {
    return (... && requires {
        bool(decl_element<Is, L const&>() < decl_element<Is, R const&>());
    });
}(sequence_for<std::remove_cvref_t<L>>);

template <tuple_like L, tuple_like R>
inline constexpr bool is_nothrow_less_comparable_v = requires {
    requires is_less_comparable_v<L, R>;
    requires ([]<size_t... Is>(std::index_sequence<Is...>) {
        return (...&& noexcept(
            decl_element<Is, L const&>() < decl_element<Is, R const&>()));
    }(sequence_for<std::remove_cvref_t<L>>));
} && is_nothrow_accessible_v<L> && is_nothrow_accessible_v<R>;

template <size_t I, tuple_like L, tuple_like R>
requires (I == std::tuple_size_v<L>)
__RXX_HIDE_FROM_ABI constexpr three_way_result_t<L, R> three_way(
    L const&, R const&) noexcept {
    return three_way_result_t<L, R>::equal;
}

template <size_t I, tuple_like L, tuple_like R>
requires (I < std::tuple_size_v<L>)
__RXX_HIDE_FROM_ABI constexpr three_way_result_t<L, R> three_way(
    L const& l, R const& r) {
    auto const result = ranges::get_element<I>(l) <=> ranges::get_element<I>(r);
    return result != 0 ? result : three_way<I + 1>(l, r);
}

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>) &&
    requires { typename three_way_result_t<L, R>; }
__RXX_HIDE_FROM_ABI constexpr three_way_result_t<L, R> three_way(
    L const& l, R const& r) noexcept(is_nothrow_three_way_v<L, R>) {
    return three_way<0>(l, r);
}

template <size_t I, tuple_like T, tuple_like U>
requires (I == std::tuple_size_v<T>)
__RXX_HIDE_FROM_ABI constexpr bool equals(T const& l, U const& r) noexcept {
    return true;
}

template <size_t I, tuple_like T, tuple_like U>
requires (I < std::tuple_size_v<T>)
__RXX_HIDE_FROM_ABI constexpr bool equals(T const& l, U const& r) {
    return (ranges::get_element<I>(l) == ranges::get_element<I>(r)) &&
        equals<I + 1>(l, r);
}

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>) &&
    is_equality_comparable_v<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN) constexpr bool equals(
    L const& l, R const& r) noexcept(is_nothrow_equality_v<L, R>) {
    return equals<0>(l, r);
}

template <size_t I, tuple_like T, tuple_like U>
requires (I == std::tuple_size_v<T>)
__RXX_HIDE_FROM_ABI constexpr bool less(T const& l, U const& r) noexcept {
    return false;
}

template <size_t I, tuple_like L, tuple_like R>
requires (I < std::tuple_size_v<L>)
__RXX_HIDE_FROM_ABI constexpr bool less(L const& l, R const& r) {
    if (ranges::get_element<I>(l) < ranges::get_element<I>(r)) {
        return true;
    }

    if (ranges::get_element<I>(r) < ranges::get_element<I>(l)) {
        return false;
    }

    return less<I + 1>(l, r);
}

template <tuple_like L, tuple_like R>
requires (std::tuple_size_v<L> == std::tuple_size_v<R>) &&
    is_less_comparable_v<R, L> && is_less_comparable_v<L, R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN) constexpr bool less(
    L const& l, R const& r) noexcept(is_nothrow_less_comparable_v<L, R> &&
    is_nothrow_less_comparable_v<R, L>) {
    return less<0>(l, r);
}

} // namespace tuple
} // namespace details

template <typename... TTypes, typename... UTypes>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool
operator==(tuple<TTypes...> const& lhs, tuple<UTypes...> const& rhs) noexcept(
    noexcept(details::tuple::equals(lhs, rhs))) {
    return details::tuple::equals(lhs, rhs);
}

template <typename... TTypes, typename... UTypes>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool
operator<(tuple<TTypes...> const& lhs, tuple<UTypes...> const& rhs) noexcept(
    noexcept(details::tuple::less(lhs, rhs))) {
    return details::tuple::less(lhs, rhs);
}

template <typename... TTypes, typename... UTypes>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto
operator<=>(tuple<TTypes...> const& lhs, tuple<UTypes...> const& rhs) noexcept(
    noexcept(details::tuple::three_way(lhs, rhs))) {
    return details::tuple::three_way(lhs, rhs);
}

template <typename... TTypes, tuple_like Tuple>
requires (sizeof...(TTypes) == std::tuple_size_v<Tuple>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr bool operator==(
    tuple<TTypes...> const& lhs,
    Tuple const& rhs) noexcept(noexcept(details::tuple::equals(lhs, rhs))) {
    return details::tuple::equals(lhs, rhs);
}

template <typename... TTypes, tuple_like Tuple>
requires (sizeof...(TTypes) == std::tuple_size_v<Tuple>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator<=>(
    tuple<TTypes...> const& lhs,
    Tuple const& rhs) noexcept(noexcept(details::tuple::three_way(lhs, rhs))) {
    return details::tuple::three_way(lhs, rhs);
}

RXX_DEFAULT_NAMESPACE_END
