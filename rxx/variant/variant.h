// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/concepts/swap.h"
#include "rxx/configuration/abi.h"
#include "rxx/functional/invoke_r.h"
#include "rxx/memory/construct_at.h"
#include "rxx/memory/destroy_at.h"
#include "rxx/preprocessor/attribute_list.h"
#include "rxx/type_traits/copy_cvref.h"                // IWYU pragma: keep
#include "rxx/type_traits/is_explicit_constructible.h" // IWYU pragma: keep
#include "rxx/type_traits/template_access.h"
#include "rxx/utility.h"
#include "rxx/variant/bad_variant_access.h"

#include <concepts>
#include <initializer_list>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T>
__RXX_HIDE_FROM_ABI inline constexpr auto visit_table_for =
    []<size_t... Is>(__RXX index_sequence<Is...>) {
        return jump_table<size_t, Is..., variant_npos>{};
    }(__RXX make_index_sequence<variant_size_v<T>>{});

template <typename T, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool holds_alternative(variant<Ts...> const& var) noexcept {
    static_assert(
        template_count_v<T, variant<Ts...>> == 1, "Invalid alternative");
    return var.index() == template_index_v<T, variant<Ts...>>;
}

namespace details {
template <size_t I>
using size_constant = std::integral_constant<size_t, I>;

template <typename...>
union multi_union;

struct valueless_t {
    __RXX_HIDE_FROM_ABI explicit constexpr valueless_t() noexcept = default;
};

struct dispatch_t {
    __RXX_HIDE_FROM_ABI explicit constexpr dispatch_t() noexcept = default;
};

inline constexpr valueless_t valueless{};
inline constexpr dispatch_t dispatch{};

template <typename Head, typename... Tail>
union multi_union<Head, Tail...> {
    using first_type = Head;
    using second_type = std::conditional_t<(sizeof...(Tail) > 1),
        multi_union<Tail...>, template_element_t<1, multi_union>>;

    __RXX_HIDE_FROM_ABI static constexpr bool is_last_union =
        (sizeof...(Tail) == 1);

    __RXX_HIDE_FROM_ABI constexpr multi_union() noexcept = delete;

    __RXX_HIDE_FROM_ABI explicit(explicit_default_constructible<
        first_type>) constexpr multi_union() noexcept
    requires std::is_trivially_default_constructible_v<first_type> &&
        std::is_trivially_default_constructible_v<second_type>
    = default;

    __RXX_HIDE_FROM_ABI constexpr multi_union(multi_union const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr multi_union(multi_union const&) noexcept
    requires std::is_copy_constructible_v<first_type> &&
        std::is_copy_constructible_v<second_type> &&
        std::is_trivially_copy_constructible_v<first_type> &&
        std::is_trivially_copy_constructible_v<second_type>
    = default;

    __RXX_HIDE_FROM_ABI constexpr multi_union(multi_union&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr multi_union(multi_union&&) noexcept
    requires std::is_move_constructible_v<first_type> &&
        std::is_move_constructible_v<second_type> &&
        std::is_trivially_move_constructible_v<first_type> &&
        std::is_trivially_move_constructible_v<second_type>
    = default;

    __RXX_HIDE_FROM_ABI constexpr multi_union& operator=(
        multi_union const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr multi_union& operator=(
        multi_union const&) noexcept
    requires std::is_copy_assignable_v<first_type> &&
        std::is_copy_assignable_v<second_type> &&
        std::is_trivially_copy_assignable_v<first_type> &&
        std::is_trivially_copy_assignable_v<second_type>
    = default;

    __RXX_HIDE_FROM_ABI constexpr multi_union& operator=(
        multi_union&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr multi_union& operator=(multi_union&&) noexcept
    requires std::is_move_assignable_v<first_type> &&
        std::is_move_assignable_v<second_type> &&
        std::is_trivially_move_assignable_v<first_type> &&
        std::is_trivially_move_assignable_v<second_type>
    = default;

    template <typename... Args>
    requires std::is_constructible_v<first_type, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<first_type>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<first_type,
        Args...>)
        : first{__RXX forward<Args>(args)...} {}

    template <typename... Args>
    requires std::is_constructible_v<first_type, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<0>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<first_type,
        Args...>)
        : first{__RXX forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires std::is_nothrow_invocable_r_v<first_type, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<0>,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : first{__RXX invoke_r<first_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)} {}

    template <typename F, typename... Args>
    requires std::is_nothrow_invocable_r_v<first_type, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_type_t<first_type>, F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : first{__RXX invoke_r<first_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)} {}

    template <typename U, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, std::in_place_type_t<U>, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        std::in_place_type_t<U>, Args...>)
        : second{tag, __RXX forward<Args>(args)...} {}

    template <size_t N, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, std::in_place_index_t<N - 1>,
            Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<N>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        std::in_place_index_t<N>, Args...>)
        : second{std::in_place_index<N - 1>, __RXX forward<Args>(args)...} {}

