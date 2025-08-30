// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/view_traits.h"
#include "rxx/utility.h"

#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
using std::ranges::drop_view;

namespace views {
namespace details {

template <typename V, typename N>
concept specialized_drop =
    __RXX ranges::details::is_repeat_view<std::remove_cvref_t<V>> &&
    requires { drop(std::declval<V>(), std::declval<N>()); };
template <typename V, typename N>
concept default_drop = !specialized_drop<V, N> &&
    requires { std::views::drop(std::declval<V>(), std::declval<N>()); };

struct drop_t : ranges::details::adaptor_non_closure<drop_t> {
    template <typename V, typename N>
    requires default_drop<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL
        constexpr decltype(auto)
        operator()(V&& view, N&& num) RXX_CONST_CALL noexcept(
            noexcept(std::views::drop(std::declval<V>(), std::declval<N>()))) {
        return std::views::drop(std::forward<V>(view), std::forward<N>(num));
    }

    template <typename V, typename N>
    requires specialized_drop<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL
        constexpr decltype(auto)
        operator()(V&& view, N&& num) RXX_CONST_CALL
        noexcept(noexcept(drop(std::declval<V>(), std::declval<N>()))) {
        return drop(std::forward<V>(view), std::forward<N>(num));
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<drop_t>::operator();

    template <typename _Tp>
    static constexpr bool _S_has_simple_extra_args =
        __RXX ranges::details::integer_like<_Tp>;

    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename N>
    requires std::constructible_from<std::decay_t<N>, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(N&& num) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<N>, N>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(*this), std::forward<N>(num));
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
