/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h"

#if RXX_COMPILER_CLANG

#  if defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
#    define RXX_TARGET_MICROSOFT 1
#  elif defined(__APPLE__)
#    define RXX_TARGET_APPLE 1
#  elif defined(RENAME_TO_SONY_IDENTIFIER) /* NDA */
#    define RXX_TARGET_SONY 1
#  elif defined(RENAME_TO_NINTENDO_IDENTIFIER) /* NDA */
#    define RXX_TARGET_NINTENDO 1
#  elif defined(__FreeBSD__) || defined(__DragonFly__) || \
      defined(__NetBSD__) || defined(__OpenBSD__)
#    define RXX_TARGET_BSD 1
#  elif defined(__linux__)
#    define RXX_TARGET_LINUX 1
#  elif defined(__unix__)
#    define RXX_TARGET_UNIX 1
#  else
#    error "Unrecognized target for Clang Compiler"
#  endif

#elif RXX_COMPILER_GCC

#  if defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
#    define RXX_TARGET_MICROSOFT 1
#  elif defined(__APPLE__)
#    define RXX_TARGET_APPLE 1
#  elif defined(__FreeBSD__) || defined(__DragonFly__) || \
      defined(__NetBSD__) || defined(__OpenBSD__)
#    define RXX_TARGET_BSD 1
#  elif defined(__linux__)
#    define RXX_TARGET_LINUX 1
#  elif defined(__unix__)
#    define RXX_TARGET_UNIX 1
#  else
#    error "Unrecognized target for GNU Compiler"
#  endif

#elif RXX_COMPILER_MSVC

#  define RXX_TARGET_MICROSOFT 1

#elif RXX_COMPILER_ICC || RXX_COMPILER_ICX

#  if defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
#    define RXX_TARGET_MICROSOFT 1
#  elif defined(__APPLE__)
#    define RXX_TARGET_APPLE 1
#  elif defined(__FreeBSD__) || defined(__DragonFly__) || \
      defined(__NetBSD__) || defined(__OpenBSD__)
#    define RXX_TARGET_BSD 1
#  elif defined(__linux__)
#    define RXX_TARGET_LINUX 1
#  elif defined(__unix__)
#    define RXX_TARGET_UNIX 1
#  else
#    error "Unrecognized target for Intel Compiler"
#  endif

#endif /* if RXX_COMPILER_CLANG */

#if RXX_TARGET_APPLE && !RXX_TARGET_UNIX
#  define RXX_TARGET_UNIX 1
#endif /* if RXX_TARGET_APPLE && !defined(RXX_TARGET_UNIX) */

#if RXX_TARGET_LINUX && !RXX_TARGET_UNIX
#  define RXX_TARGET_UNIX 1
#endif /* if RXX_TARGET_LINUX && !defined(RXX_TARGET_UNIX) */

#if RXX_TARGET_SONY
#  if !RXX_TARGET_BSD
#    define RXX_TARGET_BSD 1
#  endif
#  if !RXX_TARGET_UNIX
#    define RXX_TARGET_UNIX 1
#  endif
#endif /* if RXX_TARGET_SONY && !defined(RXX_TARGET_BSD) */

#if RXX_TARGET_BSD && !RXX_TARGET_UNIX
#  define RXX_TARGET_UNIX 1
#endif /* if RXX_TARGET_BSD && !defined(RXX_TARGET_UNIX) */

#if RXX_TARGET_SONY
// AFAIK the Pro models are not dedicated targets, I could be wrong though
#  if RENAME_TO_PS4_IDENTIFIER /* NDA */
#    define RXX_TARGET_SONY_PS4 1
#  elif RENAME_TO_PS5_IDENTIFIER /* NDA */
#    define RXX_TARGET_SONY_PS5 1
#  else
#    error "Unrecognized Sony target"
#  endif
#endif /* if RXX_TARGET_SONY */

#if RXX_TARGET_MICROSOFT

#  if defined(__CYGWIN__)
#    define RXX_TARGET_MICROSOFT_WINDOWS 1
#    define RXX_TARGET_MICROSOFT_CYGWIN 1
#  elif defined(_WIN32) || defined(_WIN64)
#    define RXX_TARGET_MICROSOFT_WINDOWS 1
#  elif RENAME_TO_GDK_XBO_IDENTIFIER /* NDA */
#    define RXX_TARGET_MICROSOFT_GDK_XBO 1
#  elif RENAME_TO_GDK_XSX_IDENTIFIER /* NDA */
#    define RXX_TARGET_MICROSOFT_GDK_XSX 1
#  else
#    error "Unrecognized Microsoft target"
#  endif

#  if RENAME_TO_DESKTOP_GDK_IDENTIFIER /* NDA */
#    define RXX_TARGET_MICROSOFT_GDK_WINDOWS 1
#  endif

#endif /* if RXX_TARGET_MICROSOFT */

#if RXX_COMPILER_CLANG | RXX_COMPILER_GCC | RXX_COMPILER_ICX | \
    (RXX_COMPILER_ICC & !RXX_TARGET_MICROSOFT)
#  define RXX_SUPPORTS_GNU_ASM 1
#endif
