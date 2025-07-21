// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/ranges/all.h"

#include <concepts>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::elements_view;
template <typename R>
using keys_view = elements_view<R, 0>;
template <typename R>
using values_view = elements_view<R, 1>;

namespace views {
namespace details {

template <size_t N>
struct elements_t : ranges::details::adaptor_closure<elements_t<N>> {
    template <typename R>
    requires requires { elements_view<all_t<R>, N>{std::declval<R>()}; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& range) RXX_CONST_CALL
        noexcept(noexcept(elements_view<all_t<R>, N>(std::forward<R>(range))))
            -> decltype(elements_view<all_t<R>, N>(std::forward<R>(range))) {
        return elements_view<all_t<R>, N>(std::forward<R>(range));
    }

#if RXX_LIBSTDCXX
    static constexpr bool _S_has_simple_call_op = true;
#endif
};

} // namespace details

inline namespace cpo {
template <size_t I>
inline constexpr details::elements_t<I> elements{};
inline constexpr details::elements_t<0> keys{};
inline constexpr details::elements_t<1> values{};
} // namespace cpo
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
