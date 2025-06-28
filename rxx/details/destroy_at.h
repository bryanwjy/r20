// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
__RXX_HIDE_FROM_ABI constexpr void destroy_at(T* ptr) noexcept {
    if constexpr (std::is_array_v<T>) {
        for (auto& element : *ptr) {
            (destroy_at)(RXX_BUILTIN_addressof(element));
        }
    } else {
        ptr->~T();
    }
}

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
