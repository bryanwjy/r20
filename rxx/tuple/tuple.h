// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple/fwd.h"

#include "rxx/concepts/swap.h"
#include "rxx/ranges/get_element.h"
#include "rxx/tuple/utils.h"
#include "rxx/type_traits/is_explicit_constructible.h" // IWYU pragma: keep
#include "rxx/utility.h"

#include <array>   // IWYU pragma: keep
#include <compare> // IWYU pragma: keep
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details::tuple {

template <typename T>
inline constexpr bool is_tuple_v = false;
template <typename... T>
inline constexpr bool is_tuple_v<__RXX tuple<T...>> = true;

template <typename T>
concept unrecognized = !is_tuple_v<std::remove_cvref_t<T>> && tuple_like<T>;

template <typename S, size_t I>
struct element {
    using type = template_element_t<I, S>;

    __RXX_HIDE_FROM_ABI constexpr element() noexcept(
        std::is_nothrow_default_constructible_v<type>) = default;
    __RXX_HIDE_FROM_ABI constexpr element(element const&) noexcept(
        std::is_nothrow_copy_constructible_v<type>) = default;
    __RXX_HIDE_FROM_ABI constexpr element(element&&) noexcept(
        std::is_nothrow_move_constructible_v<type>) = default;
    template <typename U>
    requires std::constructible_from<type, U>
    __RXX_HIDE_FROM_ABI constexpr element(U&& other) noexcept(
        std::is_nothrow_constructible_v<type, U>)
        : data(__RXX forward<U>(other)) {}
    __RXX_HIDE_FROM_ABI constexpr element& operator=(element const&) noexcept(
        std::is_nothrow_copy_assignable_v<type>) = default;
    __RXX_HIDE_FROM_ABI constexpr element& operator=(element&&) noexcept(
        std::is_nothrow_move_assignable_v<type>) = default;
    template <typename U>
    requires std::assignable_from<type&, U>
    __RXX_HIDE_FROM_ABI constexpr element& operator=(U&& other) noexcept(
        std::is_nothrow_assignable_v<type&, U>) {
        data = __RXX forward<U>(other);
        return *this;
    }

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) type data;
};

template <typename D, size_t... Is>
class storage : protected element<D, Is>... {
    static constexpr size_t element_count = sizeof...(Is);

public:
    __RXX_HIDE_FROM_ABI constexpr ~storage() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr storage() = default;
    __RXX_HIDE_FROM_ABI constexpr storage(storage const&) = default;
    __RXX_HIDE_FROM_ABI constexpr storage(storage&&) = default;
    __RXX_HIDE_FROM_ABI constexpr storage& operator=(storage const&) = default;
    __RXX_HIDE_FROM_ABI constexpr storage& operator=(storage&&) = default;

protected:
    __RXX_HIDE_FROM_ABI constexpr storage()
    requires (... && std::default_initializable<template_element_t<Is, D>>)
        : element<D, Is>{}... {}

    template <typename... Us>
    requires (sizeof...(Us) == element_count) &&
        (... && std::constructible_from<template_element_t<Is, D>, Us>)
    __RXX_HIDE_FROM_ABI constexpr storage(Us&&... args)
        : element<D, Is>(__RXX forward<Us>(args))... {}

    /** Skip allocator ctors **/

    __RXX_HIDE_FROM_ABI constexpr void swap(storage& other)
    requires (... &&
        std::swappable_with<template_element_t<Is, D>&,
            template_element_t<Is, D>&>)
    {
        (...,
            ranges::swap(
                this->template get_data<Is>(), other.template get_data<Is>()));
    }

    __RXX_HIDE_FROM_ABI constexpr void swap(storage const& other) const
    requires (... &&
        std::swappable_with<template_element_t<Is, D> const&,
            template_element_t<Is, D> const&>)
    {
        (...,
            ranges::swap(
                this->template get_data<Is>(), other.template get_data<Is>()));
    }

