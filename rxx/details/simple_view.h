// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/primitives.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {
template <typename R>
concept simple_view = std::ranges::view<R> && std::ranges::range<R const> &&
    std::same_as<iterator_t<R>, iterator_t<R const>> &&
    std::same_as<sentinel_t<R>, sentinel_t<R const>>;

} // namespace details

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
