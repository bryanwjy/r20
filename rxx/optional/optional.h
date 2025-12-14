// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/generatable.h"
#include "rxx/configuration/abi.h"
#include "rxx/configuration/builtins.h"
#include "rxx/details/const_if.h"
#include "rxx/iterator.h"
#include "rxx/memory/construct_at.h"
#include "rxx/memory/destroy_at.h"
#include "rxx/optional/nullopt.h"
#include "rxx/ranges/access.h" // IWYU pragma: keep
#include "rxx/utility.h"

#include <initializer_list>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename>
class optional;

namespace details {
template <typename T>
inline constexpr bool is_optional_v = false;
template <typename T>
inline constexpr bool is_optional_v<T const> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<T volatile> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<T const volatile> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<optional<T>> = true;

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
    __RXX_HIDE_FROM_ABI explicit constexpr opt_union(std::in_place_t,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : value(__RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    __RXX_HIDE_FROM_ABI explicit constexpr opt_union(generating_t, F&& func,
        Args&&... args) noexcept(nothrow_generatable_from<T, F, Args...>)
        : value(std::invoke(
              __RXX forward<F>(func), __RXX forward<Args>(args)...)) {}

    __RXX_HIDE_FROM_ABI explicit constexpr opt_union(nullopt_t opt) noexcept
        : nothing(opt) {}

    __RXX_HIDE_FROM_ABI constexpr ~opt_union() noexcept
    requires std::is_trivially_destructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr ~opt_union() noexcept {}

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T value;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) nullopt_t nothing;
};

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr T make_from_union(
    bool has_value, U&& arg) noexcept(std::is_nothrow_constructible_v<T,
    std::in_place_t, decltype(std::declval<U>().value)>) {
    return has_value ? T(std::in_place, __RXX forward_like<U>(arg.value))
                     : T(nullopt);
}

struct dispatch_opt_t {
    __RXX_HIDE_FROM_ABI explicit constexpr dispatch_opt_t() noexcept = default;
};

__RXX_HIDE_FROM_ABI static constexpr dispatch_opt_t dispatch_opt{};

template <typename T>
requires std::is_object_v<T>
class optional_base {
    using union_type = opt_union<T>;
    static constexpr bool place_flag_in_tail =
        __RXX details::fits_in_tail_padding_v<union_type, bool>;
    static constexpr bool allow_external_overlap = !place_flag_in_tail;

    struct container {
        template <typename... Args>
        requires std::constructible_from<T, Args...>
        __RXX_HIDE_FROM_ABI explicit constexpr container(std::in_place_t tag,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
            Args...>)
            : union_(tag, tag, __RXX forward<Args>(args)...)
            , has_value_(true) {}

        template <typename... Args>
        requires std::constructible_from<union_type, generating_t, Args...>
        __RXX_HIDE_FROM_ABI explicit constexpr container(std::in_place_t tag,
            generating_t gen,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<union_type,
            generating_t, Args...>)
            : union_(tag, gen, __RXX forward<Args>(args)...)
            , has_value_(true) {}

        __RXX_HIDE_FROM_ABI explicit constexpr container(nullopt_t opt) noexcept
            : union_(std::in_place, opt)
            , has_value_(false) {}

