// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {
template <typename T0, class T1>
__RXX_HIDE_FROM_ABI inline constexpr bool fits_in_tail_padding_v = []() {
    struct test_struct {
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T0 first;
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T1 second;
    };
    return sizeof(test_struct) == sizeof(T0);
}();

struct generating_t {
    __RXX_HIDE_FROM_ABI constexpr explicit generating_t() noexcept = default;
};

inline constexpr generating_t generating{};

template <size_t I>
struct generating_index_t {
    __RXX_HIDE_FROM_ABI constexpr explicit generating_index_t() noexcept =
        default;
};

template <size_t I>
inline constexpr generating_index_t<I> generating_index{};

template <typename T>
struct generating_type_t {
    __RXX_HIDE_FROM_ABI constexpr explicit generating_type_t() noexcept =
        default;
};

template <typename T>
inline constexpr generating_type_t<T> generating_type{};

template <typename T>
concept explicit_default_constructible =
    std::default_initializable<T> && !requires(void (*func)(T)) { func({}); };

template <bool NoUniqueAdress, typename T>
struct overlappable_if {
    __RXX_HIDE_FROM_ABI inline constexpr overlappable_if() = delete;
    __RXX_HIDE_FROM_ABI explicit(explicit_default_constructible<
        T>) inline constexpr overlappable_if() noexcept(std::
            is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    = default;

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI inline constexpr explicit overlappable_if(
        std::in_place_t, Args&&... args)
        : data{std::forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires std::regular_invocable<F, Args...>
    __RXX_HIDE_FROM_ABI inline constexpr explicit overlappable_if(
        generating_t, F&& f, Args&&... args)
        : data{std::invoke(std::forward<F>(f), std::forward<Args>(args)...)} {}

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T data;
};

template <typename T>
struct overlappable_if<false, T> {
    __RXX_HIDE_FROM_ABI inline constexpr overlappable_if() = delete;
    __RXX_HIDE_FROM_ABI explicit(explicit_default_constructible<
        T>) inline constexpr overlappable_if() noexcept(std::
            is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    = default;

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI inline constexpr explicit overlappable_if(
        std::in_place_t, Args&&... args)
        : data{std::forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires std::regular_invocable<F, Args...>
    __RXX_HIDE_FROM_ABI inline constexpr explicit overlappable_if(
        generating_t, F&& f, Args&&... args)
        : data{std::invoke(std::forward<F>(f), std::forward<Args>(args)...)} {}

    T data;
};

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
