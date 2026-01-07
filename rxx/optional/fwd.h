// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/optional_abi.h" // IWYU pragma: export
#include "rxx/type_traits/reference_constructs_from_temporary.h"

RXX_STD_NAMESPACE_BEGIN
template <typename>
class optional;
struct nullopt_t;
RXX_STD_NAMESPACE_END

RXX_DEFAULT_NAMESPACE_BEGIN
struct nullopt_t;
__RXX_INLINE_IF_NUA_ABI
namespace nua {
// nua: no_unique_address
template <typename>
class optional;
} // namespace nua

/**
 * GCC disables RVO when constructing members declared with no_unique_address.
 * This breaks the default implementation's behaviour which relies on RVO to
 * construct non-copyable/movable types when generating/transforming underlying
 * values.
 *
 * Until this is fixed, split into a separate ABI for GCC
 */
__RXX_INLINE_IF_GCC_ABI
namespace gcc {
template <typename>
class optional;
} // namespace gcc

namespace details {
template <typename T>
inline constexpr bool is_optional_v = false;
template <typename T>
inline constexpr bool is_optional_like_v = false;
template <typename T>
inline constexpr bool is_optional_v<T const> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<T volatile> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<T const volatile> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<gcc::optional<T>> = true;
template <typename T>
inline constexpr bool is_optional_v<nua::optional<T>> = true;
template <template <typename T> class Opt, typename T>
inline constexpr bool is_optional_like_v<Opt<T>> =
    requires(Opt<T>* ptr) { ptr->~optional(); };

} // namespace details

RXX_DEFAULT_NAMESPACE_END

#define RXX_SUPPORTS_OPTIONAL_REFERENCES \
    RXX_SUPPORTS_reference_constructs_from_temporary
