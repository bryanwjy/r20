// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::reverse_copy;
}

template <typename I, typename O>
using reverse_copy_result = in_out_result<I, O>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
