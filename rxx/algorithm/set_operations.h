// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::includes;
using std::ranges::inplace_merge;
using std::ranges::merge;
using std::ranges::set_difference;
using std::ranges::set_intersection;
using std::ranges::set_symmetric_difference;
using std::ranges::set_union;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
