// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/functional/invoke_r.h"
#include "rxx/utility/forward.h"
#include "rxx/variant/bad_variant_access.h"
#include "rxx/variant/variant.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename F, typename... Vs>
requires requires { (..., details::as_variant(std::declval<Vs>())); }
__RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(
    F&& callable, Vs&&... values) {
    if ((... || details::as_variant(values).valueless_by_exception())) {
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
    if ((... || details::as_variant(values).valueless_by_exception())) {
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
