// Copyright 2023-2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/type_traits/common_reference.h"
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
requires (... && std::is_void_v<Ts>)
__RXX_HIDE_FROM_ABI void make_multi_return() noexcept;

template <typename... Ts>
__RXX_HIDE_FROM_ABI auto make_multi_return() noexcept
    -> common_reference_t<Ts...>;

template <typename... Ts>
using multi_return_t RXX_NODEBUG = decltype(make_multi_return<Ts...>());

template <typename T, T... Ns>
class jump_table {
    static_assert(
        std::semiregular<T> && requires(T value) {
            typename std::integral_constant<T, (..., Ns)>;
            [](T value, std::integral_constant<T, (..., Ns)> first) {
                switch (value) {
                case (first.value):
                    return 0;
                default:
                    return 1;
                }
            }(value, {});
            { (T[]){Ns...} };
        }, "Invalid jump table index type");

    __RXX_HIDE_FROM_ABI static constexpr size_t size = sizeof...(Ns);

    __RXX_HIDE_FROM_ABI static consteval T index_to_value(size_t idx) noexcept {
        T const values[] = {Ns...};
        return values[idx];
    }

    template <size_t I>
    struct ith_type : std::integral_constant<T, index_to_value(I)> {};

    template <typename F, typename... Args>
    requires std::invocable<F, T, Args...>
    __RXX_HIDE_FROM_ABI static auto make_result_type() noexcept
        -> multi_return_t<std::invoke_result_t<F, T, Args...>,
            std::invoke_result_t<F, std::integral_constant<T, Ns>, Args...>...>;
    template <typename F, typename... Args>
    __RXX_HIDE_FROM_ABI static auto make_result_type() noexcept
        -> multi_return_t<
            std::invoke_result_t<F, std::integral_constant<T, Ns>, Args...>...>;

public:
    template <typename F, typename... Args>
    using result_type RXX_NODEBUG = decltype(make_result_type<F, Args...>());

private:
    template <typename F, typename... Args>
    __RXX_HIDE_FROM_ABI static constexpr result_type<F, Args...> default_(
        F&& callable, T value, Args&&... args) {
        if constexpr (std::invocable<F, T, Args...>) {
            return std::invoke(
                std::forward<F>(callable), value, std::forward<Args>(args)...);
        } else if constexpr (!std::is_void_v<result_type<F, Args...>>) {
            RXX_BUILTIN_unreachable();
        }
        // If the callable returns nothing, the default case can be ignored
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI static constexpr auto case_(
        F&& callable, T value, Args&&... args) -> decltype(auto) {
        if constexpr (I < size) {
            constexpr ith_type<I> case_arg{};
            return std::invoke(std::forward<F>(callable), case_arg,
                std::forward<Args>(args)...);
        } else {
            return default_(
                std::forward<F>(callable), value, std::forward<Args>(args)...);
        }
    }

    template <size_t I, typename F, typename... Args>
    __RXX_HIDE_FROM_ABI static constexpr auto next_(
        F&& callable, T value, Args&&... args) -> decltype(auto) {
        if constexpr (I < size) {
            return impl<I>(
                std::forward<F>(callable), value, std::forward<Args>(args)...);
        } else {
            return default_(
                std::forward<F>(callable), value, std::forward<Args>(args)...);
        }
    }

    template <size_t I, typename F, typename... Args>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, FLATTEN)
    static constexpr auto impl(F&& callable, T value, Args&&... args)
        -> decltype(auto) {
#define __RXX_JT_CASE(X)        \
    case index_to_value(I + X): \
        return case_<I + X>(    \
            std::forward<F>(callable), value, std::forward<Args>(args)...)
#define __RXX_JT_DEFAULT(X)  \
    default:                 \
        return next_<I + X>( \
            std::forward<F>(callable), value, std::forward<Args>(args)...)
        if constexpr (size >= 16 + I) {
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
        } else if constexpr (size >= 8 + I) {
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
        } else if constexpr (size == 7 + I) {
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
        } else if constexpr (size == 6 + I) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_CASE(5);
                __RXX_JT_DEFAULT(6);
            }
        } else if constexpr (size == 5 + I) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_CASE(4);
                __RXX_JT_DEFAULT(5);
            }
        } else if constexpr (size == 4 + I) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_CASE(3);
                __RXX_JT_DEFAULT(4);
            }
        } else if constexpr (size == 3 + I) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_CASE(2);
                __RXX_JT_DEFAULT(3);
            }
        } else if constexpr (size == 2 + I) {
            switch (value) {
                __RXX_JT_CASE(0);
                __RXX_JT_CASE(1);
                __RXX_JT_DEFAULT(2);
            }
        } else if constexpr (size == 1 + I) {
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
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(F&& callable,
        U&& value, Args&&... args) RXX_CONST_CALL->decltype(auto) {
        return impl<0>(std::forward<F>(callable), static_cast<T>(value),
            std::forward<Args>(args)...);
    }
};
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
