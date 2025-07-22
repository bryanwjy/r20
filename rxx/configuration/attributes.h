/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/builtins.h"
#include "rxx/configuration/compiler.h"
#include "rxx/configuration/declspec.h"
#include "rxx/configuration/keywords.h"

#if defined(__cplusplus) && defined(__has_cpp_attribute)
#  define RXX_HAS_CPP_ATTRIBUTE(NAME) __has_cpp_attribute(NAME)
#else /* ifdef __has_cpp_attribute */
#  define RXX_HAS_CPP_ATTRIBUTE(NAME) 0
#endif /* ifdef __has_cpp_attribute */

#ifdef __has_attribute
#  define RXX_HAS_GNU_ATTRIBUTE(NAME) __has_attribute(NAME)
#else /* ifdef __has_attribute */
#  define RXX_HAS_GNU_ATTRIBUTE(NAME) 0
#endif /* ifdef __has_attribute */

#if RXX_HAS_CPP_ATTRIBUTE(nodiscard) && RXX_CXX17
#  define RXX_NODISCARD [[nodiscard]]
#  define __RXX_ATTRIBUTE_NODISCARD nodiscard
#  define __RXX_ATTRIBUTE_TYPE_CPP_NODISCARD
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::warn_unused_result)
#  define RXX_NODISCARD [[gnu::warn_unused_result]]
#  define __RXX_ATTRIBUTE_NODISCARD gnu::warn_unused_result
#  define __RXX_ATTRIBUTE_TYPE_CPP_NODISCARD
#elif RXX_HAS_CPP_ATTRIBUTE(clang::warn_unused_result)
#  define RXX_NODISCARD [[clang::warn_unused_result]]
#  define __RXX_ATTRIBUTE_NODISCARD clang::warn_unused_result
#  define __RXX_ATTRIBUTE_TYPE_CPP_NODISCARD
#elif RXX_HAS_GNU_ATTRIBUTE(__warn_unused_result__)
#  define RXX_NODISCARD __attribute__((__warn_unused_result__))
#  define __RXX_ATTRIBUTE_NODISCARD __warn_unused_result__
#  define __RXX_ATTRIBUTE_TYPE_GNU_NODISCARD
#elif RXX_HAS_KEYWORD(_Check_return_)
#  define RXX_NODISCARD _Check_return_
#  define __RXX_ATTRIBUTE_NODISCARD _Check_return_
#  define __RXX_ATTRIBUTE_TYPE_MSVC_EXT_NODISCARD
#else
#  define RXX_NODISCARD
#endif /* RXX_HAS_CPP_ATTRIBUTE(nodiscard) */

#if RXX_HAS_CPP_ATTRIBUTE(noreturn)
#  define RXX_NORETURN [[noreturn]]
#  define __RXX_ATTRIBUTE_NORETURN noreturn
#  define __RXX_ATTRIBUTE_TYPE_CPP_NORETURN
#else
#  define RXX_NORETURN
#endif

#if RXX_HAS_CPP_ATTRIBUTE(msvc::lifetimebound)
#  define RXX_LIFETIMEBOUND [[msvc::lifetimebound]]
#  define __RXX_ATTRIBUTE_LIFETIMEBOUND msvc::lifetimebound
#  define __RXX_ATTRIBUTE_TYPE_CPP_LIFETIMEBOUND
#elif RXX_HAS_CPP_ATTRIBUTE( \
    clang::lifetimebound) /* RXX_HAS_CPP_ATTRIBUTE(msvc::lifetimebound) */
#  define RXX_LIFETIMEBOUND [[clang::lifetimebound]]
#  define __RXX_ATTRIBUTE_LIFETIMEBOUND clang::lifetimebound
#  define __RXX_ATTRIBUTE_TYPE_CPP_LIFETIMEBOUND
#elif RXX_HAS_CPP_ATTRIBUTE( \
    gnu::lifetimebound) /* RXX_HAS_CPP_ATTRIBUTE(msvc::lifetimebound) */
