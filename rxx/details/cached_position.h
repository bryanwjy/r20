// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/non_propagating_cache.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"

#include <cassert>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges::details {

template <range R>
class cached_position {
public:
    __RXX_HIDE_FROM_ABI explicit constexpr operator bool() const noexcept {
        return false;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator_t<R> get(R const&) const noexcept {
        assert(false);
        RXX_BUILTIN_unreachable();
    }

    __RXX_HIDE_FROM_ABI constexpr void set(
        R const&, iterator_t<R> const&) const noexcept {}
};

template <forward_range R>
class cached_position<R> : protected non_propagating_cache<iterator_t<R>> {
public:
    using non_propagating_cache<iterator_t<R>>::operator bool;

    __RXX_HIDE_FROM_ABI constexpr iterator_t<R> get(R const&) const
        noexcept(std::is_nothrow_copy_constructible_v<iterator_t<R>>) {
        assert(*this);
        return **this;
    }

    __RXX_HIDE_FROM_ABI constexpr void
    set(R const&, iterator_t<R> const& it) noexcept(
        std::is_nothrow_copy_constructible_v<iterator_t<R>>) {
        assert(!*this);
        this->emplace(it);
    }
};

#if RXX_COMPILER_GCC
RXX_DISABLE_WARNING_PUSH()
// For some strange reason, GCC complains about the visibility attribute in the
// specialization below
RXX_DISABLE_WARNING("-Wattributes")
#endif

template <random_access_range R>
requires (sizeof(range_difference_t<R>) <= sizeof(iterator_t<R>))
class cached_position<R> {
public:
    __RXX_HIDE_FROM_ABI constexpr cached_position() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr cached_position(
        cached_position const&) noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr cached_position(
        cached_position&& other) noexcept
        : offset_{other.offset_} {
        other.offset_ = -1;
    }

    __RXX_HIDE_FROM_ABI explicit constexpr operator bool() const noexcept {
        return offset_ >= 0;
    }

    __RXX_HIDE_FROM_ABI constexpr cached_position& operator=(
        cached_position const&) noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr cached_position& operator=(
        cached_position&& other) noexcept {
        // Propagate the cached offset, but invalidate the source.
        offset_ = other.offset_;
        other.offset_ = -1;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<R> get(R& src) const
        noexcept(noexcept(std::declval<iterator_t<R>>() +
            std::declval<range_difference_t<R> const&>())) {
        assert(*this);
        return __RXX ranges::begin(src) + offset_;
    }

    __RXX_HIDE_FROM_ABI constexpr void
    set(R& src, iterator_t<R> const& it) noexcept(noexcept(
        std::declval<iterator_t<R>>() - std::declval<iterator_t<R>>())) {
        assert(!*this);
        offset_ = it - __RXX ranges::begin(src);
    }

private:
    range_difference_t<R> offset_ = -1;
};
#if RXX_COMPILER_GCC
RXX_DISABLE_WARNING_POP()
#endif
} // namespace ranges::details

RXX_DEFAULT_NAMESPACE_END
