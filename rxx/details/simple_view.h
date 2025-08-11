// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/ranges/concepts.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {
template <typename R>
concept simple_view = view<R> && range<R const> &&
    std::same_as<iterator_t<R>, iterator_t<R const>> &&
    std::same_as<sentinel_t<R>, sentinel_t<R const>>;

} // namespace details

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
