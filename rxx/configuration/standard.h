/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h"

#if RXX_COMPILER_MSVC
#  ifdef __cplusplus
#    define RXX_CXX _MSVC_LANG
#  endif
#elif defined(__cplusplus)
#  define RXX_CXX __cplusplus
#endif

#ifdef RXX_CXX

#  include <cstddef> // IWYU pragma: export

#  if RXX_CXX < 201103L
#    error "C++11 is required"
#  endif /* RXX_CXX < 201103L */

#  define RXX_CXX11 1

#  if RXX_CXX >= 201402L
#    define RXX_CXX14 1
#    define RXX_CONSTEXPR_CXX14 constexpr
#  else /* RXX_CXX >= 201402L */
#    define RXX_CONSTEXPR_CXX14
#  endif /* RXX_CXX >= 201402L */

#  if RXX_CXX >= 201703L
#    define RXX_CXX17 1
#    define RXX_CONSTEXPR_CXX17 constexpr
#    define RXX_INLINE_CXX17 inline
#    define RXX_IF_CONSTEXPR if constexpr
#  else /* RXX_CXX >= 201703L */
#    define RXX_INLINE_CXX17
#    define RXX_CONSTEXPR_CXX17
#    define RXX_IF_CONSTEXPR if
#  endif /* RXX_CXX >= 201703L */

#  if RXX_CXX >= 202002L
#    define RXX_CXX20 1
#    define RXX_CONSTEXPR_CXX20 constexpr
#    define RXX_CONSTEVAL consteval
#    define RXX_EXPLICIT_IF(...) explicit(__VA_ARGS__)
#    define RXX_IMPLICIT_IF(...) explicit(!(__VA_ARGS__))

#    define RXX_TEMPLATE_CXX11(...)
#    define RXX_ENABLE_IF_CXX11(TYPE, ...) TYPE
#    define RXX_CONSTRAINT_CXX11(...)
#    define RXX_CONSTRAINT_CXX20(...) requires (__VA_ARGS__)
#    define RXX_CONCEPT_CXX20(...) __VA_ARGS__

#  else /* RXX_CXX >= 202002L */

#    define RXX_CONSTEXPR_CXX20
#    define RXX_CONSTEVAL constexpr
#    define RXX_EXPLICIT_IF(...)
#    define RXX_IMPLICIT_IF(...) explicit
#    define RXX_TEMPLATE_CXX11(...) , __VA_ARGS__
#    define RXX_ENABLE_IF_CXX11(TYPE, ...) \
        __DPL enable_if_t<(__VA_ARGS__), TYPE>
#    define RXX_CONSTRAINT_CXX11(...) \
        , __DPL enable_if_t<(__VA_ARGS__), int> = __LINE__
#    define RXX_CONSTRAINT_CXX20(...)
#    define RXX_CONCEPT_CXX20(...) typename
#  endif /* RXX_CXX >= 202002L */

#  if RXX_CXX >= 202302L
#    define RXX_CXX23 1
#    define RXX_CONSTEXPR_CXX23 constexpr
#    define RXX_CONSTEVAL_CXX23 consteval
#    define RXX_STATIC_CXX23 static
#    define RXX_CONST_CALL
#    define RXX_IF_CONSTEVAL(...) if consteval
#  else /* RXX_CXX >= 202302L */
#    define RXX_CONSTEXPR_CXX23
#    define RXX_CONSTEVAL_CXX23 constexpr
#    define RXX_STATIC_CXX23
#    define RXX_CONST_CALL const
#    define RXX_IF_CONSTEVAL(...) \
        if (RXX_CONSTANT_P(       \
                __VA_ARGS__)) /* This requires including 'constant_p.h' */

#  endif /* RXX_CXX >= 202302L */

#  if __cpp_constexpr >= 202306L
#    define RXX_CONSTEXPR_VOIDP constexpr
#  else
#    define RXX_CONSTEXPR_VOIDP
#  endif

#  if RXX_CXX >= 202602L
#    define RXX_CXX26 1
#    define RXX_CONSTEXPR_CXX26 constexpr
#    define RXX_CONSTEVAL_CXX26 consteval
#  else /* RXX_CXX >= 202302L */
#    define RXX_CONSTEXPR_CXX26
#    define RXX_CONSTEVAL_CXX26 constexpr

#  endif /* RXX_CXX >= 202302L */

#else /* ifdef RXX_CXX */

/* Not C++, so only define qualifiers usable on global functions/variables */
#  define RXX_CONSTEXPR_CXX14 const
#  define RXX_INLINE_CXX17
#  define RXX_CONSTEXPR_CXX17 const
#  define RXX_CONSTEXPR_CXX20 const
#  define RXX_CONSTEXPR_CXX23 const
#  define RXX_CONSTEVAL

#endif /* ifdef RXX_CXX */
