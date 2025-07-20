// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {
template <typename T>
concept class_or_enum =
    std::is_class_v<T> || std::is_union_v<T> || std::is_enum_v<T>;
}
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
