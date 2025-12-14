// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/utility/forward.h"

#include <concepts>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename T, typename F, typename... Args>
concept generatable_from =
    std::invocable<F> && requires(T* ptr, F&& func, Args&&... args) {
        ::new (ptr) T(std::invoke(
            __RXX forward<F>(func), __RXX forward<Args>(args)...));
    };

template <typename T, typename F, typename... Args>
concept nothrow_generatable_from = generatable_from<T, F, Args...> &&
    requires(T* ptr, F&& func, Args&&... args) {
        {
            ::new (ptr) T(std::invoke(
                __RXX forward<F>(func), __RXX forward<Args>(args)...))
        } noexcept;
    };

} // namespace details

RXX_DEFAULT_NAMESPACE_END
