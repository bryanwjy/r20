/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/abi.h"

#define RXX_NAMESPACE_BEGIN(NAME) namespace __RXX_PUBLIC_TYPE_VISIBILITY NAME {

#define RXX_NAMESPACE_END(NAME) }

#include <stddef.h>

#ifdef _LIBCPP_ABI_NAMESPACE
#  define RXX_STD_ABI_NAMESPACE_BEGIN inline namespace _LIBCPP_ABI_NAMESPACE {
#  define RXX_STD_ABI_NAMESPACE_END }
#elif defined(_GLIBCXX_BEGIN_NAMESPACE_VERSION)
#  define RXX_STD_ABI_NAMESPACE_BEGIN _GLIBCXX_BEGIN_NAMESPACE_VERSION
#  define RXX_STD_ABI_NAMESPACE_END _GLIBCXX_END_NAMESPACE_VERSION
#endif

#ifndef RXX_STD_ABI_NAMESPACE_BEGIN
#  define RXX_STD_ABI_NAMESPACE_BEGIN
#  define RXX_STD_ABI_NAMESPACE_END
#endif

/* extern C++ for MSVC > C++20, no effect anywhere else */
#define RXX_STD_NAMESPACE_BEGIN                                              \
    RXX_EXTERN_CXX_BEGIN namespace RXX_ATTRIBUTE(TYPE_VISIBILITY("default")) \
        std {                                                                \
        RXX_STD_ABI_NAMESPACE_BEGIN

#define RXX_STD_NAMESPACE_END \
    RXX_STD_ABI_NAMESPACE_END \
    }                         \
    RXX_EXTERN_CXX_END
