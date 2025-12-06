/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/attributes.h"
#include "rxx/configuration/compiler.h" // IWYU pragma: keep
#include "rxx/configuration/exceptions.h"
#include "rxx/configuration/standard.h"
#include "rxx/configuration/target.h"       // IWYU pragma: keep
#include "rxx/preprocessor/concatenation.h" // IWYU pragma: keep
#include "rxx/preprocessor/to_string.h"

#define RXX_ABI_MAJOR 0
#define RXX_ABI_MINOR 1

#ifndef RXX_DEFAULT_ABI_TAG
#  define RXX_DEFAULT_ABI_TAG \
      RXX_TO_STRING(RXX)      \
      RXX_TO_STRING(RXX_ABI_MAJOR) RXX_TO_STRING(_) RXX_TO_STRING(RXX_ABI_MINOR)
#  if RXX_CXX
/* Ensure that the default tag is a string */
static_assert(true, RXX_DEFAULT_ABI_TAG);
#  endif
#endif

#if RXX_COMPILER_MSVC

#  if !RXX_HAS_DECLSPEC(dllexport) || !RXX_HAS_DECLSPEC(dllimport)
#    error "Unrecognized compiler"
#  endif

#  ifdef RXX_BUILDING_LIBRARY
#    define __RXX_ATTRIBUTE__ABI_PUBLIC (DLLEXPORT)
#    define __RXX_ATTRIBUTE_TYPE_AGGREGATE__ABI_PUBLIC
#    define __RXX_ABI_PUBLIC __declspec(dllexport)
#  else
#    define __RXX_ATTRIBUTE__ABI_PUBLIC (DLLIMPORT)
#    define __RXX_ATTRIBUTE_TYPE_AGGREGATE__ABI_PUBLIC
#    define __RXX_ABI_PUBLIC __declspec(dllimport)
#  endif
#  define __RXX_ABI_PRIVATE
#  define __RXX_ATTRIBUTE__ABI_PRIVATE
#  ifdef __RXX_ATTRIBUTE_TYPE_AGGREGATE__ABI_PRIVATE
#    error '__RXX_ATTRIBUTE_TYPE_AGGREGATE__ABI_PRIVATE' cannot be defined
#  endif
#  define __RXX_PUBLIC_TEMPLATE_DATA
#else

#  if !RXX_HAS_GNU_ATTRIBUTE(__visibility__)
#    error "Unrecognized compiler"
#  endif

#  define __RXX_ATTRIBUTE__ABI_PUBLIC (VISIBILITY("default"))
#  define __RXX_ATTRIBUTE_TYPE_AGGREGATE__ABI_PUBLIC
#  define __RXX_ATTRIBUTE__ABI_PRIVATE (VISIBILITY("hidden"))
#  define __RXX_ATTRIBUTE_TYPE_AGGREGATE__ABI_PRIVATE

#  define __RXX_ABI_PUBLIC __attribute__((__visibility__("default")))
#  define __RXX_ABI_PRIVATE __attribute__((__visibility__("hidden")))
#  define __RXX_PUBLIC_TEMPLATE_DATA __attribute__((__visibility__("default")))

#endif

#if !RXX_HAS_GNU_ATTRIBUTE(TYPE_VISIBILITY)
/* For GNU compilers that don't have type visibility we must keep the templates
 * visible */
#  define __RXX_PUBLIC_TEMPLATE __attribute__((__visibility__("default")))
#  define __RXX_ATTRIBUTE__PUBLIC_TEMPLATE (VISIBILITY("default"))
#  define __RXX_ATTRIBUTE_TYPE_AGGREGATE__PUBLIC_TEMPLATE
#  define __RXX_PUBLIC_TYPE_VISIBILITY
#  define __RXX_PRIVATE_TYPE_VISIBILITY
#else
#  define __RXX_PUBLIC_TYPE_VISIBILITY \
      __attribute__((__type_visibility__("default")))
#  define __RXX_PRIVATE_TYPE_VISIBILITY \
      __attribute__((__type_visibility__("hidden")))
#  define __RXX_PUBLIC_TEMPLATE
#  define __RXX_ATTRIBUTE__PUBLIC_TEMPLATE
#  ifdef __RXX_ATTRIBUTE_TYPE_AGGREGATE__PUBLIC_TEMPLATE
#    error '__RXX_ATTRIBUTE_TYPE_AGGREGATE__PUBLIC_TEMPLATE' cannot be defined
#  endif
#endif

