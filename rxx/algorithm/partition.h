// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::is_partitioned;
using std::ranges::partition;
using std::ranges::partition_copy;
using std::ranges::partition_point;
using std::ranges::stable_partition;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
