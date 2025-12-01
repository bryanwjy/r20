// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/totally_ordered_with.h"
#include "rxx/utility.h"

#include <cstdint>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {

template <typename L, typename R>
concept no_unqualified_less_operator = !requires(L&& left, R&& right) {
    operator<(__RXX forward<L>(left), __RXX forward<R>(right));
};

template <typename L, typename R>
concept no_member_less_operator = !requires(L&& left, R&& right) {
    __RXX forward<L>(left).operator<(__RXX forward<R>(right));
};

template <typename L, typename R>
concept builtin_ptr_less =
    requires(L&& left, R&& right) {
        { left < right } -> std::same_as<bool>;
    } && std::convertible_to<L, void const volatile*> &&
    std::convertible_to<R, void const volatile*> &&
    no_unqualified_less_operator<L, R> && no_member_less_operator<L, R>;
} // namespace details

struct less {
    using is_transparent = void;

    template <typename L, typename R>
    requires totally_ordered_with<L, R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr bool operator()(
        L&& left, R&& right) RXX_CONST_CALL
        noexcept(noexcept(std::declval<L>() < std::declval<R>())) {
        if constexpr (details::builtin_ptr_less<L, R>) {
            if (std::is_constant_evaluated()) {
                return left < right;
            }

            auto const left_val = reinterpret_cast<uintptr_t>(
                static_cast<void const volatile*>(__RXX forward<L>(left)));
            auto const right_val = reinterpret_cast<uintptr_t>(
                static_cast<void const volatile*>(__RXX forward<R>(right)));
            return left_val < right_val;
        } else {
            return __RXX forward<L>(left) < __RXX forward<R>(right);
        }
    }
};

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
