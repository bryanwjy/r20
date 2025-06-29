// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {

template <typename I>
__RXX_HIDE_FROM_ABI constexpr I ceil_div(I num, I denom) noexcept {
    return (num + denom - 1) / denom;
}

} // namespace details

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
