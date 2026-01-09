// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/functional/bind_back.h"
#include "rxx/utility/forward.h"

#include <ranges> // IWYU pragma: keep
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <typename F, size_t N>
class set_arity_t {
public:
#if RXX_LIBSTDCXX
    static constexpr int _S_arity = N;
#endif

    __RXX_HIDE_FROM_ABI constexpr set_arity_t() noexcept(
        std::is_nothrow_default_constructible_v<F>) = default;

    template <typename Func>
    requires std::constructible_from<F, Func>
    __RXX_HIDE_FROM_ABI constexpr set_arity_t(Func&& func) noexcept(
        std::is_nothrow_constructible_v<F, Func>)
        : func_{__RXX forward<Func>(func)} {}

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F const&, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) const& noexcept(
        std::is_nothrow_invocable_v<F const&, Us...>) -> decltype(auto) {
        return std::invoke(func_, __RXX forward<Us>(args)...);
    }

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F const, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto
    operator()(Us&&... args) const&& noexcept(
        std::is_nothrow_invocable_v<F const, Us...>) -> decltype(auto) {
        return std::invoke(__RXX move(func_), __RXX forward<Us>(args)...);
    }

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F&, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) & noexcept(
        std::is_nothrow_invocable_v<F&, Us...>) -> decltype(auto) {
        return std::invoke(func_, __RXX forward<Us>(args)...);
    }

    template <typename... Us>
    requires (sizeof...(Us) == N) && std::regular_invocable<F, Us...>
    __RXX_HIDE_FROM_ABI constexpr auto operator()(Us&&... args) && noexcept(
        std::is_nothrow_invocable_v<F, Us...>) -> decltype(auto) {
        return std::invoke(__RXX move(func_), __RXX forward<Us>(args)...);
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) F func_;
};

template <size_t Min, typename F>
requires std::constructible_from<std::decay_t<F>, F>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto set_arity(F&& func) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
    return set_arity_t<std::decay_t<F>, Min>{__RXX forward<F>(func)};
}

/**
 * The `adaptor_non_closure_type` are deliberately left empty as it
 * provides an easy way to locate all the non_closure customization
 * point.
 */

#if RXX_MSVC_STL
template <typename Cpo>
struct adaptor_non_closure_type {};

template <typename Cpo>
using adaptor_non_closure RXX_NODEBUG = adaptor_non_closure_type<Cpo>;

template <typename Derived>
using adaptor_closure RXX_NODEBUG = std::ranges::_Pipe::_Base<Derived>;

template <typename F, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&&, Args&&... args) noexcept(
    std::is_nothrow_constructible_v<
        std::ranges::_Range_closure<std::decay_t<F>, std::decay_t<Args>...>,
        Args...>) {
    return std::ranges::_Range_closure<std::decay_t<F>, std::decay_t<Args>...>{
        __RXX forward<Args>(args)...};
}

#elif RXX_LIBCXX

template <typename Cpo>
struct adaptor_non_closure_type {};

template <typename Cpo>
using adaptor_non_closure RXX_NODEBUG = adaptor_non_closure_type<Cpo>;

#  if RXX_LIBCXX_AT_LEAST(19, 01, 00)
template <typename Derived>
using adaptor_closure RXX_NODEBUG =
    std::ranges::__range_adaptor_closure<Derived>;
#  else
template <typename Derived>
using adaptor_closure RXX_NODEBUG = std::__range_adaptor_closure<Derived>;
#  endif

template <typename F>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&& func) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
#  if RXX_LIBCXX_AT_LEAST(20, 00, 00)
    return std::ranges::__pipeable<std::decay_t<F>>{__RXX forward<F>(func)};
#  elif RXX_LIBCXX_AT_LEAST(19, 01, 00)
    return std::ranges::__range_adaptor_closure_t<std::decay_t<F>>{
        __RXX forward<F>(func)};
#  else
    return std::__range_adaptor_closure_t<std::decay_t<F>>{
        __RXX forward<F>(func)};
#  endif
}

template <typename F, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&& func, Args&&... args) noexcept(
    (std::is_nothrow_constructible_v<std::decay_t<F>, F> && ... &&
        std::is_nothrow_constructible_v<std::decay_t<Args>, Args>)) {
    return make_pipeable(
        __RXX bind_back(__RXX forward<F>(func), __RXX forward<Args>(args)...));
}

#elif RXX_LIBSTDCXX

#  if RXX_LIBSTDCXX_AT_LEAST(14)
template <typename Derived>
using std_adaptor_closure RXX_NODEBUG =
    std::views::__adaptor::_RangeAdaptorClosure<Derived>;

template <typename Adaptor>
concept is_std_adaptor_closure = requires(Adaptor const& val) {
    []<typename T, typename U>
    requires (!std::same_as<T, std_adaptor_closure<U>>)
    (T const&, std_adaptor_closure<U> const&) {}(val, val);
};

#  else
template <typename Derived>
using std_adaptor_closure RXX_NODEBUG =
    std::views::__adaptor::_RangeAdaptorClosure;

template <typename Adaptor>
concept is_std_adaptor_closure = requires(Adaptor const& val) {
    []<typename T>
    requires (!std::same_as<T, std::views::__adaptor::_RangeAdaptorClosure>)
    (T const&, std::views::__adaptor::_RangeAdaptorClosure const&) {}(val, val);
};
#  endif

