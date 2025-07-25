// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/details/const_if.h"
#include "rxx/details/integer_like.h"
#include "rxx/details/movable_box.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/details/view_traits.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/get_element.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/tuple.h"
#include "rxx/utility.h"

#include <cassert>
#include <compare>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <typename Int>
struct wider_signed_int {
private:
    static consteval auto make_type() noexcept {
        if constexpr (sizeof(Int) < sizeof(short))
            return std::type_identity<short>{};
        else if constexpr (sizeof(Int) < sizeof(int))
            return std::type_identity<int>{};
        else if constexpr (sizeof(Int) < sizeof(long))
            return std::type_identity<long>{};
        else if constexpr (sizeof(Int) < sizeof(long long))
            return std::type_identity<long long>{};
#if RXX_SUPPORTS_INT128
        else {
            static_assert(sizeof(Int) <= sizeof(__int128_t),
                "Found integer-like type that is bigger than largest integer "
                "like "
                "type.");
            return std::type_identity<__int128_t>{};
        }
#else
        else {
            static_assert(sizeof(Int) <= sizeof(long long),
                "Found integer-like type that is bigger than largest integer "
                "like "
                "type.");
            return std::type_identity<long long>{};
        }
#endif
    }

public:
    using type RXX_NODEBUG = typename decltype(make_type())::type;
};

template <typename Start>
using iota_diff_t RXX_NODEBUG = typename std::conditional_t<
    (!std::integral<Start> || sizeof(iter_difference_t<Start>) > sizeof(Start)),
    std::type_identity<iter_difference_t<Start>>,
    wider_signed_int<Start>>::type;

template <typename T>
struct repeat_view_iterator_difference {
    using type RXX_NODEBUG = iota_diff_t<T>;
};

template <signed_integer_like T>
struct repeat_view_iterator_difference<T> {
    using type RXX_NODEBUG = T;
};

template <typename T>
using repeat_view_iterator_difference_t RXX_NODEBUG =
    typename repeat_view_iterator_difference<T>::type;
} // namespace details

template <std::move_constructible T,
    std::semiregular Bound = std::unreachable_sentinel_t>
requires std::is_object_v<T> && std::same_as<T, std::remove_cv_t<T>> &&
    (details::integer_like_with_usable_difference_type<Bound> ||
        std::same_as<Bound, std::unreachable_sentinel_t>)
class repeat_view : public view_interface<repeat_view<T, Bound>> {
    class iterator;

    using index_type RXX_NODEBUG =
        std::conditional_t<std::same_as<Bound, std::unreachable_sentinel_t>,
            ptrdiff_t, Bound>;
    using difference_t RXX_NODEBUG =
        details::repeat_view_iterator_difference_t<index_type>;

