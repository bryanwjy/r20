// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/functional/invoke_r.h"
#include "rxx/utility/forward.h"
#include "rxx/utility/move.h"
#include "rxx/variant/bad_variant_access.h"
#include "rxx/variant/get.h"
#include "rxx/variant/variant.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE)
constexpr auto&& as_variant(variant<Ts...>& value) noexcept {
    return value;
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE)
constexpr auto&& as_variant(variant<Ts...> const& value) noexcept {
    return value;
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE)
constexpr auto&& as_variant(variant<Ts...>&& value) noexcept {
    return __RXX move(value);
}

template <typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE)
constexpr auto&& as_variant(variant<Ts...> const&& value) noexcept {
    return __RXX move(value);
}

template <typename F, typename V, typename... Vs>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN)
constexpr decltype(auto)
    variant_visitor(F&& callable, V&& value, Vs&&... values) {
    if constexpr (sizeof...(Vs) == 0) {
        return __RXX forward<V>(value).visit(__RXX forward<F>(callable));
    } else {
        using variant_type = std::remove_cvref_t<V>;
        return iota_table_for<variant_type>(
            [&callable, &value]<size_t I>(std::integral_constant<size_t, I>,
                Vs&&... tail) -> decltype(auto) {
                return variant_visitor(
                    [&callable, &value]<typename... T>(T&&... others) {
                        return std::invoke(__RXX forward<F>(callable),
                            __RXX get<I>(__RXX forward<V>(value)),
                            __RXX forward<T>(others)...);
                    },
                    __RXX forward<Vs>(tail)...);
            },
            value.index(), __RXX forward<Vs>(values)...);
    }
}

template <typename R, typename F, typename V, typename... Vs>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN)
constexpr R variant_visitor(F&& callable, V&& value, Vs&&... values) {
    using variant_type = std::remove_cvref_t<V>;
    if constexpr (sizeof...(Vs) == 0) {
        return __RXX forward<V>(value).template visit<R>(
            __RXX forward<F>(callable));
    } else {
        using variant_type = std::remove_cvref_t<V>;
        return iota_table_for<variant_type>(
            [&callable, &value]<size_t I>(
                std::integral_constant<size_t, I>, Vs&&... tail) -> R {
                return variant_visitor<R>(
                    [&callable, &value]<typename... T>(T&&... others) {
                        return __RXX invoke_r<R>(__RXX forward<F>(callable),
                            __RXX get<I>(__RXX forward<V>(value)),
                            __RXX forward<T>(others)...);
                    },
                    __RXX forward<Vs>(tail)...);
            },
            value.index(), __RXX forward<Vs>(values)...);
    }
}

} // namespace details

template <typename F, typename... Vs>
requires requires { (..., details::as_variant(std::declval<Vs>())); }
__RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(
    F&& callable, Vs&&... values) {
    if ((... || values.valueless_by_exception())) {
        RXX_THROW(bad_variant_access());
    }

    if constexpr (sizeof...(Vs) == 0) {
        return std::invoke(__RXX forward<F>(callable));
    } else {
        return details::variant_visitor(__RXX forward<F>(callable),
            details::as_variant(__RXX forward<Vs>(values))...);
    }
}

template <typename R, typename F, typename... Vs>
requires requires { (..., details::as_variant(std::declval<Vs>())); }
__RXX_HIDE_FROM_ABI constexpr R visit(F&& callable, Vs&&... values) {
    if ((... || values.valueless_by_exception())) {
        RXX_THROW(bad_variant_access());
    }

    if constexpr (sizeof...(Vs) == 0) {
        return __RXX invoke_r<R>(__RXX forward<F>(callable));
    } else {
        return details::variant_visitor<R>(__RXX forward<F>(callable),
            details::as_variant(__RXX forward<Vs>(values))...);
    }
}

RXX_DEFAULT_NAMESPACE_END
