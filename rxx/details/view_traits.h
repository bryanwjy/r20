// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/empty_view.h"
#include "rxx/iota_view.h"

#include <span>
#include <string_view>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {
template <typename R>
inline constexpr bool is_empty_view = false;

template <typename T>
inline constexpr bool is_empty_view<std::ranges::empty_view<T>> = true;

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
inline constexpr bool is_iota_view<std::ranges::iota_view<W, Bound>> = true;

template <typename R>
inline constexpr bool is_repeat_view = false;

} // namespace details
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
