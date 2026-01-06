// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/generatable.h"
#include "rxx/configuration/abi.h"
#include "rxx/configuration/builtins.h"
#include "rxx/format/range_format.h" // IWYU pragma: keep
#include "rxx/optional/bad_optional_access.h"
#include "rxx/optional/optional_common.h"
#include "rxx/ranges/access.h" // IWYU pragma: keep
#include "rxx/type_traits/reference_constructs_from_temporary.h"
#include "rxx/utility.h"

#include <concepts>
#include <initializer_list>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

__RXX_INLINE_IF_NUA_ABI
namespace nua {
template <typename>
class optional;
}

namespace details {
template <typename T>
inline constexpr bool is_optional_v<nua::optional<T>> = true;

namespace nua {
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
    requires std::is_constructible_v<T, Args...>
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

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T const& get_value() const& noexcept { return (this->value); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T& get_value() & noexcept { return (this->value); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T const&& get_value() const&& noexcept {
        return __RXX move(this->value);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T&& get_value() && noexcept { return __RXX move(this->value); }

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) T value;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) nullopt_t nothing;
};

template <typename T>
using optional_base = optional_storage<T, opt_union>;
} // namespace nua
} // namespace details

__RXX_INLINE_IF_NUA_ABI
namespace nua {
/**
 * Warning: the no_unique_address implementation may be compiler dependent. It
 * is NOT recommended to interoperate nua::optional between MSVC and any other
 * compilers.
 */
template <typename T>
class RXX_ATTRIBUTE(EMPTY_BASES) optional :
    private details::nua::optional_base<T>,
    public details::optional_iteration<optional<T>> {
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t> &&
        !std::is_same_v<std::remove_cv_t<T>, __RXX nullopt_t>);
    static_assert(std::is_destructible_v<T> && !std::is_rvalue_reference_v<T> &&
        !std::is_array_v<T>);

    using base_type RXX_NODEBUG = details::nua::optional_base<T>;
    template <typename U>
    friend class optional;
    friend details::optional_iteration<optional>;

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr bool allow_optional_conversion =
        requires {
            requires !std::is_constructible_v<T, optional<U>&>;
            requires !std::is_constructible_v<T, optional<U> const&>;
            requires !std::is_constructible_v<T, optional<U>&&>;
            requires !std::is_constructible_v<T, optional<U> const&&>;
            requires !std::is_convertible_v<optional<U>&, T>;
            requires !std::is_convertible_v<optional<U> const&, T>;
            requires !std::is_convertible_v<optional<U>&&, T>;
            requires !std::is_convertible_v<optional<U> const&&, T>;
        };

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr bool convertible_from_optional_of =
        std::is_same_v<std::remove_cv_t<T>, bool> ||
        allow_optional_conversion<U>;

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr bool assignable_from_optional_of =
        requires {
            requires allow_optional_conversion<U>;
            requires !std::is_assignable_v<T&, optional<U>&>;
            requires !std::is_assignable_v<T&, optional<U> const&>;
            requires !std::is_assignable_v<T&, optional<U>&&>;
            requires !std::is_assignable_v<T&, optional<U> const&&>;
        };

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr bool value_convertible_from =
        (!std::same_as<std::remove_cvref_t<U>, std::in_place_t> &&
            !std::same_as<std::remove_cvref_t<U>, optional> &&
            (!std::same_as<std::remove_cv_t<T>, bool> ||
                !details::is_optional_v<std::remove_cvref_t<U>>)) &&
        std::constructible_from<T, U>;

#if RXX_SUPPORTS_reference_constructs_from_temporary
    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr bool delete_reference_specialization =
        std::is_lvalue_reference_v<T> &&
        __RXX reference_constructs_from_temporary_v<T, U>;
#endif

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) as_base() const& noexcept {
        return static_cast<base_type const&>(*this);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) as_base() && noexcept {
        return static_cast<base_type&&>(*this);
    }

