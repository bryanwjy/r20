// Copyright 2025 Bryan Wong
#pragma once
#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {
template <typename T>
using with_ref = T&;

template <typename T>
concept referenceable = requires { typename with_ref<T>; };

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
