// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/details/view_match.h"

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
using std::ranges::take_view;

namespace views {
namespace details {

using native_take_t = std::decay_t<decltype(std::views::take)>;
struct take_t : public native_take_t {
    template <typename... Args>
    requires requires { std::views::take(std::declval<Args>()...); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr decltype(auto)
    operator()(Args&&... args) const
        noexcept(noexcept(std::views::take(std::declval<Args>()...))) {
        return native_take_t::operator()(std::forward<Args>(args)...);
    }

    template <typename V, typename N>
    requires __RXX ranges::details::is_repeat_view<V> &&
        requires { take(std::declval<V>(), std::declval<N>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr decltype(auto)
    operator()(V&& view, N&& num) const
        noexcept(noexcept(take(std::declval<V>(), std::declval<N>()))) {
        return take(std::forward<V>(view), std::forward<N>(num));
    }
};
} // namespace details
inline namespace cpo {
inline constexpr details::take_t take{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
