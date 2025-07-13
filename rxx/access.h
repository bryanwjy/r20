// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/integer_like.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/iterator.h"

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

__RXX_HIDE_FROM_ABI void end(...) noexcept = delete;

template <typename T>
concept member_end = borrowable<T> && requires(T&& val) {
    typename iterator_t<T>;
    { __RXX_AUTOCAST(val.end()) } -> std::sentinel_for<iterator_t<T>>;
};

template <typename T>
concept unqualified_end = !member_end<T> && borrowable<T> &&
    class_or_enum<std::remove_cvref_t<T>> && requires(T&& val) {
        typename iterator_t<T>;
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

    template <member_end T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(arg.end()))) {
        return __RXX_AUTOCAST(arg.end());
    }

    template <unqualified_end T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(__RXX_AUTOCAST(end(arg)))) {
        return __RXX_AUTOCAST(end(arg));
    }

    __RXX_HIDE_FROM_ABI void operator()(auto&&) const = delete;
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

namespace details {
template <input_range R>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto& possibly_const_range(R& r) noexcept {
    if constexpr (input_range<R const>) {
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
        return const_iterator<iterator_t<decltype(ref)>>(ranges::begin(ref));
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
        return const_sentinel<sentinel_t<decltype(ref)>>(ranges::end(ref));
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
concept enabled_size_range = !disable_sized_range<std::remove_cvref_t<T>>;

template <typename T>
concept member_size = enabled_size_range<T> && requires(T& arg) {
    { __RXX_AUTOCAST(arg.size()) } -> integer_like;
};

template <typename T>
concept unqualified_size =
    !member_size<T> && class_or_enum<std::remove_cvref_t<T>> &&
    enabled_size_range<T> && requires(T& arg) {
        { __RXX_AUTOCAST(size(arg)) } -> integer_like;
    };

template <typename T>
concept sentinel_size = !member_size<T> && !unqualified_size<T> &&
    class_or_enum<std::remove_cvref_t<T>> && requires(T&& arg) {
        { ranges::begin(arg) } -> std::forward_iterator;

        {
            ranges::end(arg)
        }
        -> std::sized_sentinel_for<decltype(ranges::begin(std::declval<T>()))>;

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

    template <typename T, size_t N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T (&&)[N]) RXX_CONST_CALL noexcept
    requires (sizeof(T) >= 0) // Disallow incomplete element types.
    {
        return N;
    }

    template <member_size T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(arg.size())) {
        return arg.size();
    }

    template <unqualified_size T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(size(arg))) {
        return size(arg);
    }

    template <sentinel_size T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(T&& arg) RXX_CONST_CALL
        noexcept(noexcept(
            details::to_unsigned_like(ranges::end(arg) - ranges::begin(arg)))) {
        return details::to_unsigned_like(ranges::end(arg) - ranges::begin(arg));
    }
};

} // namespace details

inline namespace cpo {
inline constexpr ranges::details::size_func_t size{};
} // namespace cpo

template <typename T>
concept sized_range = range<T> && requires(T& arg) { ranges::size(arg); };

namespace details {

struct ssize_t {
    template <typename T>
    requires requires(T&& arg) { ranges::size(arg); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& arg) RXX_CONST_CALL noexcept(noexcept(ranges::size(arg))) {
        using signed_type = std::make_signed_t<decltype(ranges::size(arg))>;
        if constexpr (sizeof(ptrdiff_t) > sizeof(signed_type)) {
            return static_cast<ptrdiff_t>(ranges::size(arg));
        } else {
            return static_cast<signed_type>(ranges::size(arg));
        }
    }
};

} // namespace details

inline namespace cpo {
inline constexpr ranges::details::ssize_t ssize{};
} // namespace cpo

namespace details {
void empty(...) = delete;

template <typename T>
concept member_empty = requires(T&& arg) { bool(arg.empty()); };

template <typename T>
concept size_invocable =
    !member_empty<T> && requires(T&& arg) { ranges::size(arg); };

template <typename T>
concept comparable_begin_and_end =
    !member_empty<T> && !size_invocable<T> && requires(T&& arg) {
        bool(ranges::begin(arg) == ranges::end(arg));
        { ranges::begin(arg) } -> std::forward_iterator;
    };

struct empty_t {
    template <member_empty T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator()(T&& arg) const
        noexcept(noexcept(bool(arg.empty()))) {
        return bool(arg.empty());
    }

    template <size_invocable T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator()(T&& arg) const
        noexcept(noexcept(ranges::size(arg))) {
        return ranges::size(arg) == 0;
    }

    template <comparable_begin_and_end T>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator()(T&& arg) const
        noexcept(noexcept(bool(ranges::begin(arg) == ranges::end(arg)))) {
        return ranges::begin(arg) == ranges::end(arg);
    }
};

} // namespace details

inline namespace cpo {
inline constexpr ranges::details::empty_t empty{};
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
