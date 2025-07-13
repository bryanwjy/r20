// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/iter_traits.h"

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <range R>
using range_reference_t = iter_reference_t<iterator_t<R>>;

template <range R>
using range_rvalue_reference_t = iter_rvalue_reference_t<iterator_t<R>>;

template <range R>
using range_common_reference_t = iter_common_reference_t<iterator_t<R>>;

template <range R>
using const_iterator_t = const_iterator<iterator_t<R>>;

template <range R>
using const_sentinel_t = const_sentinel<sentinel_t<R>>;

template <range R>
using range_const_reference_t = iter_const_reference_t<iterator_t<R>>;

template <range R>
using range_const_rvalue_reference_t =
    iter_const_rvalue_reference_t<iterator_t<R>>;

template <sized_range R>
using range_size_t = decltype(ranges::size(std::declval<R&>()));

template <range R>
using range_difference_t = iter_difference_t<iterator_t<R>>;

template <range R>
using range_value_t = iter_value_t<iterator_t<R>>;
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
