// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/concepts/generatable.h"
#include "rxx/concepts/swap.h"
#include "rxx/functional/invoke_r.h"
#include "rxx/memory/construct_at.h"
#include "rxx/memory/destroy_at.h"
#include "rxx/type_traits/copy_cvref.h"                // IWYU pragma: keep
#include "rxx/type_traits/is_explicit_constructible.h" // IWYU pragma: keep
#include "rxx/type_traits/template_access.h"
#include "rxx/utility.h"
#include "rxx/variant/bad_variant_access.h"
#include "rxx/variant/visit.h"

#include <cassert>
#include <concepts>
#include <initializer_list>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool holds_alternative(variant<Ts...> const& var) noexcept {
    static_assert(
        template_count_v<T, variant<Ts...>> == 1, "Invalid alternative");
    return var.index() == template_index_v<T, variant<Ts...>>;
}

namespace details {

template <typename...>
union multi_union;

struct valueless_var_t {
    __RXX_HIDE_FROM_ABI explicit constexpr valueless_var_t() noexcept = default;
};

struct dispatch_var_t {
    __RXX_HIDE_FROM_ABI explicit constexpr dispatch_var_t() noexcept = default;
};

inline constexpr valueless_var_t valueless_var{};
inline constexpr dispatch_var_t dispatch_var{};

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
        : first(__RXX forward<Args>(args)...) {}

    template <typename... Args>
    requires std::is_constructible_v<first_type, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<0>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<first_type,
        Args...>)
        : first(__RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    requires std::is_invocable_r_v<first_type, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_index_t<0>, F&& f, Args&&... args) noexcept( //
        std::is_nothrow_invocable_r_v<first_type, F, Args...>)
        : first(__RXX invoke_r<first_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)) {}

    template <typename F, typename... Args>
    requires std::is_nothrow_invocable_r_v<first_type, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_type_t<first_type>, F&& f, Args&&... args) noexcept( //
        std::is_nothrow_invocable_r_v<first_type, F, Args...>)
        : first(__RXX invoke_r<first_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)) {}

    template <typename U, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, std::in_place_type_t<U>, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        std::in_place_type_t<U>, Args...>)
        : second(tag, __RXX forward<Args>(args)...) {}

    template <size_t N, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, std::in_place_index_t<N - 1>,
            Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<N>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        std::in_place_index_t<N - 1>, Args...>)
        : second(std::in_place_index<N - 1>, __RXX forward<Args>(args)...) {}

    template <size_t N, typename F, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, generating_index_t<N - 1>, F,
            Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<N>,
        F&& f,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        generating_index_t<N - 1>, F, Args...>)
        : second(generating_index<N - 1>, __RXX forward<F>(f),
              __RXX forward<Args>(args)...) {}

    template <typename U, typename F, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, generating_type_t<U>, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_type_t<U> tag,
        F&& f,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        generating_type_t<U>, F, Args...>)
        : second(tag, __RXX forward<F>(f), __RXX forward<Args>(args)...) {}

    template <typename... Args>
    requires (is_last_union && std::is_constructible_v<second_type, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<second_type>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        Args...>)
        : second(__RXX forward<Args>(args)...) {}

    template <typename... Args>
    requires (is_last_union && std::is_constructible_v<second_type, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<1>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        Args...>)
        : second(__RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    requires (is_last_union && std::is_invocable_r_v<second_type, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<1>,
        F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<second_type, F,
        Args...>)
        : second(__RXX invoke_r<second_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)) {}

    template <typename F, typename... Args>
    requires (
        is_last_union && std::is_nothrow_invocable_r_v<second_type, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_type_t<second_type>, F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<second_type, F,
        Args...>)
        : second(__RXX invoke_r<second_type>(
              __RXX forward<F>(f), __RXX forward<Args>(args)...)) {}

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
    using src_type = std::remove_cvref_t<U>;
    static_assert(template_size_v<T> == template_size_v<src_type>);
    return iota_table_for<src_type>(
        [&]<size_t I>(size_constant<I>) {
            return T(std::in_place_index<I>,
                __RXX forward<U>(arg).template get<I>());
        },
        index);
}