    template <size_t N, typename F, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, generating_index_t<N - 1>, F,
            Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<N>,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{generating_index<N - 1>, __RXX forward<F>(f),
              __RXX forward<Args>(args)...} {}

    template <typename U, typename F, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, generating_type_t<U>, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_type_t<U> tag,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{tag, __RXX forward<F>(f), __RXX forward<Args>(args)...} {}

    template <typename... Args>
    requires (is_last_union && std::is_constructible_v<second_type, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<second_type>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        Args...>)
        : second{__RXX forward<Args>(args)...} {}

    template <typename... Args>
    requires (is_last_union && std::is_constructible_v<second_type, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<1>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        Args...>)
        : second{__RXX forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires (
        is_last_union && std::is_nothrow_invocable_r_v<second_type, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<1>,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{__RXX invoke_r<second_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)} {}

    template <typename F, typename... Args>
    requires (
        is_last_union && std::is_nothrow_invocable_r_v<second_type, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_type_t<second_type>, F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{__RXX invoke_r<second_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)} {}

    __RXX_HIDE_FROM_ABI constexpr ~multi_union() noexcept
    requires std::is_trivially_destructible_v<first_type> &&
        std::is_trivially_destructible_v<second_type>
    = default;
    __RXX_HIDE_FROM_ABI constexpr ~multi_union() noexcept {}

#if RXX_CXX23
    template <size_t I, typename Self>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN, NODISCARD)
    constexpr decltype(auto) get(this Self&& self) noexcept {
        static_assert(I <= sizeof...(Tail));
        if constexpr (I == 0) {
            return __RXX forward_like<Self>(self.first);
        } else if constexpr (is_last_union) {
            return __RXX forward_like<Self>(self.second);
        } else {
            return __RXX forward_like<Self>(self.second).template get<I - 1>();
        }
    }
#else

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN, NODISCARD)
    constexpr auto get() const& noexcept -> decltype(auto) {
        static_assert(I <= sizeof...(Tail));
        if constexpr (I == 0) {
            return (first);
        } else if constexpr (is_last_union) {
            return (second);
        } else {
            return second.template get<I - 1>();
        }
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN, NODISCARD)
    constexpr auto get() & noexcept -> decltype(auto) {
        static_assert(I <= sizeof...(Tail));
        if constexpr (I == 0) {
            return (first);
        } else if constexpr (is_last_union) {
            return (second);
        } else {
            return second.template get<I - 1>();
        }
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN, NODISCARD)
    constexpr auto get() const&& noexcept -> decltype(auto) {
        static_assert(I <= sizeof...(Tail));
        if constexpr (I == 0) {
            return __RXX move(first);
        } else if constexpr (is_last_union) {
            return __RXX move(second);
        } else {
            return __RXX move(second).template get<I - 1>();
        }
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN, NODISCARD)
    constexpr auto get() && noexcept -> decltype(auto) {
        static_assert(I <= sizeof...(Tail));
        if constexpr (I == 0) {
            return __RXX move(first);
        } else if constexpr (is_last_union) {
            return __RXX move(second);
        } else {
            return __RXX move(second).template get<I - 1>();
        }
    }

#endif

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) first_type first;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) second_type second;
};

template <typename T, typename U, size_t... Is>
consteval bool nothrow_make_from_multi_union(
    __RXX index_sequence<Is...>) noexcept {
    return (... &&
        std::is_nothrow_constructible_v<T, std::in_place_index_t<Is>,
            decltype(std::declval<U>().template get<Is>())>);
}

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr T
make_from_multi_union(size_t index, U&& arg) noexcept(
    nothrow_make_from_multi_union<T, U>(
        __RXX make_index_sequence_v<template_size_v<T>>)) {
    static_assert(template_size_v<T> == template_size_v<U>);
    return iota_table_for<U>(
        [&]<size_t I>(size_constant<I>) {
            return T{std::in_place_index<I>,
                __RXX forward<U>(arg).template get<I>()};
        },
        index);
}

template <typename T>
using array_t = T[1];
template <typename T, typename>
struct variant_overload {
private:
    class inaccessible_t {};

public:
    static void test(inaccessible_t) noexcept;
};
template <size_t I, typename T>
struct variant_typeid {};
template <typename T, size_t I, typename T_i>
requires requires(T val) { array_t<T_i>{__RXX move(val)}; }
struct variant_overload<T, variant_typeid<I, T_i>> {
    static std::integral_constant<size_t, I> test(T_i) noexcept;
};
template <typename T, typename... Vs>
struct variant_overload_for_t;
template <typename T, size_t... Is, typename... Vs>
struct variant_overload_for_t<T, variant_typeid<Is, Vs>...> :
    variant_overload<T, variant_typeid<Is, Vs>>... {
    using variant_overload<T, variant_typeid<Is, Vs>>::test...;
};
template <typename T, typename... Ts, size_t... Is>
auto make_variant_overload_for(__RXX index_sequence<Is...>) noexcept
    -> variant_overload_for_t<T, variant_typeid<Is, Ts>...>;
