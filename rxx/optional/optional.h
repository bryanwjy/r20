// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/optional_abi.h" // IWYU pragma: export

#if RXX_OPTIONAL_ABI_INTEROP
#  include "rxx/optional/optional_gcc.h"    // IWYU pragma: export
#  include "rxx/optional/optional_normal.h" // IWYU pragma: export
#elif RXX_OPTIONAL_ABI_GCC
#  include "rxx/optional/optional_gcc.h" // IWYU pragma: export
#else
#  include "rxx/optional/optional_normal.h" // IWYU pragma: export
#endif
