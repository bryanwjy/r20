// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::clamp;
using std::ranges::max;
using std::ranges::max_element;
using std::ranges::min;
using std::ranges::min_element;
using std::ranges::minmax;
using std::ranges::minmax_element;

template <typename T>
using minmax_result = min_max_result<T>;
template <typename I>
using minmax_element_result = min_max_result<I>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
