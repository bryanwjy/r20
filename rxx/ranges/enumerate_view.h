// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/simple_view.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/tuple.h"
#include "rxx/utility.h"

#include <compare> // IWYU pragma: keep

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {
template <typename R>
concept range_with_movable_reference =
    input_range<R> && std::move_constructible<range_reference_t<R>> &&
    std::move_constructible<range_rvalue_reference_t<R>>;
} // namespace details

template <view V>
requires details::range_with_movable_reference<V>
class enumerate_view : public view_interface<enumerate_view<V>> {
    template <bool IsConst>
    class iterator;
    template <bool IsConst>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr enumerate_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr enumerate_view(V view)
        : view_{view} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (!details::simple_view<V>)
    {
        return iterator<false>{ranges::begin(view_), 0};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        if constexpr (common_range<V> && sized_range<V>) {
            return iterator<false>(ranges::end(view_), ranges::distance(view_));
        } else {
            return sentinel<false>{ranges::end(view_)};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires details::range_with_movable_reference<V const>
    {
        return iterator<true>{ranges::begin(view_), 0};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires details::range_with_movable_reference<V const>
    {
        if constexpr (common_range<V const> && sized_range<V const>) {
            return iterator<true>(ranges::end(view_), ranges::distance(view_));
        } else {
            return sentinel<true>{ranges::end(view_)};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return ranges::size(view_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return ranges::size(view_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const&
    requires std::copy_constructible<V>
    {
        return view_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr V base() && {
        return std::move(view_);
    }

private:
    V view_{};
};

template <typename R>
enumerate_view(R&&) -> enumerate_view<views::all_t<R>>;

template <view V>
requires details::range_with_movable_reference<V>
template <bool Const>
class enumerate_view<V>::iterator {
    using Base = details::const_if<Const, V>;

    friend enumerate_view;

    static consteval auto make_iterator_concept() noexcept {
        if constexpr (random_access_range<Base>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (bidirectional_range<Base>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (forward_range<Base>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype(make_iterator_concept());
    using difference_type = range_difference_t<Base>;
    using value_type = tuple<difference_type, range_value_t<Base>>;
    using reference_type = tuple<difference_type, range_reference_t<Base>>;

    __RXX_HIDE_FROM_ABI constexpr iterator()
    requires std::default_initializable<iterator_t<Base>>
    = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_move_constructible_v<iterator_t<Base>>)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : current_{std::move(other.current_)}
        , pos_{other.pos_} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> const& base() const& noexcept {
        return current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base>&& base() && noexcept {
        return std::move(current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr difference_type index() const noexcept { return pos_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator*() const {
        return reference_type{pos_, *current_};
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        ++current_;
        ++pos_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires forward_range<Base>
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires bidirectional_range<Base>
    {
        --current_;
        --pos_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires bidirectional_range<Base>
    {
        auto copy = *this;
        --*this;
        return copy;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires random_access_range<Base>
    {
        current_ += offset;
        pos_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires random_access_range<Base>
    {
        current_ -= offset;
        pos_ -= offset;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type offset) const
    requires random_access_range<Base>
    {
        return reference_type{pos_ + offset, current_[offset]};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right) noexcept {
        return left.pos_ == right.pos_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right) noexcept {
        return left.pos_ <=> right.pos_;
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
        iterator const& left, iterator const& right) noexcept {
        static_assert(noexcept(left.pos_ - right.pos_));
        return left.pos_ - right.pos_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto iter_move(iterator const& self) noexcept(
        noexcept(ranges::iter_move(std::declval<iterator_t<Base> const&>())) &&
        std::is_nothrow_move_constructible_v<range_rvalue_reference_t<Base>>) {
        return tuple<difference_type, range_rvalue_reference_t<Base>>{
            self.pos_, ranges::iter_move(self.current_)};
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

template <view V>
requires details::range_with_movable_reference<V>
template <bool Const>
class enumerate_view<V>::sentinel {
    using Base = details::const_if<Const, V>;

    friend enumerate_view;

    __RXX_HIDE_FROM_ABI constexpr explicit sentinel(
        sentinel_t<Base> end) noexcept(std::
            is_nothrow_move_constructible_v<sentinel_t<Base>>)
        : end_{std::move(end)} {}

    template <bool OtherConst>
    __RXX_HIDE_FROM_ABI static constexpr decltype(auto) get_iter_current(
        iterator<OtherConst> const& iter) noexcept {
        return (iter.current_);
    }

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<sentinel_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other)
    requires Const && std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : end_{std::move(other.end_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr sentinel_t<Base> base() const { return end_; }

    template <bool OtherConst>
    requires std::sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool operator==(
        iterator<OtherConst> const& iter, sentinel const& self) {
        return get_iter_current(iter) == self.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, V>>
    operator-(iterator<OtherConst> const& iter, sentinel const& self) {
        return get_iter_current(iter) - self.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, V>>
    operator-(sentinel const& self, iterator<OtherConst> const& iter) {
        return -(iter - self);
    }

private:
    sentinel_t<Base> end_{};
};

namespace views {
namespace details {
struct enumerate_t : __RXX ranges::details::adaptor_closure<enumerate_t> {
    template <viewable_range R>
    requires requires { enumerate_view<all_t<R>>(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(enumerate_view<all_t<R>>(std::declval<R>())))
            -> decltype(enumerate_view<all_t<R>>(std::declval<R>())) {
        return enumerate_view<all_t<R>>(std::forward<R>(arg));
    }

#if RXX_LIBSTDCXX
    static constexpr bool _S_has_simple_call_op = true;
#endif
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
