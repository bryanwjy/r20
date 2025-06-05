// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <bool Cond, typename T>
using const_if RXX_NODEBUG = std::conditional_t<Cond, std::add_const_t<T>, T>;

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
