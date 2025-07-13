// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/concepts.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/cached_position.h"
#include "rxx/details/movable_box.h"
#include "rxx/primitives.h"

#include <cassert>
#include <compare>
#include <iterator>
#include <ranges>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {
template <typename Pred, typename V>
concept chunk_by_predicate = view<V> && std::is_object_v<Pred> &&
    std::indirect_binary_predicate<Pred, iterator_t<V>, iterator_t<V>>;
}

template <forward_range V, details::chunk_by_predicate<V> Pred>
class chunk_by_view :
    public std::ranges::view_interface<chunk_by_view<V, Pred>> {

    class iterator;

public:
    __RXX_HIDE_FROM_ABI chunk_by_view() noexcept(
        std::is_nothrow_default_constructible_v<V> &&
        std::is_nothrow_default_constructible_v<Pred>)
    requires std::default_initializable<V> && std::default_initializable<Pred>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr chunk_by_view(
        V base, Pred pred) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_move_constructible_v<Pred>)
        : base_(std::move(base))
        , pred_(std::in_place, std::move(pred)) {}

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
    constexpr Pred const& pred() const noexcept { return *pred_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr iterator begin() {
        auto first = __RXX ranges::begin(base_);
        if (!cached_begin_) {
            cached_begin_.set(base_, find_next(first));
        }

        return iterator{*this, std::move(first), cached_begin_.get(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto end() {
        if constexpr (std::ranges::common_range<V>) {
            return iterator{
                *this, __RXX ranges::end(base_), __RXX ranges::end(base_)};
        } else {
            return std::default_sentinel;
        }
    }

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> find_next(iterator_t<V> current) {
        auto const pred = [this]<typename T, typename U>(
                              T&& left, U&& right) -> bool {
            return !std::invoke(
                *pred_, std::forward<T>(left), std::forward<U>(right));
        };

        return std::ranges::next(std::ranges::adjacent_find(
                                     current, __RXX ranges::end(base_), pred),
            1, __RXX ranges::end(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> find_prev(iterator_t<V> current)
    requires bidirectional_range<V>
    {
        auto first = __RXX ranges::begin(base_);
        std::ranges::reverse_view reversed{
            std::ranges::subrange{first, current}
        };

        auto const pred = [this]<typename T, typename U>(
                              T&& left, U&& right) -> bool {
            return !std::invoke(
                *pred_, std::forward<U>(right), std::forward<T>(left));
        };

        return std::ranges::prev(
            std::ranges::adjacent_find(reversed, pred).base(), 1,
            std::move(first));
    }

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_{};
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) details::movable_box<Pred> pred_;
    details::cached_position<V> cached_begin_;
};

template <typename R, typename Pred>
chunk_by_view(R&&, Pred) -> chunk_by_view<std::views::all_t<R>, Pred>;

template <forward_range V, details::chunk_by_predicate<V> Pred>
class chunk_by_view<V, Pred>::iterator {
    friend chunk_by_view;

    __RXX_HIDE_FROM_ABI constexpr iterator(chunk_by_view& parent,
        iterator_t<V> current,
        iterator_t<V>
            next) noexcept(std::is_nothrow_move_constructible_v<iterator_t<V>>)
        : parent_(RXX_BUILTIN_addressof(parent))
        , current_(std::move(current))
        , next_(std::move(next)) {}

public:
    using value_type = std::ranges::subrange<iterator_t<V>>;
    using difference_type = range_difference_t<V>;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::conditional_t<bidirectional_range<V>,
        std::bidirectional_iterator_tag, std::forward_iterator_tag>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<V>>) = default;

    __RXX_HIDE_FROM_ABI constexpr value_type operator*() const
        noexcept(std::is_nothrow_constructible_v<value_type,
            iterator_t<V> const&, iterator_t<V> const&>) {
        return value_type{current_, next_};
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        current_ = next_;
        next_ = parent_->find_next(current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int) {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires bidirectional_range<V>
    {
        next_ = current_;
        current_ = parent_->find_prev(next_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires bidirectional_range<V>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right) {
        return left.current_ == right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& iter, std::default_sentinel_t) {
        return iter.current_ == iter.next_;
    }

private:
    chunk_by_view* parent_ = nullptr;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) iterator_t<V> current_{};
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) iterator_t<V> next_{};
};

namespace views {
namespace details {
struct chunk_by_t : ranges::details::adaptor_non_closure<chunk_by_t> {

    template <typename R, typename Pred>
    requires requires {
        chunk_by_view(std::declval<R>(), std::declval<Pred>());
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(R&& arg,
        Pred&& pred) const noexcept(noexcept(chunk_by_view(std::forward<R>(arg),
        std::forward<Pred>(pred)))) {
        return chunk_by_view(std::forward<R>(arg), std::forward<Pred>(pred));
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<chunk_by_t>::operator();
    static constexpr int _S_arity = 2;
    static constexpr bool _S_has_simple_extra_args = true;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename Pred>
    requires std::constructible_from<std::decay_t<Pred>, Pred>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        Pred&& pred) const
        noexcept(std::is_nothrow_constructible_v<std::decay_t<Pred>, Pred>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(*this),
            std::forward<Pred>(pred));
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::chunk_by_t chunk_by{};
}
} // namespace views

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
