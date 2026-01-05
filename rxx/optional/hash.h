// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/functional/hash.h"
#include "rxx/optional/optional_abi.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
__RXX_INLINE_IF_NUA_ABI
namespace nua {
template <typename>
class optional;
}

__RXX_INLINE_IF_GCC_ABI
namespace gcc {
template <typename>
class optional;
}
RXX_DEFAULT_NAMESPACE_END

template <typename T>
requires std::semiregular<std::hash<std::remove_cvref_t<T>>> &&
    requires(T const& val, std::hash<std::remove_cvref_t<T>> hasher) {
        { hasher(val) } -> std::same_as<size_t>;
    }
struct std::hash<__RXX nua::optional<T>> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX nua::optional<T> const& opt) RXX_CONST_CALL
        noexcept(std::is_nothrow_invocable_v<std::hash<std::remove_cvref_t<T>>,
            T const&>) {
        return opt.has_value() ? std::hash<std::remove_cvref_t<T>>()(*opt) : 0;
    }
};

template <typename T>
requires std::semiregular<std::hash<std::remove_cvref_t<T>>> &&
    requires(T const& val, std::hash<std::remove_cvref_t<T>> hasher) {
        { hasher(val) } -> std::same_as<size_t>;
    }
struct std::hash<__RXX gcc::optional<T>> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX gcc::optional<T> const& opt) RXX_CONST_CALL
        noexcept(std::is_nothrow_invocable_v<std::hash<std::remove_cvref_t<T>>,
            T const&>) {
        return opt.has_value() ? std::hash<std::remove_cvref_t<T>>()(*opt) : 0;
    }
};
