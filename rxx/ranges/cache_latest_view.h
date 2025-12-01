// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/non_propagating_cache.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/utility.h"

#include <cassert>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
template <input_range V>
requires view<V>
class cache_latest_view : public view_interface<cache_latest_view<V>> {
    using CacheT RXX_NODEBUG =
        std::conditional_t<std::is_reference_v<range_reference_t<V>>,
            std::add_pointer_t<range_reference_t<V>>, range_reference_t<V>>;

    class iterator;
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr cache_latest_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr cache_latest_view(V base) noexcept(
        std::is_nothrow_move_constructible_v<V>)
        : base_{__RXX move(base)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const& noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return base_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() && noexcept(std::is_nothrow_move_constructible_v<V>) {
        return __RXX move(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() { return iterator(*this); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() { return sentinel(*this); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return ranges::size(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return ranges::size(base_);
    }

private:
    V base_{};
    details::non_propagating_cache<CacheT> cache_;
};

template <typename R>
cache_latest_view(R&&) -> cache_latest_view<views::all_t<R>>;

template <input_range V>
requires view<V>
class cache_latest_view<V>::iterator {

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(cache_latest_view& parent)
        : parent_(RXX_BUILTIN_addressof(parent))
        , current_(__RXX ranges::begin(parent.base_)) {}

    friend cache_latest_view;

public:
    using difference_type = range_difference_t<V>;
    using value_type = range_value_t<V>;
    using iterator_concept = std::input_iterator_tag;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> base() && { return __RXX move(current_); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<V> const& base() const& noexcept { return current_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr range_reference_t<V>& operator*() const {
        if constexpr (std::is_reference_v<range_reference_t<V>>) {

            if (!parent_->cache_) {
                parent_->cache_ = RXX_BUILTIN_addressof(
                    [&]() -> decltype(auto) { return *current_; }());
            }

            return **parent_->cache_;

        } else {
            if (!parent_->cache_) {
                parent_->cache_.emplace_deref(current_);
            }
            return *parent_->cache_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() noexcept(
        noexcept(++current_)) {
        parent_->cache_.reset();
        ++current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_rvalue_reference_t<V>
    iter_move(iterator const& other) noexcept(
        noexcept(ranges::iter_move(std::declval<iterator_t<V> const&>()))) {
        return ranges::iter_move(other.current_);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(iterator const& left, iterator const& right) noexcept(
        noexcept(ranges::iter_swap(std::declval<iterator_t<V> const&>(),
            std::declval<iterator_t<V> const&>())))
    requires std::indirectly_swappable<iterator_t<V>>
    {
        ranges::iter_swap(left.current_, right.current_);
    }

private:
    cache_latest_view* parent_;
    iterator_t<V> current_;
};

template <input_range V>
requires view<V>
class cache_latest_view<V>::sentinel {

    __RXX_HIDE_FROM_ABI constexpr explicit sentinel(
        cache_latest_view& parent) noexcept(std::
                                                is_nothrow_move_constructible_v<
                                                    sentinel_t<V>> &&
        std::is_nothrow_invocable_v<decltype(__RXX ranges::end),
            decltype(parent.base_)>)
        : end_(__RXX ranges::end(parent.base_)) {}

    friend cache_latest_view;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr decltype(auto) get_iter_current(
        iterator const& iter) noexcept {
        return (iter.current_);
    }

public:
    __RXX_HIDE_FROM_ABI sentinel() noexcept(
        std::is_nothrow_default_constructible_v<sentinel_t<V>>) = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel_t<V> base() const
        noexcept(std::is_nothrow_copy_constructible_v<sentinel_t<V>>) {
        return end_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, sentinel const& right) {
        return get_iter_current(left) == right.end_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_difference_t<V> operator-(
        iterator const& left, sentinel const& right)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return get_iter_current(left) - right.end_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr range_difference_t<V> operator-(
        sentinel const& left, iterator const& right)
    requires std::sized_sentinel_for<sentinel_t<V>, iterator_t<V>>
    {
        return left.end_ - get_iter_current(right);
    }

private:
    sentinel_t<V> end_{};
};

namespace views {
namespace details {
struct cache_latest_t : __RXX ranges::details::adaptor_closure<cache_latest_t> {

    template <viewable_range R>
    requires requires { cache_latest_view(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(cache_latest_view(std::declval<R>()))) {
        return cache_latest_view(__RXX forward<R>(arg));
    }

#if RXX_LIBSTDCXX
    static constexpr bool _S_has_simple_call_op = true;
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::cache_latest_t cache_latest{};
} // namespace cpo
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
