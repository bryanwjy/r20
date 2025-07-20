// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
namespace views {

using std::views::all_t;

inline namespace cpo {
inline constexpr std::decay_t<decltype(std::views::all)> all{};
}
} // namespace views
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
