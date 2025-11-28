// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/utils.h"
#include "rxx/utility/forward_like.h"

#include <functional>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details::tuple {

template <typename F, tuple_like T>
inline constexpr bool is_applicable_v = []<size_t... Is>(
                                            std::index_sequence<Is...>) {
    return std::invocable<F, decl_element_t<Is, T>...>;
}(sequence_for<std::remove_cvref_t<T>>);

template <typename F, tuple_like T>
inline constexpr bool is_nothrow_applicable_v = requires {
    requires is_applicable_v<F, T>;
    requires ([]<size_t... Is>(std::index_sequence<Is...>) {
        return std::is_nothrow_invocable_v<F, decl_element_t<Is, T>...>;
    }(sequence_for<std::remove_cvref_t<T>>));
} && is_nothrow_accessible_v<T>;

} // namespace details::tuple

template <typename F, tuple_like T>
requires details::tuple::is_applicable_v<F, T>
__RXX_HIDE_FROM_ABI constexpr decltype(auto) apply(F&& func,
    T&& tuple) noexcept(details::tuple::is_nothrow_applicable_v<F, T>) {
    return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
        return std::invoke(std::forward<F>(func),
            __RXX forward_like<T>(ranges::get_element<Is>(tuple))...);
    }(details::tuple::sequence_for<std::remove_cvref_t<T>>);
}

RXX_DEFAULT_NAMESPACE_END
