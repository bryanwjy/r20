// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/const_if.h"
#include "rxx/ranges/primitives.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {

template <bool Const, typename R>
using iterator_category_of = typename std::iterator_traits<
    iterator_t<details::const_if<Const, R>>>::iterator_category;

} // namespace details

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
