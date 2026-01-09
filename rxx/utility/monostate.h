// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/functional/hash.h"

#include <compare>
#if RXX_LIBCXX && __has_include(<__concepts/regular.h>)
#  include <__concepts/regular.h>
#else
#  include <concepts>
#endif
#if RXX_LIBCXX && __has_include(<__type_traits/is_empty.h>)
#  include <__type_traits/is_empty.h>
#else
#  include <type_traits>
#endif
#if RXX_LIBCXX && __has_include(<__concepts/same_as.h>)
#  include <__concepts/same_as.h>
#else
#  include <concepts>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <typename T>
concept monostate_like = std::is_empty_v<T> && requires(T* ptr) {
    ptr->~monostate();
    T();
};
} // namespace details

struct __RXX_ABI_PUBLIC monostate {
    __RXX_HIDE_FROM_ABI constexpr monostate() noexcept = default;
    template <typename T>
    requires (!std::same_as<T, monostate>) && details::monostate_like<T>
    __RXX_HIDE_FROM_ABI constexpr monostate(T) noexcept {}
    template <typename T>
    requires (!std::same_as<T, monostate>) && details::monostate_like<T>
    __RXX_HIDE_FROM_ABI operator T() const noexcept {
        return T{};
    }
};

RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr bool operator==(monostate, monostate) noexcept {
    return true;
}

RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto operator<=>(monostate, monostate) noexcept {
    return std::strong_ordering::equal;
}

RXX_DEFAULT_NAMESPACE_END

template <>
struct std::hash<__RXX monostate> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX monostate) RXX_CONST_CALL noexcept {
        return 66740831;
    }
};
