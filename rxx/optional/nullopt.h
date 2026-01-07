// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h"

#if RXX_ENABLE_STD_INTEROP
#  include <concepts>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

struct nullopt_t {
    struct ctor_tag {
        __RXX_HIDE_FROM_ABI explicit constexpr ctor_tag() noexcept = default;
    };
    __RXX_HIDE_FROM_ABI explicit constexpr nullopt_t(
        ctor_tag, ctor_tag) noexcept {}

#if RXX_ENABLE_STD_INTEROP
    __RXX_HIDE_FROM_ABI constexpr nullopt_t(
        std::same_as<std::nullopt_t> auto) noexcept {}
#endif
};

inline constexpr nullopt_t nullopt{
    nullopt_t::ctor_tag{}, nullopt_t::ctor_tag{}};

RXX_DEFAULT_NAMESPACE_END
