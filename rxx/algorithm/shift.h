// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/move.h"
#include "rxx/borrow_traits.h"
#include "rxx/concepts.h"
#include "rxx/iterator.h"
#include "rxx/primitives.h"
#include "rxx/subrange.h"

#include <algorithm>
#include <cassert>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {
namespace details {
struct shift_left_t {
    template <std::permutable I, std::sentinel_for<I> S>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr subrange<I> operator()(
        I first, S last, iter_difference_t<I> count) RXX_CONST_CALL {
        assert(count >= 0);
        if (count == 0) {
            return {first, ranges::next(first, last)};
        }

        auto mid = ranges::next(first, count, last);
        if (mid == last) {
            return {first, first};
        }

        return {first, ranges::move(mid, last, first).out};
    }

    template <forward_range R>
    requires std::permutable<iterator_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr borrowed_subrange_t<R>
    operator()(R&& range, range_difference_t<R> count) RXX_CONST_CALL {
        return operator()(ranges::begin(range), ranges::end(range), count);
    }
};

struct shift_right_t {
    template <std::permutable I, std::sentinel_for<I> S>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr subrange<I> operator()(
        I first, S last, iter_difference_t<I> count) RXX_CONST_CALL {
        assert(count >= 0);
        if (count == 0) {
            return {first, ranges::next(first, last)};
        }

        if constexpr (std::bidirectional_iterator<I> && std::same_as<I, S>) {
            auto mid = ranges::next(last, -count, first);
            if (mid == first) {
                return {last, last};
            }

            return {ranges::move_backward(first, mid, last).out, last};
        } else {
            auto const result = ranges::next(first, count, last);
            // If shift count is greater than range length, do nothing
            if (result == last) {
                return {result, result};
            }

            auto dst_head = first;
            auto dst_tail = result;
            while (dst_head != result) {
                if (dst_tail == last) {
                    return {result, ranges::move(first, dst_head, result).out};
                }
                ++dst_head;
                ++dst_tail;
            }

            for (;;) {
                //   <----count----->
                //  [first|...|result|...|last]
                //             <-------count------->
                //  [first|...|dst_head|...|dst_tail|last]
                //     |
                //  [cursor]
                auto cursor = first;
                while (cursor != result) {
                    if (dst_tail == last) {
                        // if distance(dst_tail, last) <= count
                        //  End up in disjoint subrange:
                        //           <------ N ------>     <------ N ------>
                        //  [xxx|OOO|cursor|...|result|...|dst_head|...|last]
                        // where O is moved to, x is moved from
                        dst_head = ranges::move(cursor, result, dst_head).out;
                        return {
                            result, ranges::move(first, cursor, dst_head).out};
                    }
                    ranges::iter_swap(cursor, dst_head);
                    ++dst_head;
                    ++dst_tail;
                    ++cursor;
                }
            }
        }
    }

    template <forward_range R>
    requires std::permutable<iterator_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr borrowed_subrange_t<R>
    operator()(R&& range, range_difference_t<R> count) RXX_CONST_CALL {
        return operator()(ranges::begin(range), ranges::end(range), count);
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::shift_left_t shift_left{};
inline constexpr details::shift_right_t shift_right{};
} // namespace cpo

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
