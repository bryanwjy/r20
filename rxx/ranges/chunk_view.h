// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/ceil_div.h"
#include "rxx/details/const_if.h"
#include "rxx/details/non_propagating_cache.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/subrange.h"
#include "rxx/ranges/take_view.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/utility.h"

#include <cassert>
#include <compare>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <view V>
requires input_range<V>
class chunk_view : public view_interface<chunk_view<V>> {
    class outer_iterator;
    class inner_iterator;

public:
    __RXX_HIDE_FROM_ABI explicit constexpr chunk_view(V base,
        range_difference_t<V>
            size) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_move_constructible_v<range_difference_t<V>>)
        : base_{std::move(base)}
        , size_{std::move(size)} {
        assert(size > 0);
    }

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
    constexpr outer_iterator begin() {
        current_ = __RXX ranges::begin(base_);
        remainder_ = size_;
        return outer_iterator(*this);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return details::to_unsigned_like(
            details::ceil_div(ranges::distance(base_), size_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return details::to_unsigned_like(
            details::ceil_div(ranges::distance(base_), size_));
    }

private:
    V base_;
    range_difference_t<V> size_;
    range_difference_t<V> remainder_ = 0;
    details::non_propagating_cache<iterator_t<V>> current_;
};

template <typename R>
chunk_view(R&&, range_difference_t<R>) -> chunk_view<views::all_t<R>>;

template <view V>
requires input_range<V>
class chunk_view<V>::outer_iterator {
    friend chunk_view;

    __RXX_HIDE_FROM_ABI constexpr explicit outer_iterator(
        chunk_view<V>& parent) noexcept
        : parent_{RXX_BUILTIN_addressof(parent)} {}

public:
    using iterator_concept = std::input_iterator_tag;
    using difference_type = range_difference_t<V>;

    struct value_type : public view_interface<value_type> {
        friend outer_iterator;

        __RXX_HIDE_FROM_ABI constexpr explicit value_type(
            chunk_view<V>& parent) noexcept
            : parent_{RXX_BUILTIN_addressof(parent)} {}

    public:
        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        constexpr inner_iterator begin() const noexcept {
            return inner_iterator(*parent_);
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        constexpr std::default_sentinel_t end() const noexcept {
            return std::default_sentinel;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        constexpr auto size() const
        requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
        {
            return details::to_unsigned_like(ranges::min(parent_->remainder_,
                ranges::end(parent_->base_) - *parent_->current_));
        }

    private:
        chunk_view* parent_;
    };

    __RXX_HIDE_FROM_ABI outer_iterator(outer_iterator&&) = default;
    __RXX_HIDE_FROM_ABI outer_iterator& operator=(outer_iterator&&) = default;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr value_type operator*() const {
        assert(*this != std::default_sentinel);
        return value_type(*parent_);
    }

    __RXX_HIDE_FROM_ABI constexpr outer_iterator& operator++() {
        assert(*this != std::default_sentinel);
        ranges::advance(*parent_->current_, parent_->remainder_,
            ranges::end(parent_->base_));
        parent_->remainder_ = parent_->size_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        outer_iterator const& left, std::default_sentinel_t) {
        return *left.get_current() == ranges::end(left.get_base()) &&
            left.get_remainder() != 0;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        std::default_sentinel_t, outer_iterator const& right)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        auto const dist = ranges::end(right.get_base()) - *right.get_current();

        if (dist < right.get_remainder()) {
            return dist == 0 ? 0 : 1;
        }

        return 1 +
            details::ceil_div(dist - right.get_remainder(), right.get_size());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        outer_iterator const& left, std::default_sentinel_t right)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return -(right - left);
    }

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_base() const noexcept {
        return (parent_->base_);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_remainder() const noexcept {
        return (parent_->remainder_);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_current() const noexcept {
        return (parent_->current_);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_size() const noexcept {
        return (parent_->current_);
    }
    chunk_view* parent_;
};

template <view V>
requires input_range<V>
class chunk_view<V>::inner_iterator {

    __RXX_HIDE_FROM_ABI explicit constexpr inner_iterator(
        chunk_view& parent) noexcept
        : parent_(RXX_BUILTIN_addressof(parent)) {}

    friend outer_iterator::value_type;

public:
    using iterator_concept = std::input_iterator_tag;
    using difference_type = range_difference_t<V>;
    using value_type = range_value_t<V>;

    __RXX_HIDE_FROM_ABI inner_iterator(inner_iterator&&) = default;
    __RXX_HIDE_FROM_ABI inner_iterator& operator=(inner_iterator&&) = default;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> const& base() const& { return *parent_->current_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr range_reference_t<V> operator*() const {
        assert(*this != std::default_sentinel);
        return **parent_->current_;
    }

    __RXX_HIDE_FROM_ABI constexpr inner_iterator& operator++() {
        assert(*this != std::default_sentinel);
        ++*parent_->current_;
        if (*parent_->current_ == ranges::end(parent_->base_)) {
            parent_->remainder_ = 0;
        } else {
            --parent_->remainder_;
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        inner_iterator const& self, std::default_sentinel_t) noexcept {
        return self.get_remainder() == 0;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        std::default_sentinel_t, inner_iterator const& right)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return ranges::min(right.get_remainder(),
            ranges::end(right.get_base()) - *right.get_current());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        inner_iterator const& left, std::default_sentinel_t right)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return -(right - left);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_rvalue_reference_t<V>
    iter_move(inner_iterator const& self) noexcept(
        noexcept(ranges::iter_move(std::declval<iterator_t<V> const&>()))) {
        return ranges::iter_move(*self.get_current());
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(inner_iterator const& left, inner_iterator const& right) noexcept(
        noexcept(ranges::iter_swap(std::declval<iterator_t<V> const&>(),
            std::declval<iterator_t<V> const&>())))
    requires std::indirectly_swappable<iterator_t<V>>
    {
        return ranges::iter_swap(*left.get_current(), *right.get_current());
    }

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_base() const noexcept {
        return (parent_->base_);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_remainder() const noexcept {
        return (parent_->remainder_);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) get_current() const noexcept {
        return (parent_->current_);
    }

    chunk_view* parent_;
};

template <view V>
requires forward_range<V>
class chunk_view<V> : public view_interface<chunk_view<V>> {

    // Clang<16  has a bug, which prevents out-of-line definition of this
    // iterator
    template <bool Const>
    class iterator {
        using Parent = details::const_if<Const, chunk_view>;
        using Base = details::const_if<Const, V>;
        friend class chunk_view;

        __RXX_HIDE_FROM_ABI constexpr iterator(Parent* parent,
            iterator_t<Base> current, range_difference_t<Base> missing = 0)
            : current_(current)
            , end_(ranges::end(parent->base_))
            , size_(parent->size_)
            , missing_(missing) {}

        static consteval auto make_iterator_concept() noexcept {
            if constexpr (random_access_range<Base>) {
                return std::random_access_iterator_tag{};
            } else if constexpr (bidirectional_range<Base>) {
                return std::bidirectional_iterator_tag{};
            } else {
                return std::forward_iterator_tag{};
            }
        }

    public:
        using iterator_category = std::input_iterator_tag;
        using iterator_concept = decltype(make_iterator_concept());
        using value_type = decltype(__RXX views::take(
            subrange(std::declval<iterator_t<Base>>(),
                std::declval<sentinel_t<Base>>()),
            range_difference_t<Base>{}));
        using difference_type = range_difference_t<Base>;

        __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
            std::is_nothrow_default_constructible_v<iterator_t<Base>> &&
            std::is_nothrow_default_constructible_v<sentinel_t<Base>>) =
            default;

        __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
        requires Const
                     && std::convertible_to<iterator_t<V>, iterator_t<Base>> &&
                     std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
            : current_(std::move(other.current_))
            , end_(std::move(other.end_))
            , size_(other.size_)
            , missing_(other.missing_) {}

        __RXX_HIDE_FROM_ABI constexpr iterator_t<Base> base() const {
            return current_;
        }

        __RXX_HIDE_FROM_ABI constexpr value_type operator*() const {
            assert(current_ != end_);
            return views::take(subrange(current_, end_), size_);
        }

        __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
            assert(current_ != end_);
            missing_ = ranges::advance(current_, size_, end_);
            return *this;
        }

        __RXX_HIDE_FROM_ABI constexpr iterator operator++(int) {
            auto prev = *this;
            ++*this;
            return prev;
        }

        __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
        requires bidirectional_range<Base>
        {
            ranges::advance(current_, missing_ - size_);
            missing_ = 0;
            return *this;
        }

        __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
        requires bidirectional_range<Base>
        {
            auto prev = *this;
            --*this;
            return prev;
        }

        __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(
            difference_type offset)
        requires random_access_range<Base>
        {
            if (offset > 0) {
                assert(ranges::distance(current_, end_) > size_ * (offset - 1));
                missing_ = ranges::advance(current_, size_ * offset, end_);
            } else if (offset < 0) {
                ranges::advance(current_, size_ * offset + missing_);
                missing_ = 0;
            }
            return *this;
        }

        __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(
            difference_type offset)
        requires random_access_range<Base>
        {
            return *this += -offset;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        constexpr value_type operator[](difference_type offset) const
        requires random_access_range<Base>
        {
            return *(*this + offset);
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr bool operator==(
            iterator const& left, iterator const& right) {
            return left.current_ == right.current_;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr bool operator==(
            iterator const& left, std::default_sentinel_t) {
            return left.current_ == left.end_;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr bool operator<(
            iterator const& left, iterator const& right)
        requires random_access_range<Base>
        {
            return left.current_ > right.current_;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr bool operator>(
            iterator const& left, iterator const& right)
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
            return left.current_ <=> right.current_;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr iterator operator+(
            iterator const& iter, difference_type offset)
        requires random_access_range<Base>
        {
            auto copy = iter;
            copy += offset;
            return copy;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr iterator operator+(
            difference_type offset, iterator const& iter)
        requires random_access_range<Base>
        {
            auto copy = iter;
            copy += offset;
            return copy;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr iterator operator-(
            iterator const& iter, difference_type offset)
        requires random_access_range<Base>
        {
            auto copy = iter;
            copy -= offset;
            return copy;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr difference_type operator-(
            iterator const& left, iterator const& right)
        requires std::sized_sentinel_for<iterator_t<Base>, iterator_t<Base>>
        {
            return (left.current_ - right.current_ + left.missing_ -
                       right.missing_) /
                left.size_;
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr difference_type operator-(
            std::default_sentinel_t, iterator const& iter)
        requires std::sized_sentinel_for<sentinel_t<Base>, iterator_t<Base>>
        {
            return details::ceil_div(iter.end_ - iter.current_, iter.size_);
        }

        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        friend constexpr difference_type operator-(
            iterator const& iter, std::default_sentinel_t end)
        requires std::sized_sentinel_for<sentinel_t<Base>, iterator_t<Base>>
        {
            return -(end - iter);
        }

    private:
        iterator_t<Base> current_{};
        sentinel_t<Base> end_{};
        range_difference_t<Base> size_ = 0;
        range_difference_t<Base> missing_ = 0;
    };

public:
    __RXX_HIDE_FROM_ABI explicit constexpr chunk_view(V base,
        range_difference_t<V>
            size) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_move_constructible_v<range_difference_t<V>>)
        : base_{std::move(base)}
        , size_{std::move(size)} {
        assert(size > 0);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const&
    requires std::copy_constructible<V>
    {
        return base_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr V base() && {
        return std::move(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (!details::simple_view<V>)
    {
        return iterator<false>(this, __RXX ranges::begin(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires forward_range<V const>
    {
        return iterator<true>(this, __RXX ranges::begin(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        if constexpr (common_range<V> && sized_range<V>) {
            auto const missing =
                (size_ - ranges::distance(base_) % size_) % size_;
            return iterator<false>(this, ranges::end(base_), missing);
        } else if constexpr (common_range<V> && !bidirectional_range<V>) {
            return iterator<false>(this, ranges::end(base_));
        } else {
            return std::default_sentinel;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires forward_range<V const>
    {
        if constexpr (common_range<V const> && sized_range<V const>) {
            auto const missing =
                (size_ - ranges::distance(base_) % size_) % size_;
            return iterator<true>(this, ranges::end(base_), missing);
        } else if constexpr (common_range<V const> &&
            !bidirectional_range<V const>) {
            return iterator<true>(this, ranges::end(base_));
        } else {
            return std::default_sentinel;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return details::to_unsigned_like(
            details::ceil_div(ranges::distance(base_), size_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return details::to_unsigned_like(
            details::ceil_div(ranges::distance(base_), size_));
    }

private:
    V base_;
    range_difference_t<V> size_;
};

namespace views {
namespace details {
struct chunk_t : ranges::details::adaptor_non_closure<chunk_t> {

    template <viewable_range R, typename D = range_difference_t<R>>
    requires requires { chunk_view(std::declval<R>(), std::declval<D>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        R&& arg, std::type_identity_t<D> size) const {
        return chunk_view(std::forward<R>(arg), size);
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<chunk_t>::operator();
    template <typename T>
    static constexpr bool _S_has_simple_extra_args = true;
    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename D>
    requires std::constructible_from<std::decay_t<D>, D>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(D&& size) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<D>, D>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(chunk_t{}),
            std::forward<D>(size));
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::chunk_t chunk{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename V>
constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::chunk_view<V>> =
        forward_range<V> && std::ranges::enable_borrowed_range<V>;
