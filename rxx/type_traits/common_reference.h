// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/copy_cvref.h"
#include "rxx/ranges/get_element.h"

#include <tuple>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T, typename U, template <typename> class TQual,
    template <typename> class UQual>
using basic_common_reference_t =
    typename std::basic_common_reference<T, U, TQual, UQual>::type;

template <typename...>
struct common_reference {};

template <typename... Ts>
using common_reference_t = typename common_reference<Ts...>::type;

template <>
struct __RXX_PUBLIC_TEMPLATE common_reference<> {};
template <typename T>
struct __RXX_PUBLIC_TEMPLATE common_reference<T> {
    using type = T;
};

namespace details {
namespace common_reference {

template <typename T>
struct copy_cvref_from {
    template <typename U>
    using apply = copy_cvref_t<T, U>;
};

template <typename T0, typename T1>
struct ternary_result {};

template <typename T0, typename T1>
requires requires {
    false ? static_cast<T0 (*)()>(0)() : static_cast<T1 (*)()>(0)();
}
struct ternary_result<T0, T1> {
    using type = decltype(false ? static_cast<T0 (*)()>(0)()
                                : static_cast<T1 (*)()>(0)());
};

template <typename T0, typename T1>
using ternary_result_t = typename ternary_result<T0, T1>::type;

template <typename Target, typename... Ts>
using merge_const_t = std::conditional_t<(... || std::is_const_v<Ts>),
    std::add_const_t<Target>, Target>;
template <typename Target, typename... Ts>
using merge_volatile_t = std::conditional_t<(... || std::is_volatile_v<Ts>),
    std::add_volatile_t<Target>, Target>;

template <typename Target, typename... Ts>
using merge_cv_t = merge_volatile_t<merge_const_t<Target, Ts...>, Ts...>;

template <typename T0, typename T1>
struct simple_common_ref {};

template <typename T0, typename T1>
using simple_common_ref_t = typename simple_common_ref<T0, T1>::type;

template <typename Target, typename... Ts>
concept all_convertible_from = (... && std::is_convertible_v<Ts, Target>);

template <typename T0, typename T1>
requires all_convertible_from<
    std::add_pointer_t<
        ternary_result_t<merge_cv_t<T0, T0, T1>&, merge_cv_t<T1, T0, T1>&> //
        >,
    std::add_pointer_t<T0&>, //
    std::add_pointer_t<T1&>>
struct simple_common_ref<T0&, T1&> {
    using type =
        ternary_result_t<merge_cv_t<T0, T0, T1>&, merge_cv_t<T1, T0, T1>&>;
};

template <typename T>
using rvalue_ref = std::add_rvalue_reference<std::remove_reference_t<T>>;
template <typename T>
using rvalue_ref_t = typename rvalue_ref<T>::type;

template <typename T0, typename T1>
requires all_convertible_from<rvalue_ref_t<simple_common_ref_t<T0&, T1&>>, T0&&,
             T1&&> &&
    all_convertible_from<
        std::add_pointer_t<rvalue_ref_t<simple_common_ref_t<T0&, T1&>>>,
        std::add_pointer_t<T0&&>, std::add_pointer_t<T1&&>>
struct simple_common_ref<T0&&, T1&&> :
    rvalue_ref<simple_common_ref_t<T0&, T1&>> {};

template <typename T0, typename T1>
requires std::is_convertible_v<T1&&,
    simple_common_ref_t<T0&, std::add_const_t<T1>&>>
struct simple_common_ref<T0&, T1&&> :
    simple_common_ref<T0&, std::add_const_t<T1>&> {};

template <typename T0, typename T1>
requires requires { typename simple_common_ref_t<T1&, T0&&>; }
struct simple_common_ref<T0&&, T1&> : simple_common_ref<T1&, T0&&> {};

template <typename T, typename U>
using basic_common_ref =
    std::basic_common_reference<std::remove_cvref_t<T>, std::remove_cvref_t<U>,
        copy_cvref_from<T>::template apply, copy_cvref_from<U>::template apply>;

template <typename...>
struct first_type;

template <>
struct first_type<> {};

template <typename T, typename... Ts>
requires requires { typename T::type; }
struct first_type<T, Ts...> : T {};

template <typename T, typename... Ts>
struct first_type<T, Ts...> : first_type<Ts...> {};

template <typename T, typename U>
using impl = first_type<simple_common_ref<T, U>,
    std::basic_common_reference<std::remove_cvref_t<T>, std::remove_cvref_t<U>,
        copy_cvref_from<T>::template apply, copy_cvref_from<U>::template apply>,
    ternary_result<T, U>, std::common_type<T, U>>;

} // namespace common_reference
} // namespace details

