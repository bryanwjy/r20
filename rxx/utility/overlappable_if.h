// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/utility.h"

#include <concepts>
#include <functional>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <typename T0, class T1>
__RXX_HIDE_FROM_ABI inline constexpr bool fits_in_tail_padding_v = []() {
    struct __RXX_ABI_PRIVATE test_struct {
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T0 first;
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T1 second;
    };
    return sizeof(test_struct) == sizeof(T0);
}();

template <typename T>
concept explicit_default_constructible =
    std::default_initializable<T> && !requires(void (*func)(T)) { func({}); };
} // namespace details

template <bool NoUniqueAdress, typename T>
struct overlappable_if {
    __RXX_HIDE_FROM_ABI constexpr overlappable_if() = delete;
    __RXX_HIDE_FROM_ABI explicit(details::explicit_default_constructible<
        T>) constexpr overlappable_if() noexcept(std::
            is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    = default;

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit overlappable_if(
        std::in_place_t, Args&&... args)
        : data{__RXX forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires std::regular_invocable<F, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit overlappable_if(
        generating_t, F&& f, Args&&... args)
        : data{std::invoke(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)} {}

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T data;
};

template <typename T>
struct overlappable_if<false, T> {
    __RXX_HIDE_FROM_ABI constexpr overlappable_if() = delete;
    __RXX_HIDE_FROM_ABI explicit(details::explicit_default_constructible<
        T>) constexpr overlappable_if() noexcept(std::
            is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    = default;

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit overlappable_if(
        std::in_place_t, Args&&... args)
        : data{__RXX forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires std::regular_invocable<F, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit overlappable_if(
        generating_t, F&& f, Args&&... args)
        : data{std::invoke(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)} {}

    T data;
};

RXX_DEFAULT_NAMESPACE_END
