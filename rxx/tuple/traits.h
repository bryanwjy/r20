// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

// IWYU pragma: always_keep
// IWYU pragma: begin_exports
#if RXX_LIBCXX

#  if __has_include( \
      <__tuple/tuple_size.h>) && __has_include(<__tuple/tuple_element.h>)
#    include <__tuple/tuple_element.h>
#    include <__tuple/tuple_size.h>
#  else
#    include <array>
#  endif

#elif RXX_LIBSTDCXX
#  if __has_include(<bits/utility.h>)
#    include <bits/utility.h>
#  else
#    include <array>
#  endif

#elif RXX_MSVC_STL

#  include <utility>

#else

#  include <array>

#endif
// IWYU pragma: end_exports
