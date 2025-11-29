// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/template_access.h"
#include "rxx/ranges/get_element.h"
#include "rxx/type_traits/common_reference.h"

#include <array>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
template <typename...>
class tuple;
RXX_DEFAULT_NAMESPACE_END

namespace details {
template <typename T>
inline constexpr bool not_std_tuple = false;
template <typename... Ts>
inline constexpr bool not_std_tuple<std::tuple<Ts...>> = true;
} // namespace details

template <typename... TTypes, typename... UTypes>
requires (sizeof...(TTypes) == sizeof...(UTypes)) &&
    requires { typename __RXX tuple<std::common_type_t<TTypes, UTypes>...>; }
struct std::common_type<__RXX tuple<TTypes...>, __RXX tuple<UTypes...>> {
    using type = typename __RXX tuple<std::common_type_t<TTypes, UTypes>...>;
};

template <typename... TTypes, typename... UTypes,
    template <typename> class TQual, template <typename> class UQual>
requires (sizeof...(TTypes) == sizeof...(UTypes)) && requires {
    typename __RXX tuple<
        __RXX common_reference_t<TQual<TTypes>..., UQual<UTypes>>...>;
}
struct std::basic_common_reference<__RXX tuple<TTypes...>,
    __RXX tuple<UTypes...>, TQual, UQual> {
    using type RXX_NODEBUG =
        __RXX tuple< __RXX common_reference_t<TQual<TTypes>, UQual<UTypes>>...>;
};

template <typename... TTypes, typename... UTypes,
    template <typename> class TQual, template <typename> class UQual>
requires (sizeof...(TTypes) == sizeof...(UTypes)) && requires {
    typename __RXX tuple<
        __RXX common_reference_t<TQual<TTypes>..., UQual<UTypes>>...>;
}
struct std::basic_common_reference<std::tuple<TTypes...>, std::tuple<UTypes...>,
    TQual, UQual> {
    using type RXX_NODEBUG =
        __RXX tuple< __RXX common_reference_t<TQual<TTypes>, UQual<UTypes>>...>;
};

template <__RXX tuple_like T, __RXX tuple_like U,
    template <typename> class TQual, template <typename> class UQual>
requires (details::not_std_tuple<T> || details::not_std_tuple<U>) &&
    (std::tuple_size_v<T> == std::tuple_size_v<U>) &&
    ([]<size_t... Is>(std::index_sequence<Is...>) {
        return requires {
            typename __RXX tuple< __RXX common_reference_t<
                TQual<std::tuple_element_t<Is, T>>...,
                UQual<std::tuple_element_t<Is, U>>>...>;
        };
    }(std::make_index_sequence<std::tuple_size_v<T>>{}))
struct std::basic_common_reference<T, U, TQual, UQual> {
private:
    template <size_t... Is>
    static auto make_type(std::index_sequence<Is...>) noexcept -> __RXX tuple<
        __RXX common_reference_t<TQual<std::tuple_element_t<Is, T>>,
            UQual<std::tuple_element_t<Is, U>>>...>;

public:
    using type RXX_NODEBUG =
        decltype(make_type(std::make_index_sequence<std::tuple_size_v<T>>{}));
};

template <size_t I, typename... Ts>
requires requires {
    typename __RXX details::template_element_t<I, __RXX tuple<Ts...>>;
}
struct std::tuple_element<I, __RXX tuple<Ts...>> {
    using type = __RXX details::template_element_t<I, __RXX tuple<Ts...>>;
};

template <typename... Ts>
struct std::tuple_size<__RXX tuple<Ts...>> :
    std::integral_constant<size_t, sizeof...(Ts)> {};

RXX_DEFAULT_NAMESPACE_BEGIN

template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>>& get(tuple<Ts...>& arg RXX_LIFETIMEBOUND) noexcept;
template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>>&& get(tuple<Ts...>&& arg RXX_LIFETIMEBOUND) noexcept;
template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>> const& get(tuple<Ts...> const& arg RXX_LIFETIMEBOUND) noexcept;
template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr std::tuple_element_t<I,
    tuple<Ts...>> const&& get(tuple<Ts...> const&& arg RXX_LIFETIMEBOUND) noexcept;

RXX_DEFAULT_NAMESPACE_END
