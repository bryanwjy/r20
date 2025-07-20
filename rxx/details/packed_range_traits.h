// Copyright 2025 Bryan Wong
#pragma once
#include "rxx/config.h"

#include "rxx/details/const_if.h"
#include "rxx/ranges/concepts.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <bool Const, typename... Rs>
concept all_random_access = (... && random_access_range<const_if<Const, Rs>>);

template <bool Const, typename... Rs>
concept all_bidirectional = (... && bidirectional_range<const_if<Const, Rs>>);

template <bool Const, typename... Rs>
concept all_forward = (... && forward_range<const_if<Const, Rs>>);

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
