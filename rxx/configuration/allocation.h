/* Copyright 2023-2025 Bryan Wong */

#pragma once

#ifndef __cpp_aligned_new
#  define RXX_SUPPORTS_ALIGNED_ALLOCATION 0
#elif __cpp_aligned_new >= 201606L
#  define RXX_SUPPORTS_ALIGNED_ALLOCATION 1
#else
#  define RXX_SUPPORTS_ALIGNED_ALLOCATION 0
#endif

#ifndef __cpp_sized_deallocation
#  define RXX_SUPPORTS_SIZED_DEALLOCATION 0
#elif __cpp_sized_deallocation >= 201309L
#  define RXX_SUPPORTS_SIZED_DEALLOCATION 1
#else
#  define RXX_SUPPORTS_SIZED_DEALLOCATION 0
#endif

#ifdef __STDCPP_DEFAULT_NEW_ALIGNMENT__
#  define RXX_DEFAULT_NEW_ALIGNMENT __STDCPP_DEFAULT_NEW_ALIGNMENT__
#endif
