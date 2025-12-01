// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/class_or_enum.h"
#include "rxx/utility/forward.h"
#include "rxx/utility/move.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

void iter_move(...) = delete;

template <typename T>
concept unqualified_iter_move = class_or_enum<std::remove_cvref_t<T>> &&
    requires { iter_move(std::declval<T>()); };

template <typename T>
concept move_deref = !unqualified_iter_move<T> && requires(T&& arg) {
    *__RXX forward<T>(arg);
    requires std::is_lvalue_reference_v<decltype(*__RXX forward<T>(arg))>;
};

template <typename T>
concept value_deref =
    !unqualified_iter_move<T> && !move_deref<T> && requires(T&& arg) {
        *__RXX forward<T>(arg);
        requires (
            !std::is_lvalue_reference_v<decltype(*__RXX forward<T>(arg))>);
    };

template <typename I>
using iter_move_ref_t = decltype(*std::declval<I>());

struct iter_move_t {
    template <unqualified_iter_move I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(I&& iter) RXX_CONST_CALL
        noexcept(noexcept(iter_move(std::declval<I>()))) -> decltype(auto) {
        return iter_move(__RXX forward<I>(iter));
    }

    template <move_deref I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(I&& iter) RXX_CONST_CALL
        noexcept(noexcept(__RXX move(*std::declval<I>())))
            -> decltype(std::declval<
                std::remove_reference_t<iter_move_ref_t<I>>>()) {
        return __RXX move(*__RXX forward<I>(iter));
    }

    template <value_deref I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(I&& iter) RXX_CONST_CALL
        noexcept(noexcept(*std::declval<I>())) -> iter_move_ref_t<I> {
        return *__RXX forward<I>(iter);
    }
};

} // namespace details

inline namespace cpo {
inline constexpr details::iter_move_t iter_move{};
} // namespace cpo
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
