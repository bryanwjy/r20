// Copyright 2025 Bryan Wong

#pragma once

// IWYU pragma: always_keep

// IWYU pragma: begin_exports
#include "rxx/configuration/abi.h"
#include "rxx/configuration/allocation.h"
#include "rxx/configuration/architecture.h"
#include "rxx/configuration/attributes.h"
#include "rxx/configuration/builtins.h"
#include "rxx/configuration/compiler.h"
#include "rxx/configuration/compiler_barrier.h"
#include "rxx/configuration/exceptions.h"
#include "rxx/configuration/keywords.h"
#include "rxx/configuration/memcmp.h"
#include "rxx/configuration/memcpy.h"
#include "rxx/configuration/modules.h"
#include "rxx/configuration/namespace.h"
#include "rxx/configuration/pragma.h"
#include "rxx/configuration/simd.h"
#include "rxx/configuration/standard.h"
#include "rxx/configuration/stl.h"
#include "rxx/configuration/target.h"
#include "rxx/configuration/types.h"
#include "rxx/preprocessor/attribute_list.h"
// IWYU pragma: end_exports

#if !RXX_CXX20
#  error "C++20 requirement not met"
#endif

#ifndef RXX_NS
#  define RXX_NS rxx
#endif

#define __RXX ::RXX_NS::

#ifndef RXX_ENABLE_STD_INTEROP
#  define RXX_ENABLE_STD_INTEROP 0
#endif

#define RXX_DEFAULT_NAMESPACE_BEGIN RXX_NAMESPACE_BEGIN(RXX_NS)

#define RXX_DEFAULT_NAMESPACE_END RXX_NAMESPACE_END(RXX_NS)

#define RXX_PUBLIC_DEFAULT_NAMESPACE_BEGIN \
    RXX_EXPORT RXX_DEFAULT_NAMESPACE_BEGIN

#define RXX_PUBLIC_DEFAULT_NAMESPACE_END RXX_DEFAULT_NAMESPACE_END

RXX_PUBLIC_DEFAULT_NAMESPACE_BEGIN

using ptrdiff_t = decltype((char*)0 - (char*)0);
using size_t = decltype(sizeof(0));

#if RXX_SUPPORTS_FLOAT16
using float16 = decltype(0.0f16);
#endif
#if RXX_SUPPORTS_FLOAT32
using float32 = decltype(0.0f32);
#endif
#if RXX_SUPPORTS_FLOAT64
using float64 = decltype(0.0f64);
#endif
#if RXX_SUPPORTS_FLOAT128
using float128 = decltype(0.0f128);
#endif
#if RXX_SUPPORTS_BFLOAT16
using bfloat16 = decltype(0.0bf16);
#endif

namespace ranges {
namespace views {}
} // namespace ranges

namespace views = ranges::views;

#if RXX_WITH_EXCEPTIONS

#  define RXX_THROW(...) throw(__VA_ARGS__)
#  define RXX_RETHROW(...) throw
#  define RXX_TRY try
#  define RXX_CATCH(...) catch (__VA_ARGS__)
#else
#  include <cassert>

namespace details {
template <typename>
constexpr bool always_false() {
    return false;
}
constexpr bool catch_statement(auto&&) {
    return false;
}
} // namespace details

#  define RXX_THROW(...)                                            \
      assert(__RXX details::always_false<decltype(__VA_ARGS__)>()); \
      RXX_BUILTIN_unreachable()
#  define RXX_RETHROW(...)        \
      assert(false, __VA_ARGS__); \
      RXX_BUILTIN_unreachable()

#  define RXX_TRY if constexpr (true)

#  define RXX_CATCH(...)                                 \
      else if constexpr (__RXX details::catch_statement( \
                             [](__VA_ARGS__) -> void {}))

#endif

RXX_PUBLIC_DEFAULT_NAMESPACE_END