#  define RXX_LIFETIMEBOUND [[gnu::lifetimebound]]
#  define __RXX_ATTRIBUTE_LIFETIMEBOUND gnu::lifetimebound
#  define __RXX_ATTRIBUTE_TYPE_CPP_LIFETIMEBOUND
#elif RXX_HAS_GNU_ATTRIBUTE( \
    __lifetimebound__) /* RXX_HAS_CPP_ATTRIBUTE(msvc::lifetimebound) */
#  define RXX_LIFETIMEBOUND __attribute__((__lifetimebound__))
#  define __RXX_ATTRIBUTE_LIFETIMEBOUND __lifetimebound__
#  define __RXX_ATTRIBUTE_TYPE_GNU_LIFETIMEBOUND
#else
#  define RXX_LIFETIMEBOUND
#endif /* RXX_HAS_CPP_ATTRIBUTE(msvc::lifetimebound) */

#if RXX_HAS_CPP_ATTRIBUTE(gnu::pure)
#  define __RXX_ATTRIBUTE_PURE gnu::pure
#  define __RXX_ATTRIBUTE_TYPE_CPP_PURE
#elif RXX_HAS_GNU_ATTRIBUTE(__pure__) /* RXX_HAS_CPP_ATTRIBUTE(gnu::pure) */
#  define __RXX_ATTRIBUTE_PURE __pure__
#  define __RXX_ATTRIBUTE_TYPE_GNU_PURE
#endif /* RXX_HAS_CPP_ATTRIBUTE(gnu::pure) */

#if RXX_HAS_CPP_ATTRIBUTE(gnu::const)
#  define __RXX_ATTRIBUTE_CONST gnu::const
#  define __RXX_ATTRIBUTE_TYPE_CPP_CONST
#elif RXX_HAS_GNU_ATTRIBUTE(__const__)
#  define __RXX_ATTRIBUTE_CONST __const__
#  define __RXX_ATTRIBUTE_TYPE_GNU_CONST
#elif RXX_HAS_DECLSPEC(noalias)
#  define __RXX_ATTRIBUTE_CONST noalias
#  define __RXX_ATTRIBUTE_TYPE_DECLSPEC_CONST
#endif /* RXX_HAS_CPP_ATTRIBUTE(gnu::const) */

#if RXX_HAS_CPP_ATTRIBUTE(msvc::intrinsic)
#  define __RXX_ATTRIBUTE_INTRINSIC msvc::intrinsic
#  define __RXX_ATTRIBUTE_TYPE_CPP_INTRINSIC
#endif /* RXX_HAS_CPP_ATTRIBUTE(msvc::intrinsic) */

#if RXX_HAS_CPP_ATTRIBUTE(gnu::flatten)
#  define __RXX_ATTRIBUTE_FLATTEN gnu::flatten
#  define __RXX_ATTRIBUTE_TYPE_CPP_FLATTEN
#elif RXX_HAS_CPP_ATTRIBUTE(msvc::flatten)
#  define __RXX_ATTRIBUTE_FLATTEN msvc::flatten
#  define __RXX_ATTRIBUTE_TYPE_CPP_FLATTEN
#elif RXX_HAS_GNU_ATTRIBUTE(__flatten__)
#  define __RXX_ATTRIBUTE_FLATTEN __flatten__
#  define __RXX_ATTRIBUTE_TYPE_GNU_FLATTEN
#endif /* RXX_HAS_CPP_ATTRIBUTE(gnu::const) */

#if RXX_HAS_CPP_ATTRIBUTE(msvc::no_unique_address)
#  define __RXX_ATTRIBUTE_NO_UNIQUE_ADDRESS msvc::no_unique_address
#  define __RXX_ATTRIBUTE_TYPE_CPP_NO_UNIQUE_ADDRESS
#elif RXX_HAS_CPP_ATTRIBUTE(no_unique_address)
#  define __RXX_ATTRIBUTE_NO_UNIQUE_ADDRESS no_unique_address
#  define __RXX_ATTRIBUTE_TYPE_CPP_NO_UNIQUE_ADDRESS
#elif RXX_HAS_GNU_ATTRIBUTE(__no_unique_address__)
#  define __RXX_ATTRIBUTE_NO_UNIQUE_ADDRESS __no_unique_address__
#  define __RXX_ATTRIBUTE_TYPE_GNU_NO_UNIQUE_ADDRESS
#endif /* RXX_HAS_CPP_ATTRIBUTE(msvc::no_unique_address) */

