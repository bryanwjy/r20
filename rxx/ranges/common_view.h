// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::common_view;

namespace views {
inline namespace cpo {
inline constexpr std::decay_t<decltype(std::views::common)> common{};
}
} // namespace views
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
