// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
inline constexpr bool enable_signed_integer_like = false;
template <typename T>
inline constexpr bool enable_unsigned_integer_like = false;

template <typename T>
inline constexpr bool enable_unsigned_integer_like<T const> =
    enable_unsigned_integer_like<T>;
template <typename T>
inline constexpr bool enable_unsigned_integer_like<T volatile> =
    enable_unsigned_integer_like<T>;
template <typename T>
inline constexpr bool enable_unsigned_integer_like<T const volatile> =
    enable_unsigned_integer_like<T>;
template <typename T>
inline constexpr bool enable_signed_integer_like<T const> =
    enable_signed_integer_like<T>;
template <typename T>
inline constexpr bool enable_signed_integer_like<T volatile> =
    enable_signed_integer_like<T>;
template <typename T>
inline constexpr bool enable_signed_integer_like<T const volatile> =
    enable_signed_integer_like<T>;

#if RXX_SUPPORTS_INT128
template <>
inline constexpr bool enable_signed_integer_like<__int128_t> = true;
template <>
inline constexpr bool enable_unsigned_integer_like<__uint128_t> = true;
#endif

template <typename T>
concept integer_non_bool =
    std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;

template <typename T>
concept integer_like = integer_non_bool<T> ||
    (enable_signed_integer_like<T> ^ enable_unsigned_integer_like<T>);

template <typename T>
concept signed_integer_like = integer_like<T> &&
    (std::signed_integral<T> || enable_signed_integer_like<T>);

template <typename T>
concept unsigned_integer_like = integer_like<T> &&
    (std::unsigned_integral<T> || enable_unsigned_integer_like<T>);

template <typename T>
concept integer_like_with_usable_difference_type =
    signed_integer_like<T> || (integer_like<T> && std::weakly_incrementable<T>);
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
