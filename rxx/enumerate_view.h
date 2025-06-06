// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/simple_view.h"
#include "rxx/primitives.h"

#include <compare>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {
template <typename R>
concept range_with_movable_reference = std::ranges::input_range<R> &&
    std::move_constructible<range_reference_t<R>> &&
    std::move_constructible<range_rvalue_reference_t<R>>;
} // namespace details

template <std::ranges::view V>
requires details::range_with_movable_reference<V>
class enumerate_view : public std::ranges::view_interface<enumerate_view<V>> {
    template <bool IsConst>
    class iterator;
    template <bool IsConst>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr enumerate_view()
    requires std::default_initializable<V>
    = default;
    __RXX_HIDE_FROM_ABI explicit constexpr enumerate_view(V view)
        : view_{view} {}

    __RXX_HIDE_FROM_ABI constexpr auto begin()
    requires (!details::simple_view<V>)
    {
        return iterator<false>{std::ranges::begin(view_), 0};
    }

    __RXX_HIDE_FROM_ABI constexpr auto end()
    requires (!details::simple_view<V>)
    {
        if constexpr (std::ranges::common_range<V> &&
            std::ranges::sized_range<V>) {
            return iterator<false>(
                std::ranges::end(view_), std::ranges::distance(view_));
        } else {
            return sentinel<false>{std::ranges::end(view_)};
        }
    }

    __RXX_HIDE_FROM_ABI constexpr auto begin() const
    requires details::range_with_movable_reference<V const>
    {
        return iterator<true>{std::ranges::begin(view_), 0};
    }

    __RXX_HIDE_FROM_ABI constexpr auto end() const
    requires details::range_with_movable_reference<V const>
    {
        if constexpr (std::ranges::common_range<V const> &&
            std::ranges::sized_range<V const>) {
            return iterator<true>(
                std::ranges::end(view_), std::ranges::distance(view_));
        } else {
            return sentinel<true>{std::ranges::end(view_)};
        }
    }

    __RXX_HIDE_FROM_ABI constexpr auto size()
    requires std::ranges::sized_range<V>
    {
        return std::ranges::size(view_);
    }

    __RXX_HIDE_FROM_ABI constexpr auto size() const
    requires std::ranges::sized_range<V const>
    {
        return std::ranges::size(view_);
    }

    __RXX_HIDE_FROM_ABI constexpr V base() const&
    requires std::copy_constructible<V>
    {
        return view_;
    }

    __RXX_HIDE_FROM_ABI constexpr V base() && { return std::move(view_); }

private:
    V view_;
};

template <typename R>
enumerate_view(R&&) -> enumerate_view<std::views::all_t<R>>;

template <std::ranges::view V>
requires details::range_with_movable_reference<V>
template <bool Const>
class enumerate_view<V>::iterator {
    using Base = details::const_if<Const, V>;

    template <bool>
    friend class enumerate_view<V>::iterator;

    template <bool>
    friend class enumerate_view<V>::sentinel;

    friend class enumerate_view<V>;

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype([]() {
        if constexpr (std::ranges::random_access_range<V>)
            return std::random_access_iterator_tag{};
        else if constexpr (std::ranges::bidirectional_range<V>)
            return std::bidirectional_iterator_tag{};
        else if constexpr (std::ranges::forward_range<V>)
            return std::forward_iterator_tag{};
        else
            return std::input_iterator_tag{};
    }());
    using difference_type = range_difference_t<Base>;
    using value_type = std::tuple<difference_type, range_value_t<Base>>;
    using reference_type = std::tuple<difference_type, range_reference_t<Base>>;

