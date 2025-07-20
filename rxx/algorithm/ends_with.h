// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/equal.h"
#include "rxx/algorithm/starts_with.h"
#include "rxx/functional/equal_to.h"
#include "rxx/functional/identity.h"
#include "rxx/iterator.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"

#include <functional>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {
struct ends_with_t {
private:
    template <typename I1, typename S1, typename I2, typename S2, typename Pred,
        typename Proj1, typename Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr bool bidirectional_impl(I1 first1, S1 last1, I2 first2,
        S2 last2, Pred& pred, Proj1& proj1, Proj2& proj2) {
        auto rbegin1 = std::make_reverse_iterator(last1);
        auto rend1 = std::make_reverse_iterator(first1);
        auto rbegin2 = std::make_reverse_iterator(last2);
        auto rend2 = std::make_reverse_iterator(first2);
        return ranges::starts_with(rbegin1, rend1, rbegin2, rend2,
            std::ref(pred), std::ref(proj1), std::ref(proj2));
    }

    template <typename I1, typename S1, typename I2, typename S2, typename Pred,
        typename Proj1, typename Proj2, typename Offset>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr bool with_offset_impl(I1 first1, S1 last1, I2 first2,
        S2 last2, Pred& pred, Proj1& proj1, Proj2& proj2, Offset offset) {
        if constexpr (std::bidirectional_iterator<S1> &&
            std::bidirectional_iterator<S2> &&
            !std::random_access_iterator<S1> &&
            !std::random_access_iterator<S2>) {
            return bidirectional_impl(std::move(first1), std::move(last1),
                std::move(first2), std::move(last2), pred, proj1, proj2);

        } else {
            ranges::advance(first1, offset);
            return ranges::equal(std::move(first1), std::move(last1),
                std::move(first2), std::move(last2), std::ref(pred),
                std::ref(proj1), std::ref(proj2));
        }
    }

    template <typename I1, typename S1, typename I2, typename S2, typename Pred,
        typename Proj1, typename Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr bool impl(I1 first1, S1 last1, I2 first2, S2 last2,
        Pred& pred, Proj1& proj1, Proj2& proj2) {
        if constexpr (std::bidirectional_iterator<S1> &&
            std::bidirectional_iterator<S2> &&
            (!std::random_access_iterator<S1>)&&(
                !std::random_access_iterator<S2>)) {
            return bidirectional_impl(
                first1, last1, first2, last2, pred, proj1, proj2);

        } else {
            auto const dist1 = ranges::distance(first1, last1);
            auto const dist2 = ranges::distance(first2, last2);
            if (dist2 == 0) {
                return true;
            }
            if (dist2 > dist1) {
                return false;
            }

            return with_offset_impl(std::move(first1), std::move(last1),
                std::move(first2), std::move(last2), pred, proj1, proj2,
                dist1 - dist2);
        }
    }

public:
    template <std::input_iterator I1, std::sentinel_for<I1> S1,
        std::input_iterator I2, std::sentinel_for<I2> S2,
        typename Pred = equal_to, typename Proj1 = identity,
        typename Proj2 = identity>
    requires (std::forward_iterator<I1> || std::sized_sentinel_for<S1, I1>) &&
        (std::forward_iterator<I2> || std::sized_sentinel_for<S2, I2>) &&
        std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
        Proj1 proj1 = {}, Proj2 proj2 = {}) RXX_CONST_CALL {
        return impl(std::move(first1), std::move(last1), std::move(first2),
            std::move(last2), pred, proj1, proj2);
    }

    template <input_range R1, input_range R2, typename Pred = equal_to,
        typename Proj1 = identity, typename Proj2 = identity>
    requires (forward_range<R1> || sized_range<R1>) &&
        (forward_range<R2> || sized_range<R2>) &&
        std::indirectly_comparable<iterator_t<R1>, iterator_t<R2>, Pred, Proj1,
            Proj2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr bool
    operator()(R1&& range1, R2&& range2, Pred pred = {}, Proj1 proj1 = {},
        Proj2 proj2 = {}) RXX_CONST_CALL {
        if constexpr (sized_range<R1> && sized_range<R2>) {
            auto const dist1 = ranges::size(range1);
            auto const dist2 = ranges::size(range2);
            if (dist2 == 0) {
                return true;
            }

            if (dist2 > dist1) {
                return false;
            }

            return with_offset_impl(ranges::begin(range1), ranges::end(range1),
                ranges::begin(range2), ranges::end(range2), pred, proj1, proj2,
                dist1 - dist2);

        } else {
            return impl(ranges::begin(range1), ranges::end(range1),
                ranges::begin(range2), ranges::end(range2), pred, proj1, proj2);
        }
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::ends_with_t ends_with{};
}

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
