// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
inline namespace cpo {
using std::ranges::copy;
using std::ranges::copy_backward;
using std::ranges::copy_if;
using std::ranges::copy_n;
} // namespace cpo
template <typename I, typename O>
using copy_result = in_out_result<I, O>;
template <typename I, typename O>
using copy_if_result = in_out_result<I, O>;
template <typename I, typename O>
using copy_n_result = in_out_result<I, O>;
template <typename I1, typename I2>
using copy_backward_result = in_out_result<I1, I2>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
