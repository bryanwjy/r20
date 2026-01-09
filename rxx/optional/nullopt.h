// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

struct nullopt_t {
    struct ctor_tag {
        __RXX_HIDE_FROM_ABI explicit constexpr ctor_tag() noexcept = default;
    };
    __RXX_HIDE_FROM_ABI explicit constexpr nullopt_t(
        ctor_tag, ctor_tag) noexcept {}

    template <typename T>
    requires std::is_same_v<T, std::nullopt_t> &&
        std::is_same_v<std::nullopt_t, T>
    __RXX_HIDE_FROM_ABI constexpr nullopt_t(T) noexcept {}
};

inline constexpr nullopt_t nullopt{
    nullopt_t::ctor_tag{}, nullopt_t::ctor_tag{}};

RXX_DEFAULT_NAMESPACE_END
