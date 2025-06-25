// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/non_propogating_cache.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/primitives.h"
#include "rxx/take_view.h"

#include <cassert>
#include <compare>
#include <iterator>
#include <ranges>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <std::ranges::input_range V>
requires std::ranges::view<V> && std::ranges::input_range<range_reference_t<V>>
class join_view : public std::ranges::view_interface<join_view<V>> {

    template <bool Const>
    class iterator;
    template <bool Const>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr join_view() noexcept
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI constexpr explicit join_view(V base) noexcept(
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

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto begin() {
        if constexpr (std::ranges::forward_range<V>) {
            using result_type = iterator<details::simple_view<V> &&
                std::is_reference_v<range_reference_t<V>>>;
            return result_type{*this, std::ranges::begin(base_)};
        } else {
            outer_.emplace(std::ranges::begin(base_));
            return iterator<false>{*this};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires std::ranges::forward_range<V const> &&
        std::is_reference_v<range_reference_t<V const>> &&
        std::ranges::input_range<range_reference_t<V const>>
    {
        return iterator<true>{*this, std::ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto end() {
        if constexpr (std::ranges::forward_range<V> &&
            std::is_reference_v<InnerRange> &&
            std::ranges::forward_range<InnerRange> &&
            std::ranges::common_range<V> &&
            std::ranges::common_range<InnerRange>)
            return iterator<details::simple_view<V>>{
                *this, std::ranges::end(base_)};
        else
            return sentinel<details::simple_view<V>>{*this};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires std::ranges::forward_range<V const> &&
        std::is_reference_v<range_reference_t<V const>> &&
        std::ranges::input_range<range_reference_t<V const>>
    {
        using ConstInnerRange = range_reference_t<V const>;
        if constexpr (std::ranges::forward_range<ConstInnerRange> &&
            std::ranges::common_range<V const> &&
            std::ranges::common_range<ConstInnerRange>) {
            return iterator<true>{*this, std::ranges::end(base_)};
        } else {
            return sentinel<true>{*this};
        }
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_;
    static constexpr bool use_outer_cache = !std::ranges::forward_range<V>;
    using OuterCache RXX_NODEBUG = std::conditional_t<use_outer_cache,
        details::non_propogating_cache<iterator_t<V>>, details::empty_cache>;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) OuterCache outer_;

    using InnerRange _LIBCPP_NODEBUG = range_reference_t<V>;
    static constexpr bool use_inner_cache = !std::is_reference_v<InnerRange>;
    using InnerCache RXX_NODEBUG = std::conditional_t<use_inner_cache,
        details::non_propogating_cache<std::remove_cvref_t<InnerRange>>,
        details::empty_cache>;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) InnerCache inner_;
};

template <typename R>
explicit join_view(R&&) -> join_view<std::views::all_t<R>>;

namespace details {

template <typename>
struct join_view_iterator_category {};

template <typename V>
requires std::is_reference_v<range_reference_t<V>> &&
    std::ranges::forward_range<V> &&
    std::ranges::forward_range<range_reference_t<V>>
struct join_view_iterator_category<V> {
    using outer_type RXX_NODEBUG =
        typename std::iterator_traits<iterator_t<V>>::iterator_category;
    using inner_type RXX_NODEBUG = typename std::iterator_traits<
        iterator_t<range_reference_t<V>>>::iterator_category;

    using iterator_category = std::conditional_t<
        std::derived_from<outer_type, std::bidirectional_iterator_tag> &&
            std::derived_from<inner_type, std::bidirectional_iterator_tag> &&
            std::ranges::common_range<range_reference_t<V>>,
        std::bidirectional_iterator_tag,
        std::conditional_t<
            std::derived_from<outer_type, std::forward_iterator_tag> &&
                std::derived_from<inner_type, std::forward_iterator_tag>,
            std::forward_iterator_tag, std::input_iterator_tag>>;
};

template <typename T>
concept has_arrow = std::is_pointer_v<T> ||
    (std::is_class_v<T> && requires(T val) { val.operator->(); });

template <typename T>
__RXX_HIDE_FROM_ABI constexpr T& as_lvalue(T&& val RXX_LIFETIMEBOUND) {
    return static_cast<T&>(val);
}
} // namespace details

template <std::ranges::input_range V>
requires std::ranges::view<V> && std::ranges::input_range<range_reference_t<V>>
template <bool Const>
class join_view<V>::iterator final :
    public details::join_view_iterator_category<V> {
    struct empty_outer {};
    using Parent RXX_NODEBUG = details::const_if<Const, join_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    using OuterIter RXX_NODEBUG = iterator_t<Base>;
    using InnerIter RXX_NODEBUG = iterator_t<range_reference_t<Base>>;
    using InnerRange RXX_NODEBUG = range_reference_t<V>;

    friend join_view;

    __RXX_HIDE_FROM_ABI constexpr OuterIter& get_outer() {
        if constexpr (std::ranges::forward_range<Base>) {
            return outer_;
        } else {
            return *parent_->outer_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr OuterIter const& get_outer() const {
        if constexpr (std::ranges::forward_range<Base>) {
            return outer_;
        } else {
            return *parent_->outer_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr void satisfy() {

        for (; get_outer() != std::ranges::end(parent_->base_); ++get_outer()) {
            auto&& inner = [this]() -> auto&& {
                if constexpr (std::is_reference_v<range_reference_t<Base>>) {
                    return *get_outer();
                } else {
                    return parent_->inner_.emplace_deref(get_outer());
                }
            }();

            inner_ = std::ranges::begin(inner);
            if (*inner_ != std::ranges::end(inner)) {
                return;
            }
        }

        if constexpr (std::is_reference_v<range_reference_t<Base>>) {
            inner_.reset();
        }
    }

    __RXX_HIDE_FROM_ABI constexpr iterator(Parent& parent,
        OuterIter
            outer) noexcept(std::is_nothrow_move_constructible_v<OuterIter>)
    requires std::ranges::forward_range<Base>
        : outer_{std::move(outer)}
        , parent_{RXX_BUILTIN_addressof(parent)} {
        satisfy();
    }

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(Parent& parent) noexcept(
        std::is_nothrow_default_constructible_v<OuterIter>)
    requires (!std::ranges::forward_range<Base>)
        : parent_{RXX_BUILTIN_addressof(parent)} {
        satisfy();
    }

    __RXX_HIDE_FROM_ABI constexpr iterator(Parent* parent, OuterIter outer,
        InnerIter
            inner) noexcept(std::is_nothrow_move_constructible_v<OuterIter> &&
        std::is_nothrow_move_constructible_v<InnerIter>)
    requires std::ranges::forward_range<Base>
        : outer_(std::move(outer))
        , inner_(std::move(inner))
        , parent_(parent) {}

public:
    using iterator_concept = decltype([]() {
        if constexpr (std::is_reference_v<range_reference_t<Base>> &&
            std::ranges::bidirectional_range<Base> &&
            std::ranges::bidirectional_range<range_reference_t<Base>>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::is_reference_v<range_reference_t<Base>> &&
            std::ranges::forward_range<Base> &&
            std::ranges::forward_range<range_reference_t<Base>>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());
    using value_type = range_value_t<range_reference_t<Base>>;
    using difference_type = std::common_type_t<range_difference_t<Base>,
        range_difference_t<range_reference_t<Base>>>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<OuterIter>)
    requires std::default_initializable<InnerIter> &&
        std::default_initializable<OuterIter>
    = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const && std::convertible_to<iterator_t<V>, OuterIter> &&
                 std::convertible_to<iterator_t<InnerRange>, InnerIter>
        : outer_{std::move(other.outer_)}
        , inner_{std::move(other.inner_)}
        , parent_{other.parent_} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator*() const { return **inner_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr InnerIter operator->() const
    requires details::has_arrow<InnerIter> && std::copyable<InnerIter>
    {
        return *inner_;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        auto const get_inner_range = [&]() -> decltype(auto) {
            if constexpr (std::is_reference_v<range_reference_t<Base>>)
                return *get_outer();
            else
                return *parent_->inner_;
        };

        if (++*inner_ ==
            std::ranges::end(details::as_lvalue(get_inner_range()))) {
            ++get_outer();
            satisfy();
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires std::is_reference_v<range_reference_t<Base>> &&
        std::ranges::forward_range<Base> &&
        std::ranges::forward_range<range_reference_t<Base>>
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires std::is_reference_v<range_reference_t<Base>> &&
        std::ranges::bidirectional_range<Base> &&
        std::ranges::bidirectional_range<range_reference_t<Base>> &&
        std::ranges::common_range<range_reference_t<Base>>
    {
        if (outer_ == std::ranges::end(parent_->base_)) {
            inner_ = std::ranges::end(*--outer_);
        }

        while (*inner_ == std::ranges::begin(*outer_)) {
            inner_ = std::ranges::end(*--outer_);
        }
        --*inner_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires std::is_reference_v<range_reference_t<Base>> &&
        std::ranges::bidirectional_range<Base> &&
        std::ranges::bidirectional_range<range_reference_t<Base>> &&
        std::ranges::common_range<range_reference_t<Base>>
    {
        auto previous = *this;
        --*this;
        return previous;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires std::is_reference_v<range_reference_t<Base>> &&
        std::ranges::forward_range<Base> &&
        std::equality_comparable<iterator_t<range_reference_t<Base>>>
    {
        return left.outer_ == right.outer_ && left.inner_ == right.inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr decltype(auto) iter_move(iterator const& iter) noexcept(
        noexcept(std::ranges::iter_move(*iter.inner_))) {
        return std::ranges::iter_move(*iter.inner_);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(iterator const& left, iterator const& right) noexcept(
        noexcept(std::ranges::iter_swap(*left.inner_, *right.inner_)))
    requires std::indirectly_swappable<InnerIter>
    {
        return std::ranges::iter_swap(*left.inner_, *right.inner_);
    }

private:
    using OuterType = std::conditional_t<std::ranges::forward_range<Base>,
        OuterIter, empty_outer>;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) OuterType outer_{};
    details::optional_base<InnerIter> inner_{};
    Parent* parent_ = nullptr;
};

template <std::ranges::input_range V>
requires std::ranges::view<V> && std::ranges::input_range<range_reference_t<V>>
template <bool Const>
class join_view<V>::sentinel {
private:
    template <bool>
    friend class sentinel;

    using Parent RXX_NODEBUG = details::const_if<Const, join_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;

public:
    __RXX_HIDE_FROM_ABI sentinel() = default;

    __RXX_HIDE_FROM_ABI explicit constexpr sentinel(Parent& parent)
        : end_(std::ranges::end(parent.base_)) {}

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other)
    requires Const && std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : end_(std::move(other.end_)) {}

    template <bool OtherConst>
    requires std::sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool operator==(
        iterator<OtherConst> const& left, sentinel const& right) {
        return left.get_outer() == right.end_;
    }

private:
    sentinel_t<Base> end_{};
};

namespace views {
namespace details {
struct join_t : __RXX ranges::details::adaptor_closure<join_t> {
    template <typename R>
    requires requires { join_view<std::views::all_t<R&&>>(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        R&& arg) const
        noexcept(noexcept(join_view<std::views::all_t<R&&>>(std::declval<R>())))
            -> decltype(join_view<std::views::all_t<R&&>>(std::declval<R>())) {
        return join_view<std::views::all_t<R&&>>(std::forward<R>(arg));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::join_t join{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
