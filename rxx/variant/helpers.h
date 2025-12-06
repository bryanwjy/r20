// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/type_traits/template_access.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T>
struct variant_size : std::integral_constant<size_t, variant_size_v<T>> {};

template <size_t I, typename... Ts>
struct variant_alternative<I, variant<Ts...>> :
    template_element<I, type_list<Ts...>> {};

template <size_t I, typename T>
struct variant_alternative<I, T const> :
    std::add_const<variant_alternative_t<I, T>> {};
template <size_t I, typename T>
struct variant_alternative<I, T volatile> :
    std::add_volatile<variant_alternative_t<I, T>> {};
template <size_t I, typename T>
struct variant_alternative<I, T const volatile> :
    std::add_cv<variant_alternative_t<I, T>> {};

RXX_DEFAULT_NAMESPACE_END
