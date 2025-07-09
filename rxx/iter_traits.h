// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include <iterator>

RXX_DEFAULT_NAMESPACE_BEGIN

using std::iter_common_reference_t;
using std::iter_difference_t;
using std::iter_reference_t;
using std::iter_rvalue_reference_t;
using std::iter_value_t;

template <std::indirectly_readable It>
using iter_const_reference_t =
    std::common_reference_t<iter_value_t<It> const&&, iter_reference_t<It>>;

template <std::indirectly_readable It>
using iter_const_rvalue_reference_t =
    std::common_reference_t<iter_value_t<It> const&&,
        iter_rvalue_reference_t<It>>;

RXX_DEFAULT_NAMESPACE_END
