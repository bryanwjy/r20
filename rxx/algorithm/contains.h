// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/find.h"
#include "rxx/algorithm/search.h"
#include "rxx/functional/equal_to.h"
#include "rxx/functional/identity.h"
#include "rxx/iterator.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {
struct contains_t {
    template <std::input_iterator I, std::sentinel_for<I> S,
        typename Proj = identity, typename T = projected_value_t<I, Proj>>
    requires std::indirect_binary_predicate<equal_to, std::projected<I, Proj>,
        T const*>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(I first, S last, T const& value, Proj proj = {}) RXX_CONST_CALL {
        return ranges::find(std::move(first), last, value, proj) != last;
    }

    template <input_range R, typename Proj = identity,
        typename T = projected_value_t<iterator_t<R>, Proj>>
    requires std::indirect_binary_predicate<equal_to,
        std::projected<iterator_t<R>, Proj>, T const*>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(R&& r, T const& value, Proj proj = {}) RXX_CONST_CALL {
        return ranges::find(std::move(ranges::begin(r)), ranges::end(r), value,
                   proj) != ranges::end(r);
    }
};

struct contains_subrange_t {
private:
    template <typename I1, typename S1, typename I2, typename S2, typename Pred,
        typename Proj1, typename Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr bool impl(I1&& first1, S1&& last1, I2&& first2, S2&& last2,
        Pred&& pred, Proj1&& proj1, Proj2&& proj2) {
        return (first2 == last2) ||
            !ranges::search(first1, last1, first2, last2, std::move(pred),
                std::move(proj1), std::move(proj2))
                 .empty();
    }

public:
    template <std::forward_iterator I1, std::sentinel_for<I1> S1,
        std::forward_iterator I2, std::sentinel_for<I2> S2,
        typename Pred = equal_to, typename Proj1 = identity,
        typename Proj2 = identity>
    requires std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
        Proj1 proj1 = {}, Proj2 proj2 = {}) RXX_CONST_CALL {
        return impl(std::move(first1), std::move(last1), std::move(first2),
            std::move(last2), std::move(pred), std::move(proj1),
            std::move(proj2));
    }

    template <forward_range R1, forward_range R2, typename Pred = equal_to,
        typename Proj1 = identity, typename Proj2 = identity>
    requires std::indirectly_comparable<iterator_t<R1>, iterator_t<R2>, Pred,
        Proj1, Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {},
        Proj2 proj2 = {}) RXX_CONST_CALL {
        return impl(ranges::begin(r1), ranges::end(r1), ranges::begin(r2),
            ranges::end(r2), std::move(pred), std::move(proj1),
            std::move(proj2));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::contains_t contains{};
inline constexpr details::contains_subrange_t contains_subrange{};
} // namespace cpo
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
