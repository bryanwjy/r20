// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

// IWYU pragma: always_keep

#ifndef RXX_OPTIONAL_ABI_INTEROP
#  define RXX_OPTIONAL_ABI_INTEROP 0
#endif

#ifndef RXX_OPTIONAL_NUA_ABI
#  define RXX_OPTIONAL_NUA_ABI (!RXX_COMPILER_GCC)
#endif

#if RXX_OPTIONAL_NUA_ABI
#  define __RXX_INLINE_IF_GCC_ABI
#  define __RXX_INLINE_IF_NUA_ABI inline
#else
#  define __RXX_INLINE_IF_GCC_ABI inline
#  define __RXX_INLINE_IF_NUA_ABI
#endif
