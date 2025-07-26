// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "iter_traits.h"

#include <concepts>
#include <iterator>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <std::semiregular S>
class move_sentinel {
public:
    __RXX_HIDE_FROM_ABI move_sentinel() noexcept(
        std::is_nothrow_default_constructible_v<S>) = default;

    __RXX_HIDE_FROM_ABI constexpr explicit move_sentinel(S sent) noexcept(
        std::is_nothrow_move_constructible_v<S>)
        : last_(std::move(sent)) {}

    template <typename S2>
    requires std::convertible_to<const S2&, S>
    __RXX_HIDE_FROM_ABI constexpr move_sentinel(move_sentinel<S2> const&
            sent) noexcept(std::is_nothrow_constructible_v<S, S2 const&> &&
        std::is_nothrow_copy_constructible_v<S2>)
        : last_(sent.base()) {}

    template <typename S2>
    requires std::assignable_from<S&, S2 const&>
    __RXX_HIDE_FROM_ABI constexpr move_sentinel&
    operator=(move_sentinel<S2> const& sent) noexcept(
        std::is_nothrow_copy_constructible_v<S2> &&
        std::is_nothrow_assignable_v<S&, S2 const&>) {
        last_ = sent.base();
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr S base() const
        noexcept(std::is_nothrow_copy_constructible_v<S>) {
        return last_;
    }

private:
    S last_{};
};

RXX_DEFAULT_NAMESPACE_END