#if RXX_HAS_CPP_ATTRIBUTE(clang::always_inline)
#  define __RXX_ATTRIBUTE_ALWAYS_INLINE clang::always_inline
#  define __RXX_ATTRIBUTE_TYPE_CPP_ALWAYS_INLINE
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::always_inline)
#  define __RXX_ATTRIBUTE_ALWAYS_INLINE gnu::always_inline
#  define __RXX_ATTRIBUTE_TYPE_CPP_ALWAYS_INLINE
#elif RXX_HAS_CPP_ATTRIBUTE(msvc::forceinline)
#  define __RXX_ATTRIBUTE_ALWAYS_INLINE msvc::forceinline
#  define __RXX_ATTRIBUTE_TYPE_CPP_ALWAYS_INLINE
#elif RXX_HAS_GNU_ATTRIBUTE(__always_inline__)
#  define __RXX_ATTRIBUTE_ALWAYS_INLINE __always_inline__
#  define __RXX_ATTRIBUTE_TYPE_GNU_ALWAYS_INLINE
#elif RXX_HAS_MSVC_KEYWORD(__forceinline)
#  define __RXX_ATTRIBUTE_ALWAYS_INLINE __forceinline
#  define __RXX_ATTRIBUTE_TYPE_MSVC_EXT_ALWAYS_INLINE
#endif /* RXX_HAS_CPP_ATTRIBUTE(clang::always_inline) */

#if RXX_HAS_CPP_ATTRIBUTE(msvc::noinline)
#  define __RXX_ATTRIBUTE_NOINLINE msvc::noinline
#  define __RXX_ATTRIBUTE_TYPE_CPP_NOINLINE
#elif RXX_HAS_CPP_ATTRIBUTE(clang::noinline)
#  define __RXX_ATTRIBUTE_NOINLINE clang::noinline
#  define __RXX_ATTRIBUTE_TYPE_CPP_NOINLINE
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::noinline)
#  define __RXX_ATTRIBUTE_NOINLINE gnu::noinline
#  define __RXX_ATTRIBUTE_TYPE_CPP_NOINLINE
#elif RXX_HAS_GNU_ATTRIBUTE(__noinline__)
#  define __RXX_ATTRIBUTE_NOINLINE __noinline__
#  define __RXX_ATTRIBUTE_TYPE_GNU_NOINLINE
#elif RXX_HAS_DECLSPEC(noinline)
#  define __RXX_ATTRIBUTE_NOINLINE noinline
#  define __RXX_ATTRIBUTE_TYPE_DECLSPEC_NOINLINE
#endif /* RXX_HAS_CPP_ATTRIBUTE(clang::noinline) */

#if RXX_HAS_CPP_ATTRIBUTE(clang::malloc)
#  define __RXX_ATTRIBUTE_MALLOC clang::malloc
#  define __RXX_ATTRIBUTE_TYPE_CPP_MALLOC
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::malloc)
#  define __RXX_ATTRIBUTE_MALLOC gnu::malloc
#  define __RXX_ATTRIBUTE_TYPE_CPP_MALLOC
#elif RXX_HAS_GNU_ATTRIBUTE(__malloc__)
#  define __RXX_ATTRIBUTE_MALLOC __malloc__
#  define __RXX_ATTRIBUTE_TYPE_GNU_MALLOC
#elif RXX_HAS_DECLSPEC(allocator) | RXX_HAS_DECLSPEC(restrict)
#  if RXX_HAS_DECLSPEC(allocator) & RXX_HAS_DECLSPEC(restrict)
#    define __RXX_ATTRIBUTE_MALLOC allocator, restrict
#    define __RXX_ATTRIBUTE_TYPE_DECLSPEC_MALLOC
#  elif RXX_HAS_DECLSPEC(allocator)
#    define __RXX_ATTRIBUTE_MALLOC allocator
#    define __RXX_ATTRIBUTE_TYPE_DECLSPEC_MALLOC
#  elif RXX_HAS_DECLSPEC(restrict)
#    define __RXX_ATTRIBUTE_MALLOC restrict
#    define __RXX_ATTRIBUTE_TYPE_DECLSPEC_MALLOC
#  endif
#endif /* RXX_HAS_CPP_ATTRIBUTE(clang::malloc) */

