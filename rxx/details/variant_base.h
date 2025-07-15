// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/details/construct_at.h"
#include "rxx/details/destroy_at.h"
#include "rxx/details/jump_table.h"
#include "rxx/details/overlappable_if.h"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {
template <size_t I>
using size_constant = std::integral_constant<size_t, I>;

template <typename...>
union multi_union;

template <size_t I, typename List>
struct template_element {};

template <size_t I, typename List>
using template_element_t = typename template_element<I, List>::type;

template <template <typename...> class List, typename Head, typename... Tail>
struct template_element<0, List<Head, Tail...>> {
    using type RXX_NODEBUG = Head;
};

template <typename List>
inline constexpr size_t template_size_v = 0;

template <template <typename...> class List, typename... Args>
inline constexpr size_t template_size_v<List<Args...>> = sizeof...(Args);

template <typename T, typename TList>
inline constexpr size_t template_index_v = static_cast<size_t>(-1);

template <typename T, template <typename...> class TList, typename... Tail>
inline constexpr size_t template_index_v<T, TList<T, Tail...>> = 0;

template <typename T, template <typename...> class TList, typename Head,
    typename... Tail>
inline constexpr size_t template_index_v<T, TList<Head, Tail...>> =
    1 + template_index_v<T, TList<Tail...>>;

template <typename List>
struct template_size {};

template <template <typename...> class List, typename... Args>
struct template_size<List<Args...>> :
    std::integral_constant<size_t, template_size_v<List<Args...>>> {};

template <size_t I, template <typename...> class List, typename Head,
    typename... Tail>
struct template_element<I, List<Head, Tail...>> :
    template_element<I - 1, List<Tail...>> {};

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
        : first{std::forward<Args>(args)...} {}

    template <typename... Args>
    requires std::is_constructible_v<first_type, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<0>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<first_type,
        Args...>)
        : first{std::forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires std::is_nothrow_invocable_r_v<first_type, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<0>,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : first{invoke_r<first_type>(
              std::forward<F>(f), std::forward<Args>(args)...)} {}

    template <typename F, typename... Args>
    requires std::is_nothrow_invocable_r_v<first_type, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_type_t<first_type>, F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : first{invoke_r<first_type>(
              std::forward<F>(f), std::forward<Args>(args)...)} {}

    template <typename U, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, std::in_place_type_t<U>, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<U> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        std::in_place_type_t<U>, Args...>)
        : second{tag, std::forward<Args>(args)...} {}

    template <size_t N, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, std::in_place_index_t<N - 1>,
            Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<N>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        std::in_place_index_t<N>, Args...>)
        : second{std::in_place_index<N - 1>, std::forward<Args>(args)...} {}

    template <size_t N, typename F, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, generating_index_t<N - 1>, F,
            Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<N>,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{generating_index<N - 1>, std::forward<F>(f),
              std::forward<Args>(args)...} {}

    template <typename U, typename F, typename... Args>
    requires (!is_last_union &&
        std::is_constructible_v<second_type, generating_type_t<U>, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_type_t<U> tag,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{tag, std::forward<F>(f), std::forward<Args>(args)...} {}

    template <typename... Args>
    requires (is_last_union && std::is_constructible_v<second_type, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        std::in_place_type_t<second_type>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        Args...>)
        : second{std::forward<Args>(args)...} {}

    template <typename... Args>
    requires (is_last_union && std::is_constructible_v<second_type, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(std::in_place_index_t<1>,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<second_type,
        Args...>)
        : second{std::forward<Args>(args)...} {}

    template <typename F, typename... Args>
    requires (
        is_last_union && std::is_nothrow_invocable_r_v<second_type, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(generating_index_t<1>,
        F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{invoke_r<second_type>(
              std::forward<F>(f), std::forward<Args>(args)...)} {}

    template <typename F, typename... Args>
    requires (
        is_last_union && std::is_nothrow_invocable_r_v<second_type, F, Args...>)
    __RXX_HIDE_FROM_ABI explicit constexpr multi_union(
        generating_type_t<second_type>, F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : second{invoke_r<second_type>(
              std::forward<F>(f), std::forward<Args>(args)...)} {}

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
            return forward_like<Self>(self.first);
        } else if constexpr (is_last_union) {
            return forward_like<Self>(self.second);
        } else {
            return forward_like<Self>(self.second).template get<I - 1>();
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
            return std::move(first);
        } else if constexpr (is_last_union) {
            return std::move(second);
        } else {
            return std::move(second).template get<I - 1>();
        }
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN, NODISCARD)
    constexpr auto get() && noexcept -> decltype(auto) {
        static_assert(I <= sizeof...(Tail));
        if constexpr (I == 0) {
            return std::move(first);
        } else if constexpr (is_last_union) {
            return std::move(second);
        } else {
            return std::move(second).template get<I - 1>();
        }
    }

#endif

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) first_type first;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) second_type second;
};