template <typename T, typename... Ts>
using make_variant_overload_for_t =
    decltype(make_variant_overload_for<T, Ts...>(
        __RXX index_sequence_for<Ts...>{}));

template <typename T, typename... Ts>
inline constexpr size_t conversion_index =
    decltype(make_variant_overload_for_t<T, Ts...>::test(
        std::declval<T>()))::value;

template <typename T, typename... Ts>
using conversion_type =
    template_element_t<conversion_index<T, Ts...>, type_list<Ts...>>;

template <typename T>
inline constexpr bool is_tag_v = false;
template <typename T>
inline constexpr bool is_tag_v<T const> = is_tag_v<T>;
template <typename T>
inline constexpr bool is_tag_v<T const volatile> = is_tag_v<T>;
template <typename T>
inline constexpr bool is_tag_v<T volatile> = is_tag_v<T>;
template <>
inline constexpr bool is_tag_v<std::in_place_t> = true;
template <typename T>
inline constexpr bool is_tag_v<std::in_place_type_t<T>> = true;
template <size_t I>
inline constexpr bool is_tag_v<std::in_place_index_t<I>> = true;
template <>
inline constexpr bool is_tag_v<__RXX generating_t> = true;
template <typename T>
inline constexpr bool is_tag_v<__RXX generating_type_t<T>> = true;
template <size_t I>
inline constexpr bool is_tag_v<__RXX generating_index_t<I>> = true;

template <typename T, typename F, typename... Args>
concept generatable_from =
    std::invocable<F> && requires(T* ptr, F&& func, Args&&... args) {
        ::new (ptr) T(std::invoke(
            __RXX forward<F>(func), __RXX forward<Args>(args)...));
    };

template <typename T, typename F, typename... Args>
concept nothrow_generatable_from = generatable_from<T, F, Args...> &&
    requires(T* ptr, F&& func, Args&&... args) {
        {
            ::new (ptr) T(std::invoke(
                __RXX forward<F>(func), __RXX forward<Args>(args)...))
        } noexcept;
    };

template <typename... Ts>
class variant_base;

template <typename U, typename... Ts>
concept overloadable_conversion_to = sizeof...(Ts) > 0 &&
    !std::same_as<__RXX variant<Ts...>, std::remove_cvref_t<U>> &&
    !is_tag_v<std::remove_cvref_t<U>> && requires(U&& val) {
        typename make_variant_overload_for_t<U, Ts...>;
        make_variant_overload_for_t<U, Ts...>::test(__RXX forward<U>(val));
        typename std::in_place_index_t<conversion_index<U, Ts...>>;
        typename conversion_type<U, Ts...>;
        requires std::constructible_from<conversion_type<U, Ts...>, U>;
    };

template <typename... Ts>
class variant_base {
    using union_type = multi_union<Ts..., valueless_t>;
    using index_type = unsigned char;

    static_assert(
        sizeof...(Ts) <= static_cast<index_type>(-1), "limit exceeded");

    __RXX_HIDE_FROM_ABI static constexpr bool place_index_in_tail =
        fits_in_tail_padding_v<union_type, index_type>;
    __RXX_HIDE_FROM_ABI static constexpr bool allow_external_overlap =
        !place_index_in_tail;

    struct container {
        __RXX_HIDE_FROM_ABI constexpr container() noexcept(
            std::is_nothrow_constructible_v<union_type>) = default;

        template <typename T, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            std::in_place_type_t<T> tag, Args&&... args)
            : union_{std::in_place, tag, __RXX forward<Args>(args)...}
            , index_{template_index_v<T, union_type>} {}

