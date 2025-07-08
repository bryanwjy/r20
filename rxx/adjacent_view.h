// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/tuple_functions.h"
#include "rxx/get_element.h"
#include "rxx/primitives.h"

#include <array>
#include <compare>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {

template <size_t, typename T>
using always_type RXX_NODEBUG = T;

template <typename T, size_t... Is>
__RXX_HIDE_FROM_ABI auto repeat(std::index_sequence<Is...>) noexcept
    -> std::tuple<always_type<Is, T>...>;

template <size_t N, typename T>
using repeat_type_t RXX_NODEBUG = decltype(repeat<T>(make_index_sequence_v<N>));

} // namespace details

template <forward_range V, size_t N>
requires std::ranges::view<V> && (N > 0)
class adjacent_view : public std::ranges::view_interface<adjacent_view<V, N>> {

    struct as_sentinel_t {};
    __RXX_HIDE_FROM_ABI static constexpr as_sentinel_t as_sentinel{};

    template <bool Const>
    class iterator;
    template <bool Const>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr adjacent_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr adjacent_view(V base) noexcept(
        std::is_nothrow_move_constructible_v<V>)
        : base_{std::move(base)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const& noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return base_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() && noexcept(std::is_nothrow_move_constructible_v<V>) {
        return std::move(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (!details::simple_view<V>)
    {
        return iterator<false>(
            __RXX ranges::begin(base_), __RXX ranges::end(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires std::ranges::range<V const>
    {
        return iterator<true>(
            __RXX ranges::begin(base_), __RXX ranges::end(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        if constexpr (std::ranges::common_range<V>) {
            return iterator<false>(as_sentinel, __RXX ranges::begin(base_),
                __RXX ranges::end(base_));
        } else {
            return sentinel<false>(__RXX ranges::end(base_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires std::ranges::range<V const>
    {
        if constexpr (std::ranges::common_range<V>) {
            return iterator<true>(as_sentinel, __RXX ranges::begin(base_),
                __RXX ranges::end(base_));
        } else {
            return sentinel<true>(__RXX ranges::end(base_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires std::ranges::sized_range<V>
    {
        using SizeType = decltype(std::ranges::size(base_));
        using CommonType = std::common_type_t<SizeType, size_t>;
        auto size = static_cast<CommonType>(std::ranges::size(base_));
        size -= std::min<CommonType>(size, N - 1);
        return static_cast<SizeType>(size);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires std::ranges::sized_range<V const>
    {
        using SizeType = decltype(std::ranges::size(base_));
        using CommonType = std::common_type_t<SizeType, size_t>;
        auto size = static_cast<CommonType>(std::ranges::size(base_));
        size -= std::min<CommonType>(size, N - 1);
        return static_cast<SizeType>(size);
    }

private:
    V base_;
};

template <forward_range V, size_t N>
requires std::ranges::view<V> && (N > 0)
template <bool Const>
class adjacent_view<V, N>::iterator {
    using Base = details::const_if<Const, V>;

    friend class adjacent_view;

    __RXX_HIDE_FROM_ABI static constexpr auto
    build(iterator_t<Base> begin, sentinel_t<Base> end) noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>> &&
        std::is_nothrow_copy_assignable_v<iterator_t<Base>>) {
        std::array<iterator_t<Base>, N> output{};
        for (auto& val : output) {
            val = begin;
            std::ranges::advance(begin, 1, end);
        }
        return output;
    }

    __RXX_HIDE_FROM_ABI static constexpr auto
    build(as_sentinel_t, iterator_t<Base> begin, iterator_t<Base> end) noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>> &&
        std::is_nothrow_copy_assignable_v<iterator_t<Base>>) {
        std::array<iterator_t<Base>, N> output{};
        if constexpr (!bidirectional_range<Base>) {
            for (auto& val : output) {
                val = end;
            }
        } else {
            for (auto idx = 0u; idx < N; ++idx) {
                output[N - 1 - idx] = end;
                std::ranges::advance(begin, -1, end);
            }
        }

        return output;
    }

    __RXX_HIDE_FROM_ABI explicit constexpr iterator(iterator_t<Base> begin,
        sentinel_t<Base> end) noexcept(noexcept(build(begin, end)))
        : current_{build(begin, end)} {}

    __RXX_HIDE_FROM_ABI explicit constexpr iterator(as_sentinel_t tag,
        iterator_t<Base> begin,
        iterator_t<Base> end) noexcept(noexcept(build(tag, begin, end)))
        : current_{build(tag, begin, end)} {}

    __RXX_HIDE_FROM_ABI friend constexpr decltype(auto) get_current(
        iterator const& iter) noexcept {
        return (iter.current_);
    }

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype([]() {
        if constexpr (random_access_range<Base>)
            return std::random_access_iterator_tag{};
        else if constexpr (bidirectional_range<Base>)
            return std::bidirectional_iterator_tag{};
        else
            return std::forward_iterator_tag{};
    }());
    using value_type = details::repeat_type_t<N, range_value_t<Base>>;
    using difference_type = range_difference_t<Base>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>>) = default;
    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : current_{std::move(other.current_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator*() const {
        return details::transform(
            [](auto const& it) -> decltype(auto) { return *it; }, current_);
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        details::for_each([](auto& it) { ++it; }, current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--() {
        details::for_each([](auto& it) { --it; }, current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int) {
        auto copy = *this;
        --*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires random_access_range<Base>
    {
        details::for_each([&](auto& it) { it += offset; }, current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires random_access_range<Base>
    {
        details::for_each([&](auto& it) { it -= offset; }, current_);
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type offset) const
    requires random_access_range<Base>
    {
        return details::transform(
            [&](auto const& it) -> decltype(auto) { return it[offset]; },
            current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right) {
        return left.current_.back() == right.current_.back();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<(iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return left.current_.back() < right.current_.back();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>(iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return right < left;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<=(
        iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return !(right < left);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>=(
        iterator const& left, iterator const& right)
    requires random_access_range<Base>
    {
        return !(left < right);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires random_access_range<Base> &&
        std::three_way_comparable<iterator_t<Base>>
    {
        return left.current_.back() <=> right.current_.back();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& self, difference_type offset)
    requires random_access_range<Base>
    {
        auto copy = self;
        copy += offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type offset, iterator const& self)
    requires random_access_range<Base>
    {
        return self + offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& self, difference_type offset)
    requires random_access_range<Base>
    {
        auto copy = self;
        copy -= offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires std::sized_sentinel_for<iterator_t<Base>, iterator_t<Base>>
    {
        return left.current_.back() - right.current_.back();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto iter_move(iterator const& self) {
        return details::transform(std::ranges::iter_move, self.current_);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void iter_swap(
        iterator const& left, iterator const& right)
    requires std::indirectly_swappable<iterator_t<Base>>
    {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (...,
                std::ranges::iter_swap(left.current_[Is], right.current_[Is]));
        }(std::make_index_sequence<N>{});
    }

private:
    std::array<iterator_t<Base>, N> current_{};
};

template <forward_range V, size_t N>
requires std::ranges::view<V> && (N > 0)
template <bool Const>
class adjacent_view<V, N>::sentinel {
    using Base = details::const_if<Const, V>;

    friend class adjacent_view;

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_constructible_v<sentinel_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other) noexcept(
        std::is_nothrow_constructible_v<sentinel_t<V>, sentinel_t<Base>>)
    requires Const && std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : end_{std::move(other.end_)} {}

    template <bool OtherConst>
    requires std::sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool operator==(
        iterator<OtherConst> const& iter, sentinel const& self) {
        return get_current(iter).back() == self.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, V>>
    operator-(iterator<OtherConst> const& iter, sentinel const& self) {
        return get_current(iter).back() - self.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, V>>
    operator-(sentinel const& self, iterator<OtherConst> const& iter) {
        return self.end_ - get_current(iter).back();
    }

private:
    sentinel_t<Base> end_{};
};

namespace views {
namespace details {
template <size_t N>
struct adjacent_t : __RXX ranges::details::adaptor_closure<adjacent_t<N>> {

    template <std::ranges::viewable_range R>
    requires (N == 0) ||
        requires { adjacent_view<std::views::all_t<R>, N>(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        R&& arg) const {
        if constexpr (N == 0) {
            return std::views::empty<std::tuple<>>;
        } else {
            return adjacent_view<std::views::all_t<R>, N>(std::forward<R>(arg));
        }
    }
};
} // namespace details

inline namespace cpo {
template <size_t N>
inline constexpr details::adjacent_t<N> adjacent{};

inline constexpr details::adjacent_t<2> pairwise{};

} // namespace cpo
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename R, size_t N>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::adjacent_view<R, N>> =
        std::ranges::enable_borrowed_range<R>;