    template <typename... Us>
    requires (sizeof...(Us) == element_count)
    __RXX_HIDE_FROM_ABI constexpr D& assign(Us&&... others) {
        (..., (this->template get_data<Is>() = __RXX forward<Us>(others)));
        return static_cast<D&>(*this);
    }

    template <typename... Us>
    requires (sizeof...(Us) == element_count)
    __RXX_HIDE_FROM_ABI constexpr D const& assign(Us&&... others) const {
        (..., (this->template get_data<Is>() = __RXX forward<Us>(others)));
        return static_cast<D const&>(*this);
    }

private:
    template <size_t I>
    requires requires { typename std::tuple_element_t<I, D>; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_data() && noexcept //
        RXX_LIFETIMEBOUND -> std::tuple_element_t<I, D>&& {
        return static_cast<std::tuple_element_t<I, D>&&>(
            static_cast<element<D, I>&>(*this).data);
    }

    template <size_t I>
    requires requires { typename std::tuple_element_t<I, D>; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_data() & noexcept //
        RXX_LIFETIMEBOUND -> std::tuple_element_t<I, D>& {
        return static_cast<element<D, I>&>(*this).data;
    }

    template <size_t I>
    requires requires { typename std::tuple_element_t<I, D>; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_data() const&& noexcept //
        RXX_LIFETIMEBOUND -> std::tuple_element_t<I, D> const&& {
        return static_cast<std::tuple_element_t<I, D> const&&>(
            static_cast<element<D, I> const&>(*this).data);
    }

    template <size_t I>
    requires requires { typename std::tuple_element_t<I, D>; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_data() const& noexcept //
        RXX_LIFETIMEBOUND -> std::tuple_element_t<I, D> const& {
        return static_cast<element<D, I> const&>(*this).data;
    }
};

template <typename D, size_t... Is>
__RXX_HIDE_FROM_ABI auto make_storage_for(
    __RXX index_sequence<Is...>) noexcept -> storage<D, Is...>;

template <typename D>
using storage_for = decltype(make_storage_for<D>(sequence_for<D>));

template <typename From, typename To>
inline constexpr bool is_element_convertible_v = false;

template <tuple_like From, tuple_like To>
requires (std::tuple_size_v<std::remove_cvref_t<From>> ==
             std::tuple_size_v<std::remove_cvref_t<To>>)
inline constexpr bool is_element_convertible_v<From, To> =
    []<size_t... Is>(__RXX index_sequence<Is...>) {
        if constexpr (std::is_reference_v<To>) {
            return (... &&
                std::is_convertible_v<decl_element_t<Is, From>,
                    decl_element_t<Is, To>>);
        } else {
            return (... &&
                std::is_convertible_v<decl_element_t<Is, From>,
                    std::tuple_element_t<Is, To>>);
        }
    }(sequence_for<std::remove_cvref_t<From>>);

} // namespace details::tuple

template <typename... Ts>
class tuple : private details::tuple::storage_for<tuple<Ts...>> {
    using base_type = details::tuple::storage_for<tuple>;
    friend base_type;

    template <typename Tuple, size_t... Is>
    __RXX_HIDE_FROM_ABI constexpr tuple(
        Tuple&& other, __RXX index_sequence<Is...>)
        : base_type{
              __RXX forward_like<Tuple>(ranges::get_element<Is>(other))...} {}

    template <typename Tuple>
    __RXX_HIDE_FROM_ABI static constexpr bool use_other_overload = []() {
        if constexpr (sizeof...(Ts) != 1) {
            return false;
        } else if constexpr (std::same_as<std::remove_cvref_t<Tuple>, tuple>) {
            return true; // Should use a copy/move constructor instead.
        } else {
            using First = std::tuple_element_t<0, tuple>;
            if constexpr (std::is_convertible_v<Tuple, First>) {
                return true;
            } else if constexpr (std::is_constructible_v<First, Tuple>) {
                return true;
            }
        }
        return false;
    }();

public:
    __RXX_HIDE_FROM_ABI constexpr ~tuple() noexcept = default;
    __RXX_HIDE_FROM_ABI explicit((... ||
        is_explicit_constructible_v<Ts>)) constexpr tuple() noexcept((... &&
        std::is_nothrow_default_constructible_v<Ts>)) = default;
    __RXX_HIDE_FROM_ABI constexpr tuple(tuple const&) noexcept(
        (... && std::is_nothrow_copy_constructible_v<Ts>)) = default;
    __RXX_HIDE_FROM_ABI constexpr tuple(tuple&&) noexcept(
        (... && std::is_nothrow_move_constructible_v<Ts>)) = default;

    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple const&) noexcept(
        (... && std::is_nothrow_assignable_v<Ts&, Ts const&>)) = default;

    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple&&) noexcept(
        (... && std::is_nothrow_assignable_v<Ts&, Ts>)) = default;

    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple const& other) const
        noexcept((... && std::is_nothrow_assignable_v<Ts const&, Ts const&>))
    requires (... && std::assignable_from<Ts const&, Ts const&>)
    {
        [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            base_type::assign(ranges::get_element<Is>(other)...);
        }(details::tuple::sequence_for<tuple>);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple&& other) const
        noexcept((... && std::is_nothrow_assignable_v<Ts const&, Ts>))
    requires (... && std::assignable_from<Ts const&, Ts>)
    {
        [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            base_type::assign(__RXX move(ranges::get_element<Is>(other))...);
        }(details::tuple::sequence_for<tuple>);
        return *this;
    }

