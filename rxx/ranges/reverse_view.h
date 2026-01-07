// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/iterator.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/subrange.h"
#include "rxx/utility/forward.h"

#include <ranges>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

using std::ranges::reverse_view;

namespace views {
namespace details {
template <typename T>
inline constexpr bool is_reverse_view = false;

template <typename T>
inline constexpr bool is_reverse_view<reverse_view<T>> = true;

template <typename T>
inline constexpr bool is_sized_reverse_subrange = false;

template <typename It>
inline constexpr bool
    is_sized_reverse_subrange<ranges::subrange<std::reverse_iterator<It>,
        std::reverse_iterator<It>, ranges::subrange_kind::sized>> = true;

template <typename T>
inline constexpr bool is_unsized_reverse_subrange = false;

template <typename It, ranges::subrange_kind K>
inline constexpr bool is_unsized_reverse_subrange<
    ranges::subrange<std::reverse_iterator<It>, std::reverse_iterator<It>, K>> =
    K == subrange_kind::unsized;

template <typename T>
struct unwrapped_reverse_subrange {
    using type RXX_NODEBUG = void;
};

template <typename It, ranges::subrange_kind K>
struct unwrapped_reverse_subrange<
    subrange<std::reverse_iterator<It>, std::reverse_iterator<It>, K>> {
    using type RXX_NODEBUG = ranges::subrange<It, It, K>;
};

template <typename T>
using unwrapped = typename unwrapped_reverse_subrange<T>::type;

struct reverse_t : __RXX ranges::details::adaptor_closure<reverse_t> {
    template <typename R>
    requires is_reverse_view<std::remove_cvref_t<R>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX forward<R>(arg).base()))
            -> decltype(__RXX forward<R>(arg).base()) {
        return __RXX forward<R>(arg).base();
    }

#if RXX_SUPPORTS_OPTIONAL_REFERENCES
    template <typename R>
    requires __RXX details::is_optional_like_v<std::remove_cvref_t<R>> &&
        ranges::view<std::remove_cvref_t<R>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX forward<R>(arg)))
            -> decltype(__RXX forward<R>(arg)) {
        return __RXX forward<R>(arg);
    }
#endif

    template <typename R, typename S = unwrapped<std::remove_cvref_t<R>>>
    requires is_sized_reverse_subrange<std::remove_cvref_t<R>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL noexcept(
        noexcept(S(arg.end().base(), arg.begin().base(), arg.size()))) {
        return S(arg.end().base(), arg.begin().base(), arg.size());
    }

    template <typename R, typename S = unwrapped<std::remove_cvref_t<R>>>
    requires is_unsized_reverse_subrange<std::remove_cvref_t<R>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(S(arg.end().base(), arg.begin().base())))
            -> decltype(S(arg.end().base(), arg.begin().base())) {
        return S(arg.end().base(), arg.begin().base());
    }

    template <typename R>
    requires (!is_reverse_view<std::remove_cvref_t<R>> &&
        !is_sized_reverse_subrange<std::remove_cvref_t<R>> &&
        !is_unsized_reverse_subrange<std::remove_cvref_t<R>> &&
        !(__RXX details::is_optional_like_v<std::remove_cvref_t<R>> &&
            ranges::view<std::remove_cvref_t<R>>))
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(reverse_view(__RXX forward<R>(arg))))
            -> decltype(reverse_view(__RXX forward<R>(arg))) {
        return reverse_view(__RXX forward<R>(arg));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::reverse_t reverse{};
}
} // namespace views
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