template <typename T>
class adaptor_closure;

template <typename Adaptor>
concept is_local_adaptor_closure =
    is_std_adaptor_closure<Adaptor> && requires(Adaptor const& val) {
        []<typename T, typename U>
        requires (!std::same_as<T, adaptor_closure<U>>)
        (T const&, adaptor_closure<U> const&) {}(val, val);
    };

template <typename Adaptor>
concept is_range_adaptor_closure =
    is_std_adaptor_closure<Adaptor> || is_local_adaptor_closure<Adaptor>;

/**
 * Helper classes to bypass libstdc++ due to a bug in C++23 prior to
 * libstdc++ 14.3 when compiling with clang
 *
 * Note that, unlike libstdc++, the following implemenatation does not add
 * specializations to make overload resolution diagnostics more concise.
 */
template <typename L, typename R>
struct pipe;

template <typename Adaptor, typename... Args>
concept adaptor_invocable =
    requires { std::declval<Adaptor>()(std::declval<Args>()...); };

template <typename T>
class adaptor_closure : public std_adaptor_closure<T> {
    template <typename Range>
    requires (!is_range_adaptor_closure<Range>) && adaptor_invocable<T, Range>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator|(Range&& range, T&& self) noexcept(
        noexcept(__RXX move(self)(__RXX forward<Range>(range)))) {
        return __RXX move(self)(__RXX forward<Range>(range));
    }

    template <typename Range>
    requires (!is_range_adaptor_closure<Range>) &&
        adaptor_invocable<T const&, Range>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator|(Range&& range, T const& self) noexcept(
        noexcept(self(__RXX forward<Range>(range)))) {
        return self(__RXX forward<Range>(range));
    }

    template <is_range_adaptor_closure R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator|(T const& lhs, R&& rhs) noexcept(
        std::is_nothrow_copy_constructible_v<T> &&
        std::is_nothrow_constructible_v<std::decay_t<R>, R>) {
        return pipe<T, std::decay_t<R>>(lhs, __RXX forward<R>(rhs));
    }

    template <is_range_adaptor_closure R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator|(T&& lhs, R&& rhs) noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_constructible_v<std::decay_t<R>, R>) {
        return pipe<T, std::decay_t<R>>(__RXX move(lhs), __RXX forward<R>(rhs));
    }

    template <is_range_adaptor_closure L>
    requires (!is_local_adaptor_closure<L>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator|(L&& lhs, T const& rhs) noexcept(
        std::is_nothrow_copy_constructible_v<T> &&
        std::is_nothrow_constructible_v<std::decay_t<L>, L>) {
        return pipe<std::decay_t<L>, T>(__RXX forward<L>(lhs), rhs);
    }

    template <is_range_adaptor_closure L>
    requires (!is_local_adaptor_closure<L>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator|(L&& lhs, T&& rhs) noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_constructible_v<std::decay_t<L>, L>) {
        return pipe<std::decay_t<L>, T>(__RXX forward<L>(lhs), __RXX move(rhs));
    }
};

template <typename F, size_t Arity>
requires std::is_object_v<F>
class pipeable : public F, public adaptor_closure<pipeable<F, Arity>> {
public:
    static constexpr int _S_arity = Arity;
    using F::operator();
    template <typename U>
    __RXX_HIDE_FROM_ABI explicit constexpr pipeable(U&& func)
        : F{__RXX forward<U>(func)} {}
};

template <size_t Arity = 1, typename F>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&& func) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
    static_assert(std::is_object_v<F>);
    using Base = std::decay_t<F>;
    return pipeable<Base, Arity>(__RXX forward<F>(func));
}

template <typename F, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&& func, Args&&... args) noexcept(
    (std::is_nothrow_constructible_v<std::decay_t<F>, F> && ... &&
        std::is_nothrow_constructible_v<std::decay_t<Args>, Args>)) {
    return make_pipeable<1 + sizeof...(Args)>(
        __RXX bind_back(__RXX forward<F>(func), __RXX forward<Args>(args)...));
}

template <typename L, typename R, typename Range>
concept pipe_invocable =
    requires { std::declval<R>()(std::declval<L>()(std::declval<Range>())); };

template <typename L, typename R>
struct pipe : public adaptor_closure<pipe<L, R>> {

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) L lhs;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) R rhs;

    template <typename T, typename U>
    __RXX_HIDE_FROM_ABI explicit constexpr pipe(T&& lhs, U&& rhs) noexcept(
        std::is_nothrow_constructible_v<L, T> &&
        std::is_nothrow_constructible_v<R, U>)
        : lhs(__RXX forward<T>(lhs))
        , rhs(__RXX forward<U>(rhs)) {}

    template <typename Range>
    requires pipe_invocable<L const&, R const&, Range>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator()(Range&& range) const& {
        return rhs(lhs(__RXX forward<Range>(range)));
    }

    template <typename Range>
    requires pipe_invocable<L, R, Range>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator()(Range&& range) && {
        return __RXX move(rhs)(__RXX move(lhs)(__RXX forward<Range>(range)));
    }

    template <typename Range>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator()(Range&&) const&& = delete;
};

template <typename Cpo>
struct adaptor_non_closure_type {};

template <typename Cpo>
using adaptor_non_closure = adaptor_non_closure_type<Cpo>;

#else
#  error "Unsupported standard library"
#endif

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
