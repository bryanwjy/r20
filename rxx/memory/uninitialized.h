// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/algorithm/return_types.h"
#include "rxx/borrow_traits.h"
#include "rxx/concepts.h"
#include "rxx/details/construct_at.h"
#include "rxx/details/destroy_at.h"
#include "rxx/functional/equal_to.h"
#include "rxx/iter_traits.h"
#include "rxx/primitives.h"

#include <concepts>
#include <new>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

template <typename I, typename O>
using uninitialized_copy_result = in_out_result<I, O>;

template <typename I, typename O>
using uninitialized_copy_n_result = in_out_result<I, O>;

template <typename I, typename O>
using uninitialized_move_result = in_out_result<I, O>;

template <typename I, typename O>
using uninitialized_move_n_result = in_out_result<I, O>;

namespace details {
struct construct_at_t {
    template <typename T, typename... Args>
    requires requires(void* ptr, Args&&... args) {
        ::new (ptr) T(std::forward<Args>(args)...);
    }
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr T* operator()(
        T* location, Args&&... args) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        return construct_at(location, std::forward<Args>(args)...);
    }
};
struct destroy_at_t {
    template <std::destructible T>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr void operator()(
        T* location) RXX_CONST_CALL noexcept {
        destroy_at(location);
    }
};

template <typename I>
concept nothrow_input_iterator =
    std::input_iterator<I> && std::is_lvalue_reference_v<iter_reference_t<I>> &&
    std::same_as<std::remove_cvref_t<iter_reference_t<I>>, iter_value_t<I>>;

template <typename S, typename I>
concept nothrow_sentinel_for = std::sentinel_for<S, I>;

template <typename I>
concept nothrow_forward_iterator = nothrow_input_iterator<I> &&
    std::forward_iterator<I> && nothrow_sentinel_for<I, I>;

template <typename R>
concept nothrow_input_range =
    range<R> && nothrow_input_iterator<iterator_t<R>> &&
    nothrow_sentinel_for<sentinel_t<R>, iterator_t<R>>;

template <typename R>
concept nothrow_forward_range =
    nothrow_input_range<R> && nothrow_forward_iterator<iterator_t<R>>;

struct destroy_t {
protected:
    template <typename I, typename S>
    __RXX_HIDE_FROM_ABI static constexpr I impl(I first, S last) noexcept {
        for (; first != last; ++first) {
            destroy_at(RXX_BUILTIN_addressof(*first));
        }

        return first;
    }

public:
    template <nothrow_input_iterator I, nothrow_sentinel_for<I> S>
    requires std::destructible<iter_value_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr I operator()(
        I first, S last) RXX_CONST_CALL noexcept {
        for (; first != last; ++first) {
            destroy_at(RXX_BUILTIN_addressof(*first));
        }

        return first;
    }

    template <nothrow_input_range R>
    requires std::destructible<range_value_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr borrowed_iterator_t<R>
    operator()(R&& range) RXX_CONST_CALL noexcept {
        return operator()(ranges::begin(range), ranges::end(range));
    }
};

struct destroy_n_t {
    template <nothrow_input_iterator I>
    requires std::destructible<iter_value_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr I operator()(
        I first, iter_difference_t<I> count) RXX_CONST_CALL noexcept {
        for (; count > 0; (void)++first, --count) {
            destroy_at(RXX_BUILTIN_addressof(*first));
        }

        return first;
    }
};

struct uninitialized_copy_t : private destroy_t {
private:
    template <typename V, typename I, typename S1, typename O, typename Pred>
    __RXX_HIDE_FROM_ABI static constexpr uninitialized_copy_result<I, O> impl(
        I ifirst, S1 ilast, O ofirst, Pred&& stop_copying) {
        O idx = ofirst;
        RXX_TRY {
            for (; ifirst != ilast && !stop_copying(idx); ++ifirst, ++idx) {
                ::new (RXX_BUILTIN_addressof(*idx)) V(*ifirst);
            }
        } RXX_CATCH(...) {
            destroy_t::impl(ofirst, idx);
            RXX_RETHROW();
        }

        return {std::move(ifirst), std::move(idx)};
    }

public:
    template <std::input_iterator I, std::sentinel_for<I> S1,
        nothrow_forward_iterator O, nothrow_sentinel_for<O> S2>
    requires std::constructible_from<iter_value_t<O>, iter_reference_t<I>>
    __RXX_HIDE_FROM_ABI
        RXX_STATIC_CALL constexpr uninitialized_copy_result<I, O>
        operator()(I ifirst, S1 ilast, O ofirst, S2 olast) RXX_CONST_CALL {
        using value_type = std::remove_reference_t<iter_reference_t<O>>;

        return impl<value_type>(std::move(ifirst), std::move(ilast),
            std::move(ofirst), ranges::equal_to{});
    }

