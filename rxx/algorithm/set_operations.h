// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

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

template <typename I1, typename I2, typename O>
using merge_result = in_in_out_result<I1, I2, O>;

template <typename I, typename O>
using set_difference_result = in_out_result<I, O>;

template <typename I1, typename I2, typename O>
using set_intersection_result = in_in_out_result<I1, I2, O>;

template <typename I1, typename I2, typename O>
using set_symmetric_difference_result = in_in_out_result<I1, I2, O>;

template <typename I1, typename I2, typename O>
using set_union_result = in_in_out_result<I1, I2, O>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
