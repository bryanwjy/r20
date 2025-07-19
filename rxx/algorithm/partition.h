// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::is_partitioned;
using std::ranges::partition;
using std::ranges::partition_copy;
using std::ranges::partition_point;
using std::ranges::stable_partition;

template <typename I, typename O1, typename O2>
using partition_copy_result = in_out_out_result<I, O1, O2>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