template <typename T>
struct array_t {
    T x[1];
};
template <typename T, typename>
struct variant_overload {
public:
    void test() noexcept = delete;
};
template <size_t I, typename T>
struct variant_typeid {};

#if RXX_COMPILER_GCC
RXX_DISABLE_WARNING_PUSH()
RXX_DISABLE_WARNING("-Wattributes")
#endif
template <typename T, size_t I, typename T_i>
requires std::is_arithmetic_v<T_i> &&
    requires(T val) { array_t<T_i>{{__RXX move(val)}}; }
struct variant_overload<T, variant_typeid<I, T_i>> {
    __RXX_HIDE_FROM_ABI static std::integral_constant<size_t, I> test(
        T_i) noexcept;
};

template <typename T, size_t I, typename T_i>
requires (!std::is_arithmetic_v<T_i>)
struct variant_overload<T, variant_typeid<I, T_i>> {
    __RXX_HIDE_FROM_ABI static std::integral_constant<size_t, I> test(
        T_i) noexcept;
};
#if RXX_COMPILER_GCC
RXX_DISABLE_WARNING_POP()
#endif

template <typename T, typename... Vs>
struct variant_overload_for_t;
template <typename T, size_t... Is, typename... Vs>
struct variant_overload_for_t<T, variant_typeid<Is, Vs>...> :
    variant_overload<T, variant_typeid<Is, Vs>>... {
    using variant_overload<T, variant_typeid<Is, Vs>>::test...;
};
template <typename T, typename... Ts, size_t... Is>
auto make_variant_overload_for_impl(__RXX index_sequence<Is...>) noexcept
    -> variant_overload_for_t<T, variant_typeid<Is, Ts>...>;

template <typename T, typename... Ts>
using make_variant_overload_for_t RXX_NODEBUG =
    decltype(make_variant_overload_for_impl<T, Ts...>(
        __RXX index_sequence_for<Ts...>{}));

template <typename...>
using void_t = void;
template <typename T, typename, typename = void>
inline constexpr size_t conversion_index = variant_npos;

template <typename T, template <typename...> class V, typename... Ts>
inline constexpr size_t conversion_index<T, V<Ts...>,
    void_t<decltype(make_variant_overload_for_t<T, Ts...>::test(
        std::declval<T>()))>> =
    decltype(make_variant_overload_for_t<T, Ts...>::test(
        std::declval<T>()))::value;

template <typename T, typename V>
using conversion_type = template_element_t<conversion_index<T, V>, V>;

template <typename T>
inline constexpr bool is_variant_ctor_tag = false;
template <typename T>
inline constexpr bool is_variant_ctor_tag<T const> = is_variant_ctor_tag<T>;
template <typename T>
inline constexpr bool is_variant_ctor_tag<T const volatile> =
    is_variant_ctor_tag<T>;
template <typename T>
inline constexpr bool is_variant_ctor_tag<T volatile> = is_variant_ctor_tag<T>;
template <>
inline constexpr bool is_variant_ctor_tag<std::in_place_t> = true;
template <typename T>
inline constexpr bool is_variant_ctor_tag<std::in_place_type_t<T>> = true;
template <size_t I>
inline constexpr bool is_variant_ctor_tag<std::in_place_index_t<I>> = true;
template <>
inline constexpr bool is_variant_ctor_tag<__RXX generating_t> = true;
template <typename T>
inline constexpr bool is_variant_ctor_tag<__RXX generating_type_t<T>> = true;
template <size_t I>
inline constexpr bool is_variant_ctor_tag<__RXX generating_index_t<I>> = true;

template <typename... Ts>
class variant_base;

template <typename U, typename... Ts>
concept conversion_argument_for =
    !std::same_as<__RXX variant<Ts...>, std::remove_cvref_t<U>> &&
    !is_variant_ctor_tag<std::remove_cvref_t<U>>;

