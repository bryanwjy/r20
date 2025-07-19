// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

inline namespace cpo {
using std::ranges::is_heap;
using std::ranges::is_heap_until;
using std::ranges::make_heap;
using std::ranges::pop_heap;
using std::ranges::push_heap;
using std::ranges::sort_heap;
} // namespace cpo

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
