// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/tuple.h"

#include <functional>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename... Types>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr tuple<std::unwrap_ref_decay_t<Types>...> make_tuple(
    Types&&... args) noexcept {
    return tuple<std::unwrap_ref_decay_t<Types>...>(
        std::forward<Types>(args)...);
}

RXX_DEFAULT_NAMESPACE_END
