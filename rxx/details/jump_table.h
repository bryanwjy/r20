// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/utility.h"

#include <concepts>
#include <functional>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename T, T... Ns>
class jump_table;

template <typename T>
class jump_table<T> {
public:
    template <T N, T... Others>
    __RXX_HIDE_FROM_ABI static constexpr jump_table<T, N, Others...>
    add_case() {
        return {};
    }
};

template <typename... Ts>
requires (... || std::is_void_v<Ts>)
__RXX_HIDE_FROM_ABI void make_multi_return() noexcept;

template <typename... Ts>
__RXX_HIDE_FROM_ABI auto make_multi_return() noexcept
    -> std::common_reference_t<Ts...>;

template <typename... Ts>
using multi_return_t RXX_NODEBUG = decltype(make_multi_return<Ts...>());

template <typename T, T... Ns>
class jump_table {
    static_assert(
        std::semiregular<T> && requires(T value) {
            [](T value, std::integral_constant<T, (..., Ns)> first) {
                switch (value) {
                case (first.value):
                    return 0;
                default:
                    return 1;
                }
            }(value, {});
        }, "Invalid jump table index type");

    __RXX_HIDE_FROM_ABI static constexpr size_t size = sizeof...(Ns);

    __RXX_HIDE_FROM_ABI static consteval T index_to_value(size_t idx) noexcept {
        T const values[] = {Ns...};
        return values[idx];
    }

    using first_type RXX_NODEBUG = std::integral_constant<T, index_to_value(0)>;
    template <size_t I>
    using ith_type RXX_NODEBUG = std::integral_constant<T, index_to_value(I)>;

