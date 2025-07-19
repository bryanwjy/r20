// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/borrow_traits.h"
#include "rxx/iterator.h"
#include "rxx/return_types.h"

#include <concepts>
#include <functional>
#include <optional>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

template <typename It, typename T>
using fold_left_with_iter_result = in_value_result<It, T>;
template <typename It, typename T>
using fold_left_first_with_iter_result = in_value_result<It, T>;

namespace details {

template <typename F, typename T, typename I, typename U>
concept indirectly_binary_left_foldable_impl =
    std::movable<T> && std::movable<U> && std::convertible_to<T, U> &&
    std::invocable<F&, U, iter_reference_t<I>> &&
    std::assignable_from<U&, std::invoke_result_t<F&, U, iter_reference_t<I>>>;

template <typename F, typename T, typename I>
concept indirectly_binary_left_foldable = std::copy_constructible<F> &&
    std::indirectly_readable<I> && std::invocable<F&, T, iter_reference_t<I>> &&
    std::convertible_to<std::invoke_result_t<F&, T, iter_reference_t<I>>,
        std::decay_t<std::invoke_result_t<F&, T, iter_reference_t<I>>>> &&
    indirectly_binary_left_foldable_impl<F, T, I,
        std::decay_t<std::invoke_result_t<F&, T, iter_reference_t<I>>>>;

class fold_left_with_iter_t {
protected:
    template <typename O, typename I, typename S, typename T, typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr auto impl(I&& first, S&& last, T&& init, F f) {
        using SecondType =
            std::decay_t<std::invoke_result_t<F&, T, iter_reference_t<I>>>;
        using Result = fold_left_with_iter_result<O, SecondType>;
        if (first == last) {
            return Result{std::move(first), SecondType(std::move(init))};
        }

        SecondType accum = std::invoke(f, std::move(init), *first);

        for (++first; first != last; ++first) {
            accum = std::invoke(f, std::move(accum), *first);
        }

        return Result{std::move(first), std::move(accum)};
    }

public:
    template <std::input_iterator I, std::sentinel_for<I> S,
        typename T = iter_value_t<I>, indirectly_binary_left_foldable<T, I> F>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        I first, S last, T init, F f) RXX_CONST_CALL {
        return impl<I>(
            std::move(first), std::move(last), std::move(init), std::move(f));
    }

    template <ranges::input_range R, typename T = ranges::range_value_t<R>,
        indirectly_binary_left_foldable<T, iterator_t<R>> F>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        R&& r, T init, F f) RXX_CONST_CALL {
        return impl<borrowed_iterator_t<R>>(
            ranges::begin(r), ranges::end(r), std::move(init), std::move(f));
    }
};

struct fold_left_t : private fold_left_with_iter_t {
    template <std::input_iterator I, std::sentinel_for<I> S,
        typename T = iter_value_t<I>, indirectly_binary_left_foldable<T, I> F>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL auto operator()(
        I first, S last, T init, F func) RXX_CONST_CALL {

        return fold_left_with_iter_t::impl(
            std::move(first), std::move(last), std::move(init), std::move(func))
            .value;
    }

    template <ranges::input_range R, typename T = ranges::range_value_t<R>,
        indirectly_binary_left_foldable<T, iterator_t<R>> F>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        R&& r, T init, F f) RXX_CONST_CALL {
        return (*this)(
            ranges::begin(r), ranges::end(r), std::move(init), std::move(f));
    }
};

struct fold_left_first_with_iter_t {
protected:
    template <typename O, typename I, typename S, typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr auto impl(I&& first, S&& last, F f) {
        using SecondType = decltype(ranges::fold_left(
            std::move(first), last, iter_value_t<I>(*first), f));
        using Result = ranges::fold_left_first_with_iter_result<O,
            std::optional<SecondType>>;
        if (first == last) {
            return Result{std::move(first), std::optional<SecondType>()};
        }

        std::optional<U> init(std::in_place, *first);
        for (++first; first != last; ++first) {
            *init = std::invoke(f, std::move(*init), *first);
        }

        return Result{std::move(first), std::move(init)};
    }

public:
    template <std::input_iterator I, std::sentinel_for<I> S,
        indirectly_binary_left_foldable<iter_value_t<I>, I> F>
    requires std::constructible_from<iter_value_t<I>, iter_reference_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        I first, S last, F f) RXX_CONST_CALL {
        return impl<I>(std::move(first), std::move(last), std::move(f));
    }

    template <input_range R,
        indirectly_binary_left_foldable<range_value_t<R>, iterator_t<R>> F>
    requires std::constructible_from<range_value_t<R>, range_reference_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        R&& r, F f) RXX_CONST_CALL {
        return impl<ranges::borrowed_iterator_t<R>>(
            ranges::begin(r), ranges::end(r), std::move(f));
    }
};

