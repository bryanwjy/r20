// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/details/bind_back.h"
#include "rxx/details/view_traits.h"

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
using std::ranges::drop_view;

namespace views {
namespace details {
struct drop_t : ranges::details::adaptor_non_closure<drop_t> {
    template <typename... Args>
    requires requires { std::views::drop(std::declval<Args>()...); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr decltype(auto)
    operator()(Args&&... args) const
        noexcept(noexcept(std::views::drop(std::declval<Args>()...))) {
        return std::views::drop(std::forward<Args>(args)...);
    }

    template <typename V, typename N>
    requires __RXX ranges::details::is_repeat_view<std::remove_cvref_t<V>> &&
        requires { drop(std::declval<V>(), std::declval<N>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr decltype(auto)
    operator()(V&& view, N&& num) const
        noexcept(noexcept(drop(std::declval<V>(), std::declval<N>()))) {
        return drop(std::forward<V>(view), std::forward<N>(num));
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<drop_t>::operator();

    template <typename _Tp>
    static constexpr bool _S_has_simple_extra_args =
        RXX ranges::details::integer_like<_Tp>;

    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename N>
    requires std::constructible_from<std::decay_t<N>, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        N&& num) const
        noexcept(std::is_nothrow_constructible_v<std::decay_t<N>, N>) {
        return __RXX ranges::details::make_pipeable(
            set_arity<2>(*this), std::forward<N>(num));
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details
inline namespace cpo {
inline constexpr details::drop_t drop{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
