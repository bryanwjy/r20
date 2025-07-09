// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/details/integer_like.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/iterator.h"
#include "rxx/primitives.h"

#include <concepts>
#include <initializer_list>
#include <type_traits>

#if __has_include(<__ranges/enable_borrowed_range.h>)
#  include <__ranges/enable_borrowed_range.h>
#elif __has_include(<bits/ranges_base.h>)
#  include <bits/ranges_base.h>
#elif __has_include(<xutility>)
#  include <xutility>
#else
#  if RXX_LIBSTDCXX | RXX_MSVC_STL | RXX_LIBCXX
#    warning "Outdated standard library detected"
#  endif
#  include <ranges>
#endif

#if __has_include(<__ranges/enable_view.h>)
#  include <__ranges/enable_view.h>
#elif __has_include(<bits/ranges_base.h>)
#  include <bits/ranges_base.h>
#elif __has_include(<xutility>)
#  include <xutility>
#else
#  if RXX_LIBSTDCXX | RXX_MSVC_STL | RXX_LIBCXX
#    warning "Outdated standard library detected"
#  endif
#  include <ranges>
#endif

#if __has_include(<__ranges/size.h>)
#  include <__ranges/size.h>
#elif __has_include(<bits/ranges_base.h>)
#  include <bits/ranges_base.h>
#elif __has_include(<xutility>)
#  include <xutility>
#else
#  if RXX_LIBSTDCXX | RXX_MSVC_STL | RXX_LIBCXX
#    warning "Outdated standard library detected"
#  endif
#  include <ranges>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

#define __RXX_AUTOCAST(EXPR) static_cast<::std::decay_t<decltype((EXPR))>>(EXPR)

