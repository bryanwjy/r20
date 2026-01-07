// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/optional/fwd.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/details/view_traits.h"
#include "rxx/functional/bind_back.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/empty_view.h" // IWYU pragma: keep
#include "rxx/ranges/iota_view.h"  // IWYU pragma: keep
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/repeat_view.h" // IWYU pragma: keep
#include "rxx/ranges/subrange.h"
#include "rxx/utility.h"

#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
using std::ranges::drop_view;

namespace views {
namespace details {

template <typename V, typename N>
concept dropable =
    requires { drop_view(std::declval<V>(), std::declval<N>()); };

template <typename V, typename N>
concept droping_empty =
    dropable<V, N> && ranges::details::is_empty_view<std::remove_cvref_t<V>>;

template <typename V, typename N>
concept droping_optional = dropable<V, N> && !droping_empty<V, N> &&
    __RXX details::is_optional_v<std::remove_cvref_t<V>> &&
    view<std::remove_cvref_t<V>>;

template <typename V, typename N>
concept droping_random_sized_range = dropable<V, N> && !droping_empty<V, N> &&
    !droping_optional<V, N> && random_access_range<std::remove_cvref_t<V>> &&
    sized_range<std::remove_cvref_t<V>> &&
    (ranges::details::is_span<std::remove_cvref_t<V>> ||
        ranges::details::is_basic_string_view<std::remove_cvref_t<V>> ||
        ranges::details::is_iota_view_like<std::remove_cvref_t<V>> ||
        ranges::details::is_subrange_like<std::remove_cvref_t<V>>);

template <typename V>
inline constexpr bool subrange_stores_size = false;

template <template <typename, typename, subrange_kind K> class R, typename I,
    typename S, subrange_kind K>
inline constexpr bool subrange_stores_size<R<I, S, K>> =
    ranges::details::is_subrange_like<R<I, S, K>> &&
    K == subrange_kind::sized && !std::sized_sentinel_for<S, I>;

void drop(...) noexcept = delete;

template <typename V, typename N = range_difference_t<V>>
requires (!__RXX ranges::details::is_repeat_view<std::remove_cvref_t<V>>) &&
    __RXX ranges::details::is_repeat_view_like<std::remove_cvref_t<V>>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto drop(V&& view, std::type_identity_t<N> num) {
    // For std::ranges::repeat_view
    using Type = std::remove_cvref_t<V>;
    if constexpr (sized_range<Type>) {
        auto const dist = ranges::distance(view);
        return repeat_view(__RXX forward_like<V>(*ranges::begin(view)),
            dist - std::min<N>(dist, num));
    } else {
        return __RXX_AUTOCAST(__RXX forward<V>(view));
    }
}

template <typename V, typename N>
concept droping_repeat = dropable<V, N> && !droping_empty<V, N> &&
    !droping_optional<V, N> && !droping_random_sized_range<V, N> &&
    __RXX ranges::details::is_repeat_view_like<std::remove_cvref_t<V>> &&
    requires { drop(std::declval<V>(), std::declval<N>()); };

struct drop_t : ranges::details::adaptor_non_closure<drop_t> {

    template <viewable_range V, typename N = range_difference_t<V>>
    requires droping_empty<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N>) RXX_CONST_CALL
        noexcept(noexcept(std::remove_cvref_t<V>())) {
        return std::remove_cvref_t<V>();
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires droping_optional<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N> num) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<V>, V>) {
        return num ? std::remove_cvref_t<V>() : __RXX forward<V>(view);
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires droping_random_sized_range<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N> num) RXX_CONST_CALL {
        auto const bnum = std::min<N>(ranges::distance(view), num);
        auto const begin = ranges::begin(view) + bnum;
        auto const end = ranges::end(view);
        using Type = std::remove_cvref_t<V>;
        if constexpr (ranges::details::is_span<Type>) {
            return std::span<typename Type::element_type>(begin, end);
        } else if constexpr (ranges::details::is_basic_string_view<Type>) {
            return Type(begin, end);
        } else if constexpr (ranges::details::is_iota_view_like<Type>) {
            return Type(*begin, *end);
        } else {
            static_assert(ranges::details::is_subrange_like<Type>);
            if constexpr (subrange_stores_size<Type>) {
                return Type(begin, end,
                    ranges::details::to_unsigned_like(
                        ranges::distance(view) - bnum));
            } else {
                return Type(begin, end);
            }
        }
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires droping_repeat<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N> num) RXX_CONST_CALL
        noexcept(noexcept(drop(std::declval<V>(), std::declval<N>()))) {
        return drop(__RXX forward<V>(view), __RXX forward<N>(num));
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires dropable<V, N> &&
        (!(droping_empty<V, N> || droping_optional<V, N> ||
            droping_random_sized_range<V, N> || droping_repeat<V, N>))
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr decltype(auto)
    operator()(V&& view, std::type_identity_t<N> num) RXX_CONST_CALL noexcept(
        noexcept(drop_view(__RXX forward<V>(view), __RXX forward<N>(num)))) {
        return drop_view(__RXX forward<V>(view), __RXX forward<N>(num));
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
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(N&& num) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<N>, N>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(drop_t{}),
            __RXX forward<N>(num));
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
