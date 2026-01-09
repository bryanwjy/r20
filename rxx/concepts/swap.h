// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/common_reference_with.h"
#include "rxx/details/class_or_enum.h"
#include "rxx/utility.h"

#include <type_traits>

#if RXX_LIBCXX && __has_include(<__concepts/move_constructible.h>)
#  include <__concepts/move_constructible.h>
#else
#  include <concepts>
#endif
#if RXX_LIBCXX && __has_include(<__concepts/assignable_from.h>)
#  include <__concepts/assignable_from.h>
#else
#  include <concepts>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

struct swap_t;

template <typename T, typename U>
void swap(T&, U&) = delete;

template <typename L, typename R>
concept unqualified_swappable = (class_or_enum<std::remove_cvref_t<L>> ||
                                    class_or_enum<std::remove_cvref_t<R>>) &&
    requires(L&& left, R&& right) {
        swap(__RXX forward<L>(left), __RXX forward<R>(right));
    };

template <typename L, typename R, size_t N>
concept swappable_array = (!unqualified_swappable<L (&)[N], R (&)[N]> &&
    std::extent_v<L> == std::extent_v<R> &&
    requires(L (&left)[N], R (&right)[N], swap_t const& func) {
        func(*left, *right);
    });

template <typename T>
concept exchangable = !unqualified_swappable<T&, T&> &&
    std::move_constructible<T> && std::assignable_from<T&, T>;

struct swap_t {
    template <typename L, typename R>
    requires unqualified_swappable<L, R>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr void
    operator()(L&& left, R&& right) RXX_CONST_CALL noexcept(
        noexcept(swap(__RXX forward<L>(left), __RXX forward<R>(right)))) {
        (void)swap(__RXX forward<L>(left), __RXX forward<R>(right));
    }

    template <typename L, typename R, size_t N>
    requires swappable_array<L, R, N>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr void operator()(
        L (&left)[N], R (&right)[N]) RXX_CONST_CALL
        noexcept(noexcept(operator()(*left, *right))) {
        for (auto i = 0u; i < N; ++i) {
            operator()(left[i], right[i]);
        }
    }

    template <exchangable T>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr void operator()(
        T& left, T& right) RXX_CONST_CALL
        noexcept(std::is_nothrow_move_constructible_v<T> &&
            std::is_nothrow_move_assignable_v<T>) {
        T tmp{__RXX move(left)};
        left = __RXX move(right);
        right = __RXX move(tmp);
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::swap_t swap{};
} // namespace cpo

} // namespace ranges

template <typename T>
concept swappable = requires(T& a, T& b) { ranges::swap(a, b); };

template <typename T, typename U>
concept swappable_with =
    __RXX common_reference_with<T, U> && requires(T&& t, U&& u) {
        ranges::swap(__RXX forward<T>(t), __RXX forward<T>(t));
        ranges::swap(__RXX forward<U>(u), __RXX forward<U>(u));
        ranges::swap(__RXX forward<T>(t), __RXX forward<U>(u));
        ranges::swap(__RXX forward<U>(u), __RXX forward<T>(t));
    };

RXX_DEFAULT_NAMESPACE_END
