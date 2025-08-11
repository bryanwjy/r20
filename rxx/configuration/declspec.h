/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h" // IWYU pragma: keep

#if RXX_COMPILER_MSVC
#  include "rxx/preprocessor/concatenation.h"
#  include "rxx/preprocessor/is_empty.h"

#  define RXX_HAS_DECLSPEC(NAME) RXX_IS_EMPTY(RXX_CONCAT(RXX_DECLSPEC_, NAME))

#  define RXX_DECLSPEC_dllexport
#  define RXX_DECLSPEC_dllimport
#  define RXX_DECLSPEC_allocator
#  define RXX_DECLSPEC_deprecated
#  define RXX_DECLSPEC_empty_bases
#  define RXX_DECLSPEC_noalias
#  define RXX_DECLSPEC_noinline
#  define RXX_DECLSPEC_noreturn
#  define RXX_DECLSPEC_novtable
#  define RXX_DECLSPEC_no_sanitize_address
#  define RXX_DECLSPEC_restrict

#elif RXX_COMPILER_ARMCC /* NOT DEFINED */

#  define RXX_HAS_DECLSPEC(NAME) RXX_IS_EMPTY(RXX_CONCAT(RXX_DECLSPEC_, NAME))

#  define RXX_DECLSPEC_dllexport
#  define RXX_DECLSPEC_dllimport
#  define RXX_DECLSPEC_noinline
#  define RXX_DECLSPEC_noreturn
#  define RXX_DECLSPEC_notshared

#else
#  define RXX_HAS_DECLSPEC(NAME) 0
#endif
