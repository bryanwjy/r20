// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/view_traits.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::subrange;
using std::ranges::subrange_kind;

namespace details {
template <typename I, typename S, subrange_kind K>
inline constexpr bool is_subrange<ranges::subrange<I, S, K>> = true;

template <template <typename, typename, subrange_kind> class R,
    std::input_or_output_iterator I, std::sentinel_for<I> S,
    ranges::subrange_kind K>
requires (K == ranges::subrange_kind::sized || !std::sized_sentinel_for<S, I>)
inline constexpr bool is_subrange_like<R<I, S, K>> = requires(R<I, S, K>* ptr) {
    ptr->~subrange();
} && std::derived_from<R<I, S, K>, std::ranges::view_interface<R<I, S, K>>>;

} // namespace details

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
