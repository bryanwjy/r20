// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/primitives.h"

#include <concepts>
#include <iterator>
#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
namespace details {
template <typename T>
concept constant_iterator = std::input_iterator<T> &&
    std::same_as<iter_const_reference_t<T>, iter_reference_t<T>>;
} // namespace details

template <typename T>
concept constant_range =
    std::ranges::input_range<T> && details::constant_iterator<iterator_t<T>>;
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