    __RXX_HIDE_FROM_ABI                                          //
        explicit(!(... && std::is_convertible_v<Ts const&, Ts>)) //
        constexpr tuple(Ts const&... other)                      //
        noexcept((... && std::is_nothrow_copy_constructible_v<Ts>))
        : base_type{other...} {}

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts)) &&
        (... && std::constructible_from<Ts, Us>)
    __RXX_HIDE_FROM_ABI explicit(!(... && std::is_convertible_v<Us, Ts>)) //
        constexpr tuple(Us&&... args)                                     //
        noexcept((... && std::is_nothrow_constructible_v<Ts, Us>))
        : base_type{__RXX forward<Us>(args)...} {}

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts) &&
                 !use_other_overload<tuple<Us...>&>) &&
        (... && std::constructible_from<Ts, Us&>)
    __RXX_HIDE_FROM_ABI explicit(!(... && std::is_convertible_v<Us&, Ts>)) //
        constexpr tuple(tuple<Us...>& other)                               //
        noexcept((... && std::is_nothrow_constructible_v<Ts, Us&>))
        : tuple{other, details::tuple::sequence_for<tuple>} {}

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts) &&
                 !use_other_overload<tuple<Us...> const&>) &&
        (... && std::constructible_from<Ts, Us const&>)
    __RXX_HIDE_FROM_ABI explicit(
        !(... && std::is_convertible_v<Us const&, Ts>)) //
        constexpr tuple(tuple<Us...> const& other)      //
        noexcept((... && std::is_nothrow_constructible_v<Ts, Us const&>))
        : tuple{other, details::tuple::sequence_for<tuple>} {}

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts) &&
                 !use_other_overload<tuple<Us...> &&>) &&
        (... && std::constructible_from<Ts, Us>)
    __RXX_HIDE_FROM_ABI explicit(!(... && std::is_convertible_v<Us, Ts>)) //
        constexpr tuple(tuple<Us...>&& other)                             //
        noexcept((... && std::is_nothrow_constructible_v<Ts, Us>))
        : tuple{__RXX move(other), details::tuple::sequence_for<tuple>} {}

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts) &&
                 !use_other_overload<tuple<Us...> const &&>) &&
        (... && std::constructible_from<Ts, Us const>)
    __RXX_HIDE_FROM_ABI explicit(
        !(... && std::is_convertible_v<Us const, Ts>)) //
        constexpr tuple(tuple<Us...> const&& other)    //
        noexcept((... && std::is_nothrow_constructible_v<Ts, Us const>))
        : tuple{__RXX move(other), details::tuple::sequence_for<tuple>} {}

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts)) &&
        (... && std::assignable_from<Ts&, Us const&>)
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple<Us...> const& other) //
        noexcept((... && std::is_nothrow_assignable_v<Ts&, Us const&>)) {

        [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            base_type::assign(ranges::get_element<Is>(other)...);
        }(details::tuple::sequence_for<tuple>);
        return *this;
    }

    // clang-format off
    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts)) &&
        (... && std::assignable_from<Ts const&, Us const&>)
    __RXX_HIDE_FROM_ABI 
        constexpr tuple const& operator=(tuple<Us...> const& other) const
        noexcept((... && std::is_nothrow_assignable_v<Ts const&, Us const&>)) {

        [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            base_type::assign(ranges::get_element<Is>(other)...);
        }(details::tuple::sequence_for<tuple>);
        return *this;
    }
    // clang-format on

    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts)) &&
        (... && std::assignable_from<Ts&, Us>)
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple<Us...>&& other) //
        noexcept((... && std::is_nothrow_assignable_v<Ts&, Us>)) {

        [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            base_type::assign(__RXX move(ranges::get_element<Is>(other))...);
        }(details::tuple::sequence_for<tuple>);
        return *this;
    }

    // clang-format off
    template <typename... Us>
    requires (sizeof...(Us) == sizeof...(Ts)) &&
        (... && std::assignable_from<Ts const&, Us>)
    __RXX_HIDE_FROM_ABI
        constexpr tuple const& operator=(tuple<Us...>&& other) const
        noexcept((... && std::is_nothrow_assignable_v<Ts const&, Us>)) {

        [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            base_type::assign(__RXX move(ranges::get_element<Is>(other))...);
        }(details::tuple::sequence_for<tuple>);
        return *this;
    }
    // clang-format on

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts) &&
        !use_other_overload<Tuple &&> &&
        []<size_t... Is>(__RXX index_sequence<Is...>) {
            return (... &&
                std::constructible_from<Ts,
                    details::tuple::decl_element_t<Is, Tuple>>);
        }(details::tuple::sequence_for<tuple>))
    __RXX_HIDE_FROM_ABI                                                   //
        explicit(!details::tuple::is_element_convertible_v<Tuple, tuple>) //
        constexpr tuple(Tuple&& other)                                    //
        noexcept(details::tuple::is_nothrow_accessible_v<Tuple> &&
            []<size_t... Is>(__RXX index_sequence<Is...>) {
                return (... &&
                    std::is_nothrow_constructible_v<Ts,
                        details::tuple::decl_element_t<Is, Tuple>>);
            }(details::tuple::sequence_for<tuple>))
        : tuple{__RXX forward<Tuple>(other),
              details::tuple::sequence_for<tuple>} {}

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts) &&
        []<size_t... Is>(__RXX index_sequence<Is...>) {
            return (... &&
                std::assignable_from<Ts&,
                    details::tuple::decl_element_t<Is, Tuple>>);
        }(details::tuple::sequence_for<tuple>))
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(Tuple&& other) //
        noexcept(details::tuple::is_nothrow_accessible_v<Tuple> &&
            []<size_t... Is>(__RXX index_sequence<Is...>) {
                return (... &&
                    std::is_nothrow_assignable_v<Ts&,
                        details::tuple::decl_element_t<Is, Tuple>>);
            }(details::tuple::sequence_for<tuple>)) {

        return [&]<size_t... Is>(__RXX index_sequence<Is...>) -> tuple& {
            return base_type::assign(
                __RXX forward_like<Tuple>(ranges::get_element<Is>(other))...);
        }(details::tuple::sequence_for<tuple>);
    }

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts) &&
        []<size_t... Is>(__RXX index_sequence<Is...>) {
            return (... &&
                std::assignable_from<Ts const&,
                    details::tuple::decl_element_t<Is, Tuple>>);
        }(details::tuple::sequence_for<tuple>))
    __RXX_HIDE_FROM_ABI constexpr tuple const& operator=(Tuple&& other) const //
        noexcept(details::tuple::is_nothrow_accessible_v<Tuple> &&
            []<size_t... Is>(__RXX index_sequence<Is...>) {
                return (... &&
                    std::is_nothrow_assignable_v<Ts const&,
                        details::tuple::decl_element_t<Is, Tuple>>);
            }(details::tuple::sequence_for<tuple>)) {

        return [&]<size_t... Is>(__RXX index_sequence<Is...>) -> tuple const& {
            return base_type::assign(
                __RXX forward_like<Tuple>(ranges::get_element<Is>(other))...);
        }(details::tuple::sequence_for<tuple>);
    }

    __RXX_HIDE_FROM_ABI
    friend constexpr void swap(tuple& left, tuple& right) noexcept(
        noexcept(std::declval<tuple&>().swap(std::declval<tuple&>())))
    requires requires { std::declval<tuple&>().swap(std::declval<tuple&>()); }
    {
        left.swap(right);
    }

    __RXX_HIDE_FROM_ABI
    friend constexpr void swap(tuple const& left, tuple const& right) noexcept(
        noexcept(
            std::declval<tuple const&>().swap(std::declval<tuple const&>())))
    requires requires {
        std::declval<tuple const&>().swap(std::declval<tuple const&>());
    }
    {
        left.swap(right);
    }

    __RXX_HIDE_FROM_ABI
    constexpr void swap(tuple& other) noexcept
    requires (... && std::swappable<Ts>)
    {
        base_type::swap(other);
    }

    __RXX_HIDE_FROM_ABI
    constexpr void swap(tuple const& other) const noexcept
    requires (... && std::swappable<std::add_const_t<Ts>>)
    {
        base_type::swap(other);
    }

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts)) &&
        std::constructible_from<Tuple, Ts&...>
    __RXX_HIDE_FROM_ABI explicit(
        !(... && std::is_convertible_v<Ts&, Ts>)) constexpr
    operator Tuple() & noexcept(
        std::is_nothrow_constructible_v<Tuple, Ts&...>) {
        return [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            return Tuple(ranges::get_element<Is>(*this)...);
        }(details::tuple::sequence_for<tuple>);
    }

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts)) &&
        std::constructible_from<Tuple, Ts const&...>
    __RXX_HIDE_FROM_ABI explicit(
        !(... && std::is_convertible_v<Ts const&, Ts>)) constexpr
    operator Tuple() const& noexcept(
        std::is_nothrow_constructible_v<Tuple, Ts const&...>) {
        return [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            return Tuple(ranges::get_element<Is>(*this)...);
        }(details::tuple::sequence_for<tuple>);
    }

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts)) &&
        std::constructible_from<Tuple, Ts...>
    __RXX_HIDE_FROM_ABI explicit(
        !(... && std::is_convertible_v<Ts&&, Ts>)) constexpr
    operator Tuple() && noexcept(
        std::is_nothrow_constructible_v<Tuple, Ts...>) {
        return [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            return Tuple(__RXX move(ranges::get_element<Is>(*this))...);
        }(details::tuple::sequence_for<tuple>);
    }

    template <details::tuple::unrecognized Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == sizeof...(Ts)) &&
        std::constructible_from<Tuple, Ts const...>
    __RXX_HIDE_FROM_ABI explicit(
        !(... && std::is_convertible_v<Ts const&&, Ts>)) constexpr
    operator Tuple() const&& noexcept(
        std::is_nothrow_constructible_v<Tuple, Ts const...>) {
        return [&]<size_t... Is>(__RXX index_sequence<Is...>) {
            return Tuple(__RXX move(ranges::get_element<Is>(*this))...);
        }(details::tuple::sequence_for<tuple>);
    }
};