    template <input_range I, nothrow_forward_range O>
    requires std::constructible_from<range_value_t<O>, range_reference_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr uninitialized_copy_result<
        borrowed_iterator_t<I>, borrowed_iterator_t<O>>
    operator()(I&& in_range, O&& out_range) RXX_CONST_CALL {
        return operator()(ranges::begin(in_range), ranges::end(in_range),
            ranges::begin(out_range), ranges::end(out_range));
    }
};

struct uninitialized_copy_n_t : private destroy_t {
private:
    template <typename V, typename I, typename O, typename Pred>
    __RXX_HIDE_FROM_ABI static constexpr uninitialized_copy_n_result<I, O> impl(
        I ifirst, iter_difference_t<I> count, O ofirst, Pred&& stop_copying) {
        O idx = ofirst;
        RXX_TRY {
            for (; count > 0 && !stop_copying(idx); ++ifirst, ++idx, --count)
                ::new (RXX_BUILTIN_addressof(*idx)) V(*ifirst);
        } RXX_CATCH(...) {
            destroy_t::impl(ofirst, idx);
            RXX_RETHROW();
        }

        return {std::move(ifirst), std::move(idx)};
    }

public:
    template <std::input_iterator I, nothrow_forward_iterator O,
        nothrow_sentinel_for<O> S>
    requires std::constructible_from<iter_value_t<O>, iter_reference_t<I>>
    __RXX_HIDE_FROM_ABI
        RXX_STATIC_CALL constexpr uninitialized_copy_n_result<I, O>
        operator()(I ifirst, iter_difference_t<I> count, O ofirst,
            S olast) RXX_CONST_CALL {
        using value_type = std::remove_reference_t<iter_reference_t<O>>;
        return impl<value_type>(
            std::move(ifirst), count, std::move(ofirst), ranges::equal_to{});
    }
};

struct uninitialized_move_t : private destroy_t {
private:
    template <typename V, typename I, typename S1, typename O, typename Pred,
        typename Move>
    __RXX_HIDE_FROM_ABI static constexpr uninitialized_move_result<I, O> impl(
        I ifirst, S1 ilast, O ofirst, Pred&& stop_moving, Move&& imove) {
        auto idx = ofirst;
        RXX_TRY {
            for (; ifirst != ilast && !stop_moving(idx); ++idx, ++ifirst) {
                ::new (RXX_BUILTIN_addressof(*idx)) V(imove(ifirst));
            }
        } RXX_CATCH(...) {
            destroy_t::impl(ofirst, idx);
            RXX_RETHROW();
        }

        return {std::move(ifirst), std::move(idx)};
    }

public:
    template <std::input_iterator I, std::sentinel_for<I> S1,
        nothrow_forward_iterator O, nothrow_sentinel_for<O> S2>
    requires std::constructible_from<iter_value_t<O>,
        iter_rvalue_reference_t<I>>
    __RXX_HIDE_FROM_ABI
        RXX_STATIC_CALL constexpr uninitialized_move_result<I, O>
        operator()(I ifirst, S1 ilast, O ofirst, S2 olast) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<O>>;
        return impl<V>(std::move(ifirst), std::move(ilast), std::move(ofirst),
            ranges::equal_to{}, ranges::iter_move);
    }

    template <input_range R, nothrow_forward_range O>
    requires std::constructible_from<range_value_t<O>,
        range_rvalue_reference_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr uninitialized_move_result<
        borrowed_iterator_t<R>, borrowed_iterator_t<O>>
    operator()(R&& in_range, O&& out_range) RXX_CONST_CALL {
        return operator()(ranges::begin(in_range), ranges::end(in_range),
            ranges::begin(out_range), ranges::end(out_range));
    }
};

