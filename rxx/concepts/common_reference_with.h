// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/type_traits/common_reference.h"

#if RXX_LIBCXX && __has_include(<__concepts/same_as.h>)
#  include <__concepts/same_as.h>
#else
#  include <concepts>
#endif
#if RXX_LIBCXX && __has_include(<__concepts/convertible_to.h>)
#  include <__concepts/convertible_to.h>
#else
#  include <concepts>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename T, typename U>
concept common_reference_with = std::same_as<__RXX common_reference_t<T, U>,
                                    __RXX common_reference_t<U, T>> &&
    std::convertible_to<T, __RXX common_reference_t<T, U>> &&
    std::convertible_to<U, __RXX common_reference_t<T, U>>;

RXX_DEFAULT_NAMESPACE_END
