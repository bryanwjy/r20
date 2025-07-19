// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/find.h"
#include "rxx/borrow_traits.h"
#include "rxx/functional/equal_to.h"
#include "rxx/functional/identity.h"
#include "rxx/iter_traits.h"
#include "rxx/primitives.h"
#include "rxx/subrange.h"

#include <algorithm>
#include <functional>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {
struct find_last_t {
protected:
    template <typename I, typename S, typename Pred, typename Proj>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr subrange<I> impl(
        I first, S last, Pred&& pred, Proj& proj) {
        if (first == last) {
            return subrange<I>(first, first);
        }

        if constexpr (std::bidirectional_iterator<I>) {
            auto last_it = ranges::next(first, last);
            for (auto it = ranges::prev(last_it); it != first; --it) {
                if (std::invoke(pred, std::invoke(proj, *it))) {
                    return subrange<I>(std::move(it), std::move(last_it));
                }
            }
            if (std::invoke(pred, std::invoke(proj, *first))) {
                return subrange<I>(std::move(first), std::move(last_it));
            }
            return subrange<I>(last_it, last_it);
        } else {
            bool found = false;
            I found_it;
            for (; first != last; ++first) {
                if (std::invoke(pred, std::invoke(proj, *first))) {
                    found = true;
                    found_it = first;
                }
            }

            if (found) {
                return subrange<I>(std::move(found_it), std::move(first));
            } else {
                return subrange<I>(first, first);
            }
        }
    }

public:
    template <std::forward_iterator I, std::sentinel_for<I> S,
        typename Proj = std::identity, typename T = projected_value_t<I, Proj>>
    requires std::indirect_binary_predicate<ranges::equal_to,
        std::projected<I, Proj>, T const*>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr ranges::subrange<I>
    operator()(I first, S last, T const& value, Proj proj = {}) RXX_CONST_CALL {
        return impl(
            std::move(first), std::move(last),
            [&]<typename U>(U&& val) { return value == std::forward<U>(val); },
            proj);
    }

    template <forward_range R, typename Proj = identity,
        typename T = projected_value_t<iterator_t<R>, Proj>>
    requires std::indirect_binary_predicate<ranges::equal_to,
        std::projected<iterator_t<R>, Proj>, T const*>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL
        constexpr ranges::borrowed_subrange_t<R>
        operator()(R&& range, T const& value, Proj proj = {}) RXX_CONST_CALL {
        return impl(
            ranges::begin(range), ranges::end(range),
            [&]<typename U>(U&& val) { return value == std::forward<U>(val); },
            proj);
    }
};

struct find_last_if_t : private find_last_t {
    template <std::forward_iterator I, std::sentinel_for<I> S,
        typename Proj = identity,
        std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr subrange<I> operator()(
        I first, S last, Pred pred, Proj proj = {}) RXX_CONST_CALL {
        return find_last_t::impl(std::move(first), std::move(last), pred, proj);
    }

    template <forward_range R, typename Proj = identity,
        std::indirect_unary_predicate<std::projected<iterator_t<R>, Proj>> Pred>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr borrowed_subrange_t<R> operator()(
        R&& range, Pred pred, Proj proj = {}) RXX_CONST_CALL {
        return find_last_t::impl(
            ranges::begin(range), ranges::end(range), pred, proj);
    }
};

struct find_last_if_not_t : private find_last_t {
    template <std::forward_iterator I, std::sentinel_for<I> S,
        typename Proj = identity,
        std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr subrange<I> operator()(
        I first, S last, Pred pred, Proj proj = {}) RXX_CONST_CALL {
        return impl(
            std::move(first), std::move(last),
            [&]<typename U>(
                U&& val) { return !std::invoke(pred, std::forward<U>(val)); },
            proj);
    }

    template <forward_range R, typename Proj = identity,
        std::indirect_unary_predicate<std::projected<iterator_t<R>, Proj>> Pred>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr borrowed_subrange_t<R> operator()(
        R&& range, Pred pred, Proj proj = {}) RXX_CONST_CALL {
        return impl(
            ranges::begin(range), ranges::end(range),
            [&]<typename U>(
                U&& val) { return !std::invoke(pred, std::forward<U>(val)); },
            proj);
    }
};

} // namespace details

inline namespace cpo {
inline constexpr details::find_last_t find_last;
inline constexpr details::find_last_if_t find_last_if;
inline constexpr details::find_last_if_not_t find_last_if_not;
} // namespace cpo

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