#if RXX_COMPILER_GCC
#  if RXX_HAS_CPP_ATTRIBUTE(gnu::malloc)
#    define __RXX_ATTRIBUTE_DEALLOCATED_BY gnu::malloc
#    define __RXX_ATTRIBUTE_TYPE_CPP_DEALLOCATED_BY(FUNC, ...)
#  elif RXX_HAS_GNU_ATTRIBUTE(__malloc__)
#    define __RXX_ATTRIBUTE_DEALLOCATED_BY __malloc__
#    define __RXX_ATTRIBUTE_TYPE_GNU_DEALLOCATED_BY(FUNC, ...)
#  endif /* RXX_HAS_CPP_ATTRIBUTE(clang::malloc) */
#endif

#if RXX_HAS_CPP_ATTRIBUTE(gnu::visibility)
#  define __RXX_ATTRIBUTE_VISIBILITY gnu::visibility
#  define __RXX_ATTRIBUTE_TYPE_CPP_VISIBILITY(VISIBILITY)
#elif RXX_HAS_GNU_ATTRIBUTE(__visibility__)
#  define __RXX_ATTRIBUTE_VISIBILITY __visibility__
#  define __RXX_ATTRIBUTE_TYPE_GNU_VISIBILITY(VISIBILITY)
#endif

#if RXX_HAS_CPP_ATTRIBUTE(type_visibility) && RXX_CXX17
#  define __RXX_ATTRIBUTE_TYPE_VISIBILITY type_visibility
#  define __RXX_ATTRIBUTE_TYPE_CPP_TYPE_VISIBILITY(VISIBILITY)
#elif RXX_HAS_CPP_ATTRIBUTE(clang::type_visibility) && RXX_CXX17
#  define __RXX_ATTRIBUTE_TYPE_VISIBILITY clang::type_visibility
#  define __RXX_ATTRIBUTE_TYPE_CPP_TYPE_VISIBILITY(VISIBILITY)
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::type_visibility) && RXX_CXX17
#  define __RXX_ATTRIBUTE_TYPE_VISIBILITY gnu::type_visibility
#  define __RXX_ATTRIBUTE_TYPE_CPP_TYPE_VISIBILITY(VISIBILITY)
#elif RXX_HAS_GNU_ATTRIBUTE(__type_visibility__)
#  define __RXX_ATTRIBUTE_TYPE_VISIBILITY __type_visibility__
#  define __RXX_ATTRIBUTE_TYPE_GNU_TYPE_VISIBILITY(VISIBILITY)
#endif

#if RXX_HAS_CPP_ATTRIBUTE(gnu::dllimport)
#  define __RXX_ATTRIBUTE_DLLIMPORT gnu::dllimport
#  define __RXX_ATTRIBUTE_TYPE_CPP_DLLIMPORT
#elif RXX_HAS_GNU_ATTRIBUTE(__dllimport__)
#  define __RXX_ATTRIBUTE_DLLIMPORT __dllimport__
#  define __RXX_ATTRIBUTE_TYPE_CPP_DLLIMPORT
#elif RXX_HAS_DECLSPEC(dllimport)
#  define __RXX_ATTRIBUTE_DLLIMPORT dllimport
#  define __RXX_ATTRIBUTE_TYPE_DECLSPEC_DLLIMPORT
#endif

