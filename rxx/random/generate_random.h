// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/copy.h"
#include "rxx/algorithm/equal.h"
#include "rxx/algorithm/generate.h"
#include "rxx/algorithm/starts_with.h"
#include "rxx/borrow_traits.h"
#include "rxx/concepts.h"
#include "rxx/iterator.h"
#include "rxx/primitives.h"

#include <functional>
#include <random>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {

struct generate_random_t {
    template <typename R, typename G>
    requires output_range<R, std::invoke_result_t<G&>> &&
        std::uniform_random_bit_generator<std::remove_cvref_t<G>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr borrowed_iterator_t<R>
    operator()(R&& range, G&& generator) RXX_CONST_CALL {
        if constexpr (requires {
                          generator.generate_random(std::declval<R>());
                      }) {
            generator.generate_random(std::forward<R>(range));
            return range.end();
        } else if constexpr (sized_range<R>) {
            using I = std::invoke_result_t<G&>;
            if constexpr (requires(std::span<I, 32> s) {
                              generator.generate_random(s);
                          }) {
                auto remaining = ranges::distance(range.begin(), range.end());
                for (auto it = range.begin(); it != range.end();) {
                    if (remaining >= 32) {
                        I buffer[32];
                        generator.generate_random(buffer);
                        it = ranges::copy(buffer, it).out;
                        remaining -= 32;
                    } else { // Tail
                        return ranges::generate(
                            std::move(it), range.end(), std::ref(generator));
                    }
                }
                return range.end();
            } else if constexpr (requires(std::span<I> s) {
                                     generator.generate_random(s);
                                 }) {
                auto remaining = ranges::distance(range.begin(), range.end());
                constexpr size_t vec_size = 16;
                for (auto it = range.begin(); it != range.end();) {
                    if (remaining >= vec_size) {
                        I buffer[vec_size];
                        generator.generate_random(std::span<I>{buffer});
                        it = ranges::copy(buffer, it).out;
                        remaining -= vec_size;
                    } else { // Tail
                        return ranges::generate(
                            std::move(it), range.end(), std::ref(generator));
                    }
                }
                return range.end();
            }
        } else {
            return ranges::generate(
                std::forward<R>(range), std::ref(generator));
        }
    }

    template <typename G, std::output_iterator<std::invoke_result_t<G&>> O,
        std::sentinel_for<O> S>
    requires std::uniform_random_bit_generator<std::remove_cvref_t<G>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr O operator()(
        O first, S last, G&& generator) RXX_CONST_CALL {
        return operator()(ranges::subrange<O, S>(std::move(first), last),
            std::forward<G>(generator));
    }

    template <typename R, typename G, typename D>
    requires output_range<R, std::invoke_result_t<D&, G&>> &&
        std::invocable<D&, G&> &&
        std::uniform_random_bit_generator<std::remove_cvref_t<G>> &&
        std::is_arithmetic_v<std::invoke_result_t<D&, G&>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr ranges::borrowed_iterator_t<R>
    operator()(R&& range, G&& generator, D&& distribution) RXX_CONST_CALL {
        if constexpr (requires {
                          distribution.generate_random(
                              std::declval<R>(), generator);
                      }) {
            distribution.generate_random(std::forward<R>(range), generator);
            return range.end();
        } else if constexpr (sized_range<R>) {
            using I = std::invoke_result_t<D&, G&>;
            if constexpr (requires(std::span<I, 32> s) {
                              distribution.generate_random(s, generator);
                          }) {
                auto remaining = ranges::distance(range.begin(), range.end());
                for (auto it = range.begin(); it != range.end();) {
                    if (remaining >= 32) {
                        I buffer[32];
                        distribution.generate_random(buffer, generator);
                        it = ranges::copy(buffer, it).out;
                        remaining -= 32;
                    } else { // Tail
                        return ranges::generate(
                            std::move(it), range.end(), [&]() {
                                return std::invoke(distribution, generator);
                            });
                    }
                }
                return range.end();
            } else if constexpr (requires(std::span<I> s) {
                                     distribution.generate_random(s, generator);
                                 }) {
                constexpr size_t vec_size = 16;
                auto remaining = ranges::distance(range.begin(), range.end());
                for (auto it = range.begin(); it != range.end();) {
                    if (remaining >= vec_size) {
                        I buffer[vec_size];
                        distribution.generate_random(
                            std::span<I>{buffer}, generator);
                        it = ranges::copy(buffer, it).out;
                        remaining -= vec_size;
                    } else {
                        return ranges::generate(
                            std::move(it), range.end(), [&]() {
                                return std::invoke(distribution, generator);
                            });
                    }
                }
                return range.end();
            }
        } else {
            return ranges::generate(std::forward<R>(range),
                [&]() { return std::invoke(distribution, generator); });
        }
    }

    template <typename G, typename D,
        std::output_iterator<std::invoke_result_t<D&, G&>> O,
        std::sentinel_for<O> S>
    requires std::invocable<D&, G&> &&
        std::uniform_random_bit_generator<std::remove_cvref_t<G>> &&
        std::is_arithmetic_v<std::invoke_result_t<D&, G&>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr O generate_random(
        O first, S last, G&& generator, D&& distribution) RXX_CONST_CALL {
        return operator()(ranges::subrange<O, S>(std::move(first), last),
            std::forward<G>(generator), std::forward<D>(distribution));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::generate_random_t generate_random{};
}
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
