// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/optional_nua.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
struct non_propagating_cache {};

template <typename T>
requires std::is_object_v<T>
struct non_propagating_cache<T> : private nua::optional<T> {
    using base_type = nua::optional<T>;

public:
    __RXX_HIDE_FROM_ABI constexpr non_propagating_cache() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~non_propagating_cache() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr non_propagating_cache(
        non_propagating_cache const&) noexcept {}
    __RXX_HIDE_FROM_ABI constexpr non_propagating_cache(
        non_propagating_cache&& other) noexcept {
        other.reset();
    }

    __RXX_HIDE_FROM_ABI constexpr non_propagating_cache& operator=(
        non_propagating_cache const& other) noexcept {
        if (this != RXX_BUILTIN_addressof(other)) {
            reset();
        }
        return *this;
    }
    __RXX_HIDE_FROM_ABI constexpr non_propagating_cache& operator=(
        non_propagating_cache&& other) noexcept {
        reset();
        other.reset();
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr non_propagating_cache& operator=(T val) {
        this->reset();
        this->emplace(__RXX move(val));
        return *this;
    }

    template <typename I>
    requires requires(I const& it) { T(*it); }
    __RXX_HIDE_FROM_ABI constexpr T& emplace_deref(I const& it) noexcept(
        noexcept(T(*it))) {
        return base_type::generate(
            [&]() noexcept(noexcept(T(*it))) { return T(*it); });
    }

    template <typename U>
    requires std::assignable_from<base_type&, nua::optional<U> const&> &&
        std::is_base_of_v<nua::optional<U>, non_propagating_cache<U>>
    __RXX_HIDE_FROM_ABI non_propagating_cache&
    operator=(non_propagating_cache<U> const& other) noexcept(
        std::is_nothrow_assignable_v<base_type&, nua::optional<U> const&>) {
        base_type::operator=((nua::optional<U> const&)other);
        return *this;
    }

    template <typename U>
    requires std::assignable_from<base_type&, nua::optional<U>> &&
        std::is_base_of_v<nua::optional<U>, non_propagating_cache<U>>
    __RXX_HIDE_FROM_ABI non_propagating_cache&
    operator=(non_propagating_cache<U>&& other) noexcept(
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

struct empty_cache {};

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
