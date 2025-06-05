/* Copyright 2023-2025 Bryan Wong */

#pragma once

/**
 * Standard library macros
 */

#ifdef _LIBCPP_VERSION
#  define RXX_LIBCXX 1
#endif

#ifdef __GLIBCXX__
#  if RXX_LIBCXX
#    error "Invalid environment"
#  endif
#  define RXX_LIBSTDCXX 1
#endif

#ifdef _MSVC_STL_VERSION

#  if RXX_LIBCXX | RXX_LIBSTDCXX
#    error "Invalid environment"
#  endif

#  define RXX_MSVC_STL 1
#endif
