// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/details/invoke_r.h"
#include "rxx/ranges/get_element.h"
#include "rxx/tuple.h"
#include "rxx/utility.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <size_t N>
inline constexpr std::make_index_sequence<N> make_index_sequence_v{};

template <typename F, tuple_like Tuple>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto transform(F&& callable, Tuple&& arg) {
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
        using Result = tuple<std::invoke_result_t<F&,
            std::invoke_result_t<decltype(get_element<Is>), Tuple>>...>;

        return Result{std::invoke(
            callable, get_element<Is>(std::forward<Tuple>(arg)))...};
    }(make_index_sequence_v<std::tuple_size_v<std::remove_cvref_t<Tuple>>>);
}

template <typename F, tuple_like Tuple>
__RXX_HIDE_FROM_ABI constexpr void for_each(F&& callable, Tuple&& arg) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (...,
            invoke_r<void>(
                callable, get_element<Is>(std::forward<Tuple>(arg))));
    }(make_index_sequence_v<std::tuple_size_v<std::remove_cvref_t<Tuple>>>);
}

template <typename F, tuple_like Tuple1, tuple_like Tuple2>
requires (std::tuple_size_v<std::remove_cvref_t<Tuple1>> ==
    std::tuple_size_v<std::remove_cvref_t<Tuple2>>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto transform(
    F&& callable, Tuple1&& tuple1, Tuple2&& tuple2) {
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
        using Result = tuple<std::invoke_result_t<F&,
            std::invoke_result_t<decltype(get_element<Is>), Tuple1>,
            std::invoke_result_t<decltype(get_element<Is>), Tuple2>>...>;

        return Result{
            std::invoke(callable, get_element<Is>(std::forward<Tuple1>(tuple1)),
                get_element<Is>(std::forward<Tuple2>(tuple2)))...};
    }(make_index_sequence_v<std::tuple_size_v<std::remove_cvref_t<Tuple1>>>);
}

template <typename F, tuple_like Tuple1, tuple_like Tuple2>
requires (std::tuple_size_v<std::remove_cvref_t<Tuple1>> ==
    std::tuple_size_v<std::remove_cvref_t<Tuple2>>)
__RXX_HIDE_FROM_ABI constexpr void for_each(
    F&& callable, Tuple1&& tuple1, Tuple2&& tuple2) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (...,
            invoke_r<void>(callable,
                get_element<Is>(std::forward<Tuple1>(tuple1)),
                get_element<Is>(std::forward<Tuple2>(tuple2))));
    }(make_index_sequence_v<std::tuple_size_v<std::remove_cvref_t<Tuple1>>>);
}

} // namespace details

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
