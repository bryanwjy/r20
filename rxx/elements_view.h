// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"

#include <concepts>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::elements_view;
template <typename R>
using keys_view = elements_view<R, 0>;
template <typename R>
using values_view = elements_view<R, 1>;

namespace views {
inline namespace cpo {
template <size_t I>
inline constexpr std::decay_t<decltype(std::views::elements<I>)> elements{};
inline constexpr auto keys = elements<0>;
inline constexpr auto values = elements<1>;
} // namespace cpo
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
