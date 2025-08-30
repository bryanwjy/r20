/* Copyright 2023-2025 Bryan Wong */

#pragma once

#ifdef __is_identifier
#  define RXX_IS_RESERVED_IDENTIFIER(X) !__is_identifier(X)
#else
#  define RXX_IS_RESERVED_IDENTIFIER(...) 0
#endif

#ifdef __has_builtin
#  define RXX_HAS_BUILTIN(BUILTIN) \
      (__has_builtin(BUILTIN) || RXX_IS_RESERVED_IDENTIFIER(BUILTIN))
#else
#  define RXX_HAS_BUILTIN(...) 0
#endif /* ifdef __has_builtin */

#define __RXX_SHOULD_USE_BUILTIN(BUILTIN) \
    RXX_HAS_BUILTIN(__##BUILTIN) && !RXX_DISABLE_BUILTIN_##BUILTIN
