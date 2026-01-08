// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h"

#include "rxx/functional/hash.h"

#include <concepts>
#include <type_traits>

template <typename T>
requires std::semiregular<std::hash<std::remove_const_t<T>>> &&
    requires(T const& val, std::hash<std::remove_const_t<T>> hasher) {
        { hasher(val) } -> std::same_as<size_t>;
    }
struct std::hash<__RXX nua::optional<T>> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX nua::optional<T> const& opt) RXX_CONST_CALL
        noexcept(std::is_nothrow_invocable_v<std::hash<std::remove_const_t<T>>,
            T const&>) {
        return opt.has_value() ? std::hash<std::remove_const_t<T>>()(*opt) : 0;
    }
};

template <typename T>
requires std::semiregular<std::hash<std::remove_const_t<T>>> &&
    requires(T const& val, std::hash<std::remove_const_t<T>> hasher) {
        { hasher(val) } -> std::same_as<size_t>;
    }
struct std::hash<__RXX gcc::optional<T>> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX gcc::optional<T> const& opt) RXX_CONST_CALL
        noexcept(std::is_nothrow_invocable_v<std::hash<std::remove_const_t<T>>,
            T const&>) {
        return opt.has_value() ? std::hash<std::remove_const_t<T>>()(*opt) : 0;
    }
};
