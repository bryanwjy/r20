// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"

#include <concepts>
#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

#if RXX_CXX23
using std::ranges::view_interface;
#else
template <typename D>
requires std::is_class_v<D> && std::same_as<D, std::remove_cv_t<D>>
class view_interface : public std::ranges::view_interface<D> {
public:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto cbegin()
    requires requires(D& derived) { ranges::cbegin(derived); }
    {
        return ranges::cbegin(static_cast<D&>(*this));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto cbegin() const
    requires range<D const> &&
        requires(D const& derived) { ranges::cbegin(derived); }
    {
        return ranges::cbegin(static_cast<D const&>(*this));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto cend()
    requires requires(D& derived) { ranges::cend(derived); }
    {
        return ranges::cend(static_cast<D&>(*this));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto cend() const
    requires range<D const> &&
        requires(D const& derived) { ranges::cend(derived); }
    {
        return ranges::cend(static_cast<D const&>(*this));
    }
};
#endif

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
