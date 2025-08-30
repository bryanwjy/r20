/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/builtin_check.h"
#include "rxx/configuration/compiler.h"
#include "rxx/configuration/keywords.h"
#include "rxx/configuration/standard.h"

#if RXX_HAS_BUILTIN(__builtin_memcpy)

#  define __RXX_MEMCPY __builtin_memcpy

#else

#  if RXX_CXX
extern "C" void* memcpy(
    void* RXX_RESTRICT, void const* RXX_RESTRICT, decltype(sizeof(0)));
#  else
#    include <stddef.h>
void* memcpy(void* RXX_RESTRICT, void const* RXX_RESTRICT, size_t);
#  endif

#  if RXX_COMPILER_MSVC
#    pragma intrinsic(memcpy)
#  endif
#  define __RXX_MEMCPY ::memcpy

#endif
