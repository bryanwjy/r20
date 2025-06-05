/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h"

#ifndef _LIBCPP_HAS_NO_EXCEPTIONS
#  if defined(__cpp_exceptions)
#    define RXX_WITH_EXCEPTIONS 1
#  elif RXX_COMPILER_MSVC && defined(_CPPUNWIND)
#    if _CPPUNWIND
#      define RXX_WITH_EXCEPTIONS 1
#    endif
#  elif defined(__EXCEPTIONS)
#    define RXX_WITH_EXCEPTIONS 1
#  endif
#endif
