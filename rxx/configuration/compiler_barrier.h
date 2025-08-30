/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h" // IWYU pragma: keep
#include "rxx/configuration/pragma.h"
#include "rxx/configuration/standard.h" // IWYU pragma: keep

#if RXX_SUPPORTS_GNU_ASM
#  define RXX_COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#elif RXX_COMPILER_MSVC

#  ifdef RXX_CXX
extern "C" void _ReadWriteBarrier();
#  else
void _ReadWriteBarrier();
#  endif
#  pragma intrinsic(_ReadWriteBarrier)
#  define RXX_COMPILER_BARRIER() _ReadWriteBarrier()
#else

#  include "rxx/configuration/pragma.h"

RXX_PRAGMA_WARN("Unrecognize compiler")
#  define RXX_COMPILER_BARRIER()
#endif
