// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/type_traits/template_access.h"
#include "rxx/utility/move.h"
#include "rxx/variant/bad_variant_access.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <size_t I>
__RXX_HIDE_FROM_ABI void get_alternative(...) noexcept = delete;

template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>> const& get(
    variant<Us...> const& val RXX_LIFETIMEBOUND) {
    if (val.index() == I) [[likely]] {
        return get_alternative<I>(val);
    }

    RXX_THROW(bad_variant_access());
}
template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>>& get(
    variant<Us...>& val RXX_LIFETIMEBOUND) {
    if (val.index() == I) [[likely]] {
        return get_alternative<I>(val);
    }

    RXX_THROW(bad_variant_access());
}
template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>> const&& get(
    variant<Us...> const&& val RXX_LIFETIMEBOUND) {
    if (val.index() == I) [[likely]] {
        return get_alternative<I>(__RXX move(val));
    }

    RXX_THROW(bad_variant_access());
}
template <size_t I, typename... Us>
requires requires { typename template_element_t<I, variant<Us...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr template_element_t<I, variant<Us...>>&& get(
    variant<Us...>&& val RXX_LIFETIMEBOUND) {
    if (val.index() == I) [[likely]] {
        return get_alternative<I>(__RXX move(val));
    }

    RXX_THROW(bad_variant_access());
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T const& get(variant<Us...> const& val) {
    return __RXX get<template_index_v<T, variant<Us...>>>(val);
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T& get(variant<Us...>& val) {
    return __RXX get<template_index_v<T, variant<Us...>>>(val);
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T const&& get(variant<Us...> const&& val) {
    return __RXX get<template_index_v<T, variant<Us...>>>(__RXX move(val));
}

template <typename T, typename... Us>
requires (template_count_v<T, variant<Us...>> == 1)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T&& get(variant<Us...>&& val) {
    return __RXX get<template_index_v<T, variant<Us...>>>(__RXX move(val));
}

template <size_t I, typename... Ts>
requires requires { typename template_element_t<I, variant<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...> const* val) noexcept {
    return val && val->index() == I
        ? RXX_BUILTIN_addressof(__RXX get<I>(*val))
        : nullptr;
}

template <size_t I, typename... Ts>
requires requires { typename template_element_t<I, variant<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...>* val) noexcept {
    return val && val->index() == I
        ? RXX_BUILTIN_addressof(__RXX get<I>(*val))
        : nullptr;
}

template <typename T, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...> const* val) noexcept {
    static_assert(
        template_count_v<T, variant<Ts...>> == 1, "Invalid alternative");
    return __RXX get_if<template_index_v<T, variant<Ts...>>>(val);
}

template <typename T, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto get_if(variant<Ts...>* val) noexcept {
    static_assert(
        template_count_v<T, variant<Ts...>> == 1, "Invalid alternative");
    return __RXX get_if<template_index_v<T, variant<Ts...>>>(val);
}

RXX_DEFAULT_NAMESPACE_END
