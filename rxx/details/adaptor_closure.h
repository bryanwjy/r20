// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>
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

#  if RXX_LIBCXX_AT_LEAST(20, 01, 00)
    return std::ranges::__pipeable<std::decay_t<F>>{std::forward<F>(func)};
#  elif RXX_LIBCXX_AT_LEAST(19, 01, 00)
    return std::ranges::__range_adaptor_closure_t<std::decay_t<F>>{
        std::forward<F>(func)};
#  else
    return std::__range_adaptor_closure_t<std::decay_t<F>>{
        std::forward<F>(func)};
#  endif
}

#elif RXX_LIBSTDCXX

template <typename Cpo>
using adaptor_non_closure = std::views::__adaptor::_RangeAdaptor<Cpo>;

template <typename Derived>
using adaptor_closure RXX_NODEBUG =
    std::views::__adaptor::_RangeAdaptorClosure<Derived>

#else
#  error "Unsupported standard library"
#endif

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