template <typename... Ts>
class variant_base {
    static_assert(sizeof...(Ts) > 0, "Empty variant is not allowed");
    static_assert((... && std::is_destructible_v<Ts>), "Invalid type");
    static_assert((... && !std::is_reference_v<Ts>), "Invalid type");
    static_assert((... && !std::is_function_v<Ts>), "Invalid type");
    static_assert((... && !is_variant_ctor_tag<Ts>), "Invalid type");
    static consteval auto choose_index() noexcept {
        if constexpr (sizeof...(Ts) <= static_cast<unsigned char>(-1)) {
            return static_cast<unsigned char>(0);
        } else {
            return static_cast<unsigned short>(0);
        }
    }

    using union_type = multi_union<Ts..., valueless_var_t>;
    using index_type = decltype(choose_index());
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
            : union_(std::in_place, tag, __RXX forward<Args>(args)...)
            , index_(template_index_v<T, union_type>) {}

        template <size_t I, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            std::in_place_index_t<I> tag, Args&&... args)
            : union_(std::in_place, tag, __RXX forward<Args>(args)...)
            , index_(I) {}

        template <typename T, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            generating_type_t<T> tag, F&& callable, Args&&... args)
            : union_(std::in_place, tag, __RXX forward<F>(callable),
                  __RXX forward<Args>(args)...)
            , index_(template_index_v<T, union_type>) {}

        template <size_t I, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            generating_index_t<I> tag, F&& callable, Args&&... args)
            : union_(std::in_place, tag, __RXX forward<F>(callable),
                  __RXX forward<Args>(args)...)
            , index_(I) {}

        template <typename U>
        __RXX_HIDE_FROM_ABI constexpr container(dispatch_var_t, size_t index,
            U&& arg) noexcept(noexcept(make_from_multi_union<union_type>(index,
            std::declval<U>())))
        requires (allow_external_overlap)
            : union_(generating,
                  [&]() {
                      return make_from_multi_union<union_type>(
                          index, __RXX forward<U>(arg));
                  })
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

        template <size_t I, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr template_element_t<I, union_type>*
        construct_union(std::in_place_index_t<I> tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<union_type,
                std::in_place_index_t<I>, Args...>)
        requires (allow_external_overlap)
        {
            static_assert(I < template_size_v<union_type>);
            auto ptr = __RXX construct_at(RXX_BUILTIN_addressof(union_.data),
                tag, __RXX forward<Args>(args)...);
            index_ = I;
            return RXX_BUILTIN_addressof(ptr->template get<I>());
        }

        template <size_t I, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr template_element_t<I, union_type>*
        construct_union(generating_index_t<I> tag, F&& func,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<union_type,
            generating_index_t<I>, F, Args...>)
        requires (allow_external_overlap)
        {
            static_assert(I < template_size_v<union_type>);
            auto ptr = __RXX construct_at(RXX_BUILTIN_addressof(union_.data),
                tag, __RXX forward<F>(func), __RXX forward<Args>(args)...);
            index_ = I;
            return RXX_BUILTIN_addressof(ptr->template get<I>());
        }

        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
        overlappable_if<place_index_in_tail, union_type> union_;
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) index_type index_;

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
        static_assert(template_size_v<union_type> ==
            template_size_v<std::remove_cvref_t<U>>);
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
    requires std::is_default_constructible_v<
        template_element_t<0, variant_base>>
        : variant_base(std::in_place_index<0>) {}

    __RXX_HIDE_FROM_ABI constexpr ~variant_base() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base const&) noexcept
    requires (... && std::is_trivially_copy_constructible_v<Ts>)
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base(
        variant_base const& other) noexcept((... &&
        std::is_nothrow_copy_constructible_v<Ts>))
    requires ((... && std::is_copy_constructible_v<Ts>) &&
        !(... && std::is_trivially_copy_constructible_v<Ts>))
        : variant_base(dispatch_var, other.base_index(), other.union_ref()) {}

    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base&&) noexcept
    requires (... && std::is_trivially_move_constructible_v<Ts>)
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base(variant_base&& other) noexcept(
        (... && std::is_nothrow_move_constructible_v<Ts>))
    requires ((... && std::is_move_constructible_v<Ts>) &&
        !(... && std::is_trivially_move_constructible_v<Ts>))
        : variant_base(dispatch_var, other.base_index(),
              __RXX move(other.union_ref())) {}

    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base const&) noexcept
    requires (... && std::is_trivially_copy_constructible_v<Ts>) &&
        (... && std::is_trivially_copy_assignable_v<Ts>) &&
        (... && std::is_trivially_destructible_v<Ts>)
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base&
    operator=(variant_base const& other) noexcept(
        (... && std::is_nothrow_copy_assignable_v<Ts>) &&
        (... && std::is_nothrow_copy_constructible_v<Ts>))
    requires ((... && std::is_copy_assignable_v<Ts>) &&
        (... && std::is_copy_constructible_v<Ts>) &&
        !((... && std::is_trivially_copy_constructible_v<Ts>) &&
            (... && std::is_trivially_copy_assignable_v<Ts>) &&
            (... && std::is_trivially_destructible_v<Ts>)))
    {
        if (this->base_index() != other.base_index()) {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    using i_type = template_element_t<I, union_type>;
                    if constexpr (std::same_as<i_type, valueless_var_t>) {
                        this->reinitialize_value<I>();
                    } else if constexpr (std::is_nothrow_copy_constructible_v<
                                             i_type> ||
                        !std::is_nothrow_move_constructible_v<i_type>) {
                        this->reinitialize_value<I>(
                            other.template value_ref<I>());
                    } else {
                        i_type copy(other.template value_ref<I>());
                        this->reinitialize_value<I>(__RXX move(copy));
                    }
                },
                other.base_index());
        } else {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->template value_ref<I>() =
                        other.template value_ref<I>();
                },
                other.base_index());
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base&&) noexcept
    requires (... && std::is_trivially_move_constructible_v<Ts>) &&
        (... && std::is_trivially_move_assignable_v<Ts>) &&
        (... && std::is_trivially_destructible_v<Ts>)
    = default;
    __RXX_HIDE_FROM_ABI constexpr variant_base&
    operator=(variant_base&& other) noexcept(
        (... && std::is_nothrow_move_assignable_v<Ts>) &&
        (... && std::is_nothrow_move_constructible_v<Ts>))
    requires ((... && std::is_move_assignable_v<Ts>) &&
        (... && std::is_move_constructible_v<Ts>) &&
        !((... && std::is_trivially_move_constructible_v<Ts>) &&
            (... && std::is_trivially_move_assignable_v<Ts>) &&
            (... && std::is_trivially_destructible_v<Ts>)))
    {
        if (this->base_index() != other.base_index()) {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->reinitialize_value<I>(
                        __RXX move(other).template value_ref<I>());
                },
                other.base_index());
        } else {
            iota_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->template value_ref<I>() =
                        __RXX move(other).template value_ref<I>();
                },
                other.base_index());
        }

        return *this;
    }

    template <typename U, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_type_t<U>, Args...>)
        : container_(std::in_place, tag, __RXX forward<Args>(args)...) {}

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        std::in_place_index_t<I> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_index_t<I>, Args...>)
        : container_(std::in_place, tag, __RXX forward<Args>(args)...) {}

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
        : container_(std::in_place, tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...) {}

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