struct uninitialized_move_n_t : private destroy_t {
private:
    template <typename V, typename I, typename O, typename Pred, typename Move>
    __RXX_HIDE_FROM_ABI static constexpr uninitialized_move_n_result<I, O> impl(
        I ifirst, iter_difference_t<I> count, O ofirst, Pred&& stop_moving,
        Move&& imove) {
        auto idx = ofirst;
        RXX_TRY {
            for (; count > 0 && !stop_moving(idx); ++ifirst, ++idx, --count) {
                ::new (RXX_BUILTIN_addressof(*idx)) V(imove(ifirst));
            }
        } RXX_CATCH(...) {
            destroy_t::impl(ofirst, idx);
            RXX_RETHROW();
        }

        return {std::move(ifirst), std::move(idx)};
    }

public:
    template <std::input_iterator I, nothrow_forward_iterator O,
        nothrow_sentinel_for<O> S>
    requires std::constructible_from<iter_value_t<O>, iter_reference_t<I>>
    __RXX_HIDE_FROM_ABI
        RXX_STATIC_CALL constexpr uninitialized_move_n_result<I, O>
        operator()(I ifirst, iter_difference_t<I> count, O ofirst,
            S olast) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<O>>;
        return impl<V>(std::move(ifirst), count, std::move(ofirst),
            ranges::equal_to{}, ranges::iter_move);
    }
};

struct uninitialized_default_construct_t : private destroy_t {
private:
    template <typename V, typename I, typename S>
    __RXX_HIDE_FROM_ABI static constexpr I impl(I first, S last) {
        auto idx = first;
        RXX_TRY {
            for (; idx != last; ++idx)
                ::new (RXX_BUILTIN_addressof(*idx)) V;
        } RXX_CATCH(...) {
            destroy_t::impl(first, idx);
            RXX_RETHROW();
        }

        return idx;
    }

public:
    template <nothrow_forward_iterator I, nothrow_sentinel_for<I> S>
    requires std::default_initializable<iter_value_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr I operator()(
        I first, S last) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<I>>;
        return impl<V>(std::move(first), std::move(last));
    }

    template <nothrow_forward_range R>
    requires std::default_initializable<range_value_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr borrowed_iterator_t<R>
    operator()(R&& range) RXX_CONST_CALL {
        return operator()(ranges::begin(range), ranges::end(range));
    }
};

struct uninitialized_default_construct_n_t : private destroy_t {
private:
    template <typename V, typename I>
    __RXX_HIDE_FROM_ABI static constexpr I impl(
        I first, iter_difference_t<I> count) {
        auto idx = first;
        RXX_TRY {
            for (; count > 0; ++idx, --count) {
                ::new (RXX_BUILTIN_addressof(*idx)) V;
            }
        } RXX_CATCH(...) {
            destroy_t::impl(first, idx);
            RXX_RETHROW();
        }

        return idx;
    }

public:
    template <nothrow_forward_iterator I>
    requires std::default_initializable<iter_value_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr I operator()(
        I first, iter_difference_t<I> count) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<I>>;
        return impl<V>(std::move(first), count);
    }
};

