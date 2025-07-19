// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::swap_ranges;
}

template <typename I1, typename I2>
using swap_ranges_result = in_in_result<I1, I2>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
