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

template <typename Derived>
using adaptor_closure RXX_NODEBUG = std::__range_adaptor_closure<Derived>;

#elif RXX_LIBSTDCXX

template <typename Derived>
using adaptor_closure RXX_NODEBUG =
    std::views::__adaptor::_RangeAdaptorClosure<Derived>

#else
#  error "Unsupported standard library"
#endif

} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
