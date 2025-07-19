// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::binary_search;
using std::ranges::equal_range;
using std::ranges::lower_bound;
using std::ranges::upper_bound;
} // namespace cpo

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
