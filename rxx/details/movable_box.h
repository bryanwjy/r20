// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/optional_nua.h"

#include <concepts>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
requires std::move_constructible<T> && std::is_object_v<T>
class movable_box : private nua::optional<T> {

    using base_type = nua::optional<T>;

public:
    __RXX_HIDE_FROM_ABI constexpr ~movable_box() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr movable_box() = delete;

    __RXX_HIDE_FROM_ABI constexpr movable_box() noexcept(
        std::is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
        : base_type{std::in_place} {}

    __RXX_HIDE_FROM_ABI constexpr movable_box(movable_box const&) noexcept(
        std::is_nothrow_copy_constructible_v<base_type>) = default;
    __RXX_HIDE_FROM_ABI constexpr movable_box(movable_box&&) noexcept(
        std::is_nothrow_move_constructible_v<base_type>) = default;

    template <typename U>
    requires std::constructible_from<base_type, nua::optional<U> const&> &&
        std::is_base_of_v<nua::optional<U>, movable_box<U>>
    __RXX_HIDE_FROM_ABI explicit(!std::is_convertible_v<U const&,
        T>) constexpr movable_box(movable_box<U> const&
            other) noexcept(std::is_nothrow_constructible_v<base_type,
        nua::optional<U> const&>)
        : base_type((nua::optional<U> const&)other) {}

    template <typename U>
    requires std::constructible_from<base_type, nua::optional<U>> &&
        std::is_base_of_v<nua::optional<U>, movable_box<U>>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr movable_box(movable_box<U>&&
            other) noexcept(std::is_nothrow_constructible_v<base_type,
        nua::optional<U>>)
        : base_type((nua::optional<U>&&)__RXX move(other)) {}

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr movable_box(std::in_place_t tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base_type(tag, __RXX forward<Args>(args)...) {}

    template <typename U, typename... Args>
    requires std::constructible_from<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr movable_box(std::in_place_t tag,
        std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>)
        : base_type(tag, list, __RXX forward<Args>(args)...) {}

    template <typename U = std::remove_cv_t<T>>
    requires std::constructible_from<base_type, U>
    __RXX_HIDE_FROM_ABI explicit(
        !std::is_convertible_v<U, T>) constexpr movable_box(U&&
            other) noexcept(std::is_nothrow_constructible_v<base_type, U>)
        : base_type(__RXX forward<U>(other)) {}

    using base_type::operator=;

    __RXX_HIDE_FROM_ABI constexpr movable_box&
    operator=(movable_box const& other) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
    requires (!std::copyable<T>) && std::copy_constructible<T>
    {
        if (this != RXX_BUILTIN_addressof(other)) {
            if (other) {
                emplace(*other);
            } else {
                reset();
            }
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr movable_box& operator=(
        movable_box&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    requires (!std::movable<T>)
    {
        if (this != RXX_BUILTIN_addressof(other)) {
            if (other) {
                this->emplace(*other);
            } else {
                this->reset();
            }
        }

        return *this;
    }

    template <typename U>
    requires std::assignable_from<base_type&, nua::optional<U> const&> &&
        std::is_base_of_v<nua::optional<U>, movable_box<U>>
    __RXX_HIDE_FROM_ABI constexpr movable_box&
    operator=(movable_box<U> const& other) noexcept(
        std::is_nothrow_assignable_v<base_type&, nua::optional<U> const&>) {
        base_type::operator=((nua::optional<U> const&)other);
        return *this;
    }

    template <typename U>
    requires std::assignable_from<base_type&, nua::optional<U>> &&
        std::is_base_of_v<nua::optional<U>, movable_box<U>>
    __RXX_HIDE_FROM_ABI constexpr movable_box&
    operator=(movable_box<U>&& other) noexcept(
        std::is_nothrow_assignable_v<base_type&, nua::optional<U>>) {
        base_type::operator=((nua::optional<U>&&)__RXX move(other));
        return *this;
    }

    using base_type::has_value;
    using base_type::operator bool;
    using base_type::operator->;
    using base_type::operator*;
    using base_type::emplace;
    using base_type::reset;
};

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
