/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/preprocessor/concatenation.h"

#ifdef __apple_build_version__

#  define __RXX_APPLE_CLANG_MAJOR \
      RXX_CONCAT(__RXX_APPLE_TO_CLANG_MAJOR_, __apple_build_version__)
#  define __RXX_APPLE_CLANG_MINOR \
      RXX_CONCAT(__RXX_APPLE_TO_CLANG_MINOR_, __apple_build_version__)
#  define __RXX_APPLE_CLANG_PATCH \
      RXX_CONCAT(__RXX_APPLE_TO_CLANG_PATCH_, __apple_build_version__)

/* Define new version when needed, newest at the top */

#  define __RXX_APPLE_TO_CLANG_MAJOR_16000023 17
#  define __RXX_APPLE_TO_CLANG_MINOR_16000023 0
#  define __RXX_APPLE_TO_CLANG_PATCH_16000023 6

#  define __RXX_APPLE_TO_CLANG_MAJOR_15000309 16
#  define __RXX_APPLE_TO_CLANG_MINOR_15000309 0
#  define __RXX_APPLE_TO_CLANG_PATCH_15000309 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_15000100 16
#  define __RXX_APPLE_TO_CLANG_MINOR_15000100 0
#  define __RXX_APPLE_TO_CLANG_PATCH_15000100 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_15000040 16
#  define __RXX_APPLE_TO_CLANG_MINOR_15000040 0
#  define __RXX_APPLE_TO_CLANG_PATCH_15000040 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_14030022 15
#  define __RXX_APPLE_TO_CLANG_MINOR_14030022 0
#  define __RXX_APPLE_TO_CLANG_PATCH_14030022 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_14030022 15
#  define __RXX_APPLE_TO_CLANG_MINOR_14030022 0
#  define __RXX_APPLE_TO_CLANG_PATCH_14030022 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_14000029 14
#  define __RXX_APPLE_TO_CLANG_MINOR_14000029 0
#  define __RXX_APPLE_TO_CLANG_PATCH_14000029 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_13160021 13
#  define __RXX_APPLE_TO_CLANG_MINOR_13160021 0
#  define __RXX_APPLE_TO_CLANG_PATCH_13160021 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_13000029 12
#  define __RXX_APPLE_TO_CLANG_MINOR_13000029 0
#  define __RXX_APPLE_TO_CLANG_PATCH_13000029 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_12050022 11
#  define __RXX_APPLE_TO_CLANG_MINOR_12050022 1
#  define __RXX_APPLE_TO_CLANG_PATCH_12050022 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_12000032 10
#  define __RXX_APPLE_TO_CLANG_MINOR_12000032 0
#  define __RXX_APPLE_TO_CLANG_PATCH_12000032 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_11030032 9
#  define __RXX_APPLE_TO_CLANG_MINOR_11030032 0
#  define __RXX_APPLE_TO_CLANG_PATCH_11030032 0

#  define __RXX_APPLE_TO_CLANG_MAJOR_11000033 8
#  define __RXX_APPLE_TO_CLANG_MINOR_11000033 0
#  define __RXX_APPLE_TO_CLANG_PATCH_11000033 0

/* Xcode Version < 11.0 below if needed */

#endif
