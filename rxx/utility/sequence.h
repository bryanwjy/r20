// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/utility/forward.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T, T... Is>
struct integer_sequence {
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr auto size() noexcept { return sizeof...(Is); }
};

#if RXX_HAS_BUILTIN(__make_integer_seq)

template <typename T, T N>
using make_integer_sequence RXX_NODEBUG =
    __make_integer_seq<__RXX integer_sequence, T, N>;

#elif RXX_HAS_BUILTIN(__integer_pack)

template <typename T, T N>
using make_integer_sequence RXX_NODEBUG =
    __RXX integer_sequence<T, __integer_pack(N)...>;

#else

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
using make_integer_sequence RXX_NODEBUG =
    typename details::generate<T, (N < 0) ? 0 : N>::type;
#endif

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

template <size_t... Is, typename F>
__RXX_HIDE_FROM_ABI constexpr void for_each_index_sequence(
    index_sequence<Is...>, F callable) {
    (callable.template operator()<Is>(), ...);
}

template <size_t N, typename F>
__RXX_HIDE_FROM_ABI constexpr void for_each_index_sequence(F&& callable) {
    for_each_index_sequence(
        make_index_sequence_v<N>, __RXX forward<F>(callable));
}

RXX_DEFAULT_NAMESPACE_END