        template <size_t I, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            std::in_place_index_t<I> tag, Args&&... args)
            : union_{std::in_place, tag, __RXX forward<Args>(args)...}
            , index_{I} {}

        template <typename T, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            generating_type_t<T> tag, F&& callable, Args&&... args)
            : union_{std::in_place, tag, __RXX forward<F>(callable),
                  __RXX forward<Args>(args)...}
            , index_{template_index_v<T, union_type>} {}

        template <size_t I, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            generating_index_t<I> tag, F&& callable, Args&&... args)
            : union_{std::in_place, tag, __RXX forward<F>(callable),
                  __RXX forward<Args>(args)...}
            , index_{I} {}

        template <typename U>
        __RXX_HIDE_FROM_ABI constexpr container(dispatch_t, size_t index,
            U&& arg) noexcept(noexcept(make_from_multi_union<union_type>(index,
            std::declval<U>())))
        requires (allow_external_overlap)
            : union_{generating,
                  [&]() {
                      return make_from_multi_union<union_type>(
                          index, __RXX forward<U>(arg));
                  }}
            , index_(index) {}

        __RXX_HIDE_FROM_ABI constexpr container(container const&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container(container const&) noexcept
        requires ((... && std::is_copy_constructible_v<Ts>) &&
                     (... && std::is_trivially_copy_constructible_v<Ts>))
        = default;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) noexcept
        requires ((... && std::is_move_constructible_v<Ts>) &&
                     (... && std::is_trivially_move_constructible_v<Ts>))
        = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) noexcept
        requires ((... && std::is_copy_assignable_v<Ts>) &&
                     (... && std::is_trivially_copy_assignable_v<Ts>))
        = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container&&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(container&&) noexcept
        requires ((... && std::is_move_assignable_v<Ts>) &&
                     (... && std::is_trivially_move_assignable_v<Ts>))
        = default;

        __RXX_HIDE_FROM_ABI constexpr ~container() noexcept
        requires ((... && std::is_trivially_destructible_v<Ts>))
        = default;
        __RXX_HIDE_FROM_ABI constexpr ~container() noexcept
        requires ((... || !std::is_trivially_destructible_v<Ts>))
        {
            destroy_member();
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires (allow_external_overlap &&
            (... && std::is_trivially_destructible_v<Ts>))
        {
            __RXX destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires (allow_external_overlap &&
            (... || !std::is_trivially_destructible_v<Ts>))
        {
            destroy_member();
            __RXX destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        template <typename U, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr U*
        construct_union(std::in_place_type_t<U> tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<U, Args...>)
        requires (allow_external_overlap)
        {
            static_assert(
                template_index_v<U, union_type> < template_size_v<union_type>);
            auto ptr = construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
            index_ = template_index_v<U, union_type>;
            return RXX_BUILTIN_addressof(
                (ptr->template get<template_index_v<U, union_type>>()));
        }

        template <size_t I, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr template_element_t<I, union_type>*
        construct_union(std::in_place_index_t<I> tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<template_element_t<I, union_type>,
                Args...>)
        requires (allow_external_overlap)
        {
            static_assert(I < template_size_v<union_type>);
            auto ptr = construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
            index_ = I;
            return RXX_BUILTIN_addressof(ptr->template get<I>());
        }

        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS)
        overlappable_if<place_index_in_tail, union_type> union_;
        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS) index_type index_;

    private:
        __RXX_HIDE_FROM_ABI constexpr void destroy_member() noexcept {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    __RXX destroy_at(
                        RXX_BUILTIN_addressof(union_.data.template get<I>()));
                },
                static_cast<size_t>(index_));
        }
    };

    template <typename U, size_t... Is>
    static consteval bool nothrow_make_container(
        __RXX index_sequence<Is...>) noexcept {
        return (... &&
            std::is_nothrow_constructible_v<container,
                std::in_place_index_t<Is>,
                decltype(std::declval<U>().template get<Is>())>);
    }

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr container
    make_container(size_t index, U&& arg) noexcept(nothrow_make_container<U>(
        __RXX make_index_sequence_v<template_size_v<U>>))
    requires (place_index_in_tail)
    {
        static_assert(template_size_v<union_type> == template_size_v<U>);
        return iota_table_for<union_type>(
            [&]<size_t I>(size_constant<I>) -> container {
                return container{std::in_place_index<I>,
                    __RXX forward<U>(arg).template get<I>()};
            },
            index);
    }

public:
    __RXX_HIDE_FROM_ABI constexpr variant_base() noexcept = delete;

    __RXX_HIDE_FROM_ABI constexpr variant_base() noexcept(
        std::is_nothrow_default_constructible_v<
            template_element_t<0, variant_base>>)
    requires std::default_initializable<template_element_t<0, variant_base>>
        : variant_base{std::in_place_index<0>} {}

    __RXX_HIDE_FROM_ABI constexpr ~variant_base() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base const&) noexcept(
        (... && std::is_nothrow_copy_constructible_v<Ts>))
    requires ((... && std::is_copy_constructible_v<Ts>) &&
                 (... && std::is_trivially_copy_constructible_v<Ts>))
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base(
        variant_base const& other) noexcept((... &&
        std::is_nothrow_copy_constructible_v<Ts>))
    requires ((... && std::is_copy_constructible_v<Ts>) &&
        !(... && std::is_trivially_copy_constructible_v<Ts>))
        : variant_base(dispatch, other.index(), other.union_ref()) {}

    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base&&) noexcept
    requires ((... && std::is_move_constructible_v<Ts>) &&
                 (... && std::is_trivially_move_constructible_v<Ts>))
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base&& other) noexcept(
        (... && std::is_nothrow_move_constructible_v<Ts>))
    requires ((... && std::is_move_constructible_v<Ts>) &&
        !(... && std::is_trivially_move_constructible_v<Ts>))
        : variant_base(
              dispatch, other.index(), __RXX move(other.union_ref())) {}

    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base const&) noexcept
    requires ((... && std::is_copy_assignable_v<Ts>) &&
                 (... && std::is_trivially_copy_assignable_v<Ts>))
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base&
    operator=(variant_base const& other) noexcept(
        (... && std::is_nothrow_copy_assignable_v<Ts>) &&
        (... && std::is_nothrow_copy_constructible_v<Ts>))
    requires ((... && std::is_copy_assignable_v<Ts>) &&
        !(... && std::is_trivially_copy_assignable_v<Ts>))
    {
        if (this->index() != other.index()) {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->reinitialize_value<I>(other.template value_ref<I>());
                },
                other.index());
        } else {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->template value_ref<I>() =
                        other.template value_ref<I>();
                },
                other.index());
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base&&) noexcept
    requires ((... && std::is_move_assignable_v<Ts>) &&
                 (... && std::is_trivially_move_assignable_v<Ts>))
    = default;
    __RXX_HIDE_FROM_ABI constexpr variant_base&
    operator=(variant_base&& other) noexcept(
        (... && std::is_nothrow_move_assignable_v<Ts>) &&
        (... && std::is_nothrow_move_constructible_v<Ts>))
    requires ((... && std::is_move_assignable_v<Ts>) &&
        !(... && std::is_trivially_move_assignable_v<Ts>))
    {
        if (this->index() != other.index()) {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->reinitialize_value<I>(
                        __RXX move(other).template value_ref<I>());
                },
                other.index());
        } else {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->template value_ref<I>() =
                        __RXX move(other).template value_ref<I>();
                },
                other.index());
        }

        return *this;
    }

    template <typename U, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_type_t<U>, Args...>)
        : container_{std::in_place, tag, __RXX forward<Args>(args)...} {}

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        std::in_place_index_t<I> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_index_t<I>, Args...>)
        : container_{std::in_place, tag, __RXX forward<Args>(args)...} {}

    template <typename U, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        generating_type_t<U> tag, F&& callable,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        generating_type_t<U>, Args...>)
        : container_{std::in_place, tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...} {}

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        generating_index_t<I> tag, F&& callable,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        generating_index_t<I>, F, Args...>)
        : container_{std::in_place, tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...} {}

    __RXX_HIDE_FROM_ABI constexpr size_t index() const noexcept {
        return container_.data.index_;
    }