public:
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
        : base_type() {}

    __RXX_HIDE_FROM_ABI constexpr optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    template <typename U>
    requires requires {
        requires !std::same_as<T, U>;
        requires std::is_constructible_v<T, U const&>;
        requires convertible_from_optional_of<U>;
    }
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U const&,
        T>) constexpr optional(optional<U> const&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>)
        : base_type(details::dispatch_opt, other.as_base()) {}

    template <typename U>
    requires requires {
        requires !std::same_as<T, U>;
        requires std::is_constructible_v<T, U>;
        requires convertible_from_optional_of<U>;
    }
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U>&&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>)
        : base_type(details::dispatch_opt, __RXX move(other).as_base()) {}

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(std::in_place_t tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base_type(tag, __RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
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
    requires value_convertible_from<U>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(U&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : base_type(std::in_place, __RXX forward<U>(other)) {}

#if RXX_SUPPORTS_reference_constructs_from_temporary

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...> &&
        delete_reference_specialization<U>
    explicit optional(
        std::in_place_t, std::initializer_list<U>, Args&&...) = delete;

    template <typename U = std::remove_cv_t<T>>
    requires value_convertible_from<U> && delete_reference_specialization<U>
    explicit(!std::is_convertible_v<U, T>) constexpr optional(U&&) = delete;

    template <typename U>
    requires requires {
        requires !std::same_as<T, U>;
        requires std::is_constructible_v<T, U const&>;
        requires convertible_from_optional_of<U>;
    } && delete_reference_specialization<U>
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U const&,
        T>) constexpr optional(optional<U> const&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>) =
        delete;

    template <typename U>
    requires requires {
        requires !std::same_as<T, U>;
        requires std::is_constructible_v<T, U>;
        requires convertible_from_optional_of<U>;
    } && delete_reference_specialization<U>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U>&&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>) =
        delete;

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        (!details::generatable_from<T, F, Args...>) &&
        (std::is_lvalue_reference_v<T> &&
            !__RXX reference_constructs_from_temporary_v<T,
                std::invoke_result_t<F, Args...>>)
    __RXX_HIDE_FROM_ABI explicit constexpr optional(generating_t tag, F&& func,
        Args&&... args) noexcept(details::nothrow_generatable_from<T, F,
        Args...>)
        : base_type(
              tag, __RXX forward<F>(func), __RXX forward<Args>(args)...) {}
#endif // if RXX_SUPPORTS_reference_constructs_from_temporary

    template <typename U>
    requires requires {
        requires !std::same_as<U, T>;
        requires std::is_constructible_v<T, U const&>;
        requires std::is_assignable_v<T&, U const&>;
        requires assignable_from_optional_of<U>;
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
        requires assignable_from_optional_of<U>;
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

    template <typename U = std::remove_cv_t<T>>
    requires (!std::same_as<std::remove_cvref_t<U>, optional<T>> &&
                 (!std::same_as<std::decay_t<U>, T> || !std::is_scalar_v<T>)) &&
        std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
    __RXX_HIDE_FROM_ABI constexpr optional& operator=(U&& other) noexcept(
        std::is_nothrow_constructible_v<T, U> &&
        std::is_nothrow_assignable_v<T&, U>) {
        if (this->has_value()) {
            **this = std::forward<U>(other);
        } else {
            this->emplace(std::forward<U>(other));
        }

        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    explicit constexpr operator bool() const noexcept {
        return base_type::engaged();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::add_pointer_t<T const> operator->() const noexcept {
        return RXX_BUILTIN_addressof(base_type::data_ref());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::add_pointer_t<T> operator->() noexcept {
        return RXX_BUILTIN_addressof(base_type::data_ref());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& operator*() const& noexcept {
        return base_type::data_ref();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& operator*() & noexcept
    requires (!std::is_reference_v<T>)
    {
        return base_type::data_ref();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator*() const&& noexcept {
        if constexpr (std::is_reference_v<T>) {
            return __RXX forward<T>(base_type::data_ref());
        } else {
            return __RXX move(base_type::data_ref());
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T&& operator*() && noexcept
    requires (!std::is_reference_v<T>)
    {
        return __RXX move(base_type::data_ref());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool has_value() const noexcept { return base_type::engaged(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& value() const& {
        if (!has_value()) {
            RXX_THROW(bad_optional_access());
        }

        return base_type::data_ref();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& value() &
    requires (!std::is_reference_v<T>)
    {
        if (!has_value()) {
            RXX_THROW(bad_optional_access());
        }

        return base_type::data_ref();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) value() const&& {
        if (!has_value()) {
            RXX_THROW(bad_optional_access());
        }

        if constexpr (std::is_reference_v<T>) {
            return __RXX forward<T>(base_type::data_ref());
        } else {
            return __RXX move(base_type::data_ref());
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T&& value() && {
        if (!has_value()) {
            RXX_THROW(bad_optional_access());
        }

        return __RXX move(base_type::data_ref());
    }

    template <typename U = std::remove_cv_t<T>>
#if RXX_SUPPORTS_reference_constructs_from_temporary
    requires (!(std::is_lvalue_reference_v<T> &&
                  std::is_function_v<std::remove_reference_t<T>>) &&
        !(std::is_lvalue_reference_v<T> &&
            std::is_array_v<std::remove_reference_t<T>>))
#endif
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) const& noexcept {
        static_assert(
            std::is_copy_constructible_v<T> && std::is_convertible_v<U&&, T>);
        return has_value() ? **this
                           : static_cast<T>(__RXX forward<U>(default_value));
    }

    template <typename U = std::remove_cv_t<T>>
#if RXX_SUPPORTS_reference_constructs_from_temporary
    requires (!std::is_lvalue_reference_v<T>)
#endif
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) && noexcept {
        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<U&&, T>);
        return has_value() ? __RXX move(**this)
                           : static_cast<T>(__RXX forward<U>(default_value));
    }

#if RXX_SUPPORTS_reference_constructs_from_temporary
    template <typename U = std::remove_cv_t<T>>
    requires (std::is_lvalue_reference_v<T> &&
        !(std::is_function_v<std::remove_reference_t<T>> ||
            std::is_array_v<std::remove_reference_t<T>>))
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) && {
        static_assert(std::is_move_constructible_v<T>,
            "optional<T>::value_or: T must be move constructible");
        static_assert(std::is_convertible_v<U, T>,
            "optional<T>::value_or: U must be convertible to T");
        return this->has_value()
            ? __RXX move(**this)
            : static_cast<T>(__RXX forward<U>(default_value));
    }
#endif

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        details::generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& generate(F&& func,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        return this->dispatch_construct(
            __RXX forward<F>(func), __RXX forward<Args>(args)...);
    }
#if RXX_SUPPORTS_reference_constructs_from_temporary
    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        (!details::generatable_from<T, F, Args...>) &&
        (std::is_lvalue_reference_v<T> &&
            !__RXX reference_constructs_from_temporary_v<T,
                std::invoke_result_t<F, Args...>>)
    __RXX_HIDE_FROM_ABI constexpr T& generate(F&& func,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        return this->dispatch_construct(
            __RXX forward<F>(func), __RXX forward<Args>(args)...);
    }
#endif
    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        return this->construct(__RXX forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
#if RXX_SUPPORTS_reference_constructs_from_temporary
        && (!__RXX reference_constructs_from_temporary_v<T&, U>)
#endif
    __RXX_HIDE_FROM_ABI constexpr T& emplace(std::initializer_list<U> ilist,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>) {
        return this->construct(ilist, __RXX forward<Args>(args)...);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, REINITIALIZES)
    constexpr void reset() noexcept { base_type::disengage(); }

    __RXX_HIDE_FROM_ABI constexpr void swap(optional& other) noexcept(
        std::is_nothrow_swappable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires (!std::is_lvalue_reference_v<T>) && std::is_swappable_v<T> &&
        std::is_move_constructible_v<T>
    {
        base_type::swap(other);
    }

    __RXX_HIDE_FROM_ABI constexpr void swap(optional& other) noexcept
    requires std::is_lvalue_reference_v<T>
    {
        base_type::swap(other);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void swap(
        optional& lhs, optional& rhs) noexcept(noexcept(lhs.swap(rhs)))
    requires requires(optional& opt) { opt.swap(opt); }
    {
        lhs.swap(rhs);
    }

    using details::optional_iteration<optional>::begin;
    using details::optional_iteration<optional>::end;

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) & {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(details::is_optional_v<result_type>);
        if (*this) {
            return std::invoke(__RXX forward<F>(func), **this);
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const& {
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
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) && {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T>>;
        static_assert(details::is_optional_v<result_type>);
        if (*this) {
            return std::invoke(__RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const&& {
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
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) & {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(generating, __RXX forward<F>(func), **this);
        } else {
            return result_opt();
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const& {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(generating, __RXX forward<F>(func), **this);
        } else {
            return result_opt();
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) && {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(
                generating, __RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_opt();
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const&& {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T const>>;
        using result_opt = optional<result_type>;
        if (*this) {
            return result_opt(
                generating, __RXX forward<F>(func), __RXX move(**this));
        } else {
            return result_opt();
        }
    }

    template <std::invocable F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr optional or_else(F&& func) const&
    requires std::is_copy_constructible_v<T>
    {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(std::same_as<optional, result_type>);
        if (*this) {
            return *this;
        } else {
            return std::invoke(__RXX forward<F>(func));
        }
    }

    template <std::invocable F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr optional or_else(F&& func) &&
    requires std::is_move_constructible_v<T>
    {
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
optional(T) -> optional<T>;

template <details::make_optional_barrier = details::make_optional_barrier{},
    typename T>
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
} // namespace nua
RXX_DEFAULT_NAMESPACE_END

template <typename T>
inline constexpr bool std::ranges::enable_view<__RXX nua::optional<T>> = true;

#if RXX_SUPPORTS_OPTIONAL_REFERENCES
template <class T>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX nua::optional<T&>> = true;
#endif

#if RXX_SUPPORTS_RANGE_FORMAT
template <typename T>
constexpr auto std::format_kind<__RXX nua::optional<T>> =
    std::range_format::disabled;
#endif
