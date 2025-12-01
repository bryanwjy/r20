// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename From, typename To>
struct copy_cv {
    using type = To;
};
template <typename From, typename To>
struct copy_cv<From const, To> {
    using type = To const;
};
template <typename From, typename To>
struct copy_cv<From volatile, To> {
    using type = To volatile;
};
template <typename From, typename To>
struct copy_cv<From const volatile, To> {
    using type = To const volatile;
};

template <typename F, typename T>
using copy_cv_t = typename copy_cv<F, T>::type;

template <typename From, typename To>
struct copy_cvref : copy_cv<From, To> {};
template <typename From, typename To>
struct copy_cvref<From&, To> {
    using type = typename copy_cv<From, To>::type&;
};
template <typename From, typename To>
struct copy_cvref<From&&, To> {
    using type = typename copy_cv<From, To>::type&&;
};

template <typename F, typename T>
using copy_cvref_t = typename copy_cvref<F, T>::type;

RXX_DEFAULT_NAMESPACE_END
