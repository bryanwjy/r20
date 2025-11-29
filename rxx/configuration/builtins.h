/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/builtin_check.h"
#include "rxx/configuration/compiler.h"
#include "rxx/configuration/msvc_builtins.h" // IWYU pragma: keep
#include "rxx/configuration/standard.h"      // IWYU pragma: keep

#if RXX_HAS_BUILTIN(__builtin_launder)
#  define RXX_BUILTIN_launder(X) __builtin_launder(X)
#elif RXX_COMPILER_GCC_AT_LEAST(7, 1, 0)
#  define RXX_BUILTIN_launder(X) __builtin_launder(X)
#elif RXX_COMPILER_ICC_AT_LEAST(1900)
#  define RXX_BUILTIN_launder(X) __builtin_launder(X)
#elif RXX_COMPILER_ICX
#  define RXX_BUILTIN_launder(X) __builtin_launder(X)
#endif /* RXX_HAS_BUILTIN(__builtin_launder) */

#if RXX_HAS_BUILTIN(__builtin_invoke)
#  define RXX_BUILTIN_invoke(X) __builtin_invoke(X)
#endif

#if RXX_HAS_BUILTIN(__builtin_addressof)
#  define RXX_BUILTIN_addressof(X) __builtin_addressof(X)
#elif RXX_COMPILER_GCC_AT_LEAST(7, 1, 0)
#  define RXX_BUILTIN_addressof(X) __builtin_addressof(X)
#elif RXX_COMPILER_MSVC_AT_LEAST(1914)
#  define RXX_BUILTIN_addressof(X) __builtin_addressof(X)
#elif RXX_COMPILER_ICC_AT_LEAST(1900)
#  define RXX_BUILTIN_addressof(X) __builtin_addressof(X)
#endif /* if RXX_HAS_BUILTIN(__builtin_addressof) */

#if RXX_HAS_BUILTIN(__builtin_unreachable)
#  define RXX_BUILTIN_unreachable() __builtin_unreachable()
#elif RXX_COMPILER_GCC_AT_LEAST(4, 5, 0)
#  define RXX_BUILTIN_unreachable() __builtin_unreachable()
#elif RXX_COMPILER_ICC_AT_LEAST(1300)
#  define RXX_BUILTIN_unreachable() __builtin_unreachable()
#elif RXX_COMPILER_MSVC /* RXX_HAS_BUILTIN(__builtin_unreachable) */
#  define RXX_BUILTIN_unreachable() __assume(0)
#else /* RXX_HAS_BUILTIN(__builtin_unreachable) */

#  if RXX_HAS_BUILTIN(__builtin_expect)
#    define RXX_BUILTIN_expect(VALUE, EXP) __builtin_expect(VALUE, EXP)
#  else
#    define RXX_BUILTIN_expect(VALUE, _1) VALUE
#  endif

#  ifdef RXX_CXX
extern "C" void abort(void);
#  else
void abort(void);
#  endif

#  define RXX_BUILTIN_unreachable() abort()
#endif /* RXX_HAS_BUILTIN(__builtin_unreachable) */

#if RXX_HAS_BUILTIN(__builtin_is_constant_evaluated)
#  define RXX_BUILTIN_is_constant_evaluated() __builtin_is_constant_evaluated()
#endif /* RXX_HAS_BUILTIN(__builtin_is_constant_evaluated) */

#if RXX_HAS_BUILTIN(__builtin_bit_cast)
#  define RXX_BUILTIN_bit_cast(...) __builtin_bit_cast(__VA_ARGS__)
#endif /* RXX_HAS_BUILTIN(__builtin_is_constant_evaluated) */

#if RXX_HAS_BUILTIN(__builtin_assume)
#  define RXX_BUILTIN_assume(...) __builtin_assume(__VA_ARGS__)
#elif RXX_COMPILER_MSVC /* RXX_HAS_BUILTIN(__builtin_unreachable) */
#  define RXX_BUILTIN_assume(...) __assume(__VA_ARGS__)
#elif RXX_HAS_BUILTIN(__builtin_unreachable)
#  define RXX_BUILTIN_assume(...) \
      ((__VA_ARGS__) ? (void)0 : __builtin_unreachable())
#else
#  define RXX_BUILTIN_assume(...) (void)0
#endif /* RXX_HAS_BUILTIN(__builtin_assume) */

#if RXX_HAS_BUILTIN(__is_layout_compatible) || \
    RXX_COMPILER_GCC_AT_LEAST(12, 0, 0)
#  define RXX_BUILTIN_is_layout_compatible(...) \
      __is_layout_compatible(__VA_ARGS__)
#endif

#if RXX_HAS_BUILTIN(__is_pointer_interconvertible_base_of) || \
    RXX_COMPILER_GCC_AT_LEAST(12, 0, 0)
#  define RXX_BUILTIN_is_pointer_interconvertible_base_of(...) \
      __is_pointer_interconvertible_base_of(__VA_ARGS__)
#endif

#if RXX_HAS_BUILTIN(__builtin_source_location)
#  define RXX_BUILTIN_source_location() __builtin_source_location()
#endif

#if RXX_HAS_BUILTIN(__builtin_FILE)
#  define RXX_BUILTIN_FILE() __builtin_FILE()
#endif

#if RXX_HAS_BUILTIN(__builtin_FUNCTION)
#  define RXX_BUILTIN_FUNCTION() __builtin_FUNCTION()
#endif

#if RXX_HAS_BUILTIN(__builtin_LINE)
#  define RXX_BUILTIN_LINE() __builtin_LINE()
#endif

#if RXX_HAS_BUILTIN(__builtin_COLUMN)
#  define RXX_BUILTIN_COLUMN() __builtin_COLUMN()
#endif

#if RXX_HAS_BUILTIN(__builtin_FUNCSIG)
#  define RXX_BUILTIN_FUNCSIG() __builtin_FUNCSIG()
#endif