struct uninitialized_fill_t : private destroy_t {
private:
    template <typename V, typename I, typename S, typename T>
    __RXX_HIDE_FROM_ABI static constexpr I impl(I first, S last, T const& val) {
        I idx = first;
        RXX_TRY {
            for (; idx != last; ++idx) {
                ::new (RXX_BUILTIN_addressof(*idx)) V(val);
            }
        } RXX_CATCH(...) {
            destroy_t::impl(first, idx);
            RXX_RETHROW();
        }

        return idx;
    }

public:
    template <nothrow_forward_iterator I, nothrow_sentinel_for<I> S, typename T>
    requires std::constructible_from<iter_value_t<I>, T const&>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL I operator()(
        I first, S last, T const& val) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<I>>;
        return impl<V>(std::move(first), std::move(last), val);
    }

    template <nothrow_forward_range R, typename T>
    requires std::constructible_from<range_value_t<R>, T const&>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL borrowed_iterator_t<R> operator()(
        R&& range, T const& val) RXX_CONST_CALL {
        return operator()(ranges::begin(range), ranges::end(range), val);
    }
};
struct uninitialized_fill_n_t : private destroy_t {
private:
    template <typename V, typename I, typename T>
    __RXX_HIDE_FROM_ABI static constexpr I impl(
        I first, iter_difference_t<I> count, T const& val) {
        I idx = first;
        RXX_TRY {
            for (; count > 0; ++idx, --count) {
                ::new (RXX_BUILTIN_addressof(*idx)) V(val);
            }
        } RXX_CATCH(...) {
            destroy_t::impl(first, idx);
            RXX_RETHROW();
        }

        return idx;
    }

public:
    template <nothrow_forward_iterator I, typename T>
    requires std::constructible_from<iter_value_t<I>, T const&>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL I operator()(
        I first, iter_difference_t<I> count, T const& val) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<I>>;
        return impl<V>(std::move(first), count, val);
    }
};

struct uninitialized_value_construct_t : private destroy_t {
private:
    template <typename V, typename I, typename S>
    __RXX_HIDE_FROM_ABI static constexpr I impl(I first, S last) {
        auto idx = first;
        RXX_TRY {
            for (; idx != last; ++idx)
                ::new (RXX_BUILTIN_addressof(*idx)) V();
        } RXX_CATCH(...) {
            destroy_t::impl(first, idx);
            RXX_RETHROW();
        }

        return idx;
    }

public:
    template <nothrow_forward_iterator I, nothrow_sentinel_for<I> S>
    requires std::default_initializable<iter_value_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr I operator()(
        I first, S last) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<I>>;
        return impl<V>(std::move(first), std::move(last));
    }

    template <nothrow_forward_range R>
    requires std::default_initializable<range_value_t<R>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr borrowed_iterator_t<R>
    operator()(R&& range) RXX_CONST_CALL {
        return operator()(ranges::begin(range), ranges::end(range));
    }
};

struct uninitialized_value_construct_n_t : private destroy_t {
private:
    template <typename V, typename I>
    __RXX_HIDE_FROM_ABI static constexpr I impl(
        I first, iter_difference_t<I> count) {
        auto idx = first;
        RXX_TRY {
            for (; count > 0; ++idx, --count) {
                ::new (RXX_BUILTIN_addressof(*idx)) V();
            }
        } RXX_CATCH(...) {
            destroy_t::impl(first, idx);
            RXX_RETHROW();
        }

        return idx;
    }

public:
    template <nothrow_forward_iterator I>
    requires std::default_initializable<iter_value_t<I>>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr I operator()(
        I first, iter_difference_t<I> count) RXX_CONST_CALL {
        using V = std::remove_reference_t<iter_reference_t<I>>;
        return impl<V>(std::move(first), count);
    }
};

} // namespace details

inline namespace cpo {
inline constexpr details::construct_at_t construct_at{};
inline constexpr details::destroy_t destroy{};
inline constexpr details::destroy_at_t destroy_at{};
inline constexpr details::destroy_n_t destroy_n{};
inline constexpr details::uninitialized_copy_t uninitialized_copy{};
inline constexpr details::uninitialized_copy_n_t uninitialized_copy_n{};
inline constexpr details::uninitialized_default_construct_t
    uninitialized_default_construct{};
inline constexpr details::uninitialized_default_construct_n_t
    uninitialized_default_construct_n{};
inline constexpr details::uninitialized_fill_t uninitialized_fill{};
inline constexpr details::uninitialized_fill_n_t uninitialized_fill_n{};
inline constexpr details::uninitialized_move_t uninitialized_move{};
inline constexpr details::uninitialized_move_n_t uninitialized_move_n{};
inline constexpr details::uninitialized_value_construct_t
    uninitialized_value_construct{};
inline constexpr details::uninitialized_value_construct_n_t
    uninitialized_value_construct_n{};
} // namespace cpo
} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
