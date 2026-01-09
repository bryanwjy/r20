// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename...>
class variant;
class __RXX_ABI_PUBLIC bad_variant_access;
template <typename T>
struct variant_size;

template <typename T>
inline constexpr size_t variant_size_v = 0;
template <typename T>
inline constexpr size_t variant_size_v<T const> = variant_size_v<T>;
template <typename T>
inline constexpr size_t variant_size_v<T volatile> = variant_size_v<T>;
template <typename T>
inline constexpr size_t variant_size_v<T const volatile> = variant_size_v<T>;
template <typename... Ts>
inline constexpr size_t variant_size_v<variant<Ts...>> = sizeof...(Ts);
inline constexpr size_t variant_npos = static_cast<size_t>(-1);

template <size_t, typename>
struct variant_alternative;

template <size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

RXX_DEFAULT_NAMESPACE_END
