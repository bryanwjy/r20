// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "overlappable_if.h"

#include <initializer_list>
#include <type_traits>
#include <utility>

#if RXX_COMPILER_MSVC

#  include <new>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges::details {
template <typename T, typename... Args>
__RXX_HIDE_FROM_ABI constexpr T* construct_at(
    T* location, Args&&... args) noexcept(noexcept(::new((void*)0)
        T{std::declval<Args>()...})) {
    [[msvc::constexpr]] return ::new (location) T{std::forward<Args>(args)...};
}
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END

#elif RXX_COMPILER_CLANG | RXX_COMPILER_GCC

#  if __has_include(<bits/stl_construct.h>)
// libstdc++
#    include <bits/stl_construct.h>
#  elif __has_include(<__memory/construct_at.h>)
// libc++
#    include <__memory/construct_at.h>
#  else
#    include <memory>
#  endif

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges::details {
using std::construct_at;
}
RXX_DEFAULT_NAMESPACE_END

#else
#  error "Unsupported"
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {
template <typename T>
__RXX_HIDE_FROM_ABI constexpr void destroy_at(T* ptr) noexcept {
    if constexpr (std::is_array_v<T>) {
        for (auto& element : *ptr) {
            (destroy_at)(RXX_BUILTIN_addressof(element));
        }
    } else {
        ptr->~T();
    }
}

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr auto&& forward_like(U&& x) noexcept {
    constexpr bool is_adding_const =
        std::is_const_v<std::remove_reference_t<T>>;
    if constexpr (std::is_lvalue_reference_v<T&&>) {
        if constexpr (is_adding_const)
            return std::as_const(x);
        else
            return static_cast<U&>(x);
    } else if constexpr (is_adding_const)
        return std::move(std::as_const(x));
    else
        return std::move(x);
}

struct nothing_t {};

template <typename T>
union opt_union {
    __RXX_HIDE_FROM_ABI constexpr opt_union(opt_union const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr opt_union(opt_union const&) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
    requires std::is_copy_constructible_v<T> &&
        std::is_trivially_copy_constructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr opt_union(opt_union&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr opt_union(opt_union&&) noexcept(
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_constructible_v<T> &&
        std::is_trivially_move_constructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr opt_union& operator=(
        opt_union const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr opt_union& operator=(
        opt_union const&) noexcept(std::is_nothrow_copy_assignable_v<T>)
    requires std::is_copy_assignable_v<T> &&
        std::is_trivially_copy_assignable_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr opt_union& operator=(opt_union&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr opt_union& operator=(opt_union&&) noexcept(
        std::is_nothrow_move_assignable_v<T>)
    requires std::is_move_assignable_v<T> &&
        std::is_trivially_move_assignable_v<T>
    = default;

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr explicit opt_union(std::in_place_t,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : value{std::forward<Args>(args)...} {}

    template <typename F, typename... Args>
    __RXX_HIDE_FROM_ABI inline constexpr explicit opt_union(generating_t, F&& f,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        : value{std::invoke(std::forward<F>(f), std::forward<Args>(args)...)} {}

    __RXX_HIDE_FROM_ABI constexpr explicit opt_union(decltype(nullptr)) noexcept
        : nothing{} {}

    __RXX_HIDE_FROM_ABI constexpr ~opt_union() noexcept
    requires std::is_trivially_destructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr ~opt_union() noexcept {}

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T value;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) nothing_t nothing;
};

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr T make_from_union(
    bool has_value, U&& arg) noexcept(std::is_nothrow_constructible_v<T,
    std::in_place_t, decltype(std::declval<U>().value)>) {
    return has_value ? T{std::in_place, forward_like<U>(arg.value)}
                     : T{nullptr};
}

template <typename T>
inline constexpr bool is_optional_base_v = false;
template <typename T>
inline constexpr bool is_optional_base_v<T const> = is_optional_base_v<T>;
template <typename T>
inline constexpr bool is_optional_base_v<T volatile> = is_optional_base_v<T>;
template <typename T>
inline constexpr bool is_optional_base_v<T const volatile> =
    is_optional_base_v<T>;

template <typename T>
requires std::is_object_v<T>
class optional_base {
    using union_type = opt_union<T>;
    static constexpr bool place_flag_in_tail =
        fits_in_tail_padding_v<union_type, bool>;
    static constexpr bool allow_external_overlap = !place_flag_in_tail;

    struct container {
        template <typename... Args>
        requires std::constructible_from<T, Args...>
        __RXX_HIDE_FROM_ABI constexpr explicit container(std::in_place_t tag,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
            Args...>)
            : union_{tag, tag, std::forward<Args>(args)...}
            , has_value_{true} {}

        template <typename... Args>
        requires std::constructible_from<T, Args...>
        __RXX_HIDE_FROM_ABI constexpr explicit container(
            decltype(nullptr)) noexcept
            : union_{std::in_place, nullptr}
            , has_value_{false} {}

        template <typename U>
        __RXX_HIDE_FROM_ABI constexpr explicit container(generating_t tag,
            bool has_value,
            U&& u) noexcept(noexcept(make_from_union<union_type>(has_value,
            std::declval<U>())))
        requires (allow_external_overlap)
            : union_{tag,
                  [&]() {
                      return make_from_union<union_type>(
                          has_value, std::forward<U>(u));
                  }}
            , has_value_(has_value) {}

        __RXX_HIDE_FROM_ABI constexpr container(container const&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container(container const&) noexcept
        requires std::is_copy_constructible_v<T> &&
            std::is_trivially_copy_constructible_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) noexcept
        requires std::is_move_constructible_v<T> &&
            std::is_trivially_move_constructible_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) noexcept
        requires std::is_copy_assignable_v<T> &&
            std::is_trivially_copy_assignable_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container&&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(container&&) noexcept
        requires std::is_move_assignable_v<T> &&
            std::is_trivially_move_assignable_v<T>
        = default;

        __RXX_HIDE_FROM_ABI constexpr ~container() noexcept
        requires std::is_trivially_destructible_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr ~container() noexcept {
            destroy_member();
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires allow_external_overlap && std::is_trivially_destructible_v<T>
        {
            destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires allow_external_overlap
        {
            destroy_member();
            destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        template <typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(std::in_place_t tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires allow_external_overlap
        {
            construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                std::forward<Args>(args)...);
            has_value_ = true;
        }

        template <typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(generating_t tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires allow_external_overlap
        {
            construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                std::forward<Args>(args)...);
            has_value_ = true;
        }

        __RXX_HIDE_FROM_ABI inline constexpr void construct_union(
            decltype(nullptr)) noexcept
        requires allow_external_overlap
        {
            construct_at(RXX_BUILTIN_addressof(union_.data), nullptr);
            has_value_ = false;
        }

        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS)
        overlappable_if<place_flag_in_tail, union_type> union_;
        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS) bool has_value_;

    private:
        __RXX_HIDE_FROM_ABI constexpr void destroy_member() noexcept {
            if (has_value_) {
                destroy_at(RXX_BUILTIN_addressof(union_.data.value));
            }
        }
    };

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr container make_container(
        bool has_value,
        U&& u) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_t, decltype(std::declval<U>().value)>)
    requires (place_flag_in_tail)
    {
        return has_value ? container{std::in_place, forward_like<U>(u.value)}
                         : container{nullptr};
    }

    template <typename U>
    static consteval bool can_convert() noexcept {
        return std::is_same_v<std::remove_cv_t<T>, bool> ||
            (!std::is_constructible_v<T, optional_base<U>&> &&
                !std::is_constructible_v<T, optional_base<U> const&> &&
                !std::is_constructible_v<T, optional_base<U>&&> &&
                !std::is_constructible_v<T, optional_base<U> const&&> &&
                !std::is_convertible_v<optional_base<U>&, T> &&
                !std::is_convertible_v<optional_base<U> const&, T> &&
                !std::is_convertible_v<optional_base<U>&&, T> &&
                !std::is_convertible_v<optional_base<U> const&&, T>);
    }

    template <typename U>
    static consteval bool can_assign() noexcept {
        return !std::is_constructible_v<T, optional_base<U>&> &&
            !std::is_constructible_v<T, optional_base<U> const&> &&
            !std::is_constructible_v<T, optional_base<U>&&> &&
            !std::is_constructible_v<T, optional_base<U> const&&> &&
            !std::is_convertible_v<optional_base<U>&, T> &&
            !std::is_convertible_v<optional_base<U> const&, T> &&
            !std::is_convertible_v<optional_base<U>&&, T> &&
            !std::is_convertible_v<optional_base<U> const&&, T> &&
            !std::is_assignable_v<T&, optional_base<U>&> &&
            !std::is_assignable_v<T&, optional_base<U> const&> &&
            !std::is_assignable_v<T&, optional_base<U>&&> &&
            !std::is_assignable_v<T&, optional_base<U> const&&>;
    }

public:
    __RXX_HIDE_FROM_ABI constexpr ~optional_base() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base() noexcept
        : container_{nullptr} {}

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base const&) noexcept
    requires std::is_copy_constructible_v<T> &&
        std::is_trivially_copy_constructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base const&
            other) noexcept(std::is_nothrow_copy_constructible_v<T>)
    requires (std::is_copy_constructible_v<T> &&
        !std::is_trivially_copy_constructible_v<T>)
        : optional_base(generating, other.has_value(), *other) {}

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base&&) noexcept
    requires std::is_move_constructible_v<T> &&
        std::is_trivially_move_constructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>)
    requires (std::is_move_constructible_v<T> &&
        !std::is_trivially_move_constructible_v<T>)
        : optional_base(generating, other.has_value(), std::move(*other)) {}

    template <typename U>
    requires std::constructible_from<T, U const&> && (can_convert<U>())
    __RXX_HIDE_FROM_ABI
        explicit(!std::is_convertible_v<U const&, T>) constexpr optional_base(
            optional_base<U> const&
                other) noexcept(std::is_nothrow_constructible_v<T, U const&>)
        : optional_base(generating, other.has_value(), *other) {}

    template <typename U>
    requires std::constructible_from<T, U> && (can_convert<U>())
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional_base(optional_base<U>&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : optional_base(generating, other.has_value(), std::move(*other)) {}

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_base(std::in_place_t tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : container_(tag, tag, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    requires std::constructible_from<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_base(std::in_place_t tag,
        std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>)
        : container_(tag, tag, list, std::forward<Args>(args)...) {}

    template <typename U = std::remove_cv_t<T>>
    requires std::constructible_from<T, U> &&
        (!std::same_as<std::remove_cvref_t<U>, std::in_place_t> &&
            !std::same_as<std::remove_cvref_t<U>, optional_base<T>> &&
            (!std::same_as<std::remove_cv_t<T>, bool> ||
                !is_optional_base_v<std::remove_cvref_t<U>>))
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U, T>)
        optional_base(U&& other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : container_(std::in_place, std::in_place, std::forward<U>(other)) {}

    __RXX_HIDE_FROM_ABI optional_base& operator=(decltype(nullptr)) noexcept {
        reset();
        return *this;
    }

    __RXX_HIDE_FROM_ABI inline constexpr optional_base& operator=(
        optional_base const&) = delete;
    __RXX_HIDE_FROM_ABI inline constexpr optional_base& operator=(
        optional_base const&) noexcept
    requires std::is_copy_assignable_v<T> &&
        std::is_trivially_copy_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI inline constexpr optional_base&
    operator=(optional_base const& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> &&
        std::is_nothrow_copy_constructible_v<T>)
    requires std::is_copy_assignable_v<T> &&
        (!std::is_trivially_copy_assignable_v<T> ||
            !std::is_trivially_destructible_v<T>)
    {
        if (this == RXX_BUILTIN_addressof(other)) {
            return *this;
        }

        if (this->has_value() != other.has_value()) {
            if (other.has_value()) {
                this->emplace(*other);
            } else {
                this->reset();
            }
        } else if (other.has_value()) {
            **this = *other;
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI inline constexpr optional_base& operator=(
        optional_base&&) = delete;
    __RXX_HIDE_FROM_ABI inline constexpr optional_base& operator=(
        optional_base&&) noexcept
    requires std::is_move_assignable_v<T> &&
        std::is_trivially_move_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI inline constexpr optional_base& operator=(
        optional_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_assignable_v<T> &&
        (!std::is_trivially_move_assignable_v<T> ||
            !std::is_trivially_destructible_v<T>)
    {
        if (this->has_value() != other.has_value()) {
            if (other.has_value()) {
                this->emplace(std::move(*other));
            } else {
                this->reset();
            }
        } else if (other.has_value()) {
            **this = std::move(*other);
        }

        return *this;
    }

    template <typename U>
    requires std::constructible_from<T, U const&> &&
        std::assignable_from<T&, U const&> && (can_assign<U>())
    __RXX_HIDE_FROM_ABI optional_base&
    operator=(optional_base<U> const& other) noexcept(
        std::is_nothrow_constructible_v<T, U const&> &&
        std::is_nothrow_assignable_v<T&, U const&>) {
        if (this->has_value() != other.has_value()) {
            if (other.has_value()) {
                this->emplace(*other);
            } else {
                this->reset();
            }
        } else if (other.has_value()) {
            **this = *other;
        }

        return *this;
    }

    template <typename U>
    requires std::constructible_from<T, U> && std::assignable_from<T&, U> &&
        (can_assign<U>())
    __RXX_HIDE_FROM_ABI optional_base&
    operator=(optional_base<U>&& other) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_assignable_v<T&, U>) {
        if (this->has_value() != other.has_value()) {
            if (other.has_value()) {
                this->emplace(std::move(*other));
            } else {
                this->reset();
            }
        } else if (other.has_value()) {
            **this = std::move(*other);
        }

        return *this;
    }

    template <typename U = std::remove_cv_t<T>>
    requires std::constructible_from<T, U> && std::assignable_from<T&, U> &&
        (!std::same_as<std::remove_cvref_t<U>, optional_base<T>> &&
            (!std::same_as<std::decay_t<U>, T> || !std::is_scalar_v<T>))
    __RXX_HIDE_FROM_ABI optional_base& operator=(U&& other) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_assignable_v<T&, U>) {
        if (this->has_value() != other.has_value()) {
            if (other.has_value()) {
                this->emplace(std::forward<U>(other));
            } else {
                this->reset();
            }
        } else if (other.has_value()) {
            **this = std::forward<U>(other);
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr bool has_value() const noexcept {
        return container_.data.has_value_;
    }

    __RXX_HIDE_FROM_ABI explicit constexpr operator bool() const noexcept {
        return has_value();
    }

    __RXX_HIDE_FROM_ABI constexpr T const* operator->() const noexcept {
        return RXX_BUILTIN_addressof(container_.data.union_.data.value);
    }

    __RXX_HIDE_FROM_ABI constexpr T* operator->() noexcept {
        return RXX_BUILTIN_addressof(container_.data.union_.data.value);
    }

    __RXX_HIDE_FROM_ABI constexpr T const& operator*() const& noexcept {
        return container_.data.union_.data.value;
    }

    __RXX_HIDE_FROM_ABI constexpr T& operator*() & noexcept {
        return container_.data.union_.data.value;
    }

    __RXX_HIDE_FROM_ABI constexpr T const&& operator*() const&& noexcept {
        return std::move(container_.data.union_.data.value);
    }

    __RXX_HIDE_FROM_ABI constexpr T&& operator*() && noexcept {
        return std::move(container_.data.union_.data.value);
    }

    __RXX_HIDE_FROM_ABI constexpr void reset() noexcept {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            destroy_at(ptr);
            construct_at(ptr, nullptr);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(nullptr);
        }
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            destroy_at(ptr);
            return *construct_at(
                ptr, std::in_place, std::forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                std::in_place, std::forward<Args>(args)...);
            return **this;
        }
    }

    template <typename U, typename... Args>
    requires std::constructible_from<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& emplace(std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            destroy_at(ptr);
            return *construct_at(
                ptr, std::in_place, list, std::forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                std::in_place, list, std::forward<Args>(args)...);
            return **this;
        }
    }

    template <typename F, typename... Args>
    requires std::regular_invocable<F, Args...> &&
        std::constructible_from<T, std::invoke_result_t<F, Args...>>
    __RXX_HIDE_FROM_ABI constexpr T& generate(F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            destroy_at(ptr);
            return *construct_at(ptr, std::in_place, generating,
                std::forward<F>(func), std::forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                generating, std::forward<F>(func), std::forward<Args>(args)...);
            return **this;
        }
    }

private:
    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_base(generating_t tag,
        bool has_value,
        U&& u) noexcept(std::is_nothrow_constructible_v<container, generating_t,
        bool, U>)
    requires allow_external_overlap
        : container_{std::in_place, tag, has_value, std::forward<U>(u)} {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_base(generating_t tag,
        bool has_value,
        U&& u) noexcept(noexcept(make_container(has_value, std::declval<U>())))
    requires place_flag_in_tail
        : container_{tag,
              [&]() { return make_container(has_value, std::forward<U>(u)); }} {
    }

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};

template <typename T>
inline constexpr bool is_optional_base_v<optional_base<T>> = true;
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
