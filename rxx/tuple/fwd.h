// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/template_access.h"
#include "rxx/type_traits/common_reference.h"

#include <array>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
template <typename...>
class tuple;
RXX_DEFAULT_NAMESPACE_END

template <typename... Ts, std::common_with<Ts>... Us>
struct std::common_type<__RXX tuple<Ts...>, __RXX tuple<Us...>> {
    using type RXX_NODEBUG = __RXX tuple<std::common_type_t<Ts, Us>...>;
};

template <typename... Ts, typename... Us, template <typename> class TQual,
    template <typename> class UQual>
requires (... && std::common_reference_with<TQual<Ts>, UQual<Us>>)
struct std::basic_common_reference<__RXX tuple<Ts...>, __RXX tuple<Us...>,
    TQual, UQual> {
    using type RXX_NODEBUG =
        __RXX tuple<__RXX common_reference_t<TQual<Ts>, UQual<Us>>...>;
};

template <size_t I, typename... Ts>
requires requires {
    typename __RXX details::template_element_t<I, __RXX tuple<Ts...>>;
}
struct std::tuple_element<I, __RXX tuple<Ts...>> {
    using type = __RXX details::template_element_t<I, __RXX tuple<Ts...>>;
};

template <typename... Ts>
struct std::tuple_size<__RXX tuple<Ts...>> :
    std::integral_constant<size_t, sizeof...(Ts)> {};

RXX_DEFAULT_NAMESPACE_BEGIN

template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>>& get(tuple<Ts...>& arg RXX_LIFETIMEBOUND) noexcept;
template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>>&& get(tuple<Ts...>&& arg RXX_LIFETIMEBOUND) noexcept;
template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>> const& get(tuple<Ts...> const& arg RXX_LIFETIMEBOUND) noexcept;
template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>> const&& get(tuple<Ts...> const&& arg RXX_LIFETIMEBOUND) noexcept;

RXX_DEFAULT_NAMESPACE_END
