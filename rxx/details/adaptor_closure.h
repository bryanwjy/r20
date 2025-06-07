// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <ranges>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

#if RXX_MSVC_STL
template <typename Derived>
using adaptor_closure RXX_NODEBUG = std::ranges::_Pipe::_Base<Derived>;

#elif RXX_LIBCXX
#  if RXX_LIBCXX_AT_LEAST(19, 01, 00)
template <typename Derived>
using adaptor_closure RXX_NODEBUG =
    std::ranges::__range_adaptor_closure<Derived>;
#  else
template <typename Derived>
using adaptor_closure RXX_NODEBUG = std::__range_adaptor_closure<Derived>;
#  endif

#elif RXX_LIBSTDCXX

template <typename Derived>
using adaptor_closure RXX_NODEBUG =
    std::views::__adaptor::_RangeAdaptorClosure<Derived>

#else
#  error "Unsupported standard library"
#endif

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
