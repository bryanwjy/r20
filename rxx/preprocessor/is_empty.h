/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/preprocessor/paste.h"

/**
 * By Jens Gustedt
 * https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
 */
#define __RXX_IE_TRIGGER_PARENTHESIS() ,

/* increase if needed */
#define __RXX_IE_ARG16(                                                        \
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) \
    _15
#define __RXX_IE_HAS_COMMA(...) \
    RXX_PASTE(__RXX_IE_ARG16(   \
        __VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0))

#define __RXX_IE_CONCAT5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4
#define __RXX_IE_EMPTY_CASE_0001 ,
/* clang-format off */
#define __RXX_IS_EMPTY(_0, _1, _2, _3) __RXX_IE_HAS_COMMA(__RXX_IE_CONCAT5(__RXX_IE_EMPTY_CASE_, _0, _1, _2, _3))
#define RXX_IS_EMPTY(...)                                                            \
__RXX_IS_EMPTY(                                                                      \
          /* test if there is just one argument, eventually an empty one */          \
          RXX_PASTE(__RXX_IE_HAS_COMMA(__VA_ARGS__)),                                \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument adds a comma */\
          RXX_PASTE(__RXX_IE_HAS_COMMA(__RXX_IE_TRIGGER_PARENTHESIS __VA_ARGS__)),   \
          /* test if the argument together with a parenthesis adds a comma */        \
          RXX_PASTE(__RXX_IE_HAS_COMMA(__VA_ARGS__ ())),                             \
          /* test if placing it between RXX_TRIGGER_PARENTHESIS and the              \
             parenthesis adds a comma */                                             \
          RXX_PASTE(__RXX_IE_HAS_COMMA(__RXX_IE_TRIGGER_PARENTHESIS __VA_ARGS__ ())) \
          )
/* clang-format on */

#define __RXX_C89_STATIC_ASSERT_3(COND, MSG) \
    typedef char static_assertion[(!!(COND)) * 2 - 1]
#define __RXX_C89_STATIC_ASSERT_2(X, L) __RXX_C89_STATIC_ASSERT_3(X, LINE##L)
#define __RXX_C89_STATIC_ASSERT_1(X, L) __RXX_C89_STATIC_ASSERT_2(X, L)
#define RXX_C89_STATIC_ASSERT(X) __RXX_C89_STATIC_ASSERT_1(X, __LINE__)
RXX_C89_STATIC_ASSERT(RXX_IS_EMPTY());
RXX_C89_STATIC_ASSERT(!RXX_IS_EMPTY(XYZ));
RXX_C89_STATIC_ASSERT(!RXX_IS_EMPTY(A, B, C));

/**
 * Appends the result of IS_EMPTY to a prefix, used to resolve macro overloads
 *
 * If the number of variadic arguments is 0, the prefix will be appended with 1
 * (i.e. result of IS_EMPTY) If the number of variadic arguments is >0, the
 * prefix will be appended with 0 (i.e. result of IS_EMPTY)
 */
#define RXX_APPEND_IS_EMPTY(PREFIX, ...) \
    RXX_PASTE(RXX_CONCAT(PREFIX, RXX_PASTE(RXX_IS_EMPTY(__VA_ARGS__))))