template <typename T>
__RXX_HIDE_FROM_ABI inline constexpr auto jump_table_for = jump_table<size_t>{};

template <template <typename...> class T, typename... Args>
__RXX_HIDE_FROM_ABI inline constexpr auto jump_table_for<T<Args...>> =
    []<size_t... Is>(std::index_sequence<Is...>) {
        return jump_table<size_t, Is...>{};
    }(std::index_sequence_for<Args...>{});

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr T make_from_multi_union(size_t index,
    U&& arg) noexcept([]<size_t... Is>(std::index_sequence<Is...>) {
    return (... &&
        std::is_nothrow_constructible_v<T, std::in_place_index_t<Is>,
            decltype(std::declval<U>().template get<Is>())>);
}(std::make_index_sequence<template_size_v<T>>{})) {
    static_assert(template_size_v<T> == template_size_v<U>);
    return jump_table_for<U>(
        [&]<size_t I>(size_constant<I>) {
            return T{
                std::in_place_index<I>, std::forward<U>(arg).template get<I>()};
        },
        index);
}

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
            : union_{std::in_place, tag, std::forward<Args>(args)...}
            , index_{template_index_v<T, union_type>} {}

        template <size_t I, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            std::in_place_index_t<I> tag, Args&&... args)
            : union_{std::in_place, tag, std::forward<Args>(args)...}
            , index_{I} {}

        template <typename T, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            generating_type_t<T> tag, F&& callable, Args&&... args)
            : union_{std::in_place, tag, std::forward<F>(callable),
                  std::forward<Args>(args)...}
            , index_{template_index_v<T, union_type>} {}

        template <size_t I, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr container(
            generating_index_t<I> tag, F&& callable, Args&&... args)
            : union_{std::in_place, tag, std::forward<F>(callable),
                  std::forward<Args>(args)...}
            , index_{I} {}

        template <typename U>
        __RXX_HIDE_FROM_ABI constexpr container(dispatch_t, size_t index,
            U&& arg) noexcept(noexcept(make_from_multi_union<union_type>(index,
            std::declval<U>())))
        requires allow_external_overlap
            : union_{generating,
                  [&]() {
                      return make_from_multi_union<union_type>(
                          index, std::forward<U>(arg));
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
            __RXX ranges::details::destroy_at(
                RXX_BUILTIN_addressof(union_.data));
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires (allow_external_overlap &&
            (... || !std::is_trivially_destructible_v<Ts>))
        {
            destroy_member();
            __RXX ranges::details::destroy_at(
                RXX_BUILTIN_addressof(union_.data));
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
                std::forward<Args>(args)...);
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
                std::forward<Args>(args)...);
            index_ = I;
            return RXX_BUILTIN_addressof(ptr->template get<I>());
        }

        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS)
        overlappable_if<place_index_in_tail, union_type> union_;
        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS) index_type index_;

    private:
        __RXX_HIDE_FROM_ABI constexpr void destroy_member() noexcept {
            jump_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    destroy_at(
                        RXX_BUILTIN_addressof(union_.data.template get<I>()));
                },
                static_cast<size_t>(index_));
        }
    };

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr container make_container(size_t index,
        U&& arg) noexcept([]<size_t... Is>(std::index_sequence<Is...>) {
        return (... &&
            std::is_nothrow_constructible_v<container,
                std::in_place_index_t<Is>,
                decltype(std::declval<U>().template get<Is>())>);
    }(std::make_index_sequence<template_size_v<U>>{}))
    requires place_index_in_tail
    {
        static_assert(template_size_v<union_type> == template_size_v<U>);
        return jump_table_for<union_type>(
            [&]<size_t I>(size_constant<I>) -> container {
                return container{std::in_place_index<I>,
                    std::forward<U>(arg).template get<I>()};
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
        : variant_base(dispatch, other.index(), std::move(other.union_ref())) {}

    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr variant_base& operator=(
        variant_base const&) noexcept
    requires ((... && std::is_copy_assignable_v<Ts>) &&
                 (... && std::is_trivially_copy_assignable_v<Ts>))
    = default;

    __RXX_HIDE_FROM_ABI constexpr variant_base&
    operator=(variant_base const& other) noexcept(
        (... && std::is_nothrow_copy_assignable_v<Ts>)&&(
            ... && std::is_nothrow_copy_constructible_v<Ts>))
    requires ((... && std::is_copy_assignable_v<Ts>) &&
        !(... && std::is_trivially_copy_assignable_v<Ts>))
    {
        if (this->index() != other.index()) {
            jump_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->reinitialize_value<I>(other.template value_ref<I>());
                },
                other.index());
        } else {
            jump_table_for<union_type>(
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
        (... && std::is_nothrow_move_assignable_v<Ts>)&&(
            ... && std::is_nothrow_move_constructible_v<Ts>))
    requires ((... && std::is_move_assignable_v<Ts>) &&
        !(... && std::is_trivially_move_assignable_v<Ts>))
    {
        if (this->index() != other.index()) {
            jump_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->reinitialize_value<I>(
                        std::move(other).template value_ref<I>());
                },
                other.index());
        } else {
            jump_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    this->template value_ref<I>() =
                        std::move(other).template value_ref<I>();
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
        : container_{std::in_place, tag, std::forward<Args>(args)...} {}

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        std::in_place_index_t<I> tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_index_t<I>, Args...>)
        : container_{std::in_place, tag, std::forward<Args>(args)...} {}

    template <typename U, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        generating_type_t<U> tag, F&& callable,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        generating_type_t<U>, Args...>)
        : container_{std::in_place, tag, std::forward<F>(callable),
              std::forward<Args>(args)...} {}

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr explicit variant_base(
        generating_index_t<I> tag, F&& callable,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<container,
        generating_index_t<I>, F, Args...>)
        : container_{std::in_place, tag, std::forward<F>(callable),
              std::forward<Args>(args)...} {}

    __RXX_HIDE_FROM_ABI constexpr size_t index() const noexcept {
        return container_.data.index_;
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
        return std::move(container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() && noexcept -> decltype(auto) {
        return std::move(container_.data.union_.data);
    }

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
        return std::move(union_ref().template get<I>());
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto value_ref() && noexcept -> decltype(auto) {
        static_assert(I < template_size_v<union_type>);
        return std::move(union_ref().template get<I>());
    }

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    reinitialize_value(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        using target_type = template_element_t<I, union_type>;

        destroy();
        if constexpr (std::is_nothrow_constructible_v<target_type, Args...>) {
            return construct_value<I>(std::forward<Args>(args)...);
        } else
            try {
                return construct_value<I>(std::forward<Args>(args)...);
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
                std::forward<F>(callable), std::forward<Args>(args)...);
        } else
            try {
                return generate_value<I>(
                    std::forward<F>(callable), std::forward<Args>(args)...);
            } catch (...) {
                construct_value<sizeof...(Ts)>();
                throw;
            }
    }

    __RXX_HIDE_FROM_ABI friend constexpr bool
    operator==(variant_base const& self, variant_base const& right) noexcept((
        ...&& noexcept(std::declval<Ts const&>() == std::declval<Ts const&>())))
    requires (... && std::equality_comparable<Ts>)
    {
        return self.index() == right.index() &&
            jump_table_for<union_type>(
                [&]<size_t I>(size_constant<I>) {
                    if constexpr (I >= sizeof...(Ts)) {
                        return true;
                    } else {
                        return self.template value_ref<I>() ==
                            right.template value_ref<I>();
                    }
                },
                self.index());
    }

    __RXX_HIDE_FROM_ABI friend constexpr auto operator<=>(
        variant_base const& self, variant_base const& right) noexcept
    requires (... && std::three_way_comparable<Ts>)
    {
        using result = std::common_comparison_category<
            std::compare_three_way_result_t<index_type>,
            std::compare_three_way_result_t<Ts>...>;

        auto cmp = self.index() <=> right.index();
        if (cmp != 0) {
            return [&]() -> result { return cmp; }();
        }

        return jump_table_for<union_type>(
            [&]<size_t I>(size_constant<I>) -> result {
                if constexpr (I >= sizeof...(Ts)) {
                    return 0 <=> 0;
                } else {
                    return self.template value_ref<I>() <=>
                        right.template value_ref<I>();
                }
            },
            self.index());
    }

private:
    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr variant_base(dispatch_t tag, size_t index,
        U&& arg) noexcept(std::is_nothrow_constructible_v<container, dispatch_t,
        size_t, U>)
    requires (allow_external_overlap)
        : container_{std::in_place, tag, index, std::forward<U>(arg)} {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr variant_base(dispatch_t, size_t index,
        U&& arg) noexcept(noexcept(make_container(index, std::declval<U>())))
    requires place_index_in_tail
        : container_{generating,
              [&]() { return make_container(index, std::forward<U>(arg)); }} {}

    template <size_t I, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto construct_value(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<template_element_t<I, union_type>,
            Args...>) {
        if constexpr (place_index_in_tail) {
            construct_at(RXX_BUILTIN_addressof(container_.data),
                std::in_place_index<I>, std::forward<Args>(args)...);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else {
            return container_.data.construct_union(
                std::in_place_index<I>, std::forward<Args>(args)...);
        }
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI constexpr auto
    generate_value(F&& callable, Args&&... args) noexcept(
        std::is_nothrow_invocable_r_v<template_element_t<I, union_type>, F,
            Args...>) {
        if constexpr (place_index_in_tail) {
            construct_at(RXX_BUILTIN_addressof(container_.data),
                generating_index<I>, std::forward<F>(callable),
                std::forward<Args>(args)...);
            return RXX_BUILTIN_addressof(value_ref<I>());
        } else {
            return container_.data.construct_union(generating_index<I>,
                std::forward<F>(callable), std::forward<Args>(args)...);
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
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
