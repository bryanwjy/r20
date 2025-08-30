// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/ranges/get_element.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details::tuple {

template <size_t I, typename T>
requires requires { ranges::get_element<I>(std::declval<T>()); }
auto decl_element() noexcept
    -> decltype(ranges::get_element<I>(std::declval<T>()));

template <size_t I, typename T>
requires requires { ranges::get_element<I>(std::declval<T>()); }
using decl_element_t = decltype(ranges::get_element<I>(std::declval<T>()));

template <tuple_like Tuple>
inline constexpr bool is_nothrow_accessible_v =
    []<size_t... Is>(std::index_sequence<Is...>) {
        return (...&& noexcept(ranges::get_element<Is>(std::declval<Tuple>())));
    }(sequence_for<std::remove_cvref_t<Tuple>>);

struct ignore_t {
    __RXX_HIDE_FROM_ABI constexpr ignore_t const& operator=(
        auto const&) const noexcept {
        return *this;
    }
};

} // namespace details::tuple

inline constexpr details::tuple::ignore_t ignore{};
RXX_DEFAULT_NAMESPACE_END
