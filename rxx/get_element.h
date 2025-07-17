// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <array>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <size_t, typename T>
__RXX_HIDE_FROM_ABI void get(T&&) = delete;
template <typename T, size_t I>
concept has_adl_get = requires(T&& t) { get<I>(std::forward<T>(t)); };
template <typename T, size_t I>
concept has_member_get =
    requires(T&& t) { std::forward<T>(t).template get<I>(); };

template <size_t I>
struct get_element_t {
private:
    template <typename T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static consteval auto nothrow_member_get() noexcept {
        return false;
    }

    template <typename T>
    requires requires { std::declval<T>().template get<I>(); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI,
        NODISCARD) static consteval auto nothrow_member_get() noexcept {
        return noexcept(std::declval<T>().template get<I>());
    }

    template <typename T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static consteval auto nothrow_adl_get() noexcept {
        return false;
    }
    template <typename T>
    requires requires { get<I>(std::declval<T>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI,
        NODISCARD) static consteval auto nothrow_adl_get() noexcept {
        return noexcept(get<I>(std::declval<T>()));
    }

public:
    __RXX_HIDE_FROM_ABI constexpr explicit get_element_t() noexcept = default;

    template <has_adl_get<I> T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
    RXX_STATIC_CALL constexpr decltype(auto) operator()(
        T&& t RXX_LIFETIMEBOUND) RXX_CONST_CALL noexcept(nothrow_adl_get<T>()) {
        return get<I>(std::forward<T>(t));
    }

    template <has_member_get<I> T>
    requires (!has_adl_get<T, I>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD) RXX_STATIC_CALL
        constexpr decltype(auto)
        operator()(T&& t RXX_LIFETIMEBOUND) RXX_CONST_CALL
        noexcept(nothrow_member_get<T>()) {
        return std::forward<T>(t).template get<I>();
    }
};

} // namespace details

inline namespace cpo {
template <size_t I>
inline constexpr details::get_element_t<I> get_element{};
}

} // namespace ranges

namespace details {
namespace tuple {

template <size_t>
struct has_value;

template <typename T>
concept has_size = requires { typename has_value<std::tuple_size<T>::value>; };

template <typename T, size_t I>
concept has_element = has_size<T> && requires(T t) {
    requires I < std::tuple_size_v<T>;
    typename std::tuple_element_t<I, T>;
    ranges::get_element<I>(std::forward<T>(t));
};

template <has_size T>
inline std::make_index_sequence<std::tuple_size_v<T>> sequence_for{};
} // namespace tuple
} // namespace details

template <typename T>
concept tuple_like = details::tuple::has_size<std::remove_cvref_t<T>> &&
    []<size_t... Is>(std::index_sequence<Is...>) {
        return (... && details::tuple::has_element<std::remove_cvref_t<T>, Is>);
    }(details::tuple::sequence_for<std::remove_cvref_t<T>>);

RXX_DEFAULT_NAMESPACE_END
