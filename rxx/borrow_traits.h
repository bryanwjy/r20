// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts.h"
#include "rxx/subrange.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

template <range R>
using borrowed_iterator_t =
    std::conditional_t<borrowed_range, iterator_t<R>, dangling>;

template <range R>
using borrowed_subrange_t =
    std::conditional_t<subrange<iterator_t<R>>, iterator_t<R>, dangling>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