protected:
    template <size_t I, typename U, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    reinitialize_value(std::initializer_list<U> ilist, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        using target_type = template_element_t<I, union_type>;

        destroy();
        if constexpr (std::is_nothrow_constructible_v<target_type, Args...>) {
            return construct_value<I>(ilist, __RXX forward<Args>(args)...);
        } else
            try {
                return construct_value<I>(ilist, __RXX forward<Args>(args)...);
            } catch (...) {
                construct_value<sizeof...(Ts)>();
                throw;
            }
    }

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    reinitialize_value(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        using target_type = template_element_t<I, union_type>;

        destroy();
        if constexpr (std::is_nothrow_constructible_v<target_type, Args...>) {
            return construct_value<I>(__RXX forward<Args>(args)...);
        } else
            try {
                return construct_value<I>(__RXX forward<Args>(args)...);
            } catch (...) {
                construct_value<sizeof...(Ts)>();
                throw;
            }
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    regenerate_value(F&& callable, Args&&... args) noexcept(
        std::is_nothrow_invocable_r_v<template_element_t<I, union_type>, F,
            Args...>) {
        using target_type = template_element_t<I, union_type>;

        destroy();
        if constexpr (std::is_nothrow_invocable_r_v<
                          template_element_t<I, union_type>, F, Args...>) {
            return generate_value<I>(
                __RXX forward<F>(callable), __RXX forward<Args>(args)...);
        } else
            try {
                return generate_value<I>(
                    __RXX forward<F>(callable), __RXX forward<Args>(args)...);
            } catch (...) {
                construct_value<sizeof...(Ts)>();
                throw;
            }
    }

#if RXX_CXX23
    template <size_t I, typename Self>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr decltype(auto) value_ref(this Self&& self) noexcept {
        return __RXX forward_like<Self>(
            ((copy_cvref_t<Self, variant_base>&&)self)
                .union_ref()
                .template get<I>());
    }

    template <typename Self>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr decltype(auto) union_ref(this Self&& self) noexcept {
        return __RXX forward_like<Self>(
            ((copy_cvref_t<Self, variant_base>&&)self)
                .container_.data.union_.data);
    }

#else
    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto value_ref() const& noexcept -> decltype(auto) {
        static_assert(I < template_size_v<union_type>);
        return union_ref().template get<I>();
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto value_ref() & noexcept -> decltype(auto) {
        static_assert(I < template_size_v<union_type>);
        return union_ref().template get<I>();
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto value_ref() const&& noexcept -> decltype(auto) {
        static_assert(I < template_size_v<union_type>);
        return __RXX move(union_ref().template get<I>());
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto value_ref() && noexcept -> decltype(auto) {
        static_assert(I < template_size_v<union_type>);
        return __RXX move(union_ref().template get<I>());
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() const& noexcept -> decltype(auto) {
        return (container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() & noexcept -> decltype(auto) {
        return (container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() const&& noexcept -> decltype(auto) {
        return __RXX move(container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() && noexcept -> decltype(auto) {
        return __RXX move(container_.data.union_.data);
    }
#endif

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr variant_base(dispatch_t tag, size_t index,
        U&& arg) noexcept(std::is_nothrow_constructible_v<container, dispatch_t,
        size_t, U>)
    requires (allow_external_overlap)
        : container_{std::in_place, tag, index, __RXX forward<U>(arg)} {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr variant_base(dispatch_t, size_t index,
        U&& arg) noexcept(noexcept(make_container(index, std::declval<U>())))
    requires (place_index_in_tail)
        : container_{generating,
              [&]() { return make_container(index, __RXX forward<U>(arg)); }} {
    }

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto construct_value(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        if constexpr (place_index_in_tail) {
            construct_at(RXX_BUILTIN_addressof(container_.data),
                std::in_place_index<I>, __RXX forward<Args>(args)...);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else {
            return container_.data.construct_union(
                std::in_place_index<I>, __RXX forward<Args>(args)...);
        }
    }

    template <size_t I, typename U, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    construct_value(std::initializer_list<U> ilist, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        if constexpr (place_index_in_tail) {
            construct_at(RXX_BUILTIN_addressof(container_.data),
                std::in_place_index<I>, ilist, __RXX forward<Args>(args)...);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else {
            return container_.data.construct_union(
                std::in_place_index<I>, ilist, __RXX forward<Args>(args)...);
        }
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    generate_value(F&& callable, Args&&... args) noexcept(
        std::is_nothrow_invocable_r_v<template_element_t<I, union_type>, F,
            Args...>) {
        if constexpr (place_index_in_tail) {
            construct_at(RXX_BUILTIN_addressof(container_.data),
                generating_index<I>, __RXX forward<F>(callable),
                __RXX forward<Args>(args)...);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else {
            return container_.data.construct_union(generating_index<I>,
                __RXX forward<F>(callable), __RXX forward<Args>(args)...);
        }
    }

    __RXX_HIDE_FROM_ABI constexpr void destroy() noexcept {
        if constexpr (place_index_in_tail) {
            destroy_at(RXX_BUILTIN_addressof(container_.data));
        } else {
            container_.data.destroy_union();
        }
    }

    RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};
} // namespace details

template <typename... Ts>
class variant : private details::variant_base<Ts...> {
    static_assert(sizeof...(Ts) > 0, "Empty variant");
    static_assert((... && std::is_destructible_v<Ts>), "Invalid type");
    static_assert((... && !std::is_reference_v<Ts>), "Invalid type");
    static_assert((... && !std::is_function_v<Ts>), "Invalid type");
    static_assert((... && !details::is_tag_v<Ts>), "Invalid type");
    using first_type = template_element_t<0, variant>;
    using base_type = details::variant_base<Ts...>;
    template <typename... Us>
    friend class variant;

    template <size_t I, typename... Us>
    requires requires { typename template_element_t<I, variant<Us...>>; }
    friend constexpr template_element_t<I, variant<Us...>> const& get(
        variant<Us...> const& val) noexcept;
    template <size_t I, typename... Us>
    requires requires { typename template_element_t<I, variant<Us...>>; }
    friend constexpr template_element_t<I, variant<Us...>>& get(
        variant<Us...>& val) noexcept;
    template <size_t I, typename... Us>
    requires requires { typename template_element_t<I, variant<Us...>>; }
    friend constexpr template_element_t<I, variant<Us...>> const&& get(
        variant<Us...> const&& val) noexcept;
    template <size_t I, typename... Us>
    requires requires { typename template_element_t<I, variant<Us...>>; }
    friend constexpr template_element_t<I, variant<Us...>>&& get(
        variant<Us...>&& val) noexcept;

public:
    __RXX_HIDE_FROM_ABI constexpr explicit(
        (... || __RXX is_explicit_constructible_v<Ts>)) variant() = default;
    __RXX_HIDE_FROM_ABI constexpr variant(variant const&) = default;
    __RXX_HIDE_FROM_ABI constexpr variant(variant&&) = default;
    __RXX_HIDE_FROM_ABI constexpr variant& operator=(variant const&) = default;
    __RXX_HIDE_FROM_ABI constexpr variant& operator=(variant&&) = default;
    __RXX_HIDE_FROM_ABI constexpr ~variant() = default;

    template <typename U, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<U,
        std::in_place_type_t<U>, Args...>)
        : base_type{tag, __RXX forward<Args>(args)...} {}

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(
        std::in_place_index_t<I> tag, Args&&... args) noexcept(std::
            is_nothrow_constructible_v<template_element_t<I, variant>,
                std::in_place_index_t<I>, Args...>)
        : base_type{tag, __RXX forward<Args>(args)...} {}

    template <typename U, typename F, typename... Args>
    requires (template_count_v<U, variant> == 1) &&
        details::generatable_from<U, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(generating_type_t<U> tag,
        F&& callable,
        Args&&... args) noexcept(details::nothrow_generatable_from<U, F,
        Args...>)
        : base_type{tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...} {}

    template <size_t I, typename F, typename... Args>
    requires (I < sizeof...(Ts)) &&
        details::generatable_from<template_element_t<I, variant>, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(generating_index_t<I> tag,
        F&& callable, Args&&... args) noexcept(details::
            nothrow_generatable_from<template_element_t<I, variant>, F,
                Args...>)
        : base_type(tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...) {}

    template <details::overloadable_conversion_to<Ts...> U>
    __RXX_HIDE_FROM_ABI constexpr variant(U&& arg) noexcept(
        std::is_nothrow_constructible_v<details::conversion_type<U, Ts...>, U>)
        : variant(std::in_place_index<details::conversion_index<U, Ts...>>,
              __RXX forward<U>(arg)) {}

    template <details::overloadable_conversion_to<Ts...> U>
    __RXX_HIDE_FROM_ABI constexpr variant& operator=(U&& arg) noexcept(
        std::is_nothrow_constructible_v<details::conversion_type<U, Ts...>,
            U> &&
        std::is_nothrow_move_constructible_v<
            details::conversion_type<U, Ts...>> &&
        std::is_nothrow_assignable_v<details::conversion_type<U, Ts...>&, U>) {
        constexpr auto idx = details::conversion_index<U, Ts...>;
        if (idx == index()) {
            this->template value_ref<idx>() = __RXX forward<U>(arg);
        } else if constexpr (std::is_nothrow_constructible_v<
                                 details::conversion_type<U, Ts...>, U> ||
            !std::is_nothrow_move_constructible_v<
                details::conversion_type<U, Ts...>>) {
            this->template reinitialize_value<idx>(__RXX forward<U>(arg));
        } else {
            this->template reinitialize_value<idx>(
                conversion_type<U, Ts...>(__RXX forward<U>(arg)));
        }

        return *this;
    }

    using base_type::index;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool valueless_by_exception() const noexcept {
        return index() == sizeof...(Ts);
    }

    template <typename T, typename... Args>
    requires std::constructible_from<T, Args...> &&
        (template_count_v<T, variant> == 1)
    __RXX_HIDE_FROM_ABI constexpr T& emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        return *this->template reinitialize_value<template_index_v<T, variant>>(
            __RXX forward<Args>(args)...);
    }

    template <typename T, typename U, typename... Args>
    requires std::constructible_from<T, Args...> &&
        (template_count_v<T, variant> == 1)
    __RXX_HIDE_FROM_ABI constexpr T& emplace(std::initializer_list<U> ilist,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        return *this->template reinitialize_value<template_index_v<T, variant>>(
            ilist, __RXX forward<Args>(args)...);
    }

    template <size_t I, typename... Args>
    requires (I < sizeof...(Ts)) &&
        std::constructible_from<template_element_t<I, variant>, Args...>
    __RXX_HIDE_FROM_ABI constexpr template_element_t<I, variant>&
    emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, variant>,
            Args...>) {
        return *this->template reinitialize_value<I>(
            __RXX forward<Args>(args)...);
    }

    template <size_t I, typename U, typename... Args>
    requires (I < sizeof...(Ts)) &&
        std::constructible_from<template_element_t<I, variant>, Args...>
    __RXX_HIDE_FROM_ABI constexpr template_element_t<I, variant>&
    emplace(std::initializer_list<U> ilist, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, variant>,
            Args...>) {
        return *this->template reinitialize_value<I>(
            ilist, __RXX forward<Args>(args)...);
    }

    template <typename T, typename F, typename... Args>
    requires details::generatable_from<T, F, Args...> &&
        (template_count_v<T, variant> == 1)
    __RXX_HIDE_FROM_ABI constexpr T&
    generate(F&& generator, Args&&... args) noexcept(
        details::nothrow_generatable_from<T, F, Args...>) {
        return *this->template regenerate_value<template_index_v<T, variant>>(
            __RXX forward<F>(generator), __RXX forward<Args>(args)...);
    }

    template <size_t I, typename F, typename... Args>
    requires (I < sizeof...(Ts)) &&
        details::generatable_from<template_element_t<I, variant>, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr template_element_t<I, variant>&
    generate(F&& generator, Args&&... args) noexcept(
        details::nothrow_generatable_from<template_element_t<I, variant>, F,
            Args...>) {
        return *this->template regenerate_value<I>(
            __RXX forward<F>(generator), __RXX forward<Args>(args)...);
    }

    __RXX_HIDE_FROM_ABI constexpr void swap(variant& other) noexcept(
        (noexcept(std::declval<variant&>() = __RXX exchange(
                      std::declval<variant&>(), std::declval<variant>())) &&
            ... && std::is_nothrow_swappable_v<Ts>)) {
        if (base_type::index() == static_cast<base_type&>(other).index()) {
            iota_table_for<variant<Ts..., details::valueless_t>>(
                [&]<size_t I>(details::size_constant<I>) {
                    ranges::swap(this->template value_ref<I>(),
                        other.template value_ref<I>());
                },
                index());
        } else {
            other = __RXX exchange(*this, std::move(other));
        }
    }

    __RXX_HIDE_FROM_ABI friend constexpr void swap(
        variant& left, variant& right) noexcept(noexcept(left.swap(right))) {
        return left.swap(right);
    }

#if __cpp_explicit_this_parameter >= 202110L && (RXX_CXX23 | RXX_COMPILER_CLANG)

#  if RXX_COMPILER_CLANG && !RXX_CXX23
    RXX_DISABLE_WARNING_PUSH()
    RXX_DISABLE_WARNING("-Wc++23-extensions")
#  endif
    // NOLINTBEGIN
    template <typename Self, typename F>
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(
        this Self&& self, F&& visitor)
    requires (... && std::invocable<F, copy_cvref_t<Self, Ts>>)
    {
        if (self.valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return iota_table_for<variant>(
            [&]<size_t I>(details::size_constant<I>) -> decltype(auto) {
                return std::invoke(__RXX forward<F>(visitor),
                    __RXX forward_like<Self>(self.template value_ref<I>()));
            },
            self.index());
    }

    template <typename R, typename Self, typename F>
    __RXX_HIDE_FROM_ABI constexpr R visit(this Self&& self, F&& visitor)
    requires (... && std::is_invocable_r_v<R, F, copy_cvref_t<Self, Ts>>)
    {
        if constexpr (std::is_void_v<R>) {
            __RXX forward<Self>(self).visit(__RXX forward<F>(visitor));
        } else {
            return __RXX forward<Self>(self).visit(__RXX forward<F>(visitor));
        }
    }
    // NOLINTEND
#  if RXX_COMPILER_CLANG && !RXX_CXX23
    RXX_DISABLE_WARNING_POP()
#  endif

#else

    template <typename F>
    requires (... && std::invocable<F, Ts&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) & {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return iota_table_for<variant>(
            [&]<size_t I>(details::size_constant<I>) -> decltype(auto) {
                if constexpr (I >= sizeof...(Ts)) {
                    RXX_THROW(bad_variant_access());
                } else {
                    return std::invoke(__RXX forward<F>(visitor),
                        this->template value_ref<I>());
                }
            },
            this->index());
    }

    template <typename F>
    requires (... && std::invocable<F, Ts const&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) const& {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return iota_table_for<variant>(
            [&]<size_t I>(details::size_constant<I>) -> decltype(auto) {
                if constexpr (I >= sizeof...(Ts)) {
                    RXX_THROW(bad_variant_access());
                } else {
                    return std::invoke(__RXX forward<F>(visitor),
                        this->template value_ref<I>());
                }
            },
            this->index());
    }

    template <typename F>
    requires (... && std::invocable<F, Ts &&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) && {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return iota_table_for<variant>(
            [&]<size_t I>(details::size_constant<I>) -> decltype(auto) {
                return std::invoke(__RXX forward<F>(visitor),
                    __RXX move(this->template value_ref<I>()));
            },
            this->index());
    }

    template <typename F>
    requires (... && std::invocable<F, Ts const &&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) const&& {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return iota_table_for<variant>(
            [&]<size_t I>(details::size_constant<I>) -> decltype(auto) {
                return std::invoke(__RXX forward<F>(visitor),
                    __RXX move(this->template value_ref<I>()));
            },
            this->index());
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) & {
        if constexpr (std::is_void_v<R>) {
            this->visit(__RXX forward<F>(visitor));
        } else {
            return this->visit(__RXX forward<F>(visitor));
        }
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts const&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) const& {
        if constexpr (std::is_void_v<R>) {
            this->visit(__RXX forward<F>(visitor));
        } else {
            return this->visit(__RXX forward<F>(visitor));
        }
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts &&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) && {
        if constexpr (std::is_void_v<R>) {
            __RXX move(*this).visit(__RXX forward<F>(visitor));
        } else {
            return __RXX move(*this).visit(__RXX forward<F>(visitor));
        }
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts const &&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) const&& {
        if constexpr (std::is_void_v<R>) {
            __RXX move(*this).visit(__RXX forward<F>(visitor));
        } else {
            return __RXX move(*this).visit(__RXX forward<F>(visitor));
        }
    }

#endif

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator==(variant const& right) const noexcept((... &&
        noexcept(std::declval<Ts const&>() == std::declval<Ts const&>())))
    requires (... && std::equality_comparable<Ts>)
    {
        return this->index() == right.index() &&
            visit_table_for<variant>(
                [&]<size_t I>(details::size_constant<I>) {
                    if constexpr (I == variant_npos) {
                        return true;
                    } else {
                        return this->template value_ref<I>() ==
                            right.template value_ref<I>();
                    }
                },
                this->index());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator<=>(variant const& right) const noexcept
    requires (... && std::three_way_comparable<Ts>)
    {
        using result = std::common_comparison_category<
            std::compare_three_way_result_t<size_t>,
            std::compare_three_way_result_t<Ts>...>;

        auto cmp = this->index() <=> right.index();
        if (cmp != 0) {
            return [&]() -> result { return cmp; }();
        }

        return visit_table_for<variant>(
            [&]<size_t I>(details::size_constant<I>) -> result {
                if constexpr (I == variant_npos) {
                    return 0 <=> 0;
                } else {
                    return this->template value_ref<I>() <=>
                        right.template value_ref<I>();
                }
            },
            this->index());
    }
};

RXX_DEFAULT_NAMESPACE_END
