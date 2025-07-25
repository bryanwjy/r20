// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "iter_traits.h"
#include "rxx/concepts/boolean_testable.h"
#include "rxx/iterator/iter_move.h"
#include "rxx/iterator/iter_swap.h"
#include "rxx/iterator/move_sentinel.h"

#include <concepts>
#include <iterator>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename It>
class move_iterator_category {};

template <typename It>
requires requires { typename std::iterator_traits<It>::iterator_category; }
class move_iterator_category<It> {
    using base_category RXX_NODEBUG =
        typename std::iterator_traits<It>::iterator_category;

public:
    using iterator_category RXX_NODEBUG = std::conditional_t<
        std::derived_from<base_category, std::random_access_iterator_tag>,
        std::random_access_iterator_tag, base_category>;
};

template <typename I, typename S>
concept move_iter_comparable = requires {
    {
        std::declval<I const&>() == std::declval<S>()
    } -> std::convertible_to<bool>;
};

} // namespace details

template <typename It>
class move_iterator : public details::move_iterator_category<It> {

    static consteval auto make_iterator_concept() noexcept {
        if constexpr (std::random_access_iterator<It>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (std::bidirectional_iterator<It>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::forward_iterator<It>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

    template <typename>
    friend class move_iterator;

public:
    using iterator_type = It;
    using iterator_concept = decltype(make_iterator_concept());
    using value_type = iter_value_t<It>;
    using difference_type = iter_difference_t<It>;
    using pointer = It;
    using reference = iter_rvalue_reference_t<It>;

    __RXX_HIDE_FROM_ABI constexpr move_iterator() noexcept(
        std::is_nothrow_default_constructible_v<It>) = default;
    __RXX_HIDE_FROM_ABI constexpr explicit move_iterator(
        iterator_type other) noexcept(std::is_nothrow_move_constructible_v<It>)
        : current_{std::move(other)} {}
    template <typename U>
    requires (!std::is_same_v<U, It>) && std::convertible_to<U const&, It>
    __RXX_HIDE_FROM_ABI constexpr move_iterator(move_iterator<U> const&
            other) noexcept(std::is_nothrow_constructible_v<It, U const&>)
        : current_{other.current_} {}

    template <class U>
    requires (!std::is_same_v<U, It>) && std::convertible_to<U const&, It> &&
        std::assignable_from<It&, U const&>
    __RXX_HIDE_FROM_ABI constexpr move_iterator&
    operator=(move_iterator<U> const& other) noexcept(
        std::is_nothrow_assignable_v<It&, U const&>) {
        current_ = other.current_;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr It const& base() const& noexcept { return current_; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr It base() && noexcept(std::is_nothrow_move_constructible_v<It>) {
        return std::move(current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr reference operator*() const
        noexcept(noexcept(ranges::iter_move(std::declval<It const&>()))) {
        return ranges::iter_move(current_);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr reference operator[](difference_type offset) const noexcept(
        noexcept(ranges::iter_move(std::declval<It const&>() + offset))) {
        return ranges::iter_move(current_ + offset);
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator& operator++() noexcept(
        noexcept(++std::declval<It&>())) {
        ++current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr auto operator++(int) noexcept(
        std::is_nothrow_copy_constructible_v<move_iterator>&& noexcept(
            ++std::declval<It&>()))
    requires std::forward_iterator<It>
    {
        move_iterator prev(*this);
        ++current_;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) noexcept(
        noexcept(++std::declval<It&>())) {
        ++current_;
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator& operator--() noexcept(
        noexcept(--std::declval<It&>())) {
        --current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator operator--(int) noexcept(
        std::is_nothrow_copy_constructible_v<move_iterator>&& noexcept(
            --std::declval<It&>())) {
        move_iterator prev(*this);
        --current_;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator operator+(
        difference_type offset) const
        noexcept(std::is_nothrow_constructible_v<move_iterator, It>&& //
            noexcept(std::declval<It const&>() + offset)) {
        return move_iterator(current_ + offset);
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator& operator+=(
        difference_type offset) noexcept(noexcept(std::declval<It&>() +=
        offset)) {
        current_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator operator-(
        difference_type offset) const
        noexcept(std::is_nothrow_constructible_v<move_iterator, It>&& //
            noexcept(std::declval<It const&>() - offset)) {
        return move_iterator(current_ - offset);
    }

    __RXX_HIDE_FROM_ABI constexpr move_iterator& operator-=(
        difference_type offset) noexcept(noexcept(std::declval<It&>() -=
        offset)) {
        current_ -= offset;
        return *this;
    }

    template <std::sentinel_for<It> S>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(move_iterator const& left,
        move_sentinel<S> const& right) noexcept(noexcept(std::
            is_nothrow_copy_constructible_v<S>&& noexcept(static_cast<bool>(
                std::declval<It const&>() == std::declval<S>()))))
    requires details::move_iter_comparable<It, S>
    {
        return static_cast<bool>(left.base() == right.base());
    }

    template <std::sized_sentinel_for<It> S>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iter_difference_t<It> operator-(
        move_sentinel<S> const& left,
        move_iterator const& right) noexcept(noexcept(std::
            is_nothrow_copy_constructible_v<S>&& noexcept(
                std::declval<S>() - std::declval<It const&>()))) {
        return left.base() - right.base();
    }

    template <std::sized_sentinel_for<It> S>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iter_difference_t<It> operator-(move_iterator const& left,
        move_sentinel<S> const& right) noexcept(noexcept(std::
            is_nothrow_copy_constructible_v<S>&& noexcept(
                std::declval<It const&>() - std::declval<S>()))) {
        return left.base() - right.base();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iter_rvalue_reference_t<It>
    iter_move(move_iterator const& it) noexcept(
        noexcept(ranges::iter_move(std::declval<It const&>()))) {
        return ranges::iter_move(it.current_);
    }

    template <std::indirectly_swappable<It> It2>
    __RXX_HIDE_FROM_ABI friend constexpr void iter_swap(
        move_iterator const& left,
        move_iterator<It2> const& right) noexcept(noexcept(ranges::
            iter_swap(std::declval<It const&>(), std::declval<It const&>()))) {
        return ranges::iter_swap(left.current_, right.current_);
    }

private:
    iterator_type current_{};
};

namespace details {
template <typename It1, typename It2>
concept move_iter_eq_comparable = requires(It1 const& left, It2 const& right) {
    { left == right } -> std::convertible_to<bool>;
};

template <typename It1, typename It2>
concept move_iter_lt_comparable = requires(It1 const& left, It2 const& right) {
    { left < right } -> std::convertible_to<bool>;
};
} // namespace details

template <typename It1, details::move_iter_eq_comparable<It1> It2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator==(move_iterator<It1> const& left,
    move_iterator<It2> const&
        right) noexcept(noexcept(std::declval<It1 const&>() ==
    std::declval<It2 const&>())) {
    return left.base() == right.base();
}

template <typename It1, details::move_iter_lt_comparable<It1> It2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<(move_iterator<It1> const& left,
    move_iterator<It2> const&
        right) noexcept(noexcept(std::declval<It1 const&>() <
    std::declval<It2 const&>())) {
    return left.base() < right.base();
}

template <typename It2, details::move_iter_lt_comparable<It2> It1>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>(move_iterator<It1> const& left,
    move_iterator<It2> const&
        right) noexcept(noexcept(std::declval<It2 const&>() <
    std::declval<It1 const&>())) {
    return right.base() < left.base();
}

template <typename It2, details::move_iter_lt_comparable<It2> It1>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator<=(move_iterator<It1> const& left,
    move_iterator<It2> const&
        right) noexcept(noexcept(!(std::declval<It2 const&>() <
    std::declval<It1 const&>()))) {
    return !(right.base() < left.base());
}

template <typename It1, details::move_iter_lt_comparable<It1> It2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool operator>=(move_iterator<It1> const& left,
    move_iterator<It2> const&
        right) noexcept(noexcept(!(std::declval<It1 const&>() <
    std::declval<It2 const&>()))) {
    return !(left.base() < right.base());
}

template <typename It1, std::three_way_comparable_with<It1> It2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr std::compare_three_way_result_t<It1, It2> operator<=>(
    move_iterator<It1> const& left,
    move_iterator<It2> const&
        right) noexcept(noexcept(std::declval<It1 const&>() <=>
    std::declval<It2 const&>())) {
    return left.base() <=> right.base();
}

template <typename It1, typename It2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr typename move_iterator<It1>::difference_type operator-(
    move_iterator<It1> const& left,
    move_iterator<It2> const& right) noexcept(requires(It1 const& left,
    It2 const& right) {
    { left - right } noexcept;
}) {
    return left.base() - right.base();
}

template <typename It>
requires requires(It const& it, iter_difference_t<It> offset) {
    { it + offset } -> std::same_as<It>;
}
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr move_iterator<It> operator+(
    iter_difference_t<It> offset, move_iterator<It> const& iter) {
    return iter + offset;
}

template <typename It>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr move_iterator<It> make_move_iterator(It iter) noexcept(
    std::is_nothrow_move_constructible_v<It>) {
    return move_iterator<It>(std::move(iter));
}

RXX_DEFAULT_NAMESPACE_END

template <typename It1, typename It2>
requires (!std::sized_sentinel_for<It1, It2>)
inline constexpr bool std::disable_sized_sentinel_for<__RXX move_iterator<It1>,
    __RXX move_iterator<It2>> = true;
