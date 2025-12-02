// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/utility/move.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>> const& get(
    variant<Us...> const& val) noexcept {
    return val.template value_ref<I>();
}
template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>>& get(
    variant<Us...>& val) noexcept {
    return val.template value_ref<I>();
}
template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>> const&& get(
    variant<Us...> const&& val) noexcept {
    return __RXX move(val.template value_ref<I>());
}
template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>>&& get(
    variant<Us...>&& val) noexcept {
    return __RXX move(val.template value_ref<I>());
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T const& get(variant<Us...> const& val) noexcept {
    return __RXX get<template_index_v<T, variant<Us...>>>(val);
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T& get(variant<Us...>& val) noexcept {
    return __RXX get<template_index_v<T, variant<Us...>>>(val);
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T const&& get(variant<Us...> const&& val) noexcept {
    return __RXX get<template_index_v<T, variant<Us...>>>(__RXX move(val));
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T&& get(variant<Us...>&& val) noexcept {
    return __RXX get<template_index_v<T, variant<Us...>>>(__RXX move(val));
}

template <size_t I, typename... Ts>
requires requires { typename template_element_t<I, variant<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...> const* val) noexcept {
    if (val && val->index() == I) {
        return LEV_BUILTIN_addressof(get<I>(*val));
    }

    return nullptr;
}

template <size_t I, typename... Ts>
requires requires { typename template_element_t<I, variant<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...>* val) noexcept {
    if (val && val->index() == I) {
        return LEV_BUILTIN_addressof(get<I>(*val));
    }

    return nullptr;
}

template <typename T, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...> const* val) noexcept {
    static_assert(
        template_count_v<T, variant<Ts...>> == 1, "Invalid alternative");
    return get_if<template_index_v<T, variant<Ts...>>>(val);
}

template <typename T, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...>* val) noexcept {
    static_assert(
        template_count_v<T, variant<Ts...>> == 1, "Invalid alternative");
    return get_if<template_index_v<T, variant<Ts...>>>(val);
}

RXX_DEFAULT_NAMESPACE_END
