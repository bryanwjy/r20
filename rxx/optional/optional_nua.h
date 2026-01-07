// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/generatable.h"
#include "rxx/format/range_format.h" // IWYU pragma: keep
#include "rxx/optional/optional_common.h"
#include "rxx/ranges/access.h" // IWYU pragma: keep
#include "rxx/type_traits/reference_constructs_from_temporary.h"
#include "rxx/utility.h"
#include "rxx/utility/generating.h"

#include <concepts>
#include <initializer_list>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details::nua {
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
} // namespace details::nua

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
    private details::optional_monads<optional, T>,
    public details::optional_iteration<optional, T> {
    using base_type RXX_NODEBUG = details::nua::optional_base<T>;
    template <typename U>
    friend class optional;
    friend details::optional_iteration<optional, T>;
    friend details::optional_monads<optional, T>;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) as_base() const& noexcept {
        return static_cast<base_type const&>(*this);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) as_base() && noexcept {
        return static_cast<base_type&&>(*this);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) as_base() const&& noexcept {
        return static_cast<base_type const&&>(*this);
    }

public:
    using value_type = std::remove_reference_t<T>;

    __RXX_HIDE_FROM_ABI constexpr optional() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional(optional const&) noexcept(
        std::is_nothrow_copy_constructible_v<base_type>) = default;
    __RXX_HIDE_FROM_ABI constexpr optional(optional&&) noexcept(
        std::is_nothrow_move_constructible_v<base_type>) = default;
    __RXX_HIDE_FROM_ABI constexpr optional& operator=(optional const&) noexcept(
        std::is_nothrow_copy_assignable_v<base_type>) = default;
    __RXX_HIDE_FROM_ABI constexpr optional& operator=(optional&&) noexcept(
        std::is_nothrow_move_assignable_v<base_type>) = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional(__RXX nullopt_t opt) noexcept
        : base_type() {}

    __RXX_HIDE_FROM_ABI constexpr optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    template <typename U>
    requires details::optional_constructible_as<U const&, T> &&
        details::convertible_from_optional_of<U, optional, T>
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U const&,
        T>) constexpr optional(optional<U> const&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>)
        : base_type(details::dispatch_opt, other.as_base()) {}

    template <details::optional_constructible_as<T> U>
    requires details::convertible_from_optional_of<U, optional, T>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U>&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : base_type(details::dispatch_opt, __RXX move(other).as_base()) {}

    template <typename U>
    requires details::optional_constructible_as<U const, T> &&
        details::convertible_from_optional_of<U, optional, T>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U> const&&
            other) noexcept(std::is_nothrow_constructible_v<T, U const>)
        : base_type(details::dispatch_opt, __RXX move(other).as_base()) {}

    template <std::same_as<std::in_place_t> Tag, typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(Tag tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base_type(tag, __RXX forward<Args>(args)...) {}

    template <std::same_as<generating_t> Tag, typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        details::generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(Tag tag, F&& func,
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
    requires details::value_convertible_to_optional<U, optional, T>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(U&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : base_type(std::in_place, __RXX forward<U>(other)) {}

#if RXX_SUPPORTS_reference_constructs_from_temporary

    template <typename U = std::remove_cv_t<T>>
    requires details::value_convertible_to_optional<U, optional, T> &&
        details::delete_optional_reference_specialization<U, optional, T>
    explicit(!std::is_convertible_v<U, T>) constexpr optional(U&&) noexcept(
        std::is_nothrow_constructible_v<T, U>) = delete;

    template <typename U>
    requires details::optional_constructible_as<U const&, T> &&
        details::convertible_from_optional_of<U, optional, T> &&
        details::delete_optional_reference_specialization<U const&, optional, T>
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U const&,
        T>) constexpr optional(optional<U> const&
            other) noexcept(std::is_nothrow_constructible_v<T, U const&>) =
        delete;

    template <details::optional_constructible_as<T> U>
    requires details::convertible_from_optional_of<U, optional, T> &&
        details::delete_optional_reference_specialization<U, optional, T>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U>&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>) = delete;

    template <typename U>
    requires details::optional_constructible_as<U const, T> &&
        details::convertible_from_optional_of<U, optional, T> &&
        details::delete_optional_reference_specialization<U const, optional, T>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr optional(optional<U> const&&
            other) noexcept(std::is_nothrow_constructible_v<T, U>) = delete;

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...> &&
        details::delete_optional_reference_specialization<U, optional, T>
    explicit optional(
        std::in_place_t, std::initializer_list<U>, Args&&...) = delete;

    template <std::same_as<generating_t> Tag, typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        (!details::generatable_from<T, F, Args...>) &&
        std::is_lvalue_reference_v<T> && std::is_invocable_r_v<T, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(Tag tag, F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<T, F, Args...>)
        : base_type(
              tag, __RXX forward<F>(func), __RXX forward<Args>(args)...) {}

    template <std::same_as<generating_t> Tag, typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        (!details::generatable_from<T, F, Args...>) &&
        std::is_lvalue_reference_v<T> && std::is_invocable_r_v<T, F, Args...> &&
        details::delete_optional_reference_specialization<
            std::invoke_result_t<F, Args...>, optional, T>
    __RXX_HIDE_FROM_ABI explicit constexpr optional(Tag tag, F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<T, F, Args...>) =
        delete;
#endif // if RXX_SUPPORTS_reference_constructs_from_temporary

    template <typename U>
    requires requires {
        requires !std::same_as<U, T>;
        requires std::is_constructible_v<T, U const&>;
        requires std::is_assignable_v<T&, U const&>;
        requires details::assignable_from_optional_of<U, optional, T>;
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
            base_type::assign_value(*other);
        }

        return *this;
    }

    template <typename U>
    requires requires {
        requires !std::same_as<U, T>;
        requires std::is_constructible_v<T, U>;
        requires std::is_assignable_v<T&, U>;
        requires details::assignable_from_optional_of<U, optional, T>;
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
            base_type::assign_value(__RXX move(*other));
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
            base_type::assign_value(std::forward<U>(other));
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
    constexpr bool has_value() const noexcept { return base_type::engaged(); }

    using base_type::operator->;
    using base_type::operator*;
    using base_type::value;
    using base_type::value_or;

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        details::generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr T&
    generate(F&& func, Args&&... args) noexcept(
        details::nothrow_generatable_from<T, F, Args...>) {
        return this->dispatch_construct(
            __RXX forward<F>(func), __RXX forward<Args>(args)...);
    }
#if RXX_SUPPORTS_reference_constructs_from_temporary
    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        (!details::generatable_from<T, F, Args...>) &&
        (std::is_lvalue_reference_v<T> &&
            std::is_invocable_r_v<T, F, Args...> &&
            !__RXX reference_constructs_from_temporary_v<T,
                std::invoke_result_t<F, Args...>>)
    __RXX_HIDE_FROM_ABI constexpr T& generate(F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<T, F, Args...>) {
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

    using details::optional_iteration<optional, T>::begin;
    using details::optional_iteration<optional, T>::end;
    using details::optional_monads<optional, T>::and_then;
    using details::optional_monads<optional, T>::transform;
    using details::optional_monads<optional, T>::or_else;
};

template <typename T>
optional(T) -> optional<T>;

template <details::make_optional_barrier = details::make_optional_barrier{},
    typename T>
requires std::is_constructible_v<std::decay_t<T>, T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr optional<std::decay_t<T>> make_optional(T&& value) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<T>, T>) {
    return optional<std::decay_t<T>>(std::in_place, __RXX forward<T>(value));
}

template <typename T, typename... Args>
requires std::is_constructible_v<T, Args...>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr optional<T> make_optional(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args...>) {
    return optional<T>(std::in_place, __RXX forward<Args>(args)...);
}

template <typename T, typename U, typename... Args>
requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
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
template <typename T>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX nua::optional<T&>> = true;
#endif

#if RXX_SUPPORTS_RANGE_FORMAT
template <typename T>
constexpr auto std::format_kind<__RXX nua::optional<T>> =
    std::range_format::disabled;
#endif
