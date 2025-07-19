// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::transform;

template <typename I, typename O>
using unary_transform_result = in_out_result<I, O>;
template <typename I1, typename I2, typename O>
using binary_transform_result = in_in_out_result<I1, I2, O>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
