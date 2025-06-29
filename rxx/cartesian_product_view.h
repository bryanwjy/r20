// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/packed_range_traits.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/details/tuple_functions.h"
#include "rxx/get_element.h"
#include "rxx/primitives.h"

#include <cassert>
#include <compare>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <typename R>
concept sized_random_access_range =
    std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

template <bool Const, typename First, typename... Vs>
concept cartesian_product_is_random_access =
    (std::ranges::random_access_range<details::const_if<Const, First>> && ... &&
        sized_random_access_range<details::const_if<Const, Vs>>);

template <typename R>
concept cartesian_product_common_arg =
    std::ranges::common_range<R> || sized_random_access_range<R>;

template <bool Const, typename First, typename... Vs>
concept cartesian_product_is_bidirectional =
    (std::ranges::bidirectional_range<details::const_if<Const, First>> && ... &&
        (std::ranges::bidirectional_range<details::const_if<Const, Vs>> &&
            cartesian_product_common_arg<details::const_if<Const, Vs>>));

template <typename First, typename... Vs>
concept cartesian_product_is_common = cartesian_product_common_arg<First>;

template <typename... Vs>
concept cartesian_product_is_sized = (... && std::ranges::sized_range<Vs>);

template <bool Const, template <typename> class FirstSent, typename First,
    typename... Vs>
concept cartesian_is_sized_sentinel =
    (std::sized_sentinel_for<FirstSent<details::const_if<Const, First>>,
         iterator_t<details::const_if<Const, First>>> &&
        ... &&
        (ranges::sized_range<details::const_if<Const, Vs>> &&
            std::sized_sentinel_for<iterator_t<details::const_if<Const, Vs>>,
                iterator_t<details::const_if<Const, Vs>>>));

template <cartesian_product_common_arg R>
__RXX_HIDE_FROM_ABI constexpr auto cartesian_product_common_arg_end(R& range) {
    if constexpr (std::ranges::common_range<R>) {
        return std::ranges::end(range);
    } else {
        return std::ranges::begin(range) + std::ranges::distance(range);
    }
}
} // namespace details