protected:
    __RXX_HIDE_FROM_ABI constexpr size_t base_index() const noexcept {
        return container_.data.index_;
    }

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    reinitialize_value(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        using target_type = template_element_t<I, union_type>;

        if constexpr (std::is_nothrow_constructible_v<target_type, Args...>) {
            destroy();
            return construct_value<I>(__RXX forward<Args>(args)...);
        } else if constexpr (std::is_scalar_v<target_type>) {
            // potentially throwing conversion
            target_type tmp(__RXX forward<Args>(args)...);
            destroy();
            return construct_value<I>(tmp);
        } else if constexpr (sizeof(target_type) <= 256 &&
            std::is_trivially_copyable_v<target_type> &&
            std::is_move_assignable_v<variant_base>) {
            // potentially throwing conversion
            variant_base tmp(
                std::in_place_index<I>, __RXX forward<Args>(args)...);
            *this = __RXX move(tmp);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else RXX_TRY {
            destroy();
            return construct_value<I>(__RXX forward<Args>(args)...);
        } RXX_CATCH(...) {
            construct_value<sizeof...(Ts)>();
            RXX_RETHROW();
        }
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    regenerate_value(F&& callable, Args&&... args) noexcept(
        std::is_nothrow_invocable_r_v<template_element_t<I, union_type>, F,
            Args...>) {
        using target_type = template_element_t<I, union_type>;

        if constexpr (std::is_nothrow_invocable_r_v<target_type, F, Args...>) {
            destroy();
            return generate_value<I>(
                __RXX forward<F>(callable), __RXX forward<Args>(args)...);
        } else if constexpr (std::is_scalar_v<target_type>) {
            // potentially throwing conversion
            target_type tmp = __RXX invoke_r<target_type>(
                __RXX forward<F>(callable), __RXX forward<Args>(args)...);
            destroy();
            return construct_value<I>(tmp);
        } else if constexpr (sizeof(target_type) <= 256 &&
            std::is_trivially_copyable_v<target_type> &&
            std::is_move_assignable_v<variant_base>) {
            // potentially throwing conversion
            variant_base tmp(generating_index<I>, __RXX forward<F>(callable),
                __RXX forward<Args>(args)...);
            *this = __RXX move(tmp);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else RXX_TRY {
            destroy();
            return generate_value<I>(
                __RXX forward<F>(callable), __RXX forward<Args>(args)...);
        } RXX_CATCH(...) {
            construct_value<sizeof...(Ts)>();
            RXX_RETHROW();
        }
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

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr variant_base(dispatch_var_t tag, size_t index,
        U&& arg) noexcept(std::is_nothrow_constructible_v<container,
        dispatch_var_t, size_t, U>)
    requires (allow_external_overlap)
        : container_{std::in_place, tag, index, __RXX forward<U>(arg)} {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr variant_base(dispatch_var_t, size_t index,
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
            __RXX construct_at(RXX_BUILTIN_addressof(container_.data),
                std::in_place_index<I>, __RXX forward<Args>(args)...);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else {
            return container_.data.construct_union(
                std::in_place_index<I>, __RXX forward<Args>(args)...);
        }
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    generate_value(F&& callable, Args&&... args) noexcept(
        std::is_nothrow_invocable_r_v<template_element_t<I, union_type>, F,
            Args...>) {
        if constexpr (place_index_in_tail) {
            __RXX construct_at(RXX_BUILTIN_addressof(container_.data),
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
            __RXX destroy_at(RXX_BUILTIN_addressof(container_.data));
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
    struct visit_barrier {};
    using first_type = template_element_t<0, variant>;
    using base_type = details::variant_base<Ts...>;
    template <typename... Us>
    friend class variant;

public:
    __RXX_HIDE_FROM_ABI constexpr variant() noexcept(
        std::is_nothrow_default_constructible_v<base_type>)
    requires std::is_default_constructible_v<base_type>
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant(variant const&) = default;
    __RXX_HIDE_FROM_ABI constexpr variant(variant&&) = default;
    __RXX_HIDE_FROM_ABI constexpr variant& operator=(variant const&) = default;
    __RXX_HIDE_FROM_ABI constexpr variant& operator=(variant&&) = default;
    __RXX_HIDE_FROM_ABI constexpr ~variant() = default;

    template <typename U, typename... Args>
    requires (template_count_v<U, variant> == 1) &&
        std::is_constructible_v<U, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<U, Args...>)
        : base_type(tag, __RXX forward<Args>(args)...) {}

    template <typename U, typename V, typename... Args>
    requires (template_count_v<U, variant> == 1) &&
        std::is_constructible_v<U, std::initializer_list<V>, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(std::in_place_type_t<U> tag,
        std::initializer_list<V> ilist,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<U,
        std::initializer_list<V>, Args...>)
        : base_type(tag, ilist, __RXX forward<Args>(args)...) {}

    template <size_t I, typename... Args>
    requires (I < sizeof...(Ts)) &&
        std::is_constructible_v<template_element_t<I, variant>, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(
        std::in_place_index_t<I> tag, Args&&... args) noexcept(std::
            is_nothrow_constructible_v<template_element_t<I, variant>, Args...>)
        : base_type(tag, __RXX forward<Args>(args)...) {}

    template <size_t I, typename U, typename... Args>
    requires (I < sizeof...(Ts)) &&
        std::is_constructible_v<template_element_t<I, variant>,
            std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(std::in_place_index_t<I> tag,
        std::initializer_list<U> ilist, Args&&... args) noexcept(std::
            is_nothrow_constructible_v<template_element_t<I, variant>,
                std::initializer_list<U>&, Args...>)
        : base_type(tag, ilist, __RXX forward<Args>(args)...) {}

    // Unfortunately, GCC does not perform RVO on these generating constructors
    // due to a compiler limitation; RVO is disabled for members declared with
    // no_unique_address. As a result, non-copyable-movable types cannot be
    // generated.
    template <typename U, typename F, typename... Args>
    requires (template_count_v<U, variant> == 1) &&
        details::generatable_from<U, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(generating_type_t<U> tag,
        F&& callable,
        Args&&... args) noexcept(details::nothrow_generatable_from<U, F,
        Args...>)
        : base_type(tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...) {}

    template <size_t I, typename F, typename... Args>
    requires (I < sizeof...(Ts)) &&
        details::generatable_from<template_element_t<I, variant>, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit variant(generating_index_t<I> tag,
        F&& callable, Args&&... args) noexcept(details::
            nothrow_generatable_from<template_element_t<I, variant>, F,
                Args...>)
        : base_type(tag, __RXX forward<F>(callable),
              __RXX forward<Args>(args)...) {}

    // Use SFINAE for these as they have an edge case that lead to recursive
    // evaluation which is not possible using concepts
    template <details::conversion_argument_for<Ts...> U,
        typename = details::conversion_type<U, variant>,
        typename = std::enable_if_t<
            std::is_constructible_v<details::conversion_type<U, variant>, U>>>
    __RXX_HIDE_FROM_ABI constexpr variant(U&& arg) noexcept(
        std::is_nothrow_constructible_v<details::conversion_type<U, variant>,
            U>)
        : variant(std::in_place_index<details::conversion_index<U, variant>>,
              __RXX forward<U>(arg)) {}

    // Use SFINAE for these as they have an edge case that lead to recursive
    // evaluation which is not possible using concepts
    template <details::conversion_argument_for<Ts...> U,
        typename = details::conversion_type<U, variant>,
        typename = std::enable_if_t<
            std::is_constructible_v<details::conversion_type<U, variant>, U> &&
            std::is_assignable_v<details::conversion_type<U, variant>&, U>>>
    __RXX_HIDE_FROM_ABI constexpr variant& operator=(U&& arg) noexcept(
        std::is_nothrow_constructible_v<details::conversion_type<U, variant>,
            U> &&
        std::is_nothrow_move_constructible_v<
            details::conversion_type<U, variant>> &&
        std::is_nothrow_assignable_v<details::conversion_type<U, variant>&,
            U>) {
        constexpr auto idx = details::conversion_index<U, variant>;
        if (idx == index()) {
            this->template value_ref<idx>() = __RXX forward<U>(arg);
        } else if constexpr (std::is_nothrow_constructible_v<
                                 details::conversion_type<U, variant>, U> ||
            !std::is_nothrow_move_constructible_v<
                details::conversion_type<U, variant>>) {
            this->template reinitialize_value<idx>(__RXX forward<U>(arg));
        } else {
            this->template reinitialize_value<idx>(
                details::conversion_type<U, variant>(__RXX forward<U>(arg)));
        }

        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr size_t index() const noexcept {
        return valueless_by_exception() ? variant_npos
                                        : base_type::base_index();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool valueless_by_exception() const noexcept {
        return base_type::base_index() == sizeof...(Ts);
    }

    template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...> &&
        (template_count_v<T, variant> == 1)
    __RXX_HIDE_FROM_ABI constexpr T& emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        return *this->template reinitialize_value<template_index_v<T, variant>>(
            __RXX forward<Args>(args)...);
    }

    template <typename T, typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...> &&
        (template_count_v<T, variant> == 1)
    __RXX_HIDE_FROM_ABI constexpr T& emplace(std::initializer_list<U> ilist,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>) {
        return *this->template reinitialize_value<template_index_v<T, variant>>(
            ilist, __RXX forward<Args>(args)...);
    }

    template <size_t I, typename... Args>
    requires (I < sizeof...(Ts)) &&
        std::is_constructible_v<template_element_t<I, variant>, Args...>
    __RXX_HIDE_FROM_ABI constexpr template_element_t<I, variant>&
    emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, variant>,
            Args...>) {
        return *this->template reinitialize_value<I>(
            __RXX forward<Args>(args)...);
    }

    template <size_t I, typename U, typename... Args>
    requires (I < sizeof...(Ts)) &&
        std::is_constructible_v<template_element_t<I, variant>,
            std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI constexpr template_element_t<I, variant>&
    emplace(std::initializer_list<U> ilist, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, variant>,
            std::initializer_list<U>&, Args...>) {
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
        (... && std::is_nothrow_swappable_v<Ts>) &&
        (... && std::is_nothrow_move_constructible_v<Ts>))
    requires (... && std::is_swappable_v<Ts>) &&
        (... && std::is_move_constructible_v<Ts>)
    {
        if (index() == other.index()) {
            details::visit_table_for<variant>(
                [&]<size_t I>(details::size_constant<I>) {
                    if constexpr (I != variant_npos) {
                        ranges::swap(this->template value_ref<I>(),
                            other.template value_ref<I>());
                    }
                },
                index());
        } else if (base_type::base_index() < other.base_index()) {
            iota_table<sizeof...(Ts)>(
                [&]<size_t I>(details::size_constant<I>) {
                    iota_table<sizeof...(Ts) + 1>(
                        [&]<size_t J>
                        requires (I < J)
                        (details::size_constant<J>) {
                            auto tmp = [&]() {
                                return __RXX move(
                                    this->template value_ref<I>());
                            }();
                            using i_type = std::remove_cvref_t<
                                decltype(this->template value_ref<I>())>;

                            if constexpr (!std::is_nothrow_move_constructible_v<
                                              i_type>) {
                                this->template reinitialize_value<J>(
                                    __RXX move(
                                        other.template value_ref<J>()));
                                other.template reinitialize_value<I>(
                                    __RXX move(tmp));

                            } else RXX_TRY {
                                this->template reinitialize_value<J>(
                                    __RXX move(
                                        other.template value_ref<J>()));
                                other.template reinitialize_value<I>(
                                    __RXX move(tmp));
                            } RXX_CATCH(...) {
                                this->template reinitialize_value<I>(
                                    __RXX move(tmp));
                                RXX_RETHROW();
                            }
                        },
                        other.base_index());
                },
                base_type::base_index());
        } else {
            other.swap(*this);
        }
    }

#if __cpp_explicit_this_parameter >= 202110L && (RXX_CXX23 | RXX_COMPILER_CLANG)

#  if RXX_COMPILER_CLANG && !RXX_CXX23
    RXX_DISABLE_WARNING_PUSH()
    RXX_DISABLE_WARNING("-Wc++23-extensions")
#  endif
    // NOLINTBEGIN
    template <visit_barrier = visit_barrier{}, typename Self, typename F>
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(
        this Self&& self, F&& visitor)
    requires (... && std::invocable<F, copy_cvref_t<Self, Ts>>)
    {
        if (((copy_cvref_t<Self, variant const&>)self)
                .valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor(
            __RXX forward<F>(visitor), (copy_cvref_t<Self&&, variant>)(self));
    }

    template <typename R, typename Self, typename F>
    __RXX_HIDE_FROM_ABI constexpr R visit(this Self&& self, F&& visitor)
    requires (... && std::is_invocable_r_v<R, F, copy_cvref_t<Self, Ts>>)
    {
        if (((copy_cvref_t<Self, variant const&>)self)
                .valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor<R>(
            __RXX forward<F>(visitor), (copy_cvref_t<Self&&, variant>)(self));
    }
    // NOLINTEND
#  if RXX_COMPILER_CLANG && !RXX_CXX23
    RXX_DISABLE_WARNING_POP()
#  endif

#else

    template <visit_barrier = visit_barrier{}, typename F>
    requires (... && std::invocable<F, Ts&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) & {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor(__RXX forward<F>(visitor), *this);
    }

    template <visit_barrier = visit_barrier{}, typename F>
    requires (... && std::invocable<F, Ts const&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) const& {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor(__RXX forward<F>(visitor), *this);
    }

    template <visit_barrier = visit_barrier{}, typename F>
    requires (... && std::invocable<F, Ts &&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) && {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor(
            __RXX forward<F>(visitor), __RXX move(*this));
    }

    template <visit_barrier = visit_barrier{}, typename F>
    requires (... && std::invocable<F, Ts const &&>)
    __RXX_HIDE_FROM_ABI constexpr decltype(auto) visit(F&& visitor) const&& {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor(
            __RXX forward<F>(visitor), __RXX move(*this));
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts&>)
    __RXX_HIDE_FROM_ABI constexpr R visit(F&& visitor) & {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor<R>(
            __RXX forward<F>(visitor), *this);
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts const&>)
    __RXX_HIDE_FROM_ABI constexpr R visit(F&& visitor) const& {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor<R>(
            __RXX forward<F>(visitor), *this);
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts &&>)
    __RXX_HIDE_FROM_ABI constexpr R visit(F&& visitor) && {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor<R>(
            __RXX forward<F>(visitor), __RXX move(*this));
    }

    template <typename R, typename F>
    requires (... && std::invocable<F, Ts const &&>)
    __RXX_HIDE_FROM_ABI constexpr R visit(F&& visitor) const&& {
        if (valueless_by_exception()) {
            RXX_THROW(bad_variant_access());
        }

        return __RXX details::variant_visitor<R>(
            __RXX forward<F>(visitor), __RXX move(*this));
    }

#endif
};

template <typename... Ts>
__RXX_HIDE_FROM_ABI constexpr void swap(
    variant<Ts...>& left, variant<Ts...>& right) = delete;

template <typename... Ts>
requires requires(variant<Ts...>& val) { val.swap(val); }
__RXX_HIDE_FROM_ABI constexpr void swap(variant<Ts...>& left,
    variant<Ts...>& right) noexcept(noexcept(left.swap(right))) {
    left.swap(right);
}

template <typename... Ts>
requires (!requires(variant<Ts...>& val) { val.swap(val); }) &&
    std::is_move_constructible_v<variant<Ts...>> &&
    std::is_move_assignable_v<variant<Ts...>>
__RXX_HIDE_FROM_ABI constexpr void
swap(variant<Ts...>& left, variant<Ts...>& right) noexcept(
    std::is_nothrow_move_constructible_v<variant<Ts...>> &&
    std::is_nothrow_move_assignable_v<variant<Ts...>>) {
    right = __RXX exchange(left, __RXX move(right));
}

RXX_DEFAULT_NAMESPACE_END
