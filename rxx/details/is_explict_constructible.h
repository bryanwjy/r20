// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace details {
template <typename T>
__RXX_HIDE_FROM_ABI void implicit_conv(T) noexcept;

} // namespace details

template <typename T, typename... Ts>
inline constexpr bool is_explicit_constructible_v = false;

template <typename T, typename Head, typename Mid, typename... Tail>
requires std::is_constructible_v<T, Head, Mid, Tail...>
inline constexpr bool is_explicit_constructible_v<T, Head, Mid, Tail...> =
    !requires {
        details::implicit_conv<T>({std::declval<Head>(), std::declval<Mid>(),
            std::declval<Tail>()...});
    };

template <typename T, typename Head>
requires std::is_constructible_v<T, Head>
inline constexpr bool is_explicit_constructible_v =
    !std::is_convertible_v<Head, T>;

template <typename T>
requires std::is_default_constructible_v<T>
inline constexpr bool is_explicit_constructible_v =
    !requires { details::implicit_conv<T>({}); };

RXX_DEFAULT_NAMESPACE_END
