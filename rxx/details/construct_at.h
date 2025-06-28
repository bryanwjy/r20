// Copyright 2023-2025 Bryan Wong

#pragma once

#if RXX_COMPILER_MSVC

#  include <new>
#  include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges::details {
template <typename T, typename... Args>
__RXX_HIDE_FROM_ABI constexpr T* construct_at(
    T* location, Args&&... args) noexcept(noexcept(::new((void*)0)
        T{std::declval<Args>()...})) {
    [[msvc::constexpr]] return ::new (location) T{std::forward<Args>(args)...};
}
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END

#elif RXX_COMPILER_CLANG | RXX_COMPILER_GCC

#  if __has_include(<bits/stl_construct.h>)
// libstdc++
#    include <bits/stl_construct.h>
#  elif __has_include(<__memory/construct_at.h>)
// libc++
#    include <__memory/construct_at.h>
#  else
#    include <memory>
#  endif

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges::details {
using std::construct_at;
}
RXX_DEFAULT_NAMESPACE_END

#else
#  error "Unsupported"
#endif
