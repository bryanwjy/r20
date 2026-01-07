// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/view_traits.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::empty_view;

namespace details {
template <typename T>
inline constexpr bool is_empty_view<ranges::empty_view<T>> = true;

template <template <typename> class R, typename T>
requires std::is_object_v<T>
inline constexpr bool is_repeat_view<R<T>> =
    requires(R<T>* ptr) { ptr->~empty_view(); };
} // namespace details

namespace views {
inline namespace cpo {
template <typename T>
inline constexpr empty_view<T> empty{};
}
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
