// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/boolean_testable.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename L, typename R>
concept supports_equal_with = requires(L const& lhs, R const& rhs) {
    { lhs == rhs } -> boolean_testable;
};

template <typename L, typename R>
concept supports_inequality_with = requires(L const& lhs, R const& rhs) {
    { lhs != rhs } -> boolean_testable;
};

template <typename L, typename R>
concept supports_less_than_with = requires(L const& lhs, R const& rhs) {
    { lhs < rhs } -> boolean_testable;
};

template <typename L, typename R>
concept supports_greater_than_with = requires(L const& lhs, R const& rhs) {
    { lhs > rhs } -> boolean_testable;
};

template <typename L, typename R>
concept supports_less_equal_with = requires(L const& lhs, R const& rhs) {
    { lhs <= rhs } -> boolean_testable;
};

template <typename L, typename R>
concept supports_greater_equal_with = requires(L const& lhs, R const& rhs) {
    { lhs >= rhs } -> boolean_testable;
};

} // namespace details

RXX_DEFAULT_NAMESPACE_END