template <typename... Us>
tuple(Us...) -> tuple<Us...>;
template <typename T1, typename T2>
tuple(std::pair<T1, T2>) -> tuple<T1, T2>;

template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::tuple_element_t<I, tuple<Ts...>>& get(
    tuple<Ts...>& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(std::is_base_of_v<details::tuple::element<tuple<Ts...>, I>,
        tuple<Ts...>>);
    return ((details::tuple::element<tuple<Ts...>, I>&)arg).data;
}

template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::tuple_element_t<I, tuple<Ts...>>&& get(
    tuple<Ts...>&& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(std::is_base_of_v<details::tuple::element<tuple<Ts...>, I>,
        tuple<Ts...>>);
    using Result = std::tuple_element_t<I, tuple<Ts...>>;
    return static_cast<Result&&>(
        ((details::tuple::element<tuple<Ts...>, I>&)arg).data);
}

template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::tuple_element_t<I, tuple<Ts...>> const& get(
    tuple<Ts...> const& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(std::is_base_of_v<details::tuple::element<tuple<Ts...>, I>,
        tuple<Ts...>>);
    return ((details::tuple::element<tuple<Ts...>, I> const&)arg).data;
}

template <size_t I, typename... Ts>
requires requires { typename std::tuple_element_t<I, tuple<Ts...>>; }
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::tuple_element_t<I, tuple<Ts...>> const&& get(
    tuple<Ts...> const&& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(std::is_base_of_v<details::tuple::element<tuple<Ts...>, I>,
        tuple<Ts...>>);
    using Result = std::tuple_element_t<I, tuple<Ts...>>;
    return static_cast<Result const&&>(
        ((details::tuple::element<tuple<Ts...>, I> const&)arg).data);
}

