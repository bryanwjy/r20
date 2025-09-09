// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/ranges/access.h"

#include <cstddef>

#if RXX_LIBSTDCXX && __has_include(<bits/memoryfwd.h>)
#  include <bits/memoryfwd.h>
#elif RXX_LIBCXX && __has_include(<__fwd/memory.h>)
#  include <__fwd/memory.h>
#elif RXX_MSVC_STL && __has_include(<__msvc_iter_core.hpp>) && __has_include(<xmemory>)
#  include <__msvc_iter_core.hpp>
#  include <xmemory>
#else
#  include <memory>
#endif
// IWYU pragma: end_exports

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

template <range R, typename Alloc = std::allocator<std::byte>>
struct elements_of {
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) R range;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) Alloc allocator = Alloc();
};

template <typename R, typename Alloc = std::allocator<std::byte>>
elements_of(R&&, Alloc = Alloc()) -> elements_of<R&&, Alloc>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
