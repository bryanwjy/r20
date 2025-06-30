// Copyright 2025 Bryan Wong
#pragma once
#include "rxx/config.h"

#include "rxx/details/const_if.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <bool Const, typename... Rs>
concept all_random_access =
    (... && std::ranges::random_access_range<const_if<Const, Rs>>);

template <bool Const, typename... Rs>
concept all_bidirectional =
    (... && std::ranges::bidirectional_range<const_if<Const, Rs>>);

template <bool Const, typename... Rs>
concept all_forward = (... && std::ranges::forward_range<const_if<Const, Rs>>);

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