#if RXX_HAS_CPP_ATTRIBUTE(gnu::dllexport)
#  define __RXX_ATTRIBUTE_DLLEXPORT gnu::dllexport
#  define __RXX_ATTRIBUTE_TYPE_CPP_DLLEXPORT
#elif RXX_HAS_GNU_ATTRIBUTE(__dllexport__)
#  define __RXX_ATTRIBUTE_DLLEXPORT __dllexport__
#  define __RXX_ATTRIBUTE_TYPE_CPP_DLLEXPORT
#elif RXX_HAS_DECLSPEC(dllexport)
#  define __RXX_ATTRIBUTE_DLLIMPORT dllexport
#  define __RXX_ATTRIBUTE_TYPE_DECLSPEC_DLLIMPORT
#endif

#if RXX_HAS_CPP_ATTRIBUTE(clang::exclude_from_explicit_instantiation)
#  define __RXX_ATTRIBUTE_EXCLUDE_FROM_EXPLICIT_INSTANTIATION \
      clang::exclude_from_explicit_instantiation
#  define __RXX_ATTRIBUTE_TYPE_CPP_EXCLUDE_FROM_EXPLICIT_INSTANTIATION
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::exclude_from_explicit_instantiation)
#  define __RXX_ATTRIBUTE_EXCLUDE_FROM_EXPLICIT_INSTANTIATION \
      gnu::exclude_from_explicit_instantiation
#  define __RXX_ATTRIBUTE_TYPE_CPP_EXCLUDE_FROM_EXPLICIT_INSTANTIATION
#elif RXX_HAS_GNU_ATTRIBUTE(__exclude_from_explicit_instantiation__)
#  define __RXX_ATTRIBUTE_EXCLUDE_FROM_EXPLICIT_INSTANTIATION \
      __exclude_from_explicit_instantiation__
#  define __RXX_ATTRIBUTE_TYPE_GNU_EXCLUDE_FROM_EXPLICIT_INSTANTIATION
#endif

#if RXX_HAS_CPP_ATTRIBUTE(clang::abi_tag)
#  define __RXX_ATTRIBUTE_ABI_TAG clang::abi_tag
#  define __RXX_ATTRIBUTE_TYPE_CPP_ABI_TAG(STR)
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::abi_tag)
#  define __RXX_ATTRIBUTE_ABI_TAG gnu::abi_tag
#  define __RXX_ATTRIBUTE_TYPE_CPP_ABI_TAG(STR)
#elif RXX_HAS_GNU_ATTRIBUTE(__abi_tag__)
#  define __RXX_ATTRIBUTE_ABI_TAG __abi_tag__
#  define __RXX_ATTRIBUTE_TYPE_GNU_ABI_TAG(STR)
#endif

#if RXX_HAS_CPP_ATTRIBUTE(clang::reinitializes)
#  define __RXX_ATTRIBUTE_REINITIALIZES clang::reinitializes
#  define __RXX_ATTRIBUTE_TYPE_CPP_REINITIALIZES
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::reinitializes)
#  define __RXX_ATTRIBUTE_REINITIALIZES gnu::reinitializes
#  define __RXX_ATTRIBUTE_TYPE_CPP_REINITIALIZES
#elif RXX_HAS_GNU_ATTRIBUTE(__reinitializes__)
#  define __RXX_ATTRIBUTE_REINITIALIZES __reinitializes__
#  define __RXX_ATTRIBUTE_TYPE_GNU_REINITIALIZES
#endif /* RXX_HAS_CPP_ATTRIBUTE(clang::reinitializes) */

#if RXX_HAS_CPP_ATTRIBUTE(maybe_unused) && RXX_CXX17
#  define __RXX_ATTRIBUTE_MAYBE_UNUSED maybe_unused
#  define __RXX_ATTRIBUTE_TYPE_CPP_MAYBE_UNUSED
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::unused)
#  define __RXX_ATTRIBUTE_MAYBE_UNUSED gnu::unused
#  define __RXX_ATTRIBUTE_TYPE_CPP_MAYBE_UNUSED
#elif RXX_HAS_GNU_ATTRIBUTE(__unused__)
#  define __RXX_ATTRIBUTE_MAYBE_UNUSED __unused__
#  define __RXX_ATTRIBUTE_TYPE_GNU_MAYBE_UNUSED
#endif /* RXX_HAS_CPP_ATTRIBUTE(maybe_unused) */

