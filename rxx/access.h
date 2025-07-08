// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#if __has_include(<__ranges/enable_borrowed_range.h>)
#  include <__ranges/enable_borrowed_range.h>
#elif __has_include(<bits/ranges_base.h>)
#  include <bits/ranges_base.h>
#elif __has_include(<xutility>)
#  include <xutility>
#else
#  include <ranges>
#endif

#include <iterator>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

#define __RXX_AUTOCAST(EXPR) static_cast<::std::decay_t<decltype((EXPR))>>(EXPR)

namespace ranges {

using ::std::ranges::enable_borrowed_range;
namespace details {

template <typename T>
concept borrowable = std::is_lvalue_reference_v<T> ||
    enable_borrowed_range<std::remove_cvref_t<T>>;
template <typename T>
concept class_or_enum =
    std::is_class_v<T> || std::is_union_v<T> || std::is_enum_v<T>;

__RXX_HIDE_FROM_ABI void begin(...) noexcept = delete;

template <typename T>
concept member_begin = borrowable<T> && requires(T&& val) {
    { __RXX_AUTOCAST(val.begin()) } -> std::input_or_output_iterator;
};

template <typename T>
concept unqualified_begin = !member_begin<T> && borrowable<T> &&
    class_or_enum<std::remove_cvref_t<T>> && requires(T&& val) {
        { __RXX_AUTOCAST(begin(val)) } -> std::input_or_output_iterator;
    };

struct begin_t {
public:
    template <typename T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        T (&arg)[]) RXX_CONST_CALL noexcept
    requires (sizeof(T) >= 0) // Disallow incomplete element types.
    {
        return arg + 0;
    }

    template <typename T, size_t N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        T (&arg)[N]) RXX_CONST_CALL noexcept
    requires (sizeof(T) >= 0) // Disallow incomplete element types.
    {
        return arg + 0;
    }

    template <typename T>
    requires member_begin<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(arg.begin()))) {
        return __RXX_AUTOCAST(arg.begin());
    }

    template <typename T>
    requires unqualified_begin<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(begin(arg)))) {
        return __RXX_AUTOCAST(begin(arg));
    }

    void operator()(auto&&) const = delete;
};

} // namespace details

inline namespace cpo {
inline constexpr ranges::details::begin_t begin{};
} // namespace cpo

template <typename T>
using iterator_t = decltype(__RXX ranges::begin(std::declval<T&>()));

namespace details {

template <typename T>
concept member_end = borrowable<T> && requires(T&& val) {
    { __RXX_AUTOCAST(val.end()) } -> std::sentinel_for<iterator_t<T>>;
};

template <typename T>
concept unqualified_end = !member_end<T> && borrowable<T> &&
    class_or_enum<std::remove_cvref_t<T>> && requires(T&& val) {
        { __RXX_AUTOCAST(end(val)) } -> std::sentinel_for<iterator_t<T>>;
    };

struct end_t {
    template <typename T, size_t N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(
        T (&arg)[N]) RXX_CONST_CALL noexcept
    requires (sizeof(T) >= 0) // Disallow incomplete element types.
    {
        return arg + N;
    }

    template <typename T>
    requires member_end<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(arg.end()))) {
        return __RXX_AUTOCAST(arg.end());
    }

    template <typename T>
    requires unqualified_end<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(end(arg)))) {
        return __RXX_AUTOCAST(end(arg));
    }

    void operator()(auto&&) const = delete;
};
} // namespace details

inline namespace cpo {
inline constexpr ranges::details::end_t end{};
} // namespace cpo

template <typename T>
using sentinel_t = decltype(__RXX ranges::end(std::declval<T&>()));

namespace details {
struct cbegin_t {
    template <typename T>
    requires std::is_lvalue_reference_v<T&&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(noexcept(__RXX ranges::begin(
        static_cast<std::remove_reference_t<T> const&>(arg))))
        -> decltype(__RXX ranges::begin(
            static_cast<std::remove_reference_t<T> const&>(arg))) {
        return __RXX ranges::begin(
            static_cast<std::remove_reference_t<T> const&>(arg));
    }

    template <typename T>
    requires std::is_rvalue_reference_v<T&&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX ranges::begin(static_cast<T const&&>(arg))))
            -> decltype(__RXX ranges::begin(static_cast<T const&&>(arg))) {
        return __RXX ranges::begin(static_cast<T const&&>(arg));
    }
};

struct cend_t {
    template <typename T>
    requires std::is_lvalue_reference_v<T&&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(noexcept( __RXX ranges::end(
        static_cast<std::remove_reference_t<T> const&>(arg))))
        -> decltype(__RXX ranges::end(
            static_cast<std::remove_reference_t<T> const&>(arg))) {
        return __RXX ranges::end(
            static_cast<std::remove_reference_t<T> const&>(arg));
    }

    template <typename T>
    requires std::is_rvalue_reference_v<T&&>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX ranges::end(static_cast<T const&&>(arg))))
            -> decltype(__RXX ranges::end(static_cast<T const&&>(arg))) {
        return __RXX ranges::end(static_cast<T const&&>(arg));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr ranges::details::cbegin_t cbegin{};
inline constexpr ranges::details::cend_t cend{};
} // namespace cpo

namespace details {
template <typename T>
concept ptr_to_object =
    std::is_pointer_v<T> && std::is_object_v<std::remove_pointer_t<T>>;

template <typename T>
concept member_data = borrowable<T> && requires(T&& val) {
    { __RXX_AUTOCAST(val.data()) } -> std::sentinel_for<iterator_t<T>>;
};

template <typename T>
concept ranges_contiguous_begin =
    !member_data<T> && borrowable<T> && requires(T&& val) {
        { ranges::begin(val) } -> std::contiguous_iterator;
    };

struct data_t {
    template <member_data T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(arg.data())) {
        return arg.data();
    }

    template <ranges_contiguous_begin T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(std::to_address(ranges::begin(arg)))) {
        return std::to_address(ranges::begin(arg));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr ranges::details::data_t data{};
} // namespace cpo

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
