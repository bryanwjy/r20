// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::remove_copy;
using std::ranges::remove_copy_if;

template <typename I, typename O>
using remove_copy_result = in_out_result<I, O>;
template <typename I, typename O>
using remove_copy_if_result = in_out_result<I, O>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
