// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/type_traits/copy_cvref.h"

// IWYU pragma: begin_exports
#if RXX_LIBCXX

#  if __has_include(<__type_traits/common_reference.h>)
#    include <__type_traits/common_reference.h>
#  else
#    include <type_traits>
#  endif

#  if __has_include(<__type_traits/common_type.h>)
#    include <__type_traits/common_type.h>
#  else
#    include <type_traits>
#  endif

#else

#  include <type_traits>

#endif
// IWYU pragma: end_exports

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
requires requires { typename details::common_reference::impl<T, U>::type; }
struct __RXX_PUBLIC_TEMPLATE common_reference<T, U, Vs...> :
    common_reference<common_reference_t<T, U>, Vs...> {};

RXX_DEFAULT_NAMESPACE_END

#ifndef __cpp_lib_tuple_like
#  define __cpp_lib_tuple_like 0
#endif

#if !RXX_CXX23
#  define __RXX_DEFINES_COMMON_PAIR
#elif RXX_LIBSTDCXX
#  ifndef __glibcxx_ranges_zip
#    define __RXX_DEFINES_COMMON_PAIR
#  else
#    if !__glibcxx_ranges_zip
#      define __RXX_DEFINES_COMMON_PAIR
#    endif
#  endif
#elif RXX_LIBCXX
#  if !RXX_LIBCXX_AT_LEAST(15, 0, 0)
#    define __RXX_DEFINES_COMMON_PAIR
#  endif
#else
#  ifndef __cpp_lib_ranges_zip
#    define __RXX_DEFINES_COMMON_PAIR
#  else
#    if __cpp_lib_ranges_zip < 202110L
#      define __RXX_DEFINES_COMMON_PAIR
#    endif
#  endif
#endif

#ifdef __RXX_DEFINES_COMMON_PAIR

#  include "rxx/utility/pair.h" // IWYU pragma: keep

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

template <typename T1, typename T2, typename U1, typename U2>
requires requires {
    typename std::pair<std::common_type_t<T1, U1>, std::common_type_t<T2, U2>>;
}
struct std::common_type<std::pair<T1, T2>, std::pair<U1, U2>> {
    using type =
        std::pair<std::common_type_t<T1, U1>, std::common_type_t<T2, U2>>;
};

#endif

#ifdef __RXX_DEFINES_COMMON_PAIR
#  undef __RXX_DEFINES_COMMON_PAIR
#endif
