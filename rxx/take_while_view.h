// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::take_while_view;

namespace views {
inline namespace cpo {
inline constexpr std::decay_t<decltype(std::views::take_while)> take_while{};
}
} // namespace views
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
