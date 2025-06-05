/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h"
#include "rxx/preprocessor/concatenation.h"

#if RXX_COMPILER_CLANG | RXX_COMPILER_GCC | RXX_COMPILER_MSVC | \
    RXX_COMPILER_INTEL

#  define RXX_UNIQUE_VAR(var) RXX_CONCAT(var, __COUNTER__)

#else

/* On unrecognized platforms, use __LINE__ */
#  define RXX_UNIQUE_VAR(var) RXX_CONCAT(var, __LINE__)

#endif /* if RXX_COMPILER_CLANG */
