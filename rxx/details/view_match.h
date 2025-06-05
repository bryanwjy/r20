// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>
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

template <typename T>
concept span_type = requires {
    typename T::element_type;
    requires std::same_as<std::span<typename T::element_type, T::extent>, T>;
};

template <typename R>
inline constexpr bool is_iota_view = false;

template <typename W, typename Bound>
inline constexpr bool is_iota_view<std::ranges::iota_view<W, Bound>> = true;

} // namespace details
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
