// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
inline constexpr bool is_int128 = false;
template <typename T>
inline constexpr bool is_int128<T const> = is_int128<T>;
template <typename T>
inline constexpr bool is_int128<T volatile> = is_int128<T>;
template <typename T>
inline constexpr bool is_int128<T const volatile> = is_int128<T>;
template <typename T>
inline constexpr bool is_uint128 = false;
template <typename T>
inline constexpr bool is_uint128<T const> = is_uint128<T>;
template <typename T>
inline constexpr bool is_uint128<T volatile> = is_uint128<T>;
template <typename T>
inline constexpr bool is_uint128<T const volatile> = is_uint128<T>;
template <typename T>
inline constexpr bool is_sint128 = false;
template <typename T>
inline constexpr bool is_sint128<T const> = is_sint128<T>;
template <typename T>
inline constexpr bool is_sint128<T volatile> = is_sint128<T>;
template <typename T>
inline constexpr bool is_sint128<T const volatile> = is_sint128<T>;

#if RXX_SUPPORTS_INT128
template <>
inline constexpr bool is_int128<__int128_t> = true;
template <>
inline constexpr bool is_int128<__uint128_t> = true;
template <>
inline constexpr bool is_sint128<__int128_t> = true;
template <>
inline constexpr bool is_uint128<__uint128_t> = true;
#endif

template <typename T>
concept integer_non_bool = std::integral<T> && !std::same_as<T, bool>;

template <typename T>
concept integer_like = integer_non_bool<T> || is_int128<T>;

template <typename T>
concept signed_integer_like =
    integer_like<T> && (std::signed_integral<T> || is_sint128<T>);

template <typename T>
concept unsigned_integer_like =
    integer_like<T> && (std::unsigned_integral<T> || is_uint128<T>);

template <typename T>
concept integer_like_with_usable_difference_type =
    signed_integer_like<T> || (integer_like<T> && std::weakly_incrementable<T>);
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
