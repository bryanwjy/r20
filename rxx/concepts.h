// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/primitives.h"

#include <concepts>
#include <initializer_list>
#include <iterator>
#include <ranges>
#if __has_include(<__ranges/enable_view.h>)
#  include <__ranges/enable_view.h>
#elif __has_include(<bits/ranges_base.h>)
#  include <bits/ranges_base.h>
#elif __has_include(<xutility>)
#  include <xutility>
#else
#  include <ranges>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {
template <typename T>
inline constexpr bool is_initializer_list = false;
template <typename T>
inline constexpr bool is_initializer_list<std::initializer_list<T>> = true;
} // namespace details

using ::std::ranges::enable_view;

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

template <typename T>
concept view = range<T> && std::movable<T> && enable_view<T>;

template <class T>
concept viewable_range = range<T> &&
    ((view<std::remove_cvref_t<T>> &&
         std::constructible_from<std::remove_cvref_t<T>, T>) ||
        (!view<std::remove_cvref_t<T>> &&
            (std::is_lvalue_reference_v<T> ||
                (std::movable<std::remove_reference_t<T>> &&
                    !details::is_initializer_list<T>))));

namespace details {
template <typename T>
concept constant_iterator = std::input_iterator<T> &&
    std::same_as<iter_const_reference_t<T>, iter_reference_t<T>>;
} // namespace details

template <typename T>
concept constant_range =
    input_range<T> && details::constant_iterator<iterator_t<T>>;

template <range R>
using const_iterator_t = decltype(__RXX ranges::cbegin(std::declval<R&>()));

template <range R>
using const_sentinel_t = decltype(__RXX ranges::cend(std::declval<R&>()));

template <range R>
using range_const_reference_t =
    __RXX iter_const_reference_t<ranges::iterator_t<R>>;
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
