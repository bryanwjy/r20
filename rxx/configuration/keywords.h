/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/configuration/compiler.h"

#if RXX_COMPILER_MSVC
#  define RXX_HAS_KEYWORD(NAME)                            \
      RXX_IS_EMPTY(RXX_CONCAT(__RXX_KEYWORD_MSVC, NAME)) | \
          RXX_IS_EMPTY(RXX_CONCAT(__RXX_SAL_ANNOTATION, NAME))

#  define RXX_HAS_SAL_ANNOTATION(NAME) \
      RXX_IS_EMPTY(RXX_CONCAT(__RXX_SAL_ANNOTATION, NAME))

#  define RXX_CDECL __cdecl
#  define RXX_FASTCALL __fastcall
#  define RXX_STDCALL __stdcall
#  define RXX_RESTRICT __restrict
#  define RXX_UNALIGNED __unaligned
#  define RXX_VECTORCALL __vectorcall
#  define RXX_W64 __w64
#  define RXX_FORCEINLINE __forceinline

#  define __RXX_KEYWORD_MSVC__declspec
#  define __RXX_KEYWORD_MSVC__cdecl
#  define __RXX_KEYWORD_MSVC__fastcall
#  define __RXX_KEYWORD_MSVC__stdcall
#  define __RXX_KEYWORD_MSVC__restrict
#  define __RXX_KEYWORD_MSVC__vectorcall
#  define __RXX_KEYWORD_MSVC__unaligned
#  define __RXX_KEYWORD_MSVC__w64
#  define __RXX_KEYWORD_MSVC__forceinline

/* TODO wtf */
#  define __RXX_SAL_ANNOTATION_Check_return_

#elif RXX_COMPILER_GNU_BASED
#  define RXX_HAS_KEYWORD(NAME) \
      RXX_IS_EMPTY(RXX_CONCAT(__RXX_GNU_KEYWORD, NAME))

#  define __RXX_KEYWORD_GNU__restrict__

#  define RXX_CDECL
#  define RXX_FASTCALL
#  define RXX_STDCALL
#  define RXX_RESTRICT __restrict__
#  define RXX_UNALIGNED
#  define RXX_VECTORCALL
#  define RXX_W64
#  define RXX_FORCEINLINE
#else

#  define RXX_HAS_KEYWORD(NAME) 0
#  define RXX_CDECL
#  define RXX_FASTCALL
#  define RXX_STDCALL
#  define RXX_RESTRICT
#  define RXX_UNALIGNED
#  define RXX_VECTORCALL
#  define RXX_W64
#  define RXX_FORCEINLINE
#endif
