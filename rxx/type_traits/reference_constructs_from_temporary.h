// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

#if RXX_COMPILER_CLANG &&                                             \
    !__RXX_SHOULD_USE_BUILTIN(reference_constructs_from_temporary) && \
    __RXX_SHOULD_USE_BUILTIN(reference_binds_to_temporary)
template <typename T, typename U>
inline constexpr bool reference_constructs_from_temporary_v = false;
template <typename T, typename U>
struct reference_constructs_from_temporary :
    std::bool_constant<reference_constructs_from_temporary_v<T, U>> {};

namespace details::dangling {

/**
 * Checks valid conversion in unevaluated context
 * @tparam T - reference only, first argument of
 * reference_constructs_from_temporary
 */
template <typename T>
__RXX_HIDE_FROM_ABI void init_ref(T ref) noexcept;

/**
 * Creates a prvalue in unevaluated context
 * @tparam U - any type
 */
template <typename U>
__RXX_HIDE_FROM_ABI std::remove_cv_t<U> make_type() noexcept;

template <typename T, typename U>
inline constexpr bool constructs_from_impl =
    __reference_binds_to_temporary(T, U) ||
    std::is_convertible_v<std::remove_cvref_t<U>*, std::remove_cvref_t<T>*>;

template <typename T, typename U>
inline constexpr bool construct_from = false;

template <typename T, typename U>
requires (!std::same_as<std::remove_cvref_t<T>, std::remove_cv_t<U>>) &&
    requires { init_ref<T>(make_type<U&>()); }
inline constexpr bool construct_from<T, U&> = constructs_from_impl<T, U&>;

template <typename T, typename U>
requires (!std::same_as<std::remove_cvref_t<T>, std::remove_cv_t<U>>) &&
    requires { init_ref<T>(make_type<U&&>()); }
inline constexpr bool construct_from<T, U&&> = constructs_from_impl<T, U&&>;

template <typename T, typename U>
requires requires { init_ref<T>(make_type<U>()); }
inline constexpr bool construct_from<T, U> = constructs_from_impl<T, U>;

} // namespace details::dangling

template <typename T, typename U>
inline constexpr bool reference_constructs_from_temporary_v<T&, U> =
    details::dangling::constructs_from<T&, U>;

template <typename T, typename U>
inline constexpr bool reference_constructs_from_temporary<T&&, U> =
    details::dangling::constructs_from<T&&, U>;

#  define RXX_SUPPORTS_reference_constructs_from_temporary 1

#elif __RXX_SHOULD_USE_BUILTIN(reference_constructs_from_temporary)

template <typename T, typename U>
inline constexpr bool reference_constructs_from_temporary_v =
    __reference_constructs_from_temporary(T, U);
template <typename T, typename U>
struct reference_constructs_from_temporary :
    std::bool_constant<__reference_constructs_from_temporary(T, U)> {};

#  define RXX_SUPPORTS_reference_constructs_from_temporary 1

#else
#  define RXX_SUPPORTS_reference_constructs_from_temporary 0
#endif

RXX_DEFAULT_NAMESPACE_END
