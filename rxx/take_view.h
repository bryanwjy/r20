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
struct take_t {
    template <typename... Args>
    requires requires { std::views::take(std::declval<Args>()...); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr decltype(auto)
    operator()(Args&&... args) const
        noexcept(noexcept(std::views::take(std::declval<Args>()...))) {
        return std::views::take(std::forward<Args>(args)...);
    }

    template <typename V, typename N>
    requires __RXX ranges::details::is_repeat_view<V> &&
        requires { take(std::declval<V>(), std::declval<N>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr decltype(auto)
    operator()(V&& view, N&& num) const
        noexcept(noexcept(take(std::declval<V>(), std::declval<N>()))) {
        return take(std::forward<V>(view), std::forward<N>(num));
    }

    /**
     * TODO: This is really umaintainable, especially if libc++ keep
     * updating, should look into implementing a custom pipelining
     * mechanism that works with all the STL
     */
#if RXX_LIBSTDCXX
    template <typename _Tp>
    static constexpr bool _S_has_simple_extra_args =
        std::ranges::__detail::__is_integer_like<_Tp>;

    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX
    template <typename N>
    requires std::constructible_from<std::decay_t<N>, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        N&& num) const
        noexcept(std::is_nothrow_constructible_v<std::decay_t<N>, N>) {
        return __RXX ranges::details::make_pipeable(
            [taker = *this, num = std::forward<N>(num)]<typename V>(
                V&& arg) mutable {
                return taker(std::forward<V>(arg), std::forward<N>(num));
            });
    }
#elif RXX_MSVC_STL
    template <typename N>
    requires std::constructible_from<std::decay_t<N>, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        N&& num) const
        noexcept(std::is_nothrow_constructible_v<std::decay_t<N>, N>) {
        return std::ranges::_Range_closure<take_t, std::decay_t<N>>{
            std::forward<N>(num)};
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details
inline namespace cpo {
inline constexpr details::take_t take{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
