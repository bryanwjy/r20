// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/primitives.h"

#include <concepts>
#include <iterator>
#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

template <typename T>
concept range = requires(T& t) {
    ranges::begin(t);
    ranges::end(t);
};

template <typename T>
concept input_range = range<T> && std::input_iterator<iterator_t<T>>;

template <typename T>
concept forward_range = input_range<T> && std::forward_iterator<iterator_t<T>>;

template <typename T>
concept bidirectional_range =
    forward_range<T> && std::bidirectional_iterator<iterator_t<T>>;

template <typename T>
concept random_access_range =
    bidirectional_range<T> && std::random_access_iterator<iterator_t<T>>;

template <typename T>
concept contiguous_range = random_access_range<T> &&
    std::contiguous_iterator<ranges::iterator_t<T>> && requires(T& t) {
        {
            std::ranges::data(t)
        } -> std::same_as<std::add_pointer_t<ranges::range_reference_t<T>>>;
    };

namespace details {
template <typename T>
concept constant_iterator = std::input_iterator<T> &&
    std::same_as<iter_const_reference_t<T>, iter_reference_t<T>>;
} // namespace details

template <typename T>
concept constant_range =
    input_range<T> && details::constant_iterator<iterator_t<T>>;
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