template <std::ranges::input_range First, std::ranges::forward_range... Vs>
requires (std::ranges::view<First> && ... && std::ranges::view<Vs>)
class cartesian_product_view :
    public ranges::view_interface<cartesian_product_view<First, Vs...>> {

    template <bool>
    class iterator;
    using difference_type = decltype([]() {
        return std::common_type_t<ptrdiff_t, range_difference_t<First>,
            range_difference_t<Vs>...>{};
    }());

public:
    __RXX_HIDE_FROM_ABI constexpr cartesian_product_view() noexcept(
        (std::is_nothrow_default_constructible_v<First> && ... &&
            std::is_nothrow_default_constructible_v<Vs>))
    requires (std::default_initializable<First> && ... &&
                 std::default_initializable<Vs>)
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr cartesian_product_view(First head,
        Vs... tail) noexcept((std::is_nothrow_move_constructible_v<First> &&
        ... && std::is_nothrow_move_constructible_v<Vs>))
        : bases_{std::move(head), std::move(tail)...} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (!details::simple_view<First> || ... || !details::simple_view<Vs>)
    {
        return iterator<false>(details::transform(std::ranges::begin, bases_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires (
        std::ranges::range<First const> && ... && std::ranges::range<Vs const>)
    {
        return iterator<true>(details::transform(std::ranges::begin, bases_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (
        (!details::simple_view<First> || ... || !details::simple_view<Vs>) &&
        details::cartesian_product_is_common<First, Vs...>)
    {
        auto const is_empty =
            std::apply([](bool head, bool... tail) { return (... || tail); },
                details::transform(std::ranges::empty, bases_));

        auto const begin_or_first_end = [&](auto& range) {
            return is_empty ? std::ranges::begin(range)
                            : details::cartesian_product_common_arg_end(range);
        };

        return iterator<false>(
            [&]<size_t... Is>(std::index_sequence<0, Is...>) {
                return std::tuple{std::ranges::begin(get_element<0>(bases_)),
                    begin_or_first_end(get_element<Is>(bases_))...};
            }(details::make_index_sequence_v<sizeof...(Vs) + 1>));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires details::cartesian_product_is_common<First, Vs...>
    {
        auto const is_empty =
            std::apply([](bool head, bool... tail) { return (... || tail); },
                details::transform(std::ranges::empty, bases_));

        auto const begin_or_first_end = [&](auto& range) {
            return is_empty ? std::ranges::begin(range)
                            : details::cartesian_product_common_arg_end(range);
        };

        return iterator<true>([&]<size_t... Is>(std::index_sequence<0, Is...>) {
            return std::tuple{std::ranges::begin(get_element<0>(bases_)),
                begin_or_first_end(get_element<Is>(bases_))...};
        }(details::make_index_sequence_v<sizeof...(Vs) + 1>));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept { return std::default_sentinel; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires details::cartesian_product_is_sized<First, Vs...>
    {
        using result_type = decltype(details::to_unsigned_like(
            std::declval<difference_type>()));
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return (static_cast<result_type>(1u) * ... *
                static_cast<result_type>(
                    std::ranges::size(get_element<Is>(bases_))));
        }(details::make_index_sequence_v<sizeof...(Vs) + 1>);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires details::cartesian_product_is_sized<First const, Vs const...>
    {
        using result_type = decltype(details::to_unsigned_like(
            std::declval<difference_type>()));
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return (static_cast<result_type>(1u) * ... *
                static_cast<result_type>(
                    std::ranges::size(get_element<Is>(bases_))));
        }(details::make_index_sequence_v<sizeof...(Vs) + 1>);
    }

private:
    std::tuple<First, Vs...> bases_;
};

template <typename... Vs>
cartesian_product_view(Vs&&...)
    -> cartesian_product_view<std::views::all_t<Vs>...>;

template <std::ranges::input_range First, std::ranges::forward_range... Vs>
requires (std::ranges::view<First> && ... && std::ranges::view<Vs>)
template <bool Const>
class cartesian_product_view<First, Vs...>::iterator {
    friend cartesian_product_view;
    using Parent = details::const_if<Const, cartesian_product_view>;

public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype([]() {
        if constexpr (details::cartesian_product_is_random_access<Const, First,
                          Vs...>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (details::cartesian_product_is_bidirectional<Const,
                                 First, Vs...>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::ranges::forward_range<
                                 details::const_if<Const, First>>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());
    using value_type =
        std::tuple<range_value_t<details::const_if<Const, First>>,
            range_value_t<details::const_if<Const, Vs>>...>;
    using reference =
        std::tuple<range_reference_t<details::const_if<Const, First>>,
            range_reference_t<details::const_if<Const, Vs>>...>;
    using difference_type = typename cartesian_product_view::difference_type;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        (std::is_nothrow_default_constructible_v<iterator_t<Const, First>> &&
            ... &&
            std::is_nothrow_default_constructible_v<iterator_t<Const, Vs>>)) =
        default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const &&
                 (std::convertible_to<iterator_t<First>,
                      iterator_t<First const>> &&
                     ... &&
                     std::convertible_to<iterator_t<Vs>, iterator_t<Vs const>>)
        : parent_{other.parent_}
        , current_{std::move(other.current_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator*() const {
        return details::transform(
            [](auto const& iter) { return *iter; }, current_);
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        this->next();
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires std::ranges::forward_range<details::const_if<Const, First>>
    {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires details::cartesian_product_is_bidirectional<Const, First, Vs...>
    {
        this->prev();
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires details::cartesian_product_is_bidirectional<Const, First, Vs...>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires details::cartesian_product_is_random_access<Const, First, Vs...>
    {
        this->advance(offset);
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires details::cartesian_product_is_random_access<Const, First, Vs...>
    {
        return *this += -offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type offset) const
    requires details::cartesian_product_is_bidirectional<Const, First, Vs...>
    {
        return *((*this) + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& iter, difference_type offset)
    requires details::cartesian_product_is_random_access<Const, First, Vs...>
    {
        return iter += offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type offset, iterator const& iter)
    requires details::cartesian_product_is_random_access<Const, First, Vs...>
    {
        return iter += offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& iter, difference_type offset)
    requires details::cartesian_product_is_random_access<Const, First, Vs...>
    {
        return iter -= offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires details::cartesian_is_sized_sentinel<Const, iterator_t, First,
        Vs...>
    {
        return left.distance_from(right.current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& iter, std::default_sentinel_t)
    requires details::cartesian_is_sized_sentinel<Const, sentinel_t, First,
        Vs...>
    {
        auto output = [&]<size_t... Is>(index_sequence<Is...>) {
            return std::tuple{
                std::ranges::end(get_element<0>(iter.parent_->bases_)),
                std::ranges::begin(
                    get_element<1 + Is>(iter.parent_->bases_))...};
        }(details::make_index_sequence_v<sizeof...(Vs)>);

        return iter.distance_from(output);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        std::default_sentinel_t, iterator const& iter)
    requires details::cartesian_is_sized_sentinel<Const, sentinel_t, First,
        Vs...>
    {
        return -(iter - std::default_sentinel);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires std::equality_comparable<
        iterator_t<details::const_if<Const, First>>>
    {
        return left.current_ == right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& iter, std::default_sentinel_t) {
        return [&]<size_t... Is>(index_sequence<Is...>) {
            return (... ||
                (get_element<Is>(iter.current_) ==
                    std::ranges::end(get_element<Is>(iter.parent_->bases_))));
        }(make_index_sequence<1 + sizeof...(Vs)>{});
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires details::all_random_access<Const, First, Vs...>
    {
        return left.current_ <=> right.current_;
    }

private:
    __RXX_HIDE_FROM_ABI constexpr iterator(Parent& parent,
        std::tuple<iterator_t<details::const_if<Const, First>>,
            iterator_t<details::const_if<Const, Vs>>...>
            current) noexcept((std::
                                   is_nothrow_move_constructible_v<
                                       iterator_t<Const, First>> &&
        ... && std::is_nothrow_move_constructible_v<iterator_t<Const, Vs>>))
        : parent_{RXX_BUILTIN_addressof(parent)}
        , current_{std::move(current)} {}

    template <size_t I = sizeof...(Vs)>
    __RXX_HIDE_FROM_ABI constexpr void next() {
        auto& iter = get_element<I>(current_);
        ++iter;
        if constexpr (I > 0) {
            if (iter == std::ranges::end(get_element<I>(parent_->bases_))) {
                iter = std::ranges::begin(get_element<I>(parent_->bases_));
                this->template next<I - 1>();
            }
        }
    }

    template <size_t I = sizeof...(Vs)>
    __RXX_HIDE_FROM_ABI constexpr void prev() {
        auto& iter = get_element<I>(current_);
        if constexpr (I > 0) {
            if (iter == std::ranges::begin(get_element<I>(parent_->bases_))) {
                iter = details::cartesian_common_arg_end(
                    get_element<I>(parent_->bases_));
                this->template prev<I - 1>();
            }
        }
        --iter;
    }

    template <size_t I = sizeof...(Vs)>
    __RXX_HIDE_FROM_ABI constexpr void advance(difference_type num)
    requires details::cartesian_product_is_random_access<Const, First, Vs...>
    {
        if (num == 1) {
            this->template next<I>();
        } else if (num == -1) {
            this->template prev<I>();
        } else if (num != 0) {
            auto& base = get_element<I>(parent_->bases_);
            auto& iter = get_element<I>(current_);
            if constexpr (I == 0) {
                iter += num;
            } else {
                auto const ssize = std::ranges::ssize(base);
                auto const start = std::ranges::begin(base);
                auto offset = iter - start;
                offset += num;
                num = offset / ssize;
                offset %= ssize;
                if (offset < 0) {
                    offset = ssize + offset;
                    --num;
                }
                iter = start + offset;
                this->template advance<I - 1>(num);
            }
        }
    }

    template <typename Tuple>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr difference_type distance_from(Tuple const& tup) const {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return static_cast<difference_type>(
                this->template scaled_distance<Is>(tup) + ...);
        }(details::make_index_sequence_v<1 + sizeof...(Vs)>);
    }

    template <size_t I, typename Tuple>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr difference_type scaled_distance(Tuple const& tup) const {
        return static_cast<difference_type>(
                   get_element<I>(current_) - get_element<I>(tup)) *
            this->template scaled_size<I + 1>();
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr difference_type scaled_size() const {
        if constexpr (I <= sizeof...(Vs)) {
            return static_cast<difference_type>(
                       std::ranges::size(get_element<I>(parent_->bases_))) *
                this->template scaled_size<I + 1>();
        } else
            return static_cast<difference_type>(1);
    }

    Parent* parent_ = nullptr;
    std::tuple<iterator_t<details::const_if<Const, First>>,
        iterator_t<details::const_if<Const, Vs>>...>
        current_;
};
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
