// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/all.h"
#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/as_lvalue.h"
#include "rxx/details/const_if.h"
#include "rxx/details/iterator_category_of.h"
#include "rxx/details/non_propagating_cache.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/primitives.h"
#include "rxx/take_view.h"
#include "rxx/view_interface.h"

#include <cassert>
#include <compare>
#include <iterator>
#include <ranges>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <input_range V>
requires view<V> && input_range<range_reference_t<V>>
class join_view : public view_interface<join_view<V>> {

    template <bool Const>
    class iterator;
    template <bool Const>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr join_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr join_view(V base) noexcept(
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
        if constexpr (forward_range<V>) {
            using result_type = iterator<details::simple_view<V> &&
                std::is_reference_v<range_reference_t<V>>>;
            return result_type{*this, __RXX ranges::begin(base_)};
        } else {
            outer_.emplace(__RXX ranges::begin(base_));
            return iterator<false>{*this};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires forward_range<V const> &&
        std::is_reference_v<range_reference_t<V const>> &&
        input_range<range_reference_t<V const>>
    {
        return iterator<true>{*this, __RXX ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto end() {
        if constexpr (forward_range<V> && std::is_reference_v<InnerRange> &&
            forward_range<InnerRange> && std::ranges::common_range<V> &&
            std::ranges::common_range<InnerRange>) {
            return iterator<details::simple_view<V>>{
                *this, __RXX ranges::end(base_)};
        } else {
            return sentinel<details::simple_view<V>>{*this};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires forward_range<V const> &&
        std::is_reference_v<range_reference_t<V const>> &&
        input_range<range_reference_t<V const>>
    {
        using ConstInnerRange = range_reference_t<V const>;
        if constexpr (forward_range<ConstInnerRange> &&
            std::ranges::common_range<V const> &&
            std::ranges::common_range<ConstInnerRange>) {
            return iterator<true>{*this, __RXX ranges::end(base_)};
        } else {
            return sentinel<true>{*this};
        }
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_{};
    static constexpr bool use_outer_cache = !forward_range<V>;
    using OuterCache RXX_NODEBUG = std::conditional_t<use_outer_cache,
        details::non_propagating_cache<iterator_t<V>>, details::empty_cache>;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) OuterCache outer_;

    using InnerRange RXX_NODEBUG = range_reference_t<V>;
    // InnerCache -> empty class if InnerRange is a reference (not an object)
    using InnerCache RXX_NODEBUG =
        details::non_propagating_cache<std::remove_cv_t<InnerRange>>;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) InnerCache inner_;

    /**
     * Note to self/whomever that sees this:
     *
     * When using the following:
     *
     * using InnerCache = std::conditional_t<!std::is_reference_v<InnerRange>,
     * details::non_propagating_cache<std::remove_cv_t<InnerRange>>,
     * details::empty_cache>;
     *
     * It somehow breaks the code generated and causes GCC's join_view test05 to
     * fail as the range in that specific test becomes unbounded. The same issue
     * is seemingly also happening in libc++20. What is also strange is that
     * when another empty struct with a different name, e.g. nothing_t, is
     * defined and `details::empty_cache` is replaced with it, the issue
     * disappears.
     */
    static_assert(
        !std::is_reference_v<InnerRange> || std::is_empty_v<InnerCache>);
};

template <typename R>
explicit join_view(R&&) -> join_view<views::all_t<R>>;

namespace details {

template <bool, typename>
struct join_view_iterator_category {};

template <bool Const, typename V>
requires std::is_reference_v<range_reference_t<details::const_if<Const, V>>> &&
    forward_range<details::const_if<Const, V>> &&
    forward_range<range_reference_t<details::const_if<Const, V>>>
struct join_view_iterator_category<Const, V> {
    using outer_type RXX_NODEBUG = details::iterator_category_of<Const, V>;
    using inner_type RXX_NODEBUG =
        details::iterator_category_of<Const, range_reference_t<V>>;

    using iterator_category = decltype([]() {
        if constexpr (std::derived_from<outer_type,
                          std::bidirectional_iterator_tag> &&
            std::derived_from<inner_type, std::bidirectional_iterator_tag> &&
            std::ranges::common_range<
                range_reference_t<details::const_if<Const, V>>>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::derived_from<outer_type,
                                 std::forward_iterator_tag> &&
            std::derived_from<inner_type, std::forward_iterator_tag>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());
};

template <typename T>
concept has_arrow = std::is_pointer_v<T> ||
    (std::is_class_v<T> && requires(T val) { val.operator->(); });
} // namespace details

template <input_range V>
requires view<V> && input_range<range_reference_t<V>>
template <bool Const>
class join_view<V>::iterator final :
    public details::join_view_iterator_category<Const, V> {
    struct empty_outer {};
    using Parent RXX_NODEBUG = details::const_if<Const, join_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    using OuterIter RXX_NODEBUG = iterator_t<Base>;
    using InnerIter RXX_NODEBUG = iterator_t<range_reference_t<Base>>;
    using InnerRange RXX_NODEBUG = range_reference_t<V>;
    using OuterType RXX_NODEBUG =
        std::conditional_t<forward_range<Base>, OuterIter, empty_outer>;

    friend join_view;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr OuterIter& get_outer() noexcept {
        if constexpr (forward_range<Base>) {
            return outer_;
        } else {
            return *parent_->outer_;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr OuterIter const& get_outer() const noexcept {
        if constexpr (forward_range<Base>) {
            return outer_;
        } else {
            return *parent_->outer_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr void satisfy() {

        for (; get_outer() != __RXX ranges::end(parent_->base_);
             ++get_outer()) {
            auto&& inner = [this]() -> auto&& {
                if constexpr (std::is_reference_v<range_reference_t<Base>>) {
                    return *get_outer();
                } else {
                    return parent_->inner_.emplace_deref(get_outer());
                }
            }();

            inner_ = __RXX ranges::begin(inner);
            if (*inner_ != __RXX ranges::end(inner)) {
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
    requires forward_range<Base>
        : outer_{std::move(outer)}
        , parent_{RXX_BUILTIN_addressof(parent)} {
        satisfy();
    }

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(Parent& parent) noexcept(
        std::is_nothrow_default_constructible_v<OuterIter>)
    requires (!forward_range<Base>)
        : parent_{RXX_BUILTIN_addressof(parent)} {
        satisfy();
    }

    __RXX_HIDE_FROM_ABI constexpr iterator(Parent* parent, OuterIter outer,
        InnerIter
            inner) noexcept(std::is_nothrow_move_constructible_v<OuterIter> &&
        std::is_nothrow_move_constructible_v<InnerIter>)
    requires forward_range<Base>
        : outer_(std::move(outer))
        , inner_(std::move(inner))
        , parent_(parent) {}

public:
    using iterator_concept = decltype([]() {
        if constexpr (std::is_reference_v<range_reference_t<Base>> &&
            bidirectional_range<Base> &&
            bidirectional_range<range_reference_t<Base>> &&
            std::ranges::common_range<range_reference_t<Base>>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::is_reference_v<range_reference_t<Base>> &&
            forward_range<Base> && forward_range<range_reference_t<Base>>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());
    using value_type = range_value_t<range_reference_t<Base>>;
    using difference_type = std::common_type_t<range_difference_t<Base>,
        range_difference_t<range_reference_t<Base>>>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<OuterType>)
    requires std::default_initializable<OuterType>
    = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_constructible_v<typename iterator<!Const>::OuterType,
            OuterType> &&
        std::is_nothrow_constructible_v<typename iterator<!Const>::InnerIter,
            InnerIter>)
    requires Const &&
                 std::convertible_to<typename iterator<!Const>::OuterType,
                     OuterType> &&
                 std::convertible_to<iterator_t<InnerRange>, InnerIter>
        : outer_{std::move(other.outer_)}
        , inner_{std::move(other.inner_)}
        , parent_{other.parent_} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const noexcept(
        noexcept(*std::declval<InnerIter const&>())) -> decltype(auto) {
        return **inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr InnerIter operator->() const
        noexcept(std::is_nothrow_copy_constructible_v<InnerIter>)
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
            __RXX ranges::end(details::as_lvalue(get_inner_range()))) {
            ++get_outer();
            satisfy();
        }
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires std::is_reference_v<range_reference_t<Base>> &&
        forward_range<Base> && forward_range<range_reference_t<Base>>
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires std::is_reference_v<range_reference_t<Base>> &&
        bidirectional_range<Base> &&
        bidirectional_range<range_reference_t<Base>> &&
        std::ranges::common_range<range_reference_t<Base>>
    {
        if (outer_ == __RXX ranges::end(parent_->base_)) {
            inner_ = __RXX ranges::end(details::as_lvalue(*--outer_));
        }

        while (*inner_ == __RXX ranges::begin(details::as_lvalue(*outer_))) {
            inner_ = __RXX ranges::end(details::as_lvalue(*--outer_));
        }
        --*inner_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires std::is_reference_v<range_reference_t<Base>> &&
        bidirectional_range<Base> &&
        bidirectional_range<range_reference_t<Base>> &&
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
        forward_range<Base> &&
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
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) OuterType outer_{};
    details::optional_base<InnerIter> inner_{};
    Parent* parent_ = nullptr;
};

template <input_range V>
requires view<V> && input_range<range_reference_t<V>>
template <bool Const>
class join_view<V>::sentinel {
private:
    template <bool>
    friend class sentinel;
    friend join_view;

    using Parent RXX_NODEBUG = details::const_if<Const, join_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;

public:
    __RXX_HIDE_FROM_ABI explicit constexpr sentinel(Parent& parent) noexcept(
        noexcept(__RXX ranges::end(parent.base_)) &&
        std::is_nothrow_move_constructible_v<sentinel_t<Base>>)
        : end_(__RXX ranges::end(parent.base_)) {}

    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<sentinel_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other) noexcept(
        std::is_nothrow_constructible_v<sentinel_t<Base>, sentinel_t<V>>)
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
    requires requires { join_view<all_t<R&&>>(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(join_view<all_t<R&&>>(std::declval<R>())))
            -> decltype(join_view<all_t<R&&>>(std::declval<R>())) {
        return join_view<all_t<R&&>>(std::forward<R>(arg));
    }

#if RXX_LIBSTDCXX
    static constexpr bool _S_has_simple_call_op = true;
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::join_t join{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
