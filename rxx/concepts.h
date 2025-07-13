// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/primitives.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
// input_range defined in access.h

template <typename R, typename T>
concept output_range = range<R> && std::output_iterator<iterator_t<R>, T>;

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
        __RXX ranges::data(t)
        } -> std::same_as<std::add_pointer_t<range_reference_t<T>>>;
    };

template <typename T>
concept common_range = range<T> && std::same_as<iterator_t<T>, sentinel_t<T>>;

template <typename T>
concept view = range<T> && std::movable<T> && enable_view<T>;

template <typename T>
concept viewable_range = range<T> &&
    ((view<std::remove_cvref_t<T>> &&
         std::constructible_from<std::remove_cvref_t<T>, T>) ||
        (!view<std::remove_cvref_t<T>> &&
            (std::is_lvalue_reference_v<T> ||
                (std::movable<std::remove_reference_t<T>> &&
                    !details::is_initializer_list<T>))));

template <typename T>
concept borrowed_range = range<T> && details::borrowable<T>;

template <typename T>
concept constant_range =
    input_range<T> && __RXX details::constant_iterator<iterator_t<T>>;

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
