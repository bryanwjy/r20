/* Copyright 2023-2025 Bryan Wong */

#include "rxx/configuration/compiler.h"
#include "rxx/configuration/target.h"

#if RXX_COMPILER_MSVC && !RXX_COMPILER_MSVC_AT_LEAST(1925)
#  define RXX_PRAGMA(ARG) __pragma(ARG)
#else
#  define RXX_PRAGMA(ARG) _Pragma(#ARG)
#endif

#if RXX_TARGET_APPLE
/* Apple supposedly silently ignores the region token */
#  define RXX_SUPPORTS_PRAGMA_REGION
#  define RXX_BEGIN_REGION(LABEL) RXX_PRAGMA(region LABEL)
#  define RXX_END_REGION(LABEL) RXX_PRAGMA(endregion LABEL)
#elif RXX_COMPILER_CLANG_AT_LEAST(6, 0, 0)
#  define RXX_SUPPORTS_PRAGMA_REGION
#  define RXX_BEGIN_REGION(LABEL) RXX_PRAGMA(region LABEL)
#  define RXX_END_REGION(LABEL) RXX_PRAGMA(endregion LABEL)
#elif RXX_COMPILER_GCC_AT_LEAST(13, 0, 0)
#  define RXX_SUPPORTS_PRAGMA_REGION
#  define RXX_BEGIN_REGION(LABEL) RXX_PRAGMA(region LABEL)
#  define RXX_END_REGION(LABEL) RXX_PRAGMA(endregion LABEL)
#elif RXX_COMPILER_MSVC
#  define RXX_SUPPORTS_PRAGMA_REGION
#  define RXX_BEGIN_REGION(LABEL) RXX_PRAGMA(region LABEL)
#  define RXX_END_REGION(LABEL) RXX_PRAGMA(endregion LABEL)
#else
#  define RXX_BEGIN_REGION(_)
#  define RXX_END_REGION(_)
#endif

#if RXX_COMPILER_CLANG || RXX_COMPILER_ICX

#  define RXX_PRAGMA_WARN(x) RXX_PRAGMA(message #x)

#elif RXX_COMPILER_GCC

#  if RXX_COMPILER_GCC_AT_LEAST(9, 1, 0)
#    define RXX_PRAGMA_WARN(x) RXX_PRAGMA(GCC warning #x)
#  else /* RXX_COMPILER_GCC_AT_LEAST(9,1,0) */
#    define RXX_PRAGMA_WARN(x) RXX_PRAGMA(GCC message #x)
#  endif

#elif RXX_COMPILER_MSVC

#  define RXX_WARN_PRIVATE_STR2(x) #x
#  define RXX_WARN_PRIVATE_STR1(x) RXX_WARN_PRIVATE_STR2(x)
#  define RXX_WARN_PRIVATE_LOCATION \
      __FILE__ ":" RXX_WARN_PRIVATE_STR1(__LINE__) ":"
#  define RXX_PRAGMA_WARN(x) \
      RXX_PRAGMA(message(RXX_WARN_PRIVATE_LOCATION " warning: " #x))

#else

#  define RXX_PRAGMA_WARN(_)

#endif
/* TODO: Find out ICC's warning pragma */

#if RXX_COMPILER_MSVC | RXX_COMPILER_ICC
#  define RXX_DISABLE_WARNING_PUSH() RXX_PRAGMA(warning(push))
#  define RXX_DISABLE_WARNING_POP() RXX_PRAGMA(warning(pop))
#  define RXX_DISABLE_WARNING(warningNumber) \
      RXX_PRAGMA(warning(disable : warningNumber))
#elif RXX_COMPILER_CLANG | RXX_COMPILER_ICX
#  define RXX_DISABLE_WARNING_PUSH() RXX_PRAGMA(clang diagnostic push)
#  define RXX_DISABLE_WARNING_POP() RXX_PRAGMA(clang diagnostic pop)
#  define RXX_DISABLE_WARNING(warningName) \
      RXX_PRAGMA(clang diagnostic ignored warningName)
#elif RXX_COMPILER_GCC
#  define RXX_DISABLE_WARNING_PUSH() RXX_PRAGMA(GCC diagnostic push)
#  define RXX_DISABLE_WARNING_POP() RXX_PRAGMA(GCC diagnostic pop)
#  define RXX_DISABLE_WARNING(warningName) \
      RXX_PRAGMA(GCC diagnostic ignored warningName)
#else
#  define RXX_DISABLE_WARNING_PUSH()
#  define RXX_DISABLE_WARNING_POP()
#  define RXX_DISABLE_WARNING(warningName)
#endif
