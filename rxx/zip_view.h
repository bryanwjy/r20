// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/details/const_if.h"
#include "rxx/details/packed_range_traits.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/tuple_functions.h"
#include "rxx/get_element.h"
#include "rxx/primitives.h"

#include <compare>
#include <functional>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <typename... Rs>
concept zip_common =
    (sizeof...(Rs) == 1 && (... && std::ranges::common_range<Rs>)) ||
    (!(... && std::ranges::bidirectional_range<Rs>)&&(
        ... && std::ranges::common_range<Rs>)) ||
    ((... && std::ranges::random_access_range<Rs>)&&(
        ... && std::ranges::sized_range<Rs>));

template <typename Tuple1, typename Tuple2>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr bool any_equals(Tuple1 const& t1, Tuple2 const& t2) {
    auto const result = ranges::details::transform(std::equal_to<>{}, t1, t2);
    return std::apply([](auto... value) { return (value || ...); }, result);
}

template <typename T>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr T abs(T val) noexcept {
    return val < 0 ? -val : val;
}
} // namespace details

template <std::ranges::input_range... Rs>
requires (... && std::ranges::view<Rs>) && (sizeof...(Rs) > 0)
class zip_view : public std::ranges::view_interface<zip_view<Rs...>> {

    template <bool IsConst>
    class iterator;
    template <bool IsConst>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr zip_view() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr explicit zip_view(Rs... views) noexcept(
        (... && std::is_nothrow_move_constructible_v<Rs>))
        : views_{std::move(views)...} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (!(... && details::simple_view<Rs>))
    {
        return iterator<false>{
            details::transform(__RXX ranges::begin, views_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires (... && std::ranges::range<Rs const>)
    {
        return iterator<true>(
            details::transform(__RXX ranges::begin, views_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!(... && details::simple_view<Rs>))
    {
        if constexpr (!details::zip_common<Rs...>) {
            return sentinel<false>(
                details::transform(__RXX ranges::end, views_));
        } else if constexpr ((... && std::ranges::random_access_range<Rs>)) {
            return begin() + iter_difference_t<iterator<false>>(size());
        } else {
            return iterator<false>(
                details::transform(__RXX ranges::end, views_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires (... && std::ranges::range<Rs const>)
    {
        if constexpr (!details::zip_common<Rs const...>) {
            return sentinel<true>(
                details::transform(__RXX ranges::end, views_));
        } else if constexpr ((... &&
                                 std::ranges::random_access_range<Rs const>)) {
            return begin() + std::iter_difference_t<iterator<true>>(size());
        } else {
            return iterator<true>(
                details::transform(__RXX ranges::end, views_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires (std::ranges::sized_range<Rs> && ...)
    {
        return std::apply(
            [](auto... sizes) {
                using common = std::make_unsigned_t<
                    std::common_type_t<decltype(sizes)...>>;
                return std::ranges::min({common(sizes)...});
            },
            details::transform(std::ranges::size, views_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires (std::ranges::sized_range<Rs const> && ...)
    {
        return std::apply(
            [](auto... sizes) {
                using common = std::make_unsigned_t<
                    std::common_type_t<decltype(sizes)...>>;
                return std::ranges::min({common(sizes)...});
            },
            details::transform(std::ranges::size, views_));
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) std::tuple<Rs...> views_;
};

template <typename... Rs>
zip_view(Rs&&...) -> zip_view<std::views::all_t<Rs>...>;

namespace details {

template <bool Const, typename... Rs>
struct zip_view_iterator_category {};

template <bool Const, typename... Rs>
requires all_forward<Const, Rs...>
struct zip_view_iterator_category<Const, Rs...> {
    // Conditionally present only if all_forward is true
    using iterator_category = std::input_iterator_tag;
};
} // namespace details

template <std::ranges::input_range... Rs>
requires (... && std::ranges::view<Rs>) && (sizeof...(Rs) > 0)
template <bool Const>
class zip_view<Rs...>::iterator :
    public details::zip_view_iterator_category<Const, Rs...> {
    using current_type =
        std::tuple<iterator_t<details::const_if<Const, Rs>>...>;

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(
        current_type current) noexcept(std::
            is_nothrow_move_constructible_v<current_type>)
        : current_(std::move(current)) {}

    template <bool>
    friend class zip_view<Rs...>::iterator;

    template <bool>
    friend class zip_view<Rs...>::sentinel;

    friend class zip_view<Rs...>;

    __RXX_HIDE_FROM_ABI friend constexpr decltype(auto) get_current(
        iterator const& iter) noexcept {
        return (iter.current_);
    }

public:
    using iterator_concept = decltype([]() {
        if constexpr (details::all_random_access<Const, Rs...>)
            return std::random_access_iterator_tag{};
        else if constexpr (details::all_bidirectional<Const, Rs...>)
            return std::bidirectional_iterator_tag{};
        else if constexpr (details::all_forward<Const, Rs...>)
            return std::forward_iterator_tag{};
        else
            return std::input_iterator_tag{};
    }());
    using value_type =
        std::tuple<range_value_t<details::const_if<Const, Rs>>...>;
    using difference_type =
        std::common_type_t<range_difference_t<details::const_if<Const, Rs>>...>;

    __RXX_HIDE_FROM_ABI constexpr iterator() = default;
    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const &&
        (... &&
            std::convertible_to<iterator_t<Rs>,
                iterator_t<details::const_if<Const, Rs>>>)
        : current_{std::move(other.current_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator*() const {
        return details::transform(
            [](auto& iter) -> decltype(auto) { return *iter; }, current_);
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        details::for_each(
            [](auto& iter) -> decltype(auto) { return ++iter; }, current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires details::all_forward<Const, Rs...>
    {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires details::all_bidirectional<Const, Rs...>
    {
        details::for_each(
            [](auto& iter) -> decltype(auto) { return --iter; }, current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires details::all_bidirectional<Const, Rs...>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires details::all_random_access<Const, Rs...>
    {
        details::for_each(
            [&]<typename Iter>(
                Iter& iter) { iter += std::iter_difference_t<Iter>(offset); },
            current_);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires details::all_random_access<Const, Rs...>
    {
        details::for_each(
            [&]<typename Iter>(
                Iter& iter) { iter -= std::iter_difference_t<Iter>(offset); },
            current_);
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type idx) const
    requires details::all_random_access<Const, Rs...>
    {
        return details::transform(
            [&]<typename Iter>(Iter& iter) -> decltype(auto) {
                return iter[std::iter_difference_t<Iter>(idx)];
            },
            current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(iterator const& lhs, iterator const& rhs)
    requires (... &&
        std::equality_comparable<iterator_t<details::const_if<Const, Rs>>>)
    {
        if constexpr (details::all_bidirectional<Const, Rs...>) {
            return lhs.current_ == rhs.current_;
        } else {
            return details::any_equals(lhs.current_, rhs.current_);
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(iterator const& lhs, iterator const& rhs)
    requires details::all_random_access<Const, Rs...>
    {
        return lhs.current_ <=> rhs.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& self, difference_type offset)
    requires details::all_random_access<Const, Rs...>
    {
        auto copy = self;
        copy += offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type offset, iterator const& self)
    requires details::all_random_access<Const, Rs...>
    {
        return self + offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& self, difference_type offset)
    requires details::all_random_access<Const, Rs...>
    {
        auto copy = self;
        copy -= offset;
        return copy;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires (... &&
        std::sized_sentinel_for<iterator_t<details::const_if<Const, Rs>>,
            iterator_t<details::const_if<Const, Rs>>>)
    {
        auto const diff =
            details::transform(std::minus<>(), left.current_, right.current_);
        return std::apply(
            [](auto... val) {
                return std::ranges::min(
                    {difference_type(val)...}, [](auto lhs, auto rhs) {
                        return details::abs(lhs) < details::abs(rhs);
                    });
            },
            diff);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto iter_move(iterator const& self) noexcept(
        (...&& noexcept(std::ranges::iter_move(std::declval<
            iterator_t<details::const_if<Const, Rs>> const&>()))) &&
        (std::is_nothrow_move_constructible_v<
             range_rvalue_reference_t<details::const_if<Const, Rs>>> &&
            ...)) {
        return details::transform(std::ranges::iter_move, self.current_);
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(iterator const& left, iterator const& right) noexcept((
        noexcept(std::ranges::iter_swap(
            std::declval<iterator_t<details::const_if<Const, Rs>> const&>(),
            std::declval<iterator_t<details::const_if<Const, Rs>> const&>())) &&
        ...))
    requires (... &&
        std::indirectly_swappable<iterator_t<details::const_if<Const, Rs>>>)
    {
        details::for_each(
            std::ranges::iter_swap, left.current_, right.current_);
    }

private:
    current_type current_;
};

template <std::ranges::input_range... Rs>
requires (... && std::ranges::view<Rs>) && (sizeof...(Rs) > 0)
template <bool Const>
class zip_view<Rs...>::sentinel {
    using end_type = std::tuple<sentinel_t<details::const_if<Const, Rs>>...>;

    friend class zip_view;

    __RXX_HIDE_FROM_ABI constexpr explicit sentinel(end_type end) noexcept(
        std::is_nothrow_move_constructible_v<end_type>)
        : end_(std::move(end)) {}

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other)
    requires Const &&
        (... &&
            std::convertible_to<sentinel_t<Rs>,
                sentinel_t<details::const_if<Const, Rs>>>)
        : end_{std::move(other.end_)} {}

    template <bool OtherConst>
    requires (... &&
        std::sentinel_for<sentinel_t<details::const_if<Const, Rs>>,
            iterator_t<details::const_if<OtherConst, Rs>>>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool operator==(
        iterator<OtherConst> const& iter, sentinel const& self) {
        return details::any_equals(get_current(iter), self.end_);
    }

    template <bool OtherConst>
    requires (... &&
        std::sized_sentinel_for<sentinel_t<details::const_if<Const, Rs>>,
            iterator_t<details::const_if<OtherConst, Rs>>>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr std::
        common_type_t<range_difference_t<details::const_if<OtherConst, Rs>>...>
        operator-(iterator<OtherConst> const& iter, sentinel const& self) {
        auto const diff =
            details::transform(std::minus<>{}, get_current(iter), self.end_);
        return std::apply([](auto... val) {
            using diff_type = std::common_type_t<
                range_difference_t<details::const_if<OtherConst, Rs>>...>;
            return std::ranges::min(
                {diff_type(val)...}, [](auto lhs, auto rhs) {
                    return details::abs(lhs) < details::abs(rhs);
                });
        });
    }

    template <bool OtherConst>
    requires (... &&
        std::sized_sentinel_for<sentinel_t<details::const_if<Const, Rs>>,
            iterator_t<details::const_if<OtherConst, Rs>>>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr std::
        common_type_t<range_difference_t<details::const_if<OtherConst, Rs>>...>
        operator-(sentinel const& self, iterator<OtherConst> const& iter) {
        return -(iter - self);
    }

private:
    end_type end_;
};

namespace views {
namespace details {
struct zip_t {
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator()() const noexcept {
        return std::ranges::empty_view<std::tuple<>>{};
    }

    template <typename... Rs>
    requires requires {
        zip_view<std::views::all_t<Rs>...>(std::declval<Rs>()...);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto
    operator()(Rs&&... args) const noexcept(
        noexcept(zip_view<std::views::all_t<Rs>...>(std::declval<Rs>()...)))
        -> decltype(zip_view<std::views::all_t<Rs>...>(std::declval<Rs>()...)) {
        return zip_view<std::views::all_t<Rs>...>(std::forward<Rs>(args)...);
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::zip_t zip{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename... Rs>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::zip_view<Rs...>> =
        (... && std::ranges::enable_borrowed_range<Rs>);
