/* Copyright 2023-2025 Bryan Wong */

#pragma once

#ifndef RXX_MODULES
#  define RXX_MODULES 0
#endif

#if RXX_MODULES
#  define RXX_EXPORT export
#  define RXX_EXPORT_BEGIN export {
#  define RXX_EXPORT_END }
#else
#  define RXX_EXPORT
#  define RXX_EXPORT_BEGIN
#  define RXX_EXPORT_END
#endif
