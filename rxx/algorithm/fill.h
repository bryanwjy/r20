// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::fill;
using std::ranges::fill_n;
} // namespace cpo
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