struct fold_left_first_t : private fold_left_first_with_iter_t {
    template <input_iterator It, sentinel_for<It> S,
        indirectly_binary_left_foldable<iter_value_t<It>, It> F>
    requires std::constructible_from<iter_value_t<It>, iter_reference_t<It>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        It first, S last, F func) RXX_CONST_CALL {
        return fold_left_first_with_iter_t::impl(
            std::move(first), std::move(last), std::move(func))
            .value;
    }

    template <input_range R,
        indirectly_binary_left_foldable<range_value_t<R>, iterator_t<R>> F>
    requires std::constructible_from<range_value_t<R>, range_reference_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        R&& range, F func) RXX_CONST_CALL {
        return fold_left_first_with_iter_t::impl(
            ranges::begin(range), ranges::end(range), std::move(func))
            .value;
    }
};

template <typename F>
class flipped {
public:
    template <typename T, typename U>
    requires std::invocable<F&, U, T>
    __RXX_HIDE_FROM_ABI std::invoke_result_t<F&, U, T> operator()(T&&, U&&);

private:
    F f;
};

template <typename F, typename T, typename I>
concept indirectly_binary_right_foldable =
    indirectly_binary_left_foldable<flipped<F>, T, I>;

struct fold_right_t {
protected:
    template <typename I, typename S, typename T, typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr impl(I first, S last, T init, F func) {
        using Result =
            std::decay_t<std::invoke_result_t<F&, iter_reference_t<I>, T>>;
        if (first == last) {
            return Result(std::move(init));
        }

        I tail = ranges::next(first, last);
        Result accum = std::invoke(func, *--tail, std::move(init));
        while (first != tail) {
            accum = std::invoke(func, *--tail, std::move(accum));
        }

        return accum;
    }

public:
    template <std::bidirectional_iterator I, std::sentinel_for<I> S,
        typename T = iter_value_t<I>, indirectly_binary_right_foldable<T, I> F>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        I first, S last, T init, F func) RXX_CONST_CALL {
        return impl(std::move(first), std::move(last), std::move(init),
            std::move(func));
    }

    template <ranges::bidirectional_range R, class T = ranges::range_value_t<R>,
        indirectly_binary_right_foldable<T, ranges::iterator_t<R>> F>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        R&& r, T init, F func) RXX_CONST_CALL {
        return impl(
            ranges::begin(r), ranges::end(r), std::move(init), std::move(func));
    }
};

struct fold_right_last_t : private fold_right_t {
    template <std::bidirectional_iterator I, std::sentinel_for<I> S,
        indirectly_binary_right_foldable<iter_value_t<I>, I> F>
    requires std::constructible_from<iter_value_t<I>, iter_reference_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        I first, S last, F func) RXX_CONST_CALL {
        using Result = decltype(fold_right_t::impl(
            first, last, iter_value_t<I>(*first), std::move(func)));

        if (first == last) {
            return std::optional<Result>();
        }

        I tail = ranges::prev(ranges::next(first, std::move(last)));
        return std::optional<Result>(std::in_place,
            fold_right_t::impl(std::move(first), tail, iter_value_t<I>(*tail),
                std::move(func)));
    }

    template <std::bidirectional_range R,
        indirectly_binary_right_foldable<range_value_t<R>, iterator_t<R>> F>
    requires std::constructible_from<range_value_t<R>, range_reference_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr auto operator()(
        R&& range, F func) RXX_CONST_CALL {
        auto first = ranges::begin(range);
        auto last = ranges::begin(range);

        using Result = decltype(fold_right_t::impl(
            first, last, iter_value_t<I>(*first), std::move(func)));

        if (first == last) {
            return std::optional<Result>();
        }

        I tail = ranges::prev(ranges::next(first, std::move(last)));

        return std::optional<Result>(std::in_place,
            fold_right_t::impl(std::move(first), tail, iter_value_t<I>(*tail),
                std::move(func)));
    }
};
} // namespace details

inline constexpr fold_left_t fold_left;
inline constexpr fold_left_first_t fold_left_first;
inline constexpr fold_right_t fold_right;
inline constexpr fold_right_last_t fold_right_last;
inline constexpr fold_left_with_iter_t fold_left_with_iter;
inline constexpr fold_left_first_with_iter_t fold_left_first_with_iter;

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
