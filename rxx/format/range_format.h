// Copyright 2025-2026 Bryan Wong
#pragma once

#include "rxx/config.h"

#if RXX_CXX23 && __cpp_lib_format_ranges >= 202207L
#  if RXX_LIBSTDCXX & __has_include(<bits/formatfwd.h>)
#    include <bits/formatfwd.h>
#    define RXX_SUPPORTS_RANGE_FORMAT 1
#  elif RXX_LIBCXX
#    if __has_include(<__format/range_format.h>)
#      include <__format/range_format.h>
#      define RXX_SUPPORTS_RANGE_FORMAT 1
#    elif __has_include(<__format/range_default_formattter.h>)
#      include <__format/range_default_formattter.h>
#      define RXX_SUPPORTS_RANGE_FORMAT 1
#    endif
#  elif RXX_MSVC_STL & __has_include(<__msvc_formatter.hpp>)
#    include <__msvc_formatter.hpp>
#    define RXX_SUPPORTS_RANGE_FORMAT 1
#  else
#    include <format>
#    define RXX_SUPPORTS_RANGE_FORMAT 1
#  endif
#endif
