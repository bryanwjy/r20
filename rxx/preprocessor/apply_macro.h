/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h"
#include "rxx/configuration/standard.h"
#include "rxx/preprocessor/paste.h"

#define RXX_EVAL1(...) RXX_PASTE(RXX_PASTE(RXX_PASTE(__VA_ARGS__)))
#define RXX_EVAL2(...) RXX_EVAL1(RXX_EVAL1(RXX_EVAL1(__VA_ARGS__)))
#define RXX_EVAL3(...) RXX_EVAL2(RXX_EVAL2(RXX_EVAL2(__VA_ARGS__)))
#define RXX_EVAL4(...) RXX_EVAL3(RXX_EVAL3(RXX_EVAL3(__VA_ARGS__)))
#define RXX_EVAL(...) RXX_EVAL4(RXX_EVAL4(RXX_EVAL4(__VA_ARGS__)))

#if RXX_CXX20 && !RXX_MSVC_PREPROCESSOR

#  define RXX_APPLY_MACRO_PARENTHESIS ()
#  define RXX_APPLY_MACRO_IMPL(F, _0, ...)                                   \
      F(_0)                                                                  \
      __VA_OPT__(RXX_APPLY_MACRO_LOOP RXX_APPLY_MACRO_PARENTHESIS RXX_PASTE( \
          (F, __VA_ARGS__)))
#  define RXX_APPLY_MACRO_LOOP() RXX_APPLY_MACRO_IMPL

#  define RXX_APPLY_MACRO(F, ...) \
      __VA_OPT__(RXX_EVAL(RXX_APPLY_MACRO_IMPL(F, __VA_ARGS__)))

#else

#  define RXX_APPLY_MACRO_END(...)
#  define RXX_APPLY_MACRO_OUT
#  define RXX_APPLY_MACRO_COMMA ,

#  define RXX_APPLY_MACRO_GET_END2() 0, RXX_APPLY_MACRO_END
#  define RXX_APPLY_MACRO_GET_END1(...) RXX_APPLY_MACRO_GET_END2
#  define RXX_APPLY_MACRO_GET_END(...) RXX_APPLY_MACRO_GET_END1
#  define RXX_APPLY_MACRO_NEXT0(test, next, ...) next RXX_APPLY_MACRO_OUT
#  if !RXX_MSVC_PREPROCESSOR
#    define RXX_APPLY_MACRO_NEXT1(test, next) \
        RXX_APPLY_MACRO_NEXT0(test, next, 0)
#    define RXX_APPLY_MACRO_NEXT(test, next) \
        RXX_APPLY_MACRO_NEXT1(RXX_APPLY_MACRO_GET_END test, next)
#    define RXX_APPLY_MACRO0(f, x, peek, ...) \
        f(x) RXX_APPLY_MACRO_NEXT(peek, RXX_APPLY_MACRO1)(f, peek, __VA_ARGS__)
#    define RXX_APPLY_MACRO1(f, x, peek, ...) \
        f(x) RXX_APPLY_MACRO_NEXT(peek, RXX_APPLY_MACRO0)(f, peek, __VA_ARGS__)
#  else /* if !RXX_MSVC_PREPROCESSOR */
#    define RXX_APPLY_MACRO_NEXT1(test, next) \
        RXX_PASTE(RXX_APPLY_MACRO_NEXT0(test, next, 0))
#    define RXX_APPLY_MACRO_NEXT(test, next) \
        RXX_PASTE(RXX_APPLY_MACRO_NEXT1(RXX_APPLY_MACRO_GET_END test, next))
#    define RXX_APPLY_MACRO0(f, x, peek, ...)                        \
        f(x) RXX_PASTE(RXX_APPLY_MACRO_NEXT(peek, RXX_APPLY_MACRO1)( \
            f, peek, __VA_ARGS__))
#    define RXX_APPLY_MACRO1(f, x, peek, ...)                        \
        f(x) RXX_PASTE(RXX_APPLY_MACRO_NEXT(peek, RXX_APPLY_MACRO0)( \
            f, peek, __VA_ARGS__))
#  endif /* if !RXX_MSVC_PREPROCESSOR */

#  define RXX_APPLY_MACRO_LIST_NEXT1(test, next) \
      RXX_APPLY_MACRO_NEXT0(test, RXX_APPLY_MACRO_COMMA next, 0)
#  define RXX_APPLY_MACRO_LIST_NEXT(test, next) \
      RXX_APPLY_MACRO_LIST_NEXT1(RXX_APPLY_MACRO_GET_END test, next)

#  define RXX_APPLY_MACRO_LIST0(f, x, peek, ...)                             \
      f(x) RXX_PASTE(RXX_APPLY_MACRO_LIST_NEXT(peek, RXX_APPLY_MACRO_LIST1)( \
          f, peek, __VA_ARGS__))
#  define RXX_APPLY_MACRO_LIST1(f, x, peek, ...)                             \
      f(x) RXX_PASTE(RXX_APPLY_MACRO_LIST_NEXT(peek, RXX_APPLY_MACRO_LIST0)( \
          f, peek, __VA_ARGS__))

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#  define RXX_APPLY_MACRO(f, ...) \
      RXX_EVAL(RXX_APPLY_MACRO1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#endif /* if RXX_CXX20 */
