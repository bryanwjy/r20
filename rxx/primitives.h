// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"

#include <iterator>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

using std::iter_reference_t;
using std::iter_value_t;
template <typename T>
using iter_const_reference_t =
    std::common_reference_t<iter_value_t<T> const&&, iter_reference_t<T>>;
using std::iter_common_reference_t;
using std::iter_difference_t;
using std::iter_rvalue_reference_t;

namespace ranges {
using std::ranges::range_common_reference_t;
using std::ranges::range_difference_t;
using std::ranges::range_reference_t;
using std::ranges::range_rvalue_reference_t;
using std::ranges::range_size_t;
using std::ranges::range_value_t;
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
