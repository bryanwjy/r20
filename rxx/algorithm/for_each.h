// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::for_each;
using std::ranges::for_each_n;
} // namespace cpo

template <typename I, typename F>
using for_each_result = in_fun_result<I, F>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
