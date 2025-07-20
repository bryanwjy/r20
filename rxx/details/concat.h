// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/ranges/primitives.h"
#include "rxx/type_traits/common_reference.h"

#include <concepts>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <typename... Ts>
using concat_reference_t = common_reference_t<range_reference_t<Ts>...>;
template <typename... Ts>
using concat_value_t = std::common_type_t<range_value_t<Ts>...>;
template <typename... Ts>
using concat_rvalue_reference_t =
    common_reference_t<range_rvalue_reference_t<Ts>...>;

template <typename Ref, typename RRef, typename It>
concept concat_indirectly_readable_impl = requires(It const it) {
    { *it } -> std::convertible_to<Ref>;
    { std::ranges::iter_move(it) } -> std::convertible_to<RRef>;
};

template <typename... Ts>
concept concat_indirectly_readable =
    common_reference_with<concat_reference_t<Ts...>&&,
        concat_value_t<Ts...>&> &&
    common_reference_with<concat_reference_t<Ts...>&&,
        concat_rvalue_reference_t<Ts...>&&> &&
    common_reference_with<concat_rvalue_reference_t<Ts...>&&,
        concat_value_t<Ts...> const&> &&
    (... &&
        concat_indirectly_readable_impl<concat_reference_t<Ts...>,
            concat_rvalue_reference_t<Ts...>, iterator_t<Ts>>);

template <typename... Ts>
concept concatable = requires {
    typename concat_reference_t<Ts...>;
    typename concat_value_t<Ts...>;
    typename concat_rvalue_reference_t<Ts...>;
} && concat_indirectly_readable<Ts...>;
} // namespace details
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