#define __RXX_ABI_TAG_DELIMITER "_"
#define __RXX_ODR_SIGNATURE_1(_0, _1, _2) _0##_1##_2
#define __RXX_ODR_SIGNATURE_0(_0, _1, _2) _0 _1 _2
#define __RXX_ODR_SIGNATURE                                     \
    RXX_DEFAULT_ABI_TAG "_" RXX_TO_STRING(__RXX_HARDENING_MODE) \
        RXX_TO_STRING(__RXX_ABI_EXCEPTION_TAG)

#if RXX_HAS_GNU_ATTRIBUTE(__exclude_from_explicit_instantiation__)
#  define __RXX_EXCLUDE_FROM_EXPLICIT_INSTANTIATION \
      __attribute__((__exclude_from_explicit_instantiation__))
#else
#  define __RXX_EXCLUDE_FROM_EXPLICIT_INSTANTIATION
#endif

#if RXX_HAS_GNU_ATTRIBUTE(__abi_tag__)
#  define __RXX_ABI_TAG(TAG) __attribute__((__abi_tag__(TAG)))
#else
#  define __RXX_ABI_TAG(TAG)
#endif

#define __RXX_HIDE_FROM_ABI                                                    \
    __RXX_ABI_PRIVATE __RXX_EXCLUDE_FROM_EXPLICIT_INSTANTIATION __RXX_ABI_TAG( \
        __RXX_ODR_SIGNATURE)
#define __RXX_ATTRIBUTE__HIDE_FROM_ABI                                \
    __RXX_ATTRIBUTE__ABI_PRIVATE(EXCLUDE_FROM_EXPLICIT_INSTANTIATION) \
    (ABI_TAG(__RXX_ODR_SIGNATURE))
#define __RXX_ATTRIBUTE_TYPE_AGGREGATE__HIDE_FROM_ABI

/* virtual functions must be linked to the same symbol */
#define __RXX_HIDE_FROM_ABI_VIRTUAL \
    __RXX_ABI_PRIVATE __RXX_EXCLUDE_FROM_EXPLICIT_INSTANTIATION
#define __RXX_ATTRIBUTE__HIDE_FROM_ABI_VIRTUAL \
    __RXX_ATTRIBUTE__ABI_PRIVATE(EXCLUDE_FROM_EXPLICIT_INSTANTIATION)
#define __RXX_ATTRIBUTE_TYPE_AGGREGATE__HIDE_FROM_ABI_VIRTUAL
#define __RXX_HIDE_FROM_ABI_UNTAGGED \
    __RXX_ABI_PRIVATE __RXX_EXCLUDE_FROM_EXPLICIT_INSTANTIATION
#define __RXX_ATTRIBUTE__HIDE_FROM_ABI_UNTAGGED \
    __RXX_ATTRIBUTE__ABI_PRIVATE(EXCLUDE_FROM_EXPLICIT_INSTANTIATION)
#define __RXX_ATTRIBUTE_TYPE_AGGREGATE__HIDE_FROM_ABI_UNTAGGED

#if RXX_WITH_EXCEPTIONS
#  define __RXX_ABI_EXCEPTION_TAG e
#else
#  define __RXX_ABI_EXCEPTION_TAG n
#endif

#ifndef __RXX_ABI_EXCEPTION_TAG
#  error Undefined exception tag
#endif

/* TODO */
#define __RXX_HARDENING_MODE n

#if RXX_CXX
#  define RXX_EXTERN_C extern "C"
#  define RXX_EXTERN_C_BEGIN RXX_EXTERN_C {
#  define RXX_EXTERN_C_END }
#else
#  define RXX_EXTERN_C
#  define RXX_EXTERN_C_BEGIN
#  define RXX_EXTERN_C_END
#endif

#if RXX_COMPILER_MSVC && RXX_CXX20
#  define RXX_EXTERN_CXX extern "C++"
#  define RXX_EXTERN_CXX_BEGIN RXX_EXTERN_CXX {
#  define RXX_EXTERN_CXX_END }
#else
#  define RXX_EXTERN_CXX
#  define RXX_EXTERN_CXX_BEGIN
#  define RXX_EXTERN_CXX_END
#endif
