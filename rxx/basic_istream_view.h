// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"

#include <concepts>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::basic_istream_view;
using std::ranges::istream_view;
using std::ranges::wistream_view;

namespace views {
inline namespace cpo {
template <typename T>
inline constexpr std::decay_t<decltype(std::views::istream<T>)> istream{};
}
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
