// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/type_traits/common_reference.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename T>
concept boolean_testable_impl = std::convertible_to<T, bool> && requires {
    { std::declval<T>() ? true : false } -> std::same_as<bool>;
    requires !requires { operator&&(std::declval<T>(), std::declval<T>()); };
    requires !requires { operator||(std::declval<T>(), std::declval<T>()); };
    requires !requires { std::declval<T>().operator&&(std::declval<T>()); };
    requires !requires { std::declval<T>().operator||(std::declval<T>()); };
};

template <typename T>
concept boolean_testable = std::convertible_to<T, bool> && requires {
    { !std::declval<T>() } -> boolean_testable_impl;
};

} // namespace details

RXX_DEFAULT_NAMESPACE_END
