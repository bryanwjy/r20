// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::search_n;
}
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
