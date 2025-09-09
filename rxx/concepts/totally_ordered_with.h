// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/boolean_testable.h"
#include "rxx/concepts/equality_comparable_with.h"
#include "rxx/type_traits/common_reference.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <typename T, typename U>
concept partially_ordered_with = requires(
    std::remove_reference_t<T> const& t, std::remove_reference_t<U> const& u) {
    { t < u } -> boolean_testable;
    { t > u } -> boolean_testable;
    { t <= u } -> boolean_testable;
    { t >= u } -> boolean_testable;
    { u < t } -> boolean_testable;
    { u > t } -> boolean_testable;
    { u <= t } -> boolean_testable;
    { u >= t } -> boolean_testable;
};
}

template <typename T>
concept totally_ordered =
    equality_comparable<T> && details::partially_ordered_with<T, T>;

template <typename T, typename U>
concept totally_ordered_with = totally_ordered<T> && totally_ordered<U> &&
    equality_comparable_with<T, U> &&
    totally_ordered<common_reference_t<std::remove_reference_t<T> const&,
        std::remove_reference_t<U> const&>> &&
    details::partially_ordered_with<T, U>;

RXX_DEFAULT_NAMESPACE_END
