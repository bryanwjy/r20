// Copyright 2025-2026 Bryan Wong

#pragma once

#include "rxx/config.h"
// IWYU pragma: always_keep

#if RXX_LIBSTDCXX
#  if __has_include(<bits/functional_hash.h>)
#    include <bits/functional_hash.h>
#  else
RXX_STD_NAMESPACE_BEGIN
template <typename>
struct hash;
RXX_STD_NAMESPACE_END
#  endif
#elif RXX_LIBCXX

#  if __has_include(<__functional/hash.h>)
#    include <__functional/hash.h>
#  else
RXX_STD_NAMESPACE_BEGIN
template <typename>
struct hash;
RXX_STD_NAMESPACE_END
#  endif
#else
RXX_STD_NAMESPACE_BEGIN
template <typename>
struct hash;
RXX_STD_NAMESPACE_END
#endif
