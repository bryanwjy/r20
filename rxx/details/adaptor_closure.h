// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/functional/bind_back.h"
#include "rxx/utility/forward.h"

#include <ranges> // IWYU pragma: keep
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

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

template <typename Cpo>
using adaptor_non_closure = std::views::__adaptor::_RangeAdaptor<Cpo>;

#  if RXX_LIBSTDCXX_AT_LEAST(14)
template <typename Derived>
using adaptor_closure RXX_NODEBUG =
    std::views::__adaptor::_RangeAdaptorClosure<Derived>;
#  else
template <typename Derived>
using adaptor_closure RXX_NODEBUG = std::views::__adaptor::_RangeAdaptorClosure;
#  endif

template <typename F>
requires std::is_object_v<F>
class pipeable : public F, public adaptor_closure<pipeable<F>> {
public:
    using F::operator();
    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr pipeable(U&& func)
        : F{__RXX forward<U>(func)} {}
};

template <typename F>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&& func) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
    static_assert(std::is_object_v<F>);
    using Base = std::decay_t<F>;
    return pipeable<Base>(__RXX forward<F>(func));
}

template <typename F, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
constexpr auto make_pipeable(F&& func, Args&&... args) noexcept(
    (std::is_nothrow_constructible_v<std::decay_t<F>, F> && ... &&
        std::is_nothrow_constructible_v<std::decay_t<Args>, Args>)) {
    return make_pipeable(
        __RXX bind_back(__RXX forward<F>(func), __RXX forward<Args>(args)...));
}

#else
#  error "Unsupported standard library"
#endif

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