    static constexpr bool nothrow_difference = requires(index_type idx) {
        { static_cast<difference_t>(idx) } noexcept;
        { idx - idx } noexcept;
    };

public:
    __RXX_HIDE_FROM_ABI constexpr repeat_view() noexcept(
        std::is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr explicit repeat_view(T const& value,
        Bound bound =
            Bound{}) noexcept(std::is_nothrow_copy_constructible_v<T> &&
        std::is_nothrow_copy_constructible_v<Bound>)
    requires std::copy_constructible<T>
        : value_{value}
        , bound_{bound} {
        if constexpr (!std::same_as<Bound, std::unreachable_sentinel_t>) {
            assert(bound >= 0);
        }
    }

    __RXX_HIDE_FROM_ABI constexpr explicit repeat_view(T&& value,
        Bound bound =
            Bound{}) noexcept(std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_copy_constructible_v<Bound>)
        : value_{std::move(value)}
        , bound_{bound} {
        if constexpr (!std::same_as<Bound, std::unreachable_sentinel_t>) {
            assert(bound >= 0);
        }
    }

    template <tuple_like ArgsT, tuple_like ArgsB = tuple<>>
    requires requires {
        { __RXX make_from_tuple<T>(std::declval<ArgsT>())
        } -> std::same_as<T>;
        {
        __RXX make_from_tuple<Bound>(std::declval<ArgsB>())
        } -> std::same_as<Bound>;
    }
    __RXX_HIDE_FROM_ABI constexpr explicit repeat_view(
        std::piecewise_construct_t, ArgsT args, ArgsB bound_args = {})
        : value_(__RXX make_from_tuple<T>(std::move(args)))
        , bound_(__RXX make_from_tuple<Bound>(std::move(bound_args))) {}

    template <typename... Args, typename... BoundArgs>
    requires std::constructible_from<T, Args...> &&
                 std::constructible_from<Bound, BoundArgs...>
    __RXX_HIDE_FROM_ABI constexpr explicit repeat_view(
        std::piecewise_construct_t, tuple<Args...> args,
        tuple<BoundArgs...> bound_args = tuple<>{})
        : value_{make_from_tuple<T>(std::move(args))}
        , bound_{make_from_tuple<Bound>(std::move(bound_args))} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr iterator begin() const {
        return iterator(RXX_BUILTIN_addressof(*value_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator end() const
    requires (!std::same_as<Bound, std::unreachable_sentinel_t>)
    {
        return iterator(RXX_BUILTIN_addressof(*value_), bound_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::unreachable_sentinel_t end() const noexcept {
        return std::unreachable_sentinel;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires (!std::same_as<Bound, std::unreachable_sentinel_t>)
    {
        return details::to_unsigned_like(bound_);
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto take(repeat_view& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T&>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            return repeat_view<T, difference_t>(*self.value_,
                std::min<difference_t>(
                    ranges::distance(self), std::forward<N>(n)));
        } else {
            return repeat_view<T, difference_t>(
                *self.value_, static_cast<difference_t>(std::forward<N>(n)));
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto take(repeat_view const& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T const&>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            return repeat_view<T, difference_t>(*self.value_,
                std::min<difference_t>(
                    ranges::distance(self), std::forward<N>(n)));
        } else {
            return repeat_view<T, difference_t>(
                *self.value_, static_cast<difference_t>(std::forward<N>(n)));
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto take(repeat_view&& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            auto const dist = ranges::distance(self);
            return repeat_view<T, difference_t>(std::move(*self.value_),
                std::min<difference_t>(dist, std::forward<N>(n)));
        } else {
            return repeat_view<T, difference_t>(std::move(*self.value_),
                static_cast<difference_t>(std::forward<N>(n)));
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto take(repeat_view const&& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T const>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            auto const dist = ranges::distance(self);
            return repeat_view<T, difference_t>(std::move(*self.value_),
                std::min<difference_t>(dist, std::forward<N>(n)));
        } else {
            return repeat_view<T, difference_t>(std::move(*self.value_),
                static_cast<difference_t>(std::forward<N>(n)));
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto drop(repeat_view& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T&>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            auto const dist = ranges::distance(self);
            return repeat_view<T, difference_t>(*self.value_,
                dist - std::min<difference_t>(dist, std::forward<N>(n)));
        } else {
            return __RXX_AUTOCAST(self);
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto drop(repeat_view const& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T const&>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            auto const dist = ranges::distance(self);
            return repeat_view<T, difference_t>(*self.value_,
                dist - std::min<difference_t>(dist, std::forward<N>(n)));
        } else {
            return __RXX_AUTOCAST(self);
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto drop(repeat_view&& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            auto const dist = ranges::distance(self);
            return repeat_view<T, difference_t>(std::move(*self.value_),
                dist - std::min<difference_t>(dist, std::forward<N>(n)));
        } else {
            return __RXX_AUTOCAST(std::move(self));
        }
    }

    template <std::convertible_to<difference_t> N>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto drop(repeat_view const&& self, N&& n) noexcept(
        nothrow_difference&& noexcept(
            repeat_view<T, difference_t>(std::declval<T const>(),
                std::min<difference_t>(
                    std::declval<difference_t>(), std::forward<N>(n))))) {
        if constexpr (sized_range<repeat_view>) {
            auto const dist = ranges::distance(self);
            return repeat_view<T, difference_t>(std::move(*self.value_),
                dist - std::min<difference_t>(dist, std::forward<N>(n)));
        } else {
            return __RXX_AUTOCAST(std::move(self));
        }
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) details::movable_box<T> value_;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) Bound bound_{};
};

template <typename T, typename Bound = std::unreachable_sentinel_t>
repeat_view(T, Bound = Bound()) -> repeat_view<T, Bound>;

template <std::move_constructible T, std::semiregular Bound>
requires std::is_object_v<T> && std::same_as<T, std::remove_cv_t<T>> &&
    (details::integer_like_with_usable_difference_type<Bound> ||
        std::same_as<Bound, std::unreachable_sentinel_t>)
class repeat_view<T, Bound>::iterator {
    friend repeat_view<T, Bound>;

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(
        T const* value, index_type sentinel = index_type())
        : value_(value)
        , current_(sentinel) {}

public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = difference_t;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr const T& operator*() const noexcept {
        return *value_;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        ++current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--() {
        if constexpr (!std::same_as<Bound, std::unreachable_sentinel_t>) {
            assert(current_ > 0);
        }
        --current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int) {
        auto copy = *this;
        --*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset) {
        if constexpr (!std::same_as<Bound, std::unreachable_sentinel_t>) {
            assert(current_ + offset >= 0);
        }
        current_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset) {
        if constexpr (!std::same_as<Bound, std::unreachable_sentinel_t>) {
            assert(current_ - offset >= 0);
        }
        current_ -= offset;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& operator[](difference_type offset) const noexcept {
        return *(*this + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(iterator const& lhs, iterator const& rhs) {
        return lhs.current_ == rhs.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& lhs, iterator const& rhs) {
        return lhs.current_ <=> rhs.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(iterator self, difference_type offset) {
        self += offset;
        return self;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(difference_type offset, iterator self) {
        self += offset;
        return self;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(iterator self, difference_type offset) {
        self -= offset;
        return self;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& lhs, iterator const& rhs) {
        return static_cast<difference_type>(lhs.current_) -
            static_cast<difference_type>(rhs.current_);
    }

private:
    T const* value_ = nullptr;
    index_type current_{};
};

namespace details {
template <typename T, typename Bound>
inline constexpr bool is_repeat_view<repeat_view<T, Bound>> = true;
}

namespace views {
namespace details {
struct repeat_t {
    template <typename T>
    requires requires(T&& value) {
        ranges::repeat_view<std::decay_t<T>>(std::forward<T>(value));
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& value) RXX_CONST_CALL noexcept(
        noexcept(ranges::repeat_view<std::decay_t<T>>(std::forward<T>(value))))
        -> decltype(ranges::repeat_view<std::decay_t<T>>(
            std::forward<T>(value))) {
        return ranges::repeat_view<std::decay_t<T>>(std::forward<T>(value));
    }

    template <typename T, typename Bound>
    requires requires(T&& value, Bound&& sentinel) {
        ranges::repeat_view(
            std::forward<T>(value), std::forward<Bound>(sentinel));
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& value, Bound&& sentinel) RXX_CONST_CALL
        noexcept(noexcept(ranges::repeat_view(
            std::forward<T>(value), std::forward<Bound>(sentinel))))
            -> decltype(ranges::repeat_view(
                std::forward<T>(value), std::forward<Bound>(sentinel))) {
        return ranges::repeat_view(
            std::forward<T>(value), std::forward<Bound>(sentinel));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::repeat_t repeat{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