    __RXX_HIDE_FROM_ABI constexpr iterator()
    requires std::default_initializable<iterator_t<Base>>
    = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_move_constructible_v<iterator_t<Base>>)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : current_{std::move(other.current_)}
        , pos_{other.pos_} {}

    __RXX_HIDE_FROM_ABI constexpr iterator_t<Base> const&
    base() const& noexcept {
        return current_;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator_t<Base>&& base() && noexcept {
        return std::move(current_);
    }

    __RXX_HIDE_FROM_ABI constexpr difference_type index() const noexcept {
        return pos_;
    }

    __RXX_HIDE_FROM_ABI constexpr auto operator*() const {
        return reference_type{pos_, *current_};
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        ++current_;
        ++pos_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++(int)
    requires std::ranges::forward_range<Base>
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires std::ranges::bidirectional_range<Base>
    {
        --current_;
        --pos_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--(int)
    requires std::ranges::bidirectional_range<Base>
    {
        auto copy = *this;
        --*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        current_ += offset;
        pos_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        current_ -= offset;
        pos_ -= offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr auto operator[](difference_type offset) const
    requires std::ranges::random_access_range<Base>
    {
        return reference_type{pos_ + offset, current_[offset]};
    }

    __RXX_HIDE_FROM_ABI friend constexpr bool operator==(
        iterator const& left, iterator const& right) noexcept {
        return left.pos_ == right.pos_;
    }

    __RXX_HIDE_FROM_ABI friend constexpr auto operator<=>(
        iterator const& left, iterator const& right) noexcept {
        return left.pos_ <=> right.pos_;
    }

    __RXX_HIDE_FROM_ABI friend constexpr iterator operator+(
        iterator const& self, difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        auto copy = self;
        copy += offset;
        return copy;
    }

    __RXX_HIDE_FROM_ABI friend constexpr iterator operator+(
        difference_type offset, iterator const& self)
    requires std::ranges::random_access_range<Base>
    {
        return self + offset;
    }

    __RXX_HIDE_FROM_ABI friend constexpr iterator operator-(
        iterator const& self, difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        auto copy = self;
        copy -= offset;
        return copy;
    }

    __RXX_HIDE_FROM_ABI friend constexpr difference_type operator-(
        iterator const& left, iterator const& right) {
        return left.pos_ - right.pos_;
    }
    __RXX_HIDE_FROM_ABI friend constexpr auto
    iter_move(iterator const& self) noexcept(
        noexcept(std::ranges::iter_move(self.current_)) &&
        std::is_nothrow_move_constructible_v<range_rvalue_reference_t<Base>>) {
        return std::tuple<difference_type, range_rvalue_reference_t<Base>>{
            self.pos_, std::ranges::iter_move(self.current_)};
    }

private:
    __RXX_HIDE_FROM_ABI constexpr iterator(
        iterator_t<Base> current, difference_type pos) noexcept(std::
            is_nothrow_move_constructible_v<iterator_t<Base>>)
        : current_{std::move(current)}
        , pos_{pos} {}

    iterator_t<Base> current_;
    difference_type pos_ = 0;
};

template <std::ranges::view V>
requires details::range_with_movable_reference<V>
template <bool Const>
class enumerate_view<V>::sentinel {
    using Base = details::const_if<Const, V>;

    friend class enumerate_view<V>;
    template <bool>
    friend class enumerate_view<V>::sentinel;

    template <bool OtherConst>
    __RXX_HIDE_FROM_ABI static constexpr decltype(auto) get_current(
        enumerate_view<V>::iterator<OtherConst> const& iter) {
        return (iter.current_); // parenthesized to return reference
    }

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other)
    requires Const && std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : end_{std::move(other.end_)} {}

    __RXX_HIDE_FROM_ABI constexpr sentinel_t<Base> base() const { return end_; }

    template <bool OtherConst>
    requires std::sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    __RXX_HIDE_FROM_ABI friend constexpr bool operator==(
        iterator<OtherConst> const& iter, sentinel const& self) {
        return get_current(iter) == self.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    __RXX_HIDE_FROM_ABI friend constexpr range_difference_t<
        details::const_if<OtherConst, V>>
    operator-(iterator<OtherConst> const& iter, sentinel const& self) {
        return get_current(iter) - self.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    __RXX_HIDE_FROM_ABI friend constexpr range_difference_t<
        details::const_if<OtherConst, V>>
    operator-(sentinel const& self, iterator<OtherConst> const& iter) {
        return -(iter - self);
    }

private:
    __RXX_HIDE_FROM_ABI constexpr explicit sentinel(sentinel_t<Base> end)
        : end_{std::move(end)} {}
    sentinel_t<Base> end_{};
};

namespace views {
namespace details {
struct enumerate_t : __RXX ranges::details::adaptor_closure<enumerate_t> {
    template <std::ranges::viewable_range R>
    requires requires {
        enumerate_view<std::views::all_t<R>>(std::declval<R>());
    }
    __RXX_HIDE_FROM_ABI constexpr auto operator()(R&& arg) const noexcept(
        noexcept(enumerate_view<std::views::all_t<R>>(std::declval<R>())))
        -> decltype(enumerate_view<std::views::all_t<R>>(std::declval<R>())) {
        return enumerate_view<std::views::all_t<R>>(std::forward<R>(arg));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::enumerate_t enumerate{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename View>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::enumerate_view<View>> =
        std::ranges::enable_borrowed_range<View>;
