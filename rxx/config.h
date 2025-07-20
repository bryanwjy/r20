// Copyright 2025 Bryan Wong

#pragma once

// IWYU pragma: always_keep

// IWYU pragma: begin_exports
#include "rxx/configuration/abi.h"              // IWYU pragma: keep
#include "rxx/configuration/allocation.h"       // IWYU pragma: keep
#include "rxx/configuration/architecture.h"     // IWYU pragma: keep
#include "rxx/configuration/attributes.h"       // IWYU pragma: keep
#include "rxx/configuration/builtins.h"         // IWYU pragma: keep
#include "rxx/configuration/compiler.h"         // IWYU pragma: keep
#include "rxx/configuration/compiler_barrier.h" // IWYU pragma: keep
#include "rxx/configuration/exceptions.h"       // IWYU pragma: keep
#include "rxx/configuration/keywords.h"         // IWYU pragma: keep
#include "rxx/configuration/memcmp.h"           // IWYU pragma: keep
#include "rxx/configuration/memcpy.h"           // IWYU pragma: keep
#include "rxx/configuration/modules.h"          // IWYU pragma: keep
#include "rxx/configuration/namespace.h"        // IWYU pragma: keep
#include "rxx/configuration/pragma.h"           // IWYU pragma: keep
#include "rxx/configuration/simd.h"             // IWYU pragma: keep
#include "rxx/configuration/standard.h"         // IWYU pragma: keep
#include "rxx/configuration/stl.h"              // IWYU pragma: keep
#include "rxx/configuration/target.h"           // IWYU pragma: keep
#include "rxx/configuration/types.h"            // IWYU pragma: keep
#include "rxx/preprocessor/attribute_list.h"    // IWYU pragma: keep
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
