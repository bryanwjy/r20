// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::empty_view;

namespace views {
inline namespace cpo {
template <typename T>
inline constexpr empty_view<T> empty{};
}
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
