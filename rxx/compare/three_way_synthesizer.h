// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <compare>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
inline constexpr struct three_way_synthesizer_t {
private:
    template <typename L, typename R>
    static consteval auto nothrow_call() noexcept {
        if constexpr (std::three_way_comparable_with<L, R>) {
            return noexcept(
                std::declval<L const&>() <=> std::declval<R const&>());
        } else {
            return noexcept(std::declval<L const&>() <
                std::declval<R const&>())&& noexcept(std::declval<R const&>() <
                std::declval<L const&>());
        }
    }

public:
    template <typename L, typename R>
    requires requires(L const& left, R const& right) {
        left < right ? true : false;
        right < left ? false : true;
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(L const& left, R const& right) RXX_CONST_CALL
        noexcept(nothrow_call<L, R>()) {
        if constexpr (std::three_way_comparable_with<L, R>) {
            return left <=> right;
        } else {
            if (left < right) {
                return std::weak_ordering::less;
            } else if (right < left) {
                return std::weak_ordering::greater;
            } else {
                return std::weak_ordering::equivalent;
            }
        }
    }
} three_way_synthesizer = {};

template <typename L, typename R>
using three_way_result_t = std::invoke_result_t<three_way_synthesizer_t, L, R>;
} // namespace details

RXX_DEFAULT_NAMESPACE_END
