// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/overlappable_if.h"

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename F, typename... Ts>
class bind_back_t {
public:
    template <typename Func, typename... Args>
    requires (std::constructible_from<F, Func> && ... &&
        std::constructible_from<Ts, Args>)
    __RXX_HIDE_FROM_ABI constexpr bind_back_t(Func&& func,
        Args&&... args) noexcept((std::is_nothrow_constructible_v<F, Func> &&
        ... && std::is_nothrow_constructible_v<Ts, Args>))
        : container_{std::in_place, std::forward<Func>(func),
              std::forward<Args>(args)...} {}

    __RXX_HIDE_FROM_ABI constexpr bind_back_t(bind_back_t const&) = default;
    __RXX_HIDE_FROM_ABI constexpr bind_back_t& operator=(
        bind_back_t const&) = default;
    __RXX_HIDE_FROM_ABI constexpr bind_back_t(bind_back_t&&) = default;
    __RXX_HIDE_FROM_ABI constexpr bind_back_t& operator=(
        bind_back_t&&) = default;

    template <typename... Us>
    requires std::regular_invocable<F const&, Us..., Ts const&...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) const& noexcept(
        std::is_nothrow_invocable_v<F const&, Us..., Ts const&...>)
        -> decltype(auto) {
        return std::apply(
            [&](Ts const&... bound) -> decltype(auto) {
                return std::invoke(container_.data.func.data,
                    std::forward<Us>(args)..., bound...);
            },
            container_.data.args);
    }

    template <typename... Us>
    requires std::regular_invocable<F const, Us..., Ts const...>
    __RXX_HIDE_FROM_ABI constexpr auto
    operator()(Us&&... args) const&& noexcept(
        std::is_nothrow_invocable_v<F const, Us..., Ts const...>)
        -> decltype(auto) {
        return std::apply(
            [&](Ts const&&... bound) -> decltype(auto) {
                return std::invoke(std::move(container_.data.func.data),
                    std::forward<Us>(args)..., std::move(bound)...);
            },
            std::move(container_.data.args));
    }

    template <typename... Us>
    requires std::regular_invocable<F&, Us..., Ts&...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) & noexcept(
        std::is_nothrow_invocable_v<F&, Us..., Ts&...>) -> decltype(auto) {
        return std::apply(
            [&](Ts&... bound) -> decltype(auto) {
                return std::invoke(container_.data.func.data,
                    std::forward<Us>(args)..., bound...);
            },
            container_.data.args);
    }

    template <typename... Us>
    requires std::regular_invocable<F, Us..., Ts...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) && noexcept(
        std::is_nothrow_invocable_v<F, Us..., Ts...>) -> decltype(auto) {
        return std::apply(
            [&](Ts&&... bound) -> decltype(auto) {
                return std::invoke(std::move(container_.data.func.data),
                    std::forward<Us>(args)..., std::move(bound)...);
            },
            std::move(container_.data.args));
    }

private:
    __RXX_HIDE_FROM_ABI static constexpr bool place_args_in_tail =
        fits_in_tail_padding_v<F, std::tuple<Ts...>>;
    __RXX_HIDE_FROM_ABI static constexpr bool allow_external_overlap =
        !place_args_in_tail;

    struct container {
        template <typename Func, typename... Args>
        requires (std::constructible_from<F, Func> && ... &&
                     std::constructible_from<Ts, Args>)
        __RXX_HIDE_FROM_ABI constexpr container(Func&& func,
            Args&&... args) noexcept((std::is_nothrow_constructible_v<F,
                                          Func> &&
            ... && std::is_nothrow_constructible_v<Ts, Args>))
            : func{std::in_place, std::forward<Func>(func)}
            , args{std::forward<Args>(args)...} {}
        __RXX_HIDE_FROM_ABI constexpr container(container const&) = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) = default;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container&&) = default;

        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
        overlappable_if<place_args_in_tail, F> func;
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) std::tuple<Ts...> args;
    };

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};

template <typename F, typename... Args>
requires (std::constructible_from<std::decay_t<F>, F> && ... &&
    std::constructible_from<std::decay_t<Args>, Args>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto bind_back(F&& func,
    Args&&... args) noexcept((std::is_nothrow_constructible_v<std::decay_t<F>,
                                  F> &&
    ... && std::is_nothrow_constructible_v<std::decay_t<Args>, Args>)) {
    return bind_back_t<std::decay_t<F>, std::decay_t<Args>...>{
        std::forward<F>(func), std::forward<Args>(args)...};
}

template <typename F, size_t N>
class set_arity_t {
public:
    __RXX_HIDE_FROM_ABI constexpr set_arity_t() noexcept(
        std::is_nothrow_default_constructible_v<F>) = default;

    template <typename Func>
    requires std::constructible_from<F, Func>
    __RXX_HIDE_FROM_ABI constexpr set_arity_t(Func&& func) noexcept(
        std::is_nothrow_constructible_v<F, Func>)
        : func_{std::forward<Func>(func)} {}

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F const&, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) const& noexcept(
        std::is_nothrow_invocable_v<F const&, Us...>) -> decltype(auto) {
        return std::invoke(func_, std::forward<Us>(args)...);
    }

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F const, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto
    operator()(Us&&... args) const&& noexcept(
        std::is_nothrow_invocable_v<F const, Us...>) -> decltype(auto) {
        return std::invoke(std::move(func_), std::forward<Us>(args)...);
    }

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F&, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) & noexcept(
        std::is_nothrow_invocable_v<F&, Us...>) -> decltype(auto) {
        return std::invoke(func_, std::forward<Us>(args)...);
    }

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) && noexcept(
        std::is_nothrow_invocable_v<F, Us...>) -> decltype(auto) {
        return std::invoke(std::move(func_), std::forward<Us>(args)...);
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) F func_;
};

template <size_t Min, typename F>
requires std::constructible_from<std::decay_t<F>, F>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto set_arity(
    F&& func) noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
    return set_arity_t<std::decay_t<F>, Min>{std::forward<F>(func)};
}

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
