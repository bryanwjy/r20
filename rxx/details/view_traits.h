// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#if RXX_LIBCXX
#  if __has_include(<__fwd/span.h>)
#    include <__fwd/span.h>
#  else
RXX_STD_NAMESPACE_BEGIN
template <typename, size_t>
class span;
RXX_STD_NAMESPACE_END
#  endif
#  if __has_include(<__fwd/string_view.h>)
#    include <__fwd/string_view.h>
#  else
RXX_STD_NAMESPACE_BEGIN
template <typename, typename>
class basic_string_view;
RXX_STD_NAMESPACE_END
#  endif
#else

RXX_STD_NAMESPACE_BEGIN
template <typename, typename>
class basic_string_view;
template <typename, size_t>
class span;
RXX_STD_NAMESPACE_END
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {
template <typename R>
inline constexpr bool is_empty_view = false;

template <typename R>
inline constexpr bool is_empty_view_like = false;

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

template <typename R>
inline constexpr bool is_iota_view_like = false;

template <typename R>
inline constexpr bool is_repeat_view = false;

template <typename R>
inline constexpr bool is_repeat_view_like = false;

template <typename R>
inline constexpr bool is_subrange = false;

template <typename R>
inline constexpr bool is_subrange_like = false;

} // namespace details
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