        template <typename U>
        __RXX_HIDE_FROM_ABI explicit constexpr container(dispatch_opt_t,
            bool has_value,
            U&& u) noexcept(noexcept(make_from_union<union_type>(has_value,
            std::declval<U>())))
        requires (allow_external_overlap)
            : union_(generating,
                  [&]() {
                      return make_from_union<union_type>(
                          has_value, __RXX forward<U>(u));
                  })
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
            __RXX destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires allow_external_overlap &&
            (!std::is_trivially_destructible_v<T>)
        {
            destroy_member();
            __RXX destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        template <typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(std::in_place_t tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
            has_value_ = true;
        }

        template <typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(generating_t tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
            has_value_ = true;
        }

        __RXX_HIDE_FROM_ABI inline constexpr void construct_union(
            decltype(nullptr)) noexcept
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), nullptr);
            has_value_ = false;
        }

        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS)
        overlappable_if<place_flag_in_tail, union_type> union_;
        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS) bool has_value_;

    private:
        __RXX_HIDE_FROM_ABI constexpr void destroy_member() noexcept {
            if (has_value_) {
                __RXX destroy_at(RXX_BUILTIN_addressof(union_.data.value));
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
        return has_value
            ? container(std::in_place, __RXX forward_like<U>(u.value))
            : container(nullopt);
    }

public:
    __RXX_HIDE_FROM_ABI constexpr ~optional_base() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base() noexcept
        : container_(std::in_place, nullopt) {}

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base const&) noexcept
    requires std::is_trivially_copy_constructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base const&
            other) noexcept(std::is_nothrow_copy_constructible_v<T>)
    requires (std::is_copy_constructible_v<T> &&
        !std::is_trivially_copy_constructible_v<T>)
        : optional_base(dispatch_opt, other.engaged(), other.union_ref()) {}

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base&&) noexcept
    requires std::is_trivially_move_constructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base(optional_base&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>)
    requires (std::is_move_constructible_v<T> &&
        !std::is_trivially_move_constructible_v<T>)
        : optional_base(
              dispatch_opt, other.engaged(), __RXX move(other.union_ref())) {}

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_base(std::in_place_t tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : container_(tag, tag, __RXX forward<Args>(args)...) {}

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_base(std::in_place_t tag,
        std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>)
        : container_(tag, tag, list, __RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    requires generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_base(generating_t tag,
        F func,
        Args&&... args) noexcept(nothrow_generatable_from<T, F, Args...>)
        : container_(std::in_place, std::in_place, tag, __RXX forward<F>(func),
              __RXX forward<Args>(args)...) {}

    __RXX_HIDE_FROM_ABI constexpr optional_base& operator=(
        optional_base const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_base& operator=(
        optional_base const&) noexcept
    requires std::is_trivially_copy_constructible_v<T> &&
        std::is_trivially_copy_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_base&
    operator=(optional_base const& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> &&
        std::is_nothrow_copy_constructible_v<T>)
    requires std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T> &&
        (!(std::is_trivially_copy_constructible_v<T> &&
            std::is_trivially_copy_assignable_v<T> &&
            std::is_trivially_destructible_v<T>))
    {
        if (this == RXX_BUILTIN_addressof(other)) {
            return *this;
        }

        if (this->engaged() != other.engaged()) {
            if (other.engaged()) {
                this->construct(*other);
            } else {
                this->disengage();
            }
        } else if (other.engaged()) {
            **this = *other;
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr optional_base& operator=(
        optional_base&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_base& operator=(
        optional_base&&) noexcept
    requires std::is_move_assignable_v<T> &&
        std::is_trivially_move_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr optional_base& operator=(
        optional_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_assignable_v<T> &&
        (!std::is_trivially_move_assignable_v<T> ||
            !std::is_trivially_destructible_v<T>)
    {
        if (this->engaged() != other.engaged()) {
            if (other.engaged()) {
                this->construct(__RXX move(*other));
            } else {
                this->disengage();
            }
        } else if (other.engaged()) {
            **this = __RXX move(*other);
        }

        return *this;
    }

    template <typename U>
    requires (!std::same_as<U, T>) && std::constructible_from<T, U const&> &&
        std::assignable_from<T&, U const&> && (can_assign<U>())
    __RXX_HIDE_FROM_ABI constexpr optional_base&
    operator=(optional_base<U> const& other) noexcept(
        std::is_nothrow_constructible_v<T, U const&> &&
        std::is_nothrow_assignable_v<T&, U const&>) {
        if (this->engaged() != other.engaged()) {
            if (other.engaged()) {
                this->construct(*other);
            } else {
                this->disengage();
            }
        } else if (other.engaged()) {
            **this = *other;
        }

        return *this;
    }

    template <typename U>
    requires (!std::same_as<U, T>) && std::constructible_from<T, U> &&
        std::assignable_from<T&, U> && (can_assign<U>())
    __RXX_HIDE_FROM_ABI constexpr optional_base&
    operator=(optional_base<U>&& other) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_assignable_v<T&, U>) {
        if (this->engaged() != other.engaged()) {
            if (other.engaged()) {
                this->construct(__RXX move(*other));
            } else {
                this->disengage();
            }
        } else if (other.engaged()) {
            **this = __RXX move(*other);
        }

        return *this;
    }

    template <typename U = std::remove_cv_t<T>>
    requires (!std::same_as<std::remove_cvref_t<U>, optional_base<T>> &&
                 (!std::same_as<std::decay_t<U>, T> || !std::is_scalar_v<T>)) &&
        std::constructible_from<T, U> && std::assignable_from<T&, U>
    __RXX_HIDE_FROM_ABI constexpr optional_base& operator=(U&& other) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_assignable_v<T&, U>) {
        if (this->engaged()) {
            **this = __RXX forward<U>(other);
        } else {
            this->construct(__RXX forward<U>(other));
        }

        return *this;
    }

protected:
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

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool engaged() const noexcept {
        return container_.data.has_value_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    explicit constexpr operator bool() const noexcept { return engaged(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const* operator->() const noexcept {
        return RXX_BUILTIN_addressof(union_ref().value);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T* operator->() noexcept {
        return RXX_BUILTIN_addressof(union_ref().value);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& operator*() const& noexcept { return union_ref().value; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& operator*() & noexcept { return union_ref().value; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const&& operator*() const&& noexcept {
        return __RXX move(union_ref().value);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T&& operator*() && noexcept {
        return __RXX move(union_ref().value);
    }

    __RXX_HIDE_FROM_ABI constexpr void disengage() noexcept {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(ptr, nullptr);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(nullptr);
        }
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& construct(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(
                ptr, std::in_place, __RXX forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                std::in_place, __RXX forward<Args>(args)...);
        }
        return **this;
    }

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& construct(std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(
                ptr, std::in_place, list, __RXX forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                std::in_place, list, __RXX forward<Args>(args)...);
        }
        return **this;
    }

    template <typename F, typename... Args>
    requires std::regular_invocable<F, Args...> &&
        requires { T{std::invoke(std::declval<F>(), std::declval<Args>()...)}; }
    __RXX_HIDE_FROM_ABI constexpr T& dispatch_construct(F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...> &&
        noexcept(T{static_cast<std::invoke_result_t<F, Args...> (*)()>(0)()})) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(ptr, std::in_place, generating,
                __RXX forward<F>(func), __RXX forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(generating, __RXX forward<F>(func),
                __RXX forward<Args>(args)...);
        }
        return **this;
    }

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_base(dispatch_opt_t tag,
        bool has_value,
        U&& u) noexcept(std::is_nothrow_constructible_v<container,
        dispatch_opt_t, bool, U>)
    requires (allow_external_overlap)
        : container_(std::in_place, tag, has_value, __RXX forward<U>(u)) {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_base(dispatch_opt_t tag,
        bool has_value,
        U&& u) noexcept(noexcept(make_container(has_value, std::declval<U>())))
    requires (place_flag_in_tail)
        : container_(tag, [&]() {
            return make_container(has_value, __RXX forward<U>(u));
        }) {}

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};
} // namespace details

template <typename T>
class optional : private details::optional_base<T> {
    using base_type = details::optional_base<T>;
    template <typename U>
    friend class optional;

    template <typename U>
    static consteval bool can_convert() noexcept {
        return std::is_same_v<std::remove_cv_t<T>, bool> ||
            (!std::is_constructible_v<T, optional<U>&> &&
                !std::is_constructible_v<T, optional<U> const&> &&
                !std::is_constructible_v<T, optional<U>&&> &&
                !std::is_constructible_v<T, optional<U> const&&> &&
                !std::is_convertible_v<optional<U>&, T> &&
                !std::is_convertible_v<optional<U> const&, T> &&
                !std::is_convertible_v<optional<U>&&, T> &&
                !std::is_convertible_v<optional<U> const&&, T>);
    }

    template <typename U>
    static consteval bool can_assign() noexcept {
        return !std::is_constructible_v<T, optional<U>&> &&
            !std::is_constructible_v<T, optional<U> const&> &&
            !std::is_constructible_v<T, optional<U>&&> &&
            !std::is_constructible_v<T, optional<U> const&&> &&
            !std::is_convertible_v<optional<U>&, T> &&
            !std::is_convertible_v<optional<U> const&, T> &&
            !std::is_convertible_v<optional<U>&&, T> &&
            !std::is_convertible_v<optional<U> const&&, T> &&
            !std::is_assignable_v<T&, optional<U>&> &&
            !std::is_assignable_v<T&, optional<U> const&> &&
            !std::is_assignable_v<T&, optional<U>&&> &&
            !std::is_assignable_v<T&, optional<U> const&&>;
    }

    template <bool Const>
    class iterator_t;

public:
    using iterator = iterator_t<false>;
    using const_iterator = iterator_t<true>;
    using value_type = T;

    __RXX_HIDE_FROM_ABI constexpr optional() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional(optional const&) noexcept(
        std::is_nothrow_copy_constructible_v<T>) = default;
    __RXX_HIDE_FROM_ABI constexpr optional(optional&&) noexcept(
        std::is_nothrow_copy_constructible_v<T>) = default;
    __RXX_HIDE_FROM_ABI constexpr optional& operator=(optional const&) noexcept(
        std::is_nothrow_copy_assignable_v<T>) = default;
    __RXX_HIDE_FROM_ABI constexpr optional& operator=(optional&&) noexcept(
        std::is_nothrow_move_assignable_v<T>) = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional(__RXX nullopt_t opt) noexcept
        : base_type(opt) {}

    __RXX_HIDE_FROM_ABI constexpr optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    template <typename U>
    requires requires {
        requires !std::same_as<T, U>;
        requires std::is_constructible_v<T, U const&>;
        requires can_convert<U>();
    }
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U const&,
        T>) constexpr optional(optional<U> const&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>)
        : base_type(
              details::dispatch_opt, other.has_value(), other.union_ref()) {}

    template <typename U>
    requires requires {
        requires !std::same_as<T, U>;
        requires std::is_constructible_v<T, U>;
        requires can_convert<U>();
    }
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U>&&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>)
        : base_type(details::dispatch_opt, other.has_value(),
              __RXX move(other.union_ref())) {}

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(std::in_place_t tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base_type(tag, __RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    requires std::is_invocable_v<T, Args...> &&
        details::generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(generating_t tag, F&& func,
        Args&&... args) noexcept(details::nothrow_generatable_from<T, F,
        Args...>)
        : base_type(
              tag, __RXX forward<F>(func), __RXX forward<Args>(args)...) {}

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(std::in_place_t tag,
        std::initializer_list<U> ilist,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base_type(tag, ilist, __RXX forward<Args>(args)...) {}

    template <typename U = std::remove_cv_t<T>>
    requires (!std::same_as<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::same_as<std::remove_cvref_t<U>, optional> &&
                 (!std::same_as<std::remove_cv_t<T>, bool> ||
                     !details::is_optional_v<std::remove_cvref_t<U>>)) &&
        std::constructible_from<T, U>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(U&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : base_type(std::in_place, __RXX forward<U>(other)) {}

    template <typename U>
    requires requires {
        requires !std::same_as<U, T>;
        requires std::is_constructible_v<T, U const&>;
        requires std::is_assignable_v<T&, U const&>;
        requires can_assign<U>();
    }
    __RXX_HIDE_FROM_ABI constexpr optional&
    operator=(optional<U> const& other) noexcept(
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
    requires requires {
        requires !std::same_as<U, T>;
        requires std::is_constructible_v<T, U>;
        requires std::is_assignable_v<T&, U>;
        requires can_assign<U>();
    }
    __RXX_HIDE_FROM_ABI constexpr optional& operator=(
        optional<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_assignable_v<T&, U>) {
        if (this->has_value() != other.has_value()) {
            if (other.has_value()) {
                this->emplace(__RXX move(*other));
            } else {
                this->reset();
            }
        } else if (other.has_value()) {
            **this = __RXX move(*other);
        }

        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) operator bool() const noexcept {
        return base_type::engaged();
    }

    using base_type::operator*;
    using base_type::operator->;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) bool has_value() const noexcept {
        return base_type::engaged();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& value() const& noexcept {
        return this->union_ref().value;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr T& value() & noexcept {
        return this->union_ref().value;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const&& value() const&& noexcept {
        return __RXX move(this->union_ref().value);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T&& value() && noexcept {
        return __RXX move(this->union_ref().value);
    }

    template <typename U = std::remove_cv_t<T>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) const& noexcept {
        static_assert(
            std::is_copy_constructible_v<T> && std::is_convertible_v<U&&, T>);
        return has_value() ? **this
                           : static_cast<T>(__RXX forward<U>(default_value));
    }

    template <typename U = std::remove_cv_t<T>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) && noexcept {
        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<U&&, T>);
        return has_value() ? __RXX move(**this)
                           : static_cast<T>(__RXX forward<U>(default_value));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) iterator begin() noexcept {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return iterator(has_value()
                    ? RXX_BUILTIN_addressof(this->union_ref().value)
                    : nullptr);
        } else {
            return iterator(RXX_BUILTIN_addressof(this->union_ref().value));
        }
    }

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        details::generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& generate(F&& func,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        return this->dispatch_construct(
            __RXX forward<F>(func), __RXX forward<Args>(args)...);
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        return this->construct(__RXX forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& emplace(std::initializer_list<U> ilist,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>) {
        return this->construct(ilist, __RXX forward<Args>(args)...);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, REINITIALIZES) void reset() noexcept {
        base_type::disengage();
    }

    __RXX_HIDE_FROM_ABI constexpr void swap(optional& other) noexcept(
        std::is_nothrow_swappable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_swappable_v<T> && std::is_move_constructible_v<T>
    {
        if (other.has_value() != this->has_value()) {
            if (other.has_value()) {
                this->emplace(__RXX move(*other));
                other.reset();
            } else {
                other.emplace(__RXX move(**this));
                this->reset();
            }
        } else if (this->has_value()) {
            __RXX ranges::swap(*other, **this);
        }
    }
    __RXX_HIDE_FROM_ABI friend constexpr void swap(
        optional& lhs, optional& rhs) noexcept(std::is_nothrow_swappable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_swappable_v<T> && std::is_move_constructible_v<T>
    {
        lhs.swap(rhs);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    const_iterator begin() const noexcept {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return iterator(has_value()
                    ? RXX_BUILTIN_addressof(this->union_ref().value)
                    : nullptr);
        } else {
            return iterator(RXX_BUILTIN_addressof(this->union_ref().value));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) iterator end() noexcept {
        return begin() + static_cast<int>(has_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    const_iterator end() const noexcept {
        return begin() + static_cast<int>(has_value());
    }

    template <typename F>
    requires std::is_invocable_v<F, T&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) & noexcept(
        std::is_nothrow_invocable_v<F, T&>) {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(details::is_optional_v<result_type>);
        if (*this) {
            return std::invoke(__RXX forward<F>(func), **this);
        } else {
            return result_type{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T const&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const& noexcept(
        std::is_nothrow_invocable_v<F, T const&>) {
        using result_type =
            std::remove_cvref_t<std::invoke_result_t<F, T const&>>;
        static_assert(details::is_optional_v<result_type>);
        if (*this) {
            return std::invoke(__RXX forward<F>(func), **this);
        } else {
            return result_type{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) && noexcept(
        std::is_nothrow_invocable_v<F, T>) {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T>>;
        static_assert(details::is_optional_v<result_type>);
        if (*this) {
            return std::invoke(__RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T const>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const&& noexcept(
        std::is_nothrow_invocable_v<F, T const>) {
        using result_type =
            std::remove_cvref_t<std::invoke_result_t<F, T const>>;
        static_assert(details::is_optional_v<result_type>);
        if (*this) {
            return std::invoke(__RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) & noexcept(
        std::is_nothrow_invocable_v<F, T&>) {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(generating, __RXX forward<F>(func), **this);
        } else {
            return result_opt{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T const&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const& noexcept(
        std::is_nothrow_invocable_v<F, T const&>) {
        using result_type =
            std::remove_cvref_t<std::invoke_result_t<F, T const&>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(generating, __RXX forward<F>(func), **this);
        } else {
            return result_opt{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) && noexcept(
        std::is_nothrow_invocable_v<F, T>) {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(
                generating, __RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_opt{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F, T const>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const&& noexcept(
        std::is_nothrow_invocable_v<F, T const>) {
        using result_type =
            std::remove_cvref_t<std::invoke_result_t<F, T const>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(
                generating, __RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_opt{};
        }
    }

    template <typename F>
    requires std::is_invocable_v<F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr optional
        or_else(F&& func) const& noexcept(std::is_nothrow_invocable_v<F>) {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(std::same_as<optional, result_type>);
        if (*this) {
            return *this;
        } else {
            return std::invoke(__RXX forward<F>(func));
        }
    }

    template <typename F>
    requires std::is_invocable_v<F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr optional
        or_else(F&& func) && noexcept(std::is_nothrow_invocable_v<F>) {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(std::same_as<optional, result_type>);
        if (*this) {
            return __RXX move(*this);
        } else {
            return std::invoke(__RXX forward<F>(func));
        }
    }
};

template <typename T>
template <bool Const>
class optional<T>::iterator_t {
    friend optional;

private:
    __RXX_HIDE_FROM_ABI constexpr iterator_t(
        details::const_if<Const, T>* ptr) noexcept
        : ptr_(ptr) {}

public:
    using value_type = T;
    using pointer = details::const_if<Const, T>*;
    using reference = details::const_if<Const, T>&;
    using difference_type = std::pointer_traits<pointer>::difference_type;
    using iterator_concept = std::contiguous_iterator_tag;

    __RXX_HIDE_FROM_ABI constexpr iterator_t(iterator_t<false> other) noexcept
    requires (Const)
        : ptr_(other.ptr_) {}

    RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
    constexpr value_type& operator*() const noexcept { return *ptr_; }

    RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
    constexpr value_type* operator->() const noexcept { return ptr_; }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    friend inline constexpr iterator operator+(
        difference_type offset, iterator_t const& it) noexcept {
        return iterator_t(it.ptr_ + offset);
    }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    constexpr auto operator-(difference_type offset) const noexcept {
        return iterator_t(ptr_ - offset);
    }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    friend constexpr difference_type operator-(
        iterator_t const& left, iterator_t const& right) noexcept {
        return left.ptr_ - left.ptr_;
    }

    __RXX_HIDE_FROM_ABI
    constexpr iterator_t& operator+=(difference_type offset) noexcept {
        ptr_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr iterator_t& operator-=(difference_type offset) noexcept {
        ptr_ -= offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr iterator_t& operator++() noexcept {
        ++ptr_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr iterator_t operator++(int) noexcept {
        iterator_t before = *this;
        ++ptr_;
        return before;
    }

    __RXX_HIDE_FROM_ABI
    constexpr iterator_t& operator--() noexcept {
        --ptr_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr iterator_t operator--(int) noexcept {
        auto before = *this;
        --ptr_;
        return before;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr value_type& operator[](difference_type offset) const noexcept {
        return ptr_[offset];
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator==(iterator_t const& right) const noexcept {
        return ptr_ == right.ptr_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator<=>(iterator_t const& right) const noexcept {
        return ptr_ <=> right.ptr_;
    }

private:
    T* ptr_;
};

template <typename T>
optional(T) -> optional<T>;

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr optional<std::decay_t<T>> make_optional(T&& value) noexcept(
    std::is_nothrow_move_constructible_v<std::decay_t<T>>) {
    return optional<std::decay_t<T>>(std::in_place, __RXX forward<T>(value));
}

template <typename T, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr optional<T> make_optional(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args...>) {
    return optional<T>(std::in_place, __RXX forward<Args>(args)...);
}

template <typename T, typename U, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr optional<T> make_optional(std::initializer_list<U> ilist,
    Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
    std::initializer_list<U>&, Args...>) {
    return optional<T>(std::in_place, ilist, __RXX forward<Args>(args)...);
}

RXX_DEFAULT_NAMESPACE_END

template <typename T>
inline constexpr bool std::ranges::enable_view<__RXX optional<T>> = true;
