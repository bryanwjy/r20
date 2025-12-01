// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T, T... Is>
struct integer_sequence {};

namespace details {

template <typename, typename>
struct combine;

template <typename T, T... Is, T... Js>
struct combine<integer_sequence<T, Is...>, integer_sequence<T, Js...>> {
    using type RXX_NODEBUG =
        integer_sequence<T, Is..., (sizeof...(Is) + Js)...>;
};

template <typename T>
struct combine<integer_sequence<T>, integer_sequence<T>> {
    using type RXX_NODEBUG = integer_sequence<T>;
};

template <typename T, size_t N>
struct generate;

template <typename T>
struct generate<T, 0> {
    using type RXX_NODEBUG = integer_sequence<T>;
};

template <typename T>
struct generate<T, 1> {
    using type RXX_NODEBUG = integer_sequence<T, 0>;
};

template <typename T, size_t N>
struct generate :
    combine<typename generate<T, N / 2>::type,
        typename generate<T, N - N / 2>::type> {};

} // namespace details

template <typename T, T N>
using make_integer_sequence =
    typename details::generate<T, (N < 0) ? 0 : N>::type;

template <size_t... Is>
using index_sequence = integer_sequence<size_t, Is...>;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template <typename... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

template <typename T, T N>
inline constexpr make_integer_sequence<T, N> make_integer_sequence_v{};
template <size_t N>
inline constexpr make_index_sequence<N> make_index_sequence_v{};
template <typename... Ts>
inline constexpr index_sequence_for<Ts...> index_sequence_for_v{};

RXX_DEFAULT_NAMESPACE_END