#if RXX_HAS_CPP_ATTRIBUTE(fallthrough) && RXX_CXX17
#  define RXX_FALLTHROUGH [[fallthrough]]
#  define __RXX_ATTRIBUTE_FALLTHROUGH fallthrough
#  define __RXX_ATTRIBUTE_TYPE_CPP_FALLTHROUGH
#elif RXX_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#  define RXX_FALLTHROUGH [[gnu::fallthrough]]
#  define __RXX_ATTRIBUTE_FALLTHROUGH gnu::fallthrough
#  define __RXX_ATTRIBUTE_TYPE_CPP_FALLTHROUGH
#elif RXX_HAS_GNU_ATTRIBUTE(__fallthrough__)
#  define RXX_FALLTHROUGH __attribute__((__fallthrough__))
#  define __RXX_ATTRIBUTE_FALLTHROUGH __fallthrough__
#  define __RXX_ATTRIBUTE_TYPE_GNU_FALLTHROUGH
#else
#  define RXX_FALLTHROUGH
#endif

#if RXX_HAS_CPP_ATTRIBUTE(clang::noescape)
#  define __RXX_ATTRIBUTE_NOESCAPE clang::noescape
#  define __RXX_ATTRIBUTE_TYPE_CPP_NOESCAPE
#endif

#if RXX_HAS_GNU_ATTRIBUTE(__optimize__)
#  define __RXX_ATTRIBUTE_OPTIMIZE __optimize__
#  define __RXX_ATTRIBUTE_TYPE_GNU_OPTIMIZE(STR)
#endif

#if RXX_HAS_CPP_ATTRIBUTE(clang::nodebug)
#  define RXX_NODEBUG [[clang::nodebug]]
#  define __RXX_ATTRIBUTE_NODEBUG clang::nodebug
#  define __RXX_ATTRIBUTE_TYPE_CPP_NODEBUG
#else
#  define RXX_NODEBUG
#endif

#if RXX_HAS_CPP_ATTRIBUTE(likely) && RXX_CXX20
#  define RXX_LIKELY [[likely]]
#  define __RXX_ATTRIBUTE_LIKELY likely
#  define __RXX_ATTRIBUTE_TYPE_CPP_LIKELY
#elif RXX_HAS_CPP_ATTRIBUTE(clang::likely)
#  define RXX_LIKELY [[clang::likely]]
#  define __RXX_ATTRIBUTE_LIKELY clang::likely
#  define __RXX_ATTRIBUTE_TYPE_CPP_LIKELY
#else
#  define RXX_LIKELY
#endif

#if RXX_HAS_CPP_ATTRIBUTE(unlikely) && RXX_CXX20
#  define RXX_UNLIKELY [[unlikely]]
#  define __RXX_ATTRIBUTE_UNLIKELY unlikely
#  define __RXX_ATTRIBUTE_TYPE_CPP_UNLIKELY
#elif RXX_HAS_CPP_ATTRIBUTE(clang::unlikely)
#  define RXX_UNLIKELY [[clang::unlikely]]
#  define __RXX_ATTRIBUTE_UNLIKELY clang::unlikely
#  define __RXX_ATTRIBUTE_TYPE_CPP_UNLIKELY
#else
#  define RXX_UNLIKELY
#endif

#if RXX_HAS_CPP_ATTRIBUTE(clang::vectorcall)
#  define __RXX_ATTRIBUTE_VECTORCALL clang::vectorcall
#  define __RXX_ATTRIBUTE_TYPE_CPP_VECTORCALL
#elif RXX_HAS_KEYWORD(__vectorcall)
#  define RXX_NODISCARD __vectorcall
#  define __RXX_ATTRIBUTE_VECTORCALL __vectorcall
#  define __RXX_ATTRIBUTE_TYPE_MSVC_EXT_VECTORCALL
#endif /* RXX_HAS_CPP_ATTRIBUTE(clang::vectorcall) */
