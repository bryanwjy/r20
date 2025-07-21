// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/iterator/iter_traits.h"
#include "rxx/ranges/access.h"

#include <concepts>
#include <iterator>
#include <type_traits>

#if __has_include(<__ranges/dangling.h>)
#  include <__ranges/dangling.h>
#elif RXX_LIBCXX
#  error "Unsupported compilation"
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

using std::ranges::dangling;

template <range R>
using range_reference_t = iter_reference_t<iterator_t<R>>;

template <range R>
using range_rvalue_reference_t = iter_rvalue_reference_t<iterator_t<R>>;

template <range R>
using range_common_reference_t = iter_common_reference_t<iterator_t<R>>;

template <range R>
using const_iterator_t = const_iterator<iterator_t<R>>;

template <range R>
using const_sentinel_t = const_sentinel<sentinel_t<R>>;

template <range R>
using range_const_reference_t = iter_const_reference_t<iterator_t<R>>;

template <range R>
using range_const_rvalue_reference_t =
    iter_const_rvalue_reference_t<iterator_t<R>>;

template <sized_range R>
using range_size_t = decltype(ranges::size(std::declval<R&>()));

template <range R>
using range_difference_t = iter_difference_t<iterator_t<R>>;

template <range R>
using range_value_t = iter_value_t<iterator_t<R>>;

namespace details {

struct advance_t final {

    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL void
    operator&() RXX_CONST_CALL = delete;

    template <std::input_or_output_iterator It>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr void operator()(
        It& iter, iter_difference_t<It> offset) RXX_CONST_CALL {
        if constexpr (std::random_access_iterator<It>)
            iter += offset;
        else if constexpr (std::bidirectional_iterator<It>) {
            if (offset > 0) {
                do {
                    ++iter;
                } while (--offset);
            } else if (offset < 0) {
                do {
                    --iter;
                } while (++offset);
            }
        } else {
            assert(offset >= 0);
            while (offset-- > 0) {
                ++iter;
            }
        }
    }

    template <std::input_or_output_iterator It, std::sentinel_for<It> Sent>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr void operator()(
        It& iter, Sent bound) RXX_CONST_CALL {
        if constexpr (std::assignable_from<It&, Sent>) {
            iter = std::move(bound);
        } else if constexpr (std::sized_sentinel_for<Sent, It>) {
            operator()(iter, bound - iter);
        } else {
            while (iter != bound) {
                ++iter;
            }
        }
    }

    template <std::input_or_output_iterator It, std::sentinel_for<It> Sent>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr iter_difference_t<It>
    operator()(
        It& iter, iter_difference_t<It> offset, Sent bound) RXX_CONST_CALL {
        if constexpr (std::sized_sentinel_for<Sent, It>) {
            auto const diff = bound - iter;

            if (diff == 0) {
                return offset;
            } else if (diff > 0 ? offset >= diff : offset <= diff) {
                (*this)(iter, bound);
                return offset - diff;
            } else if (offset != 0) [[likely]] {
                // n and bound must not lead in opposite directions:
                assert((offset < 0) == (diff < 0));

                (*this)(iter, offset);
                return 0;
            } else {
                return 0;
            }
        } else if (iter == bound || offset == 0) {
            return offset;
        } else if (offset > 0) {
            iter_difference_t<It> count = 0;
            do {
                ++iter;
                ++count;
            } while (count != offset && iter != bound);
            return offset - count;

        } else if constexpr (std::bidirectional_iterator<It> &&
            std::same_as<It, Sent>) {

            iter_difference_t<It> count = 0;
            do {
                --iter;
                --count;
            } while (count != offset && iter != bound);
            return offset - count;
        } else {
            // cannot decrement a non-bidirectional iterator
            assert(offset >= 0);
            return offset;
        }
    }
};

struct distance_t final {

    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL void
    operator&() RXX_CONST_CALL = delete;

    template <typename It, std::sentinel_for<It> Sent>
    requires (!std::sized_sentinel_for<Sent, It>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL
        constexpr iter_difference_t<It>
        operator()(It first, Sent last) RXX_CONST_CALL {
        iter_difference_t<It> count = 0;
        while (first != last) {
            ++first;
            ++count;
        }
        return count;
    }

    template <typename It, std::sized_sentinel_for<std::decay_t<It>> Sent>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr iter_difference_t<std::decay_t<It>> operator()(
        It&& first, Sent last) RXX_CONST_CALL {
        return last - static_cast<std::decay_t<It> const&>(first);
    }

    template <range R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr range_difference_t<R> operator()(
        R&& range) RXX_CONST_CALL {
        if constexpr (sized_range<R>) {
            return static_cast<range_difference_t<R>>(ranges::size(range));
        } else {
            return operator()(ranges::begin(range), ranges::end(range));
        }
    }
};
} // namespace details

inline namespace cpo {
inline constexpr ranges::details::advance_t advance{};
inline constexpr ranges::details::distance_t distance{};
} // namespace cpo

namespace details {
struct next_t final {

    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL void
    operator&() RXX_CONST_CALL = delete;

    template <std::input_or_output_iterator It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(It arg) RXX_CONST_CALL {
        ++arg;
        return arg;
    }

    template <std::input_or_output_iterator It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(
        It arg, iter_difference_t<It> offset) RXX_CONST_CALL {
        ranges::advance(arg, offset);
        return arg;
    }

    template <std::input_or_output_iterator It, std::sentinel_for<It> Sent>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(It arg, Sent bound) RXX_CONST_CALL {
        ranges::advance(arg, bound);
        return arg;
    }

    template <std::input_or_output_iterator It, std::sentinel_for<It> Sent>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(
        It arg, iter_difference_t<It> offset, Sent bound) RXX_CONST_CALL {
        ranges::advance(arg, offset, bound);
        return arg;
    }
};

struct prev_t final {

    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL void
    operator&() RXX_CONST_CALL = delete;

    template <std::bidirectional_iterator It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(It iter) RXX_CONST_CALL {
        --iter;
        return iter;
    }

    template <std::bidirectional_iterator It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(
        It iter, iter_difference_t<It> offset) RXX_CONST_CALL {
        ranges::advance(iter, -offset);
        return iter;
    }

    template <std::bidirectional_iterator It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr It operator()(
        It iter, iter_difference_t<It> offset, It bound) RXX_CONST_CALL {
        ranges::advance(iter, -offset, bound);
        return iter;
    }
};

} // namespace details

inline namespace cpo {
inline constexpr ranges::details::next_t next{};
inline constexpr ranges::details::prev_t prev{};
} // namespace cpo
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
