/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/builtin_check.h"

#if RXX_HAS_BUILTIN(__builtin_memcmp)

#  define __RXX_MEMCMP __builtin_memcmp

#else

#  include "rxx/configuration/compiler.h"
#  include "rxx/configuration/standard.h"

#  if RXX_CXX
extern "C" int memcmp(void const*, void const*, decltype(sizeof(0)));
#  else
#    include <stddef.h>
int memcmp(void const*, void const*, size_t);
#  endif

#  if RXX_COMPILER_MSVC
#    pragma intrinsic(memcmp)
#  endif
#  define __RXX_MEMCMP ::memcmp

#endif
