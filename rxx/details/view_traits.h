// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/ranges/empty_view.h"
#include "rxx/ranges/iota_view.h"

#if RXX_LIBCXX
#  if __has_include(<__fwd/span.h>)
#    include <__fwd/span.h>
#  else
#    include <span>
#  endif
#  if __has_include(<__fwd/string_view.h>)
#    include <__fwd/string_view.h>
#  else
#    include <string_view>
#  endif
#else

#  include <string_view>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {
template <typename R>
inline constexpr bool is_empty_view = false;

template <typename T>
inline constexpr bool is_empty_view<ranges::empty_view<T>> = true;

template <typename R>
inline constexpr bool is_basic_string_view = false;

template <typename Char, typename Traits>
inline constexpr bool
    is_basic_string_view<std::basic_string_view<Char, Traits>> = true;

template <typename R>
inline constexpr bool is_span = false;

template <typename T, size_t N>
inline constexpr bool is_span<std::span<T, N>> = true;

template <typename R>
inline constexpr bool is_iota_view = false;

template <typename W, typename Bound>
inline constexpr bool is_iota_view<ranges::iota_view<W, Bound>> = true;

template <typename R>
inline constexpr bool is_repeat_view = false;

} // namespace details
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