template <typename Target, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) //
constexpr Target& get(tuple<Ts...>& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(template_count_v<Target, tuple<Ts...>> == 1,
        "Cannot access non-unique type");
    return get<template_index_v<Target, tuple<Ts...>>>(arg);
}

template <typename Target, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) //
constexpr Target&& get(tuple<Ts...>&& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(template_count_v<Target, tuple<Ts...>> == 1,
        "Cannot access non-unique type");
    return get<template_index_v<Target, tuple<Ts...>>>( __RXX move(arg));
}

template <typename Target, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) //
constexpr Target const& get(
    tuple<Ts...> const& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(template_count_v<Target, tuple<Ts...>> == 1,
        "Cannot access non-unique type");
    return get<template_index_v<Target, tuple<Ts...>>>(arg);
}

template <typename Target, typename... Ts>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) //
constexpr Target const&& get(
    tuple<Ts...> const&& arg RXX_LIFETIMEBOUND) noexcept {
    static_assert(template_count_v<Target, tuple<Ts...>> == 1,
        "Cannot access non-unique type");
    return get<template_index_v<Target, tuple<Ts...>>>( __RXX move(arg));
}

template <>
class tuple<> {
public:
    __RXX_HIDE_FROM_ABI constexpr ~tuple() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr tuple() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr tuple(tuple const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr tuple(tuple&&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(
        tuple const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(tuple&&) noexcept = default;

    template <tuple_like Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0)
    __RXX_HIDE_FROM_ABI constexpr tuple(Tuple const&) noexcept {}

    template <tuple_like Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0)
    __RXX_HIDE_FROM_ABI constexpr tuple(Tuple&&) noexcept {}

    template <tuple_like Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0)
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(Tuple const&) noexcept {
        return *this;
    }

    template <tuple_like Tuple>
    requires (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0)
    __RXX_HIDE_FROM_ABI constexpr tuple& operator=(Tuple&&) noexcept {
        return *this;
    }

    __RXX_HIDE_FROM_ABI friend constexpr void swap(tuple&, tuple&) noexcept {}

    __RXX_HIDE_FROM_ABI constexpr void swap(tuple&) noexcept {}
};

RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(tuple<> const&, tuple<> const&) noexcept {
    return true;
}

RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=>(tuple<> const&, tuple<> const&) noexcept {
    return 0 <=> 0;
}

template <tuple_like Tuple>
requires (std::tuple_size_v<Tuple> == 0)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(tuple<> const&, Tuple const&) noexcept {
    return true;
}

template <tuple_like Tuple>
requires (std::tuple_size_v<Tuple> == 0)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto operator<=>(tuple<> const&, Tuple const&) noexcept {
    return 0 <=> 0;
}

RXX_DEFAULT_NAMESPACE_END
