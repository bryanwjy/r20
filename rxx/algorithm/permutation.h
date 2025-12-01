// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/all_of.h"
#include "rxx/algorithm/count.h"
#include "rxx/algorithm/find.h"
#include "rxx/algorithm/mismatch.h"
#include "rxx/algorithm/return_types.h"
#include "rxx/functional/equal_to.h"
#include "rxx/functional/identity.h"
#include "rxx/iterator.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/iota_view.h"
#include "rxx/ranges/primitives.h"

#include <algorithm>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {

struct is_permutation_t {
private:
    template <typename I1, typename S1, typename I2, typename S2,
        typename Proj1, typename Proj2, typename Pred>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr bool impl(I1 first1, S1 last1, I2 first2, S2 last2,
        Pred&& pred, Proj1&& proj1, Proj2&& proj2) {
        // skip common prefix
        auto const ret =
            ranges::mismatch(first1, last1, first2, last2, pred, proj1, proj2);
        first1 = ret.in1, first2 = ret.in2;

        // iterate over the rest, counting how many times each element
        // from [first1, last1) appears in [first2, last2)
        return ranges::all_of(views::iota(first1, last1), [&](auto&& it) {
            auto const val = std::invoke(proj1, *it);
            auto const cmp = [&]<typename R>(R&& right) {
                return std::invoke(pred, val, __RXX forward<R>(right));
            };

            if (it != ranges::find_if(first1, it, cmp, proj1)) {
                return true;
            }

            auto const count = ranges::count_if(first2, last2, cmp, proj2);
            return count != 0 &&
                count == ranges::count_if(it, last1, cmp, proj1);
        });
    }

public:
    template <std::forward_iterator I1, std::sentinel_for<I1> S1,
        std::forward_iterator I2, std::sentinel_for<I2> S2,
        typename Proj1 = identity, typename Proj2 = identity,
        std::indirect_equivalence_relation<std::projected<I1, Proj1>,
            std::projected<I2, Proj2>>
            Pred = equal_to>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr bool operator()(I1 first1, S1 last1, I2 first2,
        S2 last2, Pred pred = {}, Proj1 proj1 = {},
        Proj2 proj2 = {}) RXX_CONST_CALL {
        return impl(
            first1, last1, first2, last2,
            [&]<typename L, typename R>(L&& left, R&& right) -> decltype(auto) {
                return std::invoke(
                    pred, __RXX forward<L>(left), __RXX forward<R>(right));
            },
            [&]<typename T>(T&& arg) -> decltype(auto) {
                return std::invoke(proj1, __RXX forward<T>(arg));
            },
            [&]<typename T>(T&& arg) -> decltype(auto) {
                return std::invoke(proj2, __RXX forward<T>(arg));
            });
    }

    template <ranges::forward_range R1, ranges::forward_range R2,
        typename Proj1 = identity, typename Proj2 = identity,
        std::indirect_equivalence_relation<
            std::projected<iterator_t<R1>, Proj1>,
            std::projected<iterator_t<R2>, Proj2>>
            Pred = equal_to>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {},
        Proj1 proj1 = {}, Proj2 proj2 = {}) RXX_CONST_CALL {
        if constexpr (sized_range<R1> && sized_range<R2>) {
            if (ranges::distance(r1) != ranges::distance(r2)) {
                return false;
            }
        }

        return impl(
            ranges::begin(r1), ranges::end(r1), ranges::begin(r2),
            ranges::end(r2),
            [&]<typename L, typename R>(L&& left, R&& right) -> decltype(auto) {
                return std::invoke(
                    pred, __RXX forward<L>(left), __RXX forward<R>(right));
            },
            [&]<typename T>(T&& arg) -> decltype(auto) {
                return std::invoke(proj1, __RXX forward<T>(arg));
            },
            [&]<typename T>(T&& arg) -> decltype(auto) {
                return std::invoke(proj2, __RXX forward<T>(arg));
            });
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::is_permutation_t is_permutation{};
using std::ranges::next_permutation;
using std::ranges::prev_permutation;
} // namespace cpo

template <typename I>
using next_permutation_result = in_found_result<I>;
template <typename I>
using prev_permutation_result = in_found_result<I>;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
