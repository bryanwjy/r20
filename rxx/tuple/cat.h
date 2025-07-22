// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/forward.h"
#include "rxx/tuple/tuple.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
namespace tuple {

template <typename T>
inline constexpr bool is_all_reference_v = []<size_t... Is>(
                                               std::index_sequence<Is...>) {
    return (... &&
        std::is_reference_v<std::tuple_element_t<Is, std::remove_cvref_t<T>>>);
}(sequence_for<std::remove_cvref_t<T>>);

template <typename... Ts>
struct concat_elements;

template <typename... Ts>
using concat_elements_t RXX_NODEBUG = typename concat_elements<Ts...>::type;

template <typename... Ts>
struct concat_elements<__RXX tuple<Ts...>> {
    using type RXX_NODEBUG = __RXX tuple<Ts...>;
};

template <typename... Ls, typename... Rs>
struct concat_elements<__RXX tuple<Ls...>, __RXX tuple<Rs...>> {
    using type RXX_NODEBUG = __RXX tuple<Ls..., Rs...>;
};

template <typename T0, typename T1, typename... Ts>
struct concat_elements<T0, T1, Ts...> :
    concat_elements<concat_elements_t<T0, T1>, Ts...> {};

template <tuple_like Tuple, size_t... Is>
__RXX_HIDE_FROM_ABI auto decay_tuple(std::index_sequence<Is...>) noexcept
    -> __RXX tuple<std::tuple_element_t<Is, std::remove_cvref_t<Tuple>>...>;

template <tuple_like Tuple, size_t... Is>
__RXX_HIDE_FROM_ABI auto decl_tuple(std::index_sequence<Is...>) noexcept
    -> __RXX tuple<decl_element_t<Is, Tuple>...>;

template <tuple_like Tuple>
using decay_tuple_t RXX_NODEBUG =
    decltype(decay_tuple<Tuple>(sequence_for<std::remove_cvref_t<Tuple>>));

template <tuple_like Tuple>
using decl_tuple_t RXX_NODEBUG =
    decltype(decl_tuple<Tuple>(sequence_for<std::remove_cvref_t<Tuple>>));

template <tuple_like... Ts>
using concat_result_t RXX_NODEBUG = concat_elements_t<decay_tuple_t<Ts>...>;

template <tuple_like... Ts>
using concat_reference_t RXX_NODEBUG = concat_elements_t<decl_tuple_t<Ts>...>;

// unpacks into references
template <tuple_like Tuple>
__RXX_HIDE_FROM_ABI constexpr auto as_reference(Tuple&& tuple) noexcept(
    is_nothrow_accessible_v<Tuple>) {
    return apply(
        []<typename... Ts>(Ts&&... args) {
            return forward_as_tuple(std::forward<Ts>(args)...);
        },
        std::forward<Tuple>(tuple));
}

template <typename L, typename R>
RXX_ATTRIBUTES(NODISCARD, FLATTEN, _HIDE_FROM_ABI)
constexpr auto ref_cat(L&& lhs, R&& rhs) {
    static_assert(is_all_reference_v<L> && is_all_reference_v<R>);
    if constexpr (std::tuple_size_v<L> == 0 && std::tuple_size_v<R> == 0) {
        return __RXX tuple<>();
    } else if constexpr (std::tuple_size_v<L> == 0) {
        return std::forward<R>(rhs);
    } else if constexpr (std::tuple_size_v<R> == 0) {
        return std::forward<L>(lhs);
    } else {
        return [&]<size_t... Is, size_t... Js>(
                   std::index_sequence<Is...>, std::index_sequence<Js...>) {
            return concat_elements_t<L, R>{
                static_cast<std::tuple_element_t<Is, L>>(
                    ranges::get_element<Is>(lhs))...,
                static_cast<std::tuple_element_t<Js, R>>(
                    ranges::get_element<Js>(rhs))...};
        }(sequence_for<std::remove_cvref_t<L>>,
                   sequence_for<std::remove_cvref_t<R>>);
    }
}

template <typename H, typename M, typename T, typename... Os>
constexpr auto ref_cat(H&& head, M&& mid, T&& tail, Os&&... others) {
    return ref_cat(ref_cat(std::forward<H>(head), std::forward<M>(mid)),
        std::forward<T>(tail), std::forward<Os>(others)...);
}

} // namespace tuple
} // namespace details

template <tuple_like... Tuples>
requires requires {
    typename details::tuple::concat_result_t<Tuples...>;
    typename details::tuple::concat_reference_t<Tuples...>;
} &&
    std::constructible_from<details::tuple::concat_result_t<Tuples...>,
        details::tuple::concat_reference_t<Tuples...>>
RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI) constexpr auto tuple_cat(
    Tuples&&... args) noexcept((... &&
    details::tuple::is_nothrow_accessible_v<Tuples>)&& //
    std::is_nothrow_constructible_v<details::tuple::concat_result_t<Tuples...>,
        details::tuple::concat_reference_t<Tuples...>>) //
{
    if constexpr (sizeof...(Tuples) == 0) {
        return tuple<>{};
    } else if constexpr (sizeof...(Tuples) == 1) {
        return details::tuple::concat_result_t<Tuples...>(
            details::tuple::as_reference(std::forward<Tuples>(args))...);
    } else {
        return details::tuple::concat_result_t<Tuples...>(
            details::tuple::ref_cat(
                details::tuple::as_reference(std::forward<Tuples>(args))...));
    }
}

RXX_DEFAULT_NAMESPACE_END
