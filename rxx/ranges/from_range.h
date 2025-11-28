// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#if RXX_CXX23 && __cpp_lib_containers_ranges >= 202202L
#  if RXX_LIBCXX && __has_include(<__ranges/from_range.h>)
#    include <__ranges/from_range.h>
#    define RXX_SUPPORTS_FROM_RANGE 1
#  elif RXX_LIBSTDCXX && __has_include(<bits/ranges_base.h>)
#    ifdef __glibcxx_containers_ranges
#      if __glibcxx_containers_ranges
#        include <bits/ranges_base.h>
#        define RXX_SUPPORTS_FROM_RANGE 1
#      endif
#    endif
#  elif RXX_MSVC_STL && __has_include(<xmemory>)
#    include <xmemory>
#    define RXX_SUPPORTS_FROM_RANGE 1
#  else
#    include <ranges>
#    define RXX_SUPPORTS_FROM_RANGE 1
#  endif
#endif

#if RXX_SUPPORTS_FROM_RANGE

RXX_DEFAULT_NAMESPACE_BEGIN

inline constexpr std::from_range_t from_range{};

RXX_DEFAULT_NAMESPACE_END
#endif
