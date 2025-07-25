// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/ranges/get_element.h"
#include "rxx/tuple/utils.h"
#include "rxx/utility/forward_like.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <typename T, typename Tuple>
inline constexpr bool can_make_from_tuple = false;

template <typename T, tuple_like Tuple>
inline constexpr bool can_make_from_tuple<T, Tuple> =
    []<size_t... Is>(std::index_sequence<Is...>) {
        return std::is_constructible_v<T, tuple::decl_element_t<Is, Tuple>...>;
    }(tuple::sequence_for<std::remove_cvref_t<Tuple>>);

template <typename T, typename Tuple>
inline constexpr bool nothrow_make_from_tuple = requires {
    requires can_make_from_tuple<T, Tuple>;
    requires ([]<size_t... Is>(std::index_sequence<Is...>) {
        return std::is_nothrow_constructible_v<T,
            tuple::decl_element_t<Is, Tuple>...>;
    }(tuple::sequence_for<std::remove_cvref_t<Tuple>>));
};

} // namespace details

template <typename T, tuple_like Tuple>
requires details::can_make_from_tuple<T, Tuple>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr T make_from_tuple(
    Tuple&& args) noexcept(details::nothrow_make_from_tuple<T, Tuple>) {

    return [&]<size_t... Is>(std::index_sequence<Is...>) -> T {
        return T(ranges::get_element<Is>(std::forward<Tuple>(args))...);
    }(details::tuple::sequence_for<std::remove_cvref_t<Tuple>>);
}

RXX_DEFAULT_NAMESPACE_END
