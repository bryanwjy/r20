/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/glibcxx.h"  // IWYU pragma: keep
#include "rxx/configuration/standard.h" // IWYU pragma: keep
#include "rxx/preprocessor/concatenation.h"

/**
 * Standard library macros
 */

#ifdef _LIBCPP_VERSION
#  define RXX_LIBCXX 1

/**
 * All arguments must be two character decimal digits, if the value is single
 * digit the argument should be prefixed with '0'
 */
#  define RXX_LIBCXX_AT_LEAST(MAJOR, MINOR, PATCH) \
      (_LIBCPP_VERSION >= RXX_CONCAT(RXX_CONCAT(MAJOR, MINOR), PATCH))
#else
#  define RXX_LIBCXX_AT_LEAST(MAJOR, MINOR, PATCH) 0
#endif

#ifdef __RXX_GLIBCXX_GCC_MAJOR
#  if RXX_LIBCXX
#    error "Invalid environment"
#  endif
/**
 * Only major version is exposed
 */
#  define RXX_LIBSTDCXX_AT_LEAST(MAJOR) (__RXX_GLIBCXX_GCC_MAJOR >= MAJOR)
#  define RXX_LIBSTDCXX_AFTER(YYYY, MM, DD) (__GLIBCXX__ > YYYY##MM##DD)
#  define RXX_LIBSTDCXX 1
#else
#  define RXX_LIBSTDCXX_AFTER(YYYY, MM, DD) 0
#  define RXX_LIBSTDCXX_AT_LEAST(MAJOR) 0
#endif

#ifdef _MSVC_STL_VERSION

#  ifndef _MSVC_STL_UPDATE
#    error "Invalid environment"
#  endif
#  if RXX_LIBCXX | RXX_LIBSTDCXX
#    error "Invalid environment"
#  endif

#  define RXX_MSVC_STL 1

#  define RXX_MSVC_STL_AT_LEAST(VERSION, UPDATE) \
      (_MSVC_STL_VERSION > VERSION ||            \
          (_MSVC_STL_VERSION == VERSION && _MSVC_STL_UPDATE >= UPDATE))
#else
#  define RXX_MSVC_STL_AT_LEAST(VERSION, UPDATE) 0
#endif
