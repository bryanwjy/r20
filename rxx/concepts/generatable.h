// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/utility/forward.h"

#include <concepts>
#include <new> // IWYU pragma: keep

#if RXX_HAS_BUILTIN(__builtin_invoke)
RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <typename T, typename F, typename... Args>
concept generatable_from =
    std::invocable<F, Args...> && requires(T* ptr, F&& func, Args&&... args) {
        ::new (ptr) T(__builtin_invoke(
            __RXX forward<F>(func), __RXX forward<Args>(args)...));
    };

template <typename T, typename F, typename... Args>
concept nothrow_generatable_from = generatable_from<T, F, Args...> &&
    requires(T* ptr, F&& func, Args&&... args) {
        {
            ::new (ptr) T(__builtin_invoke(
                __RXX forward<F>(func), __RXX forward<Args>(args)...))
        } noexcept;
    };
} // namespace details

RXX_DEFAULT_NAMESPACE_END

#else

#  if RXX_LIBSTDCXX
#    include <functional>
#  elif RXX_LIBCXX
#    if __has_include(<__functional/invoke.h>)
#      include <__functional/invoke.h>
#    else
#      include <functional>
#    endif
#  elif RXX_MSVC_STL
#    include <type_traits>
#  else
#    include <functional>
#  endif

RXX_DEFAULT_NAMESPACE_BEGIN
namespace details {
template <typename T, typename F, typename... Args>
concept generatable_from =
    std::invocable<F, Args...> && requires(T* ptr, F&& func, Args&&... args) {
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
#endif
