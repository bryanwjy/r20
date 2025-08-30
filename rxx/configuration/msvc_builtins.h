/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h" // IWYU pragma: keep

#if RXX_COMPILER_MSVC

#  include "rxx/preprocessor/concatenation.h"
#  include "rxx/preprocessor/is_empty.h"

#  ifdef __has_builtin
#    error '__has_builtin' definition collision
#  endif
#  define __has_builtin(BUILTIN) \
      RXX_IS_EMPTY(RXX_CONCAT(__RXX_MSVC_BUILTIN, BUILTIN))

#  if RXX_COMPILER_MSVC_AT_LEAST(1915)
#    define __RXX_MSVC_BUILTIN__is_aggregate
#  endif

#  if RXX_CXX17
#    define __RXX_MSVC_BUILTIN__builtin_launder
#  endif

#  if RXX_COMPILER_MSVC_AT_LEAST(1925)
#    define __RXX_MSVC_BUILTIN__builtin_is_constant_evaluated
#  endif

#  if RXX_COMPILER_MSVC_AT_LEAST(1926)
#    define __RXX_MSVC_BUILTIN__builtin_bit_cast
#  endif

#  if RXX_COMPILER_MSVC_AT_LEAST(1927)
#    define __RXX_MSVC_BUILTIN__builtin_FILE
#    define __RXX_MSVC_BUILTIN__builtin_FUNCTION
#    define __RXX_MSVC_BUILTIN__builtin_LINE
#  endif

#  if RXX_COMPILER_MSVC_AT_LEAST(1929)
#    define __RXX_MSVC_BUILTIN__is_layout_compatible
#    define __RXX_MSVC_BUILTIN__is_pointer_interconvertible_base_of
#    define __RXX_MSVC_BUILTIN__is_corresponding_member
#  endif

#  if RXX_COMPILER_MSVC_AT_LEAST(1935)
#    define __RXX_MSVC_BUILTIN__builtin_FUNCSIG
#  endif

#  define __RXX_MSVC_BUILTIN__builtin_addressof

#  define __RXX_MSVC_BUILTIN__has_unique_object_representations
#  define __RXX_MSVC_BUILTIN__is_constructible
#  define __RXX_MSVC_BUILTIN__is_assignable
#  define __RXX_MSVC_BUILTIN__is_union
#  define __RXX_MSVC_BUILTIN__is_class
#  define __RXX_MSVC_BUILTIN__is_convertible_to
#  define __RXX_MSVC_BUILTIN__is_convertible
#  define __RXX_MSVC_BUILTIN__is_enum
#  define __RXX_MSVC_BUILTIN__is_member_function_pointer
#  define __RXX_MSVC_BUILTIN__is_member_object_pointer
#  define __RXX_MSVC_BUILTIN__is_empty
#  define __RXX_MSVC_BUILTIN__is_final
#  define __RXX_MSVC_BUILTIN__is_polymorphic
#  define __RXX_MSVC_BUILTIN__is_abstract
#  define __RXX_MSVC_BUILTIN__is_member_pointer
#  define __RXX_MSVC_BUILTIN__is_standard_layout
#  define __RXX_MSVC_BUILTIN__is_trivial
#  define __RXX_MSVC_BUILTIN__is_trivially_copyable
#  define __RXX_MSVC_BUILTIN__has_virtual_destructor
#  define __RXX_MSVC_BUILTIN__is_destructible
#  define __RXX_MSVC_BUILTIN__is_trivially_constructible
#  define __RXX_MSVC_BUILTIN__is_trivially_assignable
#  define __RXX_MSVC_BUILTIN__is_trivially_destructible
#  define __RXX_MSVC_BUILTIN__is_nothrow_constructible
#  define __RXX_MSVC_BUILTIN__is_base_of
#  define __RXX_MSVC_BUILTIN__underlying_type

#  ifdef __is_convertible
#    error '__is_convertible' definition collision
#  endif
#  define __is_convertible __is_convertible_to

#endif
