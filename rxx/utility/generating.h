// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

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

RXX_DEFAULT_NAMESPACE_END
