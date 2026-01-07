// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/view_traits.h"

#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::iota_view;

namespace details {
template <typename W, typename Bound>
inline constexpr bool is_iota_view<iota_view<W, Bound>> = true;

template <template <typename, typename> class R, std::weakly_incrementable W,
    std::semiregular Bound>
inline constexpr bool is_iota_view_like<R<W, Bound>> =
    requires(R<W, Bound>* ptr) { ptr->~iota_view(); };
} // namespace details

namespace views {
inline namespace cpo {
inline constexpr std::decay_t<decltype(std::views::iota)> iota{};
}
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
