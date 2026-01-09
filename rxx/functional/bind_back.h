// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/tuple.h"
#include "rxx/utility.h"

#include <concepts>
#include <type_traits>

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
        : container_{std::in_place, __RXX forward<Func>(func),
              __RXX forward<Args>(args)...} {}

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
        return apply(
            [&](Ts const&... bound) -> decltype(auto) {
                return std::invoke(container_.data.func.data,
                    __RXX forward<Us>(args)..., bound...);
            },
            container_.data.args);
    }

    template <typename... Us>
    requires std::regular_invocable<F const, Us..., Ts const...>
    __RXX_HIDE_FROM_ABI constexpr auto
    operator()(Us&&... args) const&& noexcept(
        std::is_nothrow_invocable_v<F const, Us..., Ts const...>)
        -> decltype(auto) {
        return apply(
            [&](Ts const&&... bound) -> decltype(auto) {
                return std::invoke(__RXX move(container_.data.func.data),
                    __RXX forward<Us>(args)..., __RXX move(bound)...);
            },
            __RXX move(container_.data.args));
    }

    template <typename... Us>
    requires std::regular_invocable<F&, Us..., Ts&...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) & noexcept(
        std::is_nothrow_invocable_v<F&, Us..., Ts&...>) -> decltype(auto) {
        return apply(
            [&](Ts&... bound) -> decltype(auto) {
                return std::invoke(container_.data.func.data,
                    __RXX forward<Us>(args)..., bound...);
            },
            container_.data.args);
    }

    template <typename... Us>
    requires std::regular_invocable<F, Us..., Ts...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) && noexcept(
        std::is_nothrow_invocable_v<F, Us..., Ts...>) -> decltype(auto) {
        return apply(
            [&](Ts&&... bound) -> decltype(auto) {
                return std::invoke(__RXX move(container_.data.func.data),
                    __RXX forward<Us>(args)..., __RXX move(bound)...);
            },
            __RXX move(container_.data.args));
    }

private:
    __RXX_HIDE_FROM_ABI static constexpr bool place_args_in_tail =
        __RXX details::fits_in_tail_padding_v<F, tuple<Ts...>>;
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
            : func{std::in_place, __RXX forward<Func>(func)}
            , args{__RXX forward<Args>(args)...} {}
        __RXX_HIDE_FROM_ABI constexpr container(container const&) = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) = default;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container&&) = default;

        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
        overlappable_if<place_args_in_tail, F> func;
        RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) tuple<Ts...> args;
    };

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};
} // namespace ranges::details

template <typename F, typename... Args>
requires (std::constructible_from<std::decay_t<F>, F> && ... &&
    std::constructible_from<std::decay_t<Args>, Args>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto bind_back(F&& func, Args&&... args) noexcept(
    (std::is_nothrow_constructible_v<std::decay_t<F>, F> && ... &&
        std::is_nothrow_constructible_v<std::decay_t<Args>, Args>)) {
    return ranges::details::bind_back_t<std::decay_t<F>, std::decay_t<Args>...>{
        __RXX forward<F>(func), __RXX forward<Args>(args)...};
}

RXX_DEFAULT_NAMESPACE_END
