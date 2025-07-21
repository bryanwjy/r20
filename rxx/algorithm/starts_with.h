// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/mismatch.h"
#include "rxx/functional/equal_to.h"
#include "rxx/functional/identity.h"
#include "rxx/iterator.h"
#include "rxx/ranges/concepts.h"

#include <functional>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
namespace details {
struct starts_with_t {
    template <std::input_iterator I1, std::sentinel_for<I1> S1,
        std::input_iterator I2, std::sentinel_for<I2> S2,
        typename Pred = equal_to, typename Proj1 = identity,
        typename Proj2 = identity>
    requires std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
        Proj1 proj1 = {}, Proj2 proj2 = {}) RXX_CONST_CALL {
        return ranges::mismatch(std::move(first1), std::move(last1),
                   std::move(first2), std::move(last2), std::ref(pred),
                   std::ref(proj1), std::ref(proj2))
                   .in2 == last2;
    }

    template <input_range R1, input_range R2, typename Pred = equal_to,
        typename Proj1 = identity, typename Proj2 = identity>
    requires std::indirectly_comparable<iterator_t<R1>, iterator_t<R2>, Pred,
        Proj1, Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(R1&& range1, R2&& range2, Pred pred = {}, Proj1 proj1 = {},
        Proj2 proj2 = {}) RXX_CONST_CALL {
        return ranges::mismatch(ranges::begin(range1), ranges::end(range1),
                   ranges::begin(range2), ranges::end(range2), std::ref(pred),
                   std::ref(proj1), std::ref(proj2))
                   .in2 == ranges::end(range2);
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::starts_with_t starts_with{};
}
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