namespace ranges {

using ::std::ranges::disable_sized_range;
using ::std::ranges::enable_borrowed_range;
using ::std::ranges::enable_view;

namespace details {

template <typename T>
inline constexpr bool is_initializer_list = false;
template <typename T>
inline constexpr bool is_initializer_list<std::initializer_list<T>> = true;

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

template <typename T>
concept range = requires(T& t) {
    __RXX ranges::begin(t);
    __RXX ranges::end(t);
};

template <typename T>
concept input_range = range<T> && std::input_iterator<iterator_t<T>>;

template <typename T>
concept forward_range = input_range<T> && std::forward_iterator<iterator_t<T>>;

template <typename T>
concept bidirectional_range =
    forward_range<T> && std::bidirectional_iterator<iterator_t<T>>;

template <typename T>
concept random_access_range =
    bidirectional_range<T> && std::random_access_iterator<iterator_t<T>>;

template <typename T>
concept contiguous_range = random_access_range<T> &&
    std::contiguous_iterator<ranges::iterator_t<T>> && requires(T& t) {
        {
        __RXX ranges::data(t)
        } -> std::same_as<std::add_pointer_t<ranges::range_reference_t<T>>>;
    };

template <typename T>
concept common_range = range<T> && std::same_as<iterator_t<T>, sentinel_t<T>>;

template <typename T>
concept view = range<T> && std::movable<T> && enable_view<T>;

template <class T>
concept viewable_range = range<T> &&
    ((view<std::remove_cvref_t<T>> &&
         std::constructible_from<std::remove_cvref_t<T>, T>) ||
        (!view<std::remove_cvref_t<T>> &&
            (std::is_lvalue_reference_v<T> ||
                (std::movable<std::remove_reference_t<T>> &&
                    !details::is_initializer_list<T>))));

template <typename T>
concept constant_range =
    input_range<T> && __RXX details::constant_iterator<iterator_t<T>>;

template <range R>
using const_iterator_t = const_iterator<iterator_t<R>>;

template <range R>
using const_sentinel_t = const_sentinel<sentinel_t<R>>;

template <range R>
using range_const_reference_t = iter_const_reference_t<ranges::iterator_t<R>>;

namespace details {
template <std::ranges::input_range R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto& possibly_const_range(R& r) noexcept {
    if constexpr (std::ranges::input_range<R const>) {
        return const_cast<R const&>(r);
    } else {
        return r;
    }
}

struct cbegin_t {
    template <borrowable T>
    requires requires(T&& arg) {
        make_const_iterator(ranges::begin(possibly_const_range(arg)));
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(
        noexcept(make_const_iterator(ranges::begin(possibly_const_range(arg)))))
        -> decltype(auto) {

        auto& ref = details::possibly_const_range(arg);
        return const_iterator_t<decltype(ref)>(ranges::begin(ref));
    }
};

struct cend_t {
    template <borrowable T>
    requires requires(T&& arg) {
        make_const_sentinel(ranges::end(possibly_const_range(arg)));
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(
        noexcept(make_const_sentinel(ranges::end(possibly_const_range(arg)))))
        -> decltype(auto) {

        auto& ref = details::possibly_const_range(arg);
        return const_sentinel_t<decltype(ref)>(ranges::end(ref));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr ranges::details::cbegin_t cbegin{};
inline constexpr ranges::details::cend_t cend{};
} // namespace cpo

namespace details {
void size(...) = delete;

template <typename T>
concept member_size =
    !disable_sized_range<std::remove_cvref_t<T>> && requires(T& arg) {
        { __RXX_AUTOCAST(arg.size()) } -> integer_like;
    };

template <typename T>
concept unqualified_size = class_or_enum<std::remove_reference_t<T>> &&
    !disable_sized_range<std::remove_cvref_t<T>> && requires(T& arg) {
        { __RXX_AUTOCAST(size(arg)) } -> integer_like;
    };

template <typename T>
concept sentinel_size = requires(T& arg) {
    requires (!std::is_unbounded_array_v<std::remove_reference_t<T>>);

    { ranges::begin(arg) } -> std::forward_iterator;

    {
        ranges::end(arg)
    } -> std::sized_sentinel_for<decltype(ranges::begin(arg))>;

    details::to_unsigned_like(ranges::end(arg) - ranges::begin(arg));
};

struct size_func_t {

    template <typename T, size_t N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T (&)[N]) RXX_CONST_CALL noexcept
    requires (sizeof(T) >= 0) // Disallow incomplete element types.
    {
        return N;
    }

    template <typename T>
    requires member_size<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(noexcept(arg.size())) {
        return arg.size();
    }

    template <typename T>
    requires unqualified_size<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(noexcept(size(arg))) {
        return size(arg);
    }

    template <typename T>
    requires sentinel_size<T> || (!member_size<T> && !unqualified_size<T>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(noexcept(
        details::to_unsigned_like(ranges::end(arg) - ranges::begin(arg)))) {
        return details::to_unsigned_like(ranges::end(arg) - ranges::begin(arg));
    }
};

} // namespace details

inline namespace cpo {
inline constexpr ranges::details::size_func_t size{};
} // namespace cpo

namespace details {

void reserve_hint(...) noexcept = delete;
template <typename T>
concept member_reserve_hint = borrowable<T> && requires(T&& val) {
    { __RXX_AUTOCAST(val.reserve_hint()) } -> integer_like;
};

template <typename T>
concept unqualified_reserve_hint = !member_reserve_hint<T> && borrowable<T> &&
    class_or_enum<std::remove_cvref_t<T>> && requires(T&& val) {
        { __RXX_AUTOCAST(reserve_hint(val)) } -> integer_like;
    };

struct reserve_hint_t {
    template <typename T>
    requires requires(T&& arg) { ranges::size(arg); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept {
        return ranges::size(arg);
    }

    template <typename T>
    requires member_reserve_hint<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(arg.reserve_hint()))) {
        return __RXX_AUTOCAST(arg.reserve_hint());
    }

    template <typename T>
    requires unqualified_reserve_hint<T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(reserve_hint(arg)))) {
        return __RXX_AUTOCAST(reserve_hint(arg));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr ranges::details::reserve_hint_t reserve_hint{};
} // namespace cpo

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
