// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::is_permutation;
using std::ranges::next_permutation;
using std::ranges::prev_permutation;

template <typename I>
using next_permutation_result = in_found_result<I>;
template <typename I>
using prev_permutation_result = in_found_result<I>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
