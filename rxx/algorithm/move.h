// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::move;
using std::ranges::move_backward;
} // namespace cpo

template <typename I, typename O>
using move_result = in_out_result<I, O>;
template <typename I, typename O>
using move_backward_result = in_out_result<I, O>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
