// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/optional/fwd.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/integer_like.h" // IWYU pragma: keep
#include "rxx/details/view_traits.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/empty_view.h" // IWYU pragma: keep
#include "rxx/ranges/iota_view.h"  // IWYU pragma: keep
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/repeat_view.h" // IWYU pragma: keep
#include "rxx/ranges/subrange.h"
#include "rxx/utility.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
using std::ranges::take_view;
namespace views {
namespace details {

template <typename V, typename N>
concept takeable =
    requires { take_view(std::declval<V>(), std::declval<N>()); };

template <typename V, typename N>
concept taking_empty =
    takeable<V, N> && ranges::details::is_empty_view<std::remove_cvref_t<V>>;

template <typename V, typename N>
concept taking_optional = takeable<V, N> && !taking_empty<V, N> &&
    __RXX details::is_optional_like_v<std::remove_cvref_t<V>> &&
    view<std::remove_cvref_t<V>>;

template <typename V, typename N>
concept taking_random_sized_range = takeable<V, N> && !taking_empty<V, N> &&
    !taking_optional<V, N> && random_access_range<std::remove_cvref_t<V>> &&
    sized_range<std::remove_cvref_t<V>> &&
    (ranges::details::is_span<std::remove_cvref_t<V>> ||
        ranges::details::is_basic_string_view<std::remove_cvref_t<V>> ||
        ranges::details::is_iota_view_like<std::remove_cvref_t<V>> ||
        ranges::details::is_subrange_like<std::remove_cvref_t<V>>);

void take(...) noexcept = delete;

template <typename V, typename N = range_difference_t<V>>
requires (!__RXX ranges::details::is_repeat_view<std::remove_cvref_t<V>>) &&
    __RXX ranges::details::is_repeat_view_like<std::remove_cvref_t<V>> &&
    std::is_constructible_v<std::remove_cvref_t<V>,
        decltype(__RXX forward_like<V>(std::declval<range_reference_t<V>>())),
        N>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto take(V&& view, std::type_identity_t<N> num) {
    // For std::ranges::repeat_view
    using Type = std::remove_cvref_t<V>;
    if constexpr (sized_range<Type>) {
        auto const dist = ranges::distance(view);
        return Type(__RXX forward_like<V>(*ranges::begin(view)),
            std::min<N>(dist, num));
    } else {
        return Type(__RXX forward_like<V>(*ranges::begin(view)), num);
    }
}

template <typename V, typename N>
concept taking_repeat = takeable<V, N> && !taking_empty<V, N> &&
    !taking_optional<V, N> && !taking_random_sized_range<V, N> &&
    __RXX ranges::details::is_repeat_view_like<std::remove_cvref_t<V>> &&
    requires { take(std::declval<V>(), std::declval<N>()); };

struct take_t : ranges::details::adaptor_non_closure<take_t> {

    template <viewable_range V, typename N = range_difference_t<V>>
    requires taking_empty<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N>) RXX_CONST_CALL
        noexcept(noexcept(std::remove_cvref_t<V>())) {
        return std::remove_cvref_t<V>();
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires taking_optional<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N> num) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<V>, V>) {
        return num ? __RXX forward<V>(view) : std::remove_cvref_t<V>();
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires taking_random_sized_range<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N> num) RXX_CONST_CALL {
        num = std::min<N>(ranges::distance(view), num);
        auto const begin = ranges::begin(view);
        auto const end = begin + num;
        using Type = std::remove_cvref_t<V>;
        if constexpr (ranges::details::is_span<Type>) {
            return std::span<typename Type::element_type>(begin, end);
        } else if constexpr (ranges::details::is_basic_string_view<Type>) {
            return Type(begin, end);
        } else if constexpr (ranges::details::is_iota_view_like<Type>) {
            return iota_view(*begin, *end);
        } else {
            static_assert(ranges::details::is_subrange_like<Type>);
            return subrange<iterator_t<Type>>(begin, end);
        }
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires taking_repeat<V, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        V&& view, std::type_identity_t<N> num) RXX_CONST_CALL
        noexcept(noexcept(take(std::declval<V>(), std::declval<N>()))) {
        return take(__RXX forward<V>(view), __RXX forward<N>(num));
    }

    template <viewable_range V, typename N = range_difference_t<V>>
    requires takeable<V, N> &&
        (!(taking_empty<V, N> || taking_optional<V, N> ||
            taking_random_sized_range<V, N> || taking_repeat<V, N>))
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr decltype(auto)
    operator()(V&& view, std::type_identity_t<N> num) RXX_CONST_CALL noexcept(
        noexcept(take_view(__RXX forward<V>(view), __RXX forward<N>(num)))) {
        return take_view(__RXX forward<V>(view), __RXX forward<N>(num));
    }

    template <typename N>
    requires std::constructible_from<std::decay_t<N>, N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(N&& num) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<N>, N>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(take_t{}),
            __RXX forward<N>(num));
    }
};
} // namespace details
inline namespace cpo {
inline constexpr details::take_t take{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
