// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename From, typename To>
concept core_convertible_to =
    requires(From (*arg)(), void (*check)(To)) { check(arg()); };

} // namespace details

RXX_DEFAULT_NAMESPACE_END
