// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
namespace views {
inline namespace cpo {
inline constexpr std::decay_t<decltype(std::views::counted)> counted{};
}
} // namespace views
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
