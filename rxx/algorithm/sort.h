// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::is_sorted;
using std::ranges::is_sorted_until;
using std::ranges::nth_element;
using std::ranges::partial_sort;
using std::ranges::partial_sort_copy;
using std::ranges::sort;
using std::ranges::stable_sort;
} // namespace cpo

template <typename I, typename O>
using partial_sort_copy_result = in_out_result<I, O>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
