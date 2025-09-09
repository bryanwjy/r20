// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <concepts>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

template <std::move_constructible T, typename U = T>
requires std::assignable_from<T&, U>
__RXX_HIDE_FROM_ABI constexpr T exchange(T& obj, U&& new_value) noexcept(
    std::is_nothrow_move_constructible_v<T> &&
    std::is_nothrow_assignable_v<T&, U>) {
    T previous(std::move(obj));
    obj = std::forward<U>(new_value);
    return previous;
}

RXX_DEFAULT_NAMESPACE_END