template <typename T, typename U>
struct __RXX_PUBLIC_TEMPLATE common_reference<T, U> :
    details::common_reference::impl<T, U> {};

template <typename T, typename U, typename... Vs>
requires requires { typename common_reference_t<T, U>; }
struct __RXX_PUBLIC_TEMPLATE common_reference<T, U, Vs...> :
    common_reference<common_reference_t<T, U>, Vs...> {};

template <typename T, typename U>
concept common_reference_with =
    std::same_as<common_reference_t<T, U>, common_reference_t<U, T>> &&
    std::convertible_to<T, common_reference_t<T, U>> &&
    std::convertible_to<U, common_reference_t<T, U>>;

RXX_DEFAULT_NAMESPACE_END

#if !RXX_CXX23

template <typename... TTypes, typename... UTypes,
    template <typename> class TQual, template <typename> class UQual>
requires (sizeof...(TTypes) == sizeof...(UTypes)) && requires {
    typename std::tuple<
        __RXX common_reference_t<TQual<TTypes>..., UQual<UTypes>>...>;
}
struct std::basic_common_reference<std::tuple<TTypes...>, std::tuple<UTypes...>,
    TQual, UQual> {
    using type =
        std::tuple< __RXX common_reference_t<TQual<TTypes>, UQual<UTypes>>...>;
};

template <typename T1, typename T2, typename U1, typename U2,
    template <typename> class TQual, template <typename> class UQual>
requires requires {
    typename std::pair< __RXX common_reference_t<TQual<T1>, UQual<U1>>,
        __RXX common_reference_t<TQual<T2>, UQual<U2>>>;
}
struct std::basic_common_reference<std::pair<T1, T2>, std::pair<U1, U2>, TQual,
    UQual> {
    using type = std::pair< __RXX common_reference_t<TQual<T1>, UQual<U1>>,
        __RXX common_reference_t<TQual<T2>, UQual<U2>>>;
};

template <__RXX tuple_like T, __RXX tuple_like U,
    template <typename> class TQual, template <typename> class UQual>
requires std::same_as<std::decay_t<T>, T> && std::same_as<std::decay_t<U>, U> &&
    (std::tuple_size_v<T> == std::tuple_size_v<U>)
struct std::basic_common_reference<T, U, TQual, UQual> {
    using type = typename decltype( //
        []<size_t... Is>(std::index_sequence<Is...>) {
            using Result = std::tuple< //
                __RXX common_reference_t<TQual<std::tuple_element_t<Is, T>>,
                    UQual<std::tuple_element_t<Is, U>> //
                    >...                               //
                >;
            return std::type_identity<Result>{};
        }(std::make_index_sequence<std::tuple_size_v<T>>{}))::type;
};

template <typename... TTypes, typename... UTypes>
requires (sizeof...(TTypes) == sizeof...(UTypes)) &&
    requires { typename std::tuple<std::common_type_t<TTypes, UTypes>...>; }
struct std::common_type<std::tuple<TTypes...>, std::tuple<UTypes...>> {
    using type = typename std::tuple<std::common_type_t<TTypes, UTypes>...>;
};

template <typename T1, typename T2, typename U1, typename U2>
requires requires {
    typename std::pair<std::common_type_t<T1, U1>, std::common_type_t<T2, U2>>;
}
struct std::common_type<std::pair<T1, T2>, std::pair<U1, U2>> {
    using type =
        std::pair<std::common_type_t<T1, U1>, std::common_type_t<T2, U2>>;
};

template <__RXX tuple_like T, __RXX tuple_like U>
requires std::same_as<std::decay_t<T>, T> && std::same_as<std::decay_t<U>, U> &&
    (std::tuple_size_v<T> == std::tuple_size_v<U>)
struct std::common_type<T, U> {
    using type = typename decltype( //
        []<size_t... Is>(std::index_sequence<Is...>) {
            using Result =
                std::tuple<std::common_type_t<std::tuple_element_t<Is, T>,
                    std::tuple_element_t<Is, U>>...>;
            return std::type_identity<Result>{};
        }(std::make_index_sequence<std::tuple_size_v<T>>{}))::type;
};

#endif
