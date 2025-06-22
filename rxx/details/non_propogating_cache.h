// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/optional_base.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T>
struct non_propogating_cache {};

template <typename T>
requires std::is_object_v<T>
struct non_propogating_cache<T> : private optional_base<T> {
    using base_type = optional_base<T>;

public:
    __RXX_HIDE_FROM_ABI constexpr non_propogating_cache() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~non_propogating_cache() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr non_propogating_cache(
        non_propogating_cache const&) noexcept {}
    __RXX_HIDE_FROM_ABI constexpr non_propogating_cache(
        non_propogating_cache&& other) noexcept {
        other.reset();
    }

    __RXX_HIDE_FROM_ABI constexpr non_propogating_cache& operator=(
        non_propogating_cache const& other) noexcept {
        if (this != RXX_BUILTIN_addressof(other)) {
            reset();
        }
        return *this;
    }
    __RXX_HIDE_FROM_ABI constexpr non_propogating_cache& operator=(
        non_propogating_cache&& other) noexcept {
        reset();
        other.reset();
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr non_propogating_cache& operator=(T val) {
        this->reset();
        this->emplace(std::move(val));
        return *this;
    }

    template <typename I>
    requires requires(I const& it) { T(*it); }
    __RXX_HIDE_FROM_ABI constexpr T& emplace_deref(I const& it) {
        return base_type::generate([&]() { return T(*it); });
    }

    template <typename U>
    requires std::assignable_from<base_type&, optional_base<U> const&> &&
        std::is_base_of_v<optional_base<U>, non_propogating_cache<U>>
    __RXX_HIDE_FROM_ABI non_propogating_cache&
    operator=(non_propogating_cache<U> const& other) noexcept(
        std::is_nothrow_assignable_v<base_type&, optional_base<U> const&>) {
        base_type::operator=((optional_base<U> const&)other);
        return *this;
    }

    template <typename U>
    requires std::assignable_from<base_type&, optional_base<U>> &&
        std::is_base_of_v<optional_base<U>, non_propogating_cache<U>>
    __RXX_HIDE_FROM_ABI non_propogating_cache&
    operator=(non_propogating_cache<U>&& other) noexcept(
        std::is_nothrow_assignable_v<base_type&, optional_base<U>>) {
        base_type::operator=((optional_base<U>&&)std::move(other));
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
