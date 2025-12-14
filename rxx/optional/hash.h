// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/functional/hash.h"
#include "rxx/optional/optional.h"

#include <type_traits>

template <typename T>
requires std::semiregular<std::hash<std::remove_cvref_t<T>>> &&
    requires(T const& val, std::hash<std::remove_cvref_t<T>> const hasher) {
        { hasher(val) } -> std::same_as<size_t>;
    }
struct std::hash<__RXX optional<T>> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX optional<T> const& opt) RXX_CONST_CALL
        noexcept(std::is_nothrow_invocable_v<std::hash<std::remove_cvref_t<T>>,
            T const&>) {
        return opt.has_value() ? std::hash<std::remove_cvref_t<T>>()(*opt) : 0;
    }
};