    template <typename F, typename... Args>
    requires std::invocable<F, T, Args...>
    __RXX_HIDE_FROM_ABI static auto result_type_impl() noexcept
        -> multi_return_t<std::invoke_result_t<F, T, Args...>,
            std::invoke_result_t<F, std::integral_constant<T, Ns>, Args...>...>;
    template <typename F, typename... Args>
    __RXX_HIDE_FROM_ABI static auto result_type_impl() noexcept
        -> multi_return_t<
            std::invoke_result_t<F, std::integral_constant<T, Ns>, Args...>...>;

public:
    template <typename F, typename... Args>
    using result_type RXX_NODEBUG = decltype(result_type_impl<F, Args...>());

private:
    template <typename F, typename... Args>
    requires std::invocable<F, T, Args...>
    __RXX_HIDE_FROM_ABI static constexpr auto default_(F&& callable, T value,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, T, Args...>)
        -> decltype(auto) {
        return std::invoke(
            std::forward<F>(callable), value, std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
    requires (!std::invocable<F, T, Args...>)
    __RXX_HIDE_FROM_ABI static constexpr result_type<F, Args...> default_(
        F&& callable, T, Args&&...) noexcept {
        static_assert(sizeof...(Args) == 0);
        if constexpr (!std::is_void_v<result_type<F, Args...>>) {
            RXX_BUILTIN_unreachable();
        }
        // If the callable returns nothing, the default case can be ignored
    }

    template <size_t I, typename F, typename... Args>
    requires (I < size)
    __RXX_HIDE_FROM_ABI static constexpr auto
    case_(F&& callable, T, Args&&... args) noexcept(
        std::is_nothrow_invocable_v<F, ith_type<I>, Args...>)
        -> decltype(auto) {
        return std::invoke(std::forward<F>(callable), ith_type<I>{},
            std::forward<Args>(args)...);
    }

    template <size_t I, typename F, typename... Args>
    requires (I >= size)
    __RXX_HIDE_FROM_ABI static constexpr auto case_(F&& callable, T value,
        Args&&... args) noexcept(noexcept(default_(std::declval<F>(),
        std::declval<T>(), std::declval<Args>()...))) -> decltype(auto) {
        return default_(
            std::forward<F>(callable), value, std::forward<Args>(args)...);
    }

    template <size_t O, typename F, typename... Args>
    requires (O >= size)
    __RXX_HIDE_FROM_ABI static constexpr auto next_(F&& callable, T value,
        Args&&... args) noexcept(noexcept(default_(std::declval<F>(),
        std::declval<T>(), std::declval<Args>()...))) -> decltype(auto) {
        return default_(
            std::forward<F>(callable), value, std::forward<Args>(args)...);
    }

    template <size_t O, typename F, typename... Args>
    requires (O < size)
    __RXX_HIDE_FROM_ABI static constexpr auto next_(F&& callable, T value,
        Args&&... args) noexcept(noexcept(impl<O>(std::declval<F>(),
        std::declval<T>(), std::declval<Args>()...))) -> decltype(auto) {
        return impl<O>(
            std::forward<F>(callable), value, std::forward<Args>(args)...);
    }

    __RXX_HIDE_FROM_ABI static consteval size_t page_size(size_t idx) noexcept {
        if (size >= 16 + idx) {
            return 16;
        }

        if (size >= 8 + idx) {
            return 8;
        }

        return size - idx;
    }

    template <size_t O, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI static constexpr bool is_nothrow_dispatchable_v =
        []<size_t... Is>(std::index_sequence<Is...>) {
            return (noexcept(next_<O + page_size(O)>(std::declval<F>(),
                        std::declval<T>(), std::declval<Args>()...)) &&
                ...&& noexcept(case_<O + Is>(std::declval<F>(),
                    std::declval<T>(), std::declval<Args>()...)));
        }(std::make_index_sequence<page_size(O)>{});

    template <size_t O, typename F, typename... Args>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN)
    static constexpr auto impl(F&& callable, T value, Args&&... args) noexcept(
        is_nothrow_dispatchable_v<O, F, Args...>) -> decltype(auto) {
#define __RXX_JT_CASE(X)        \
    case index_to_value(O + X): \
        return case_<O + X>(    \
            std::forward<F>(callable), value, std::forward<Args>(args)...)
#define __RXX_JT_DEFAULT(X)  \
    default:                 \
        return next_<O + X>( \
            std::forward<F>(callable), value, std::forward<Args>(args)...)

        if constexpr (size >= 16 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_CASE(5);
                __RXX_JT_CASE(6);
                __RXX_JT_CASE(7);
                __RXX_JT_CASE(8);
                __RXX_JT_CASE(9);
                __RXX_JT_CASE(10);
                __RXX_JT_CASE(11);
                __RXX_JT_CASE(12);
                __RXX_JT_CASE(13);
                __RXX_JT_CASE(14);
                __RXX_JT_CASE(15);
                __RXX_JT_DEFAULT(16);
            }
        } else if constexpr (size >= 8 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_CASE(5);
                __RXX_JT_CASE(6);
                __RXX_JT_CASE(7);
                __RXX_JT_DEFAULT(8);
            }
        } else if constexpr (size == 7 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_CASE(5);
                __RXX_JT_CASE(6);
                __RXX_JT_DEFAULT(7);
            }
        } else if constexpr (size == 6 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_CASE(5);
                __RXX_JT_DEFAULT(6);
            }
        } else if constexpr (size == 5 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_DEFAULT(5);
            }
        } else if constexpr (size == 4 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_DEFAULT(4);
            }
        } else if constexpr (size == 3 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_DEFAULT(3);
            }
        } else if constexpr (size == 2 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_DEFAULT(2);
            }
        } else if constexpr (size == 1 + O) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_DEFAULT(1);
            }
        }

        RXX_BUILTIN_unreachable();
#undef __RXX_JT_CASE
#undef __RXX_JT_DEFAULT
    }

public:
    template <T N0, T... Others>
    __RXX_HIDE_FROM_ABI static constexpr jump_table<T, Ns..., N0, Others...>
    add_case() noexcept {
        return {};
    }

    // if return type is not void, must have a default_case
    // if return type void, default_case is optional
    template <typename F, std::convertible_to<T> U, typename... Args>
    requires requires { typename result_type<F, Args...>; }
    __RXX_HIDE_FROM_ABI constexpr auto operator()(F&& callable, U&& value,
        Args&&... args) const noexcept(noexcept(impl<0>(std::declval<F>(),
        static_cast<T>(std::declval<U>()), std::declval<Args>()...)))
        -> decltype(auto) {
        return impl<0>(std::forward<F>(callable), static_cast<T>(value),
            std::forward<Args>(args)...);
    }
};
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
