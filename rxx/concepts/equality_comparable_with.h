// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/concepts/boolean_testable.h"
#include "rxx/type_traits/common_reference.h"

#include <concepts>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename T, typename U,
    typename C = common_reference_t<T const&, U const&>>
concept comparison_common_type_with_impl =
    std::same_as<common_reference_t<T const&, U const&>,
        common_reference_t<U const&, T const&>> &&
    requires {
        requires std::convertible_to<T const&, C const&> ||
            std::convertible_to<T, C const&>;
        requires std::convertible_to<U const&, C const&> ||
            std::convertible_to<U, C const&>;
    };

template <typename T, typename U>
concept comparison_common_type_with =
    comparison_common_type_with_impl<std::remove_cvref_t<T>,
        std::remove_cvref_t<U>>;

template <typename L, typename R>
concept weakly_equality_comparable_with =
    requires(std::remove_reference_t<L> const& left,
        std::remove_reference_t<R> const& right) {
        { left == right } -> boolean_testable;
        { left != right } -> boolean_testable;
        { right == left } -> boolean_testable;
        { right != left } -> boolean_testable;
    };
} // namespace details

template <typename T>
concept equality_comparable = details::weakly_equality_comparable_with<T, T>;

template <typename T, typename U>
concept equality_comparable_with = equality_comparable<T> &&
    equality_comparable<U> && details::comparison_common_type_with<T, U> &&
    equality_comparable<common_reference_t<std::remove_reference_t<T> const&,
        std::remove_reference_t<U> const&>> &&
    details::weakly_equality_comparable_with<T, U>;

RXX_DEFAULT_NAMESPACE_END
