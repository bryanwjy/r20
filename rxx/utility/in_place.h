// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#if RXX_LIBSTDCXX & __has_include(<bits/utility.h>)
#  include <bits/utility.h>
#elif RXX_LIBCXX & __has_include(<__utility/in_place.h>)
#  include <__utility/in_place.h>
#else
#  include <utility>
#endif
