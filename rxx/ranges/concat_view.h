// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/configuration/builtins.h"
#include "rxx/details/concat.h"
#include "rxx/details/packed_range_traits.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/details/tuple_functions.h"
#include "rxx/details/variant_base.h"
#include "rxx/iterator.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/get_element.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/tuple.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <input_range... Vs>
requires (... && view<Vs>) && (sizeof...(Vs) > 0) && details::concatable<Vs...>
class concat_view : public view_interface<concat_view<Vs...>> {

    template <bool>
    class iterator;

    template <size_t I>
    using View = std::tuple_element_t<I, tuple<Vs...>>;

public:
    __RXX_HIDE_FROM_ABI constexpr concat_view() noexcept(
        (... && std::is_nothrow_default_constructible_v<Vs>))
    requires (... && std::default_initializable<Vs>)
    = default;

    __RXX_HIDE_FROM_ABI constexpr concat_view(Vs... views) noexcept(
        (... && std::is_nothrow_move_constructible_v<Vs>))
        : views_{std::move(views)...} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator<false> begin() noexcept(
        std::is_nothrow_constructible_v<iterator<false>, concat_view&,
            std::in_place_index_t<0>,
            iterator_t<View<0>>>&&                           //
        noexcept(ranges::begin(std::declval<View<0>&>())) && //
        noexcept(std::declval<iterator<false>&>().template satisfy<0>()))
    requires (!(... && details::simple_view<Vs>))
    {
        auto it = iterator<false>{*this, std::in_place_index<0>,
            ranges::begin(get_element<0>(views_))};
        it.template satisfy<0>();
        return it;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator<true> begin() const noexcept(
        std::is_nothrow_constructible_v<iterator<true>, concat_view const&,
            std::in_place_index_t<0>,
            iterator_t<View<0>>>&&                              //
        noexcept(ranges::begin(std::declval<View<0> const&>())) //
            && noexcept(std::declval<iterator<true>&>().template satisfy<0>()))
    requires (... && range<Vs const>) && details::concatable<Vs const...>
    {
        auto it = iterator<true>{*this, std::in_place_index<0>,
            ranges::begin(get_element<0>(views_))};
        it.template satisfy<0>();
        return it;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() noexcept(
        std::is_nothrow_constructible_v<iterator<false>, concat_view&,
            std::in_place_index_t<sizeof...(Vs) - 1>,
            sentinel_t<View<sizeof...(Vs) - 1>>>&&                           //
        noexcept(ranges::begin(std::declval<View<sizeof...(Vs) - 1>&>())) && //
        noexcept(std::declval<iterator<false>&>()
                     .template satisfy<sizeof...(Vs) - 1>()))
    requires (!(... && details::simple_view<Vs>))
    {
        constexpr auto N = sizeof...(Vs);
        using LastView = View<N - 1>;
        if constexpr (details::all_forward<false, Vs...> &&
            common_range<LastView>) {
            return iterator<false>{*this, std::in_place_index<N - 1>,
                ranges::end(get_element<N - 1>(views_))};
        } else {
            return std::default_sentinel;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept(
        std::is_nothrow_constructible_v<iterator<false>, concat_view const&,
            std::in_place_index_t<sizeof...(Vs) - 1>,
            sentinel_t<View<sizeof...(Vs) - 1>>>&& //
        noexcept(
            ranges::begin(std::declval<View<sizeof...(Vs) - 1> const&>())) && //
        noexcept(std::declval<iterator<false>&>()
                     .template satisfy<sizeof...(Vs) - 1>()))
    requires (... && range<Vs const>) && details::concatable<Vs const...>
    {
        constexpr auto N = sizeof...(Vs);
        using LastView = View<N - 1>;
        if constexpr (details::all_forward<true, Vs...> &&
            common_range<LastView const>) {
            return iterator<true>{*this, std::in_place_index<N - 1>,
                ranges::end(get_element<N - 1>(views_))};
        } else {
            return std::default_sentinel;
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() noexcept(
        (...&& noexcept(ranges::size(std::declval<Vs&>()))) && //
        (... &&
            std::is_nothrow_convertible_v<range_size_t<Vs>,
                std::common_type_t<range_size_t<Vs>...>>))
    requires (... && sized_range<Vs>)
    {
        return __RXX apply(
            [](auto... sizes) {
                using Type = std::common_type_t<decltype(sizes)...>;
                return (... + details::to_unsigned_like<Type>(sizes));
            },
            details::transform(ranges::size, views_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
        noexcept((...&& noexcept(ranges::size(std::declval<Vs const&>()))) && //
            (... &&
                std::is_nothrow_convertible_v<range_size_t<Vs const>,
                    std::common_type_t<range_size_t<Vs const>...>>))
    requires (... && sized_range<Vs const>)
    {
        return __RXX apply(
            [](auto... sizes) {
                using Type = std::common_type_t<decltype(sizes)...>;
                return (... + details::to_unsigned_like<Type>(sizes));
            },
            details::transform(ranges::size, views_));
    }

private:
    tuple<Vs...> views_;
};

template <typename... Rs>
concat_view(Rs&&...) -> concat_view<views::all_t<Rs>...>;

namespace details {

template <bool Const, typename R, typename... Rs>
consteval bool all_but_last_common() noexcept {
    if constexpr (sizeof...(Rs) == 0) {
        return true;
    } else {
        return requires {
            requires (common_range<const_if<Const, R>> &&
                all_but_last_common<Const, Rs...>());
        };
    };
}

template <bool Const, typename... Rs>
concept concat_is_random_access = details::all_random_access<Const, Rs...> &&
    all_but_last_common<Const, Rs...>();

template <bool Const, typename... Rs>
concept concat_is_bidirectional = details::all_bidirectional<Const, Rs...> &&
    all_but_last_common<Const, Rs...>();

template <bool Const, typename... Vs>
struct concat_view_iterator_category {};

template <bool Const, typename... Vs>
requires all_forward<Const, Vs...>
struct concat_view_iterator_category<Const, Vs...> {
private:
    static consteval auto make_iterator_category() noexcept {
        if constexpr (!std::is_reference_v<concat_reference_t<
                          details::const_if<Const, Vs>...>>) {
            return std::input_iterator_tag{};
        } else if constexpr (
            (... &&
                std::derived_from<
                    typename std::iterator_traits<iterator_t<
                        details::const_if<Const, Vs>>>::iterator_category,
                    std::
                        random_access_iterator_tag>)&&concat_is_random_access<Const,
                Vs...>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (
            (... &&
                std::derived_from<
                    typename std::iterator_traits<iterator_t<
                        details::const_if<Const, Vs>>>::iterator_category,
                    std::
                        bidirectional_iterator_tag>)&&concat_is_bidirectional<Const,
                Vs...>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr ((... &&
                                 std::derived_from<
                                     typename std::iterator_traits<
                                         iterator_t<details::const_if<Const,
                                             Vs>>>::iterator_category,
                                     std::forward_iterator_tag>)) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

public:
    using iterator_category = decltype(make_iterator_category());
};

template <bool Const, typename V, typename... Vs>
consteval bool all_but_first_sized_range() noexcept {
    return requires {
        requires (... && sized_range<details::const_if<Const, Vs>>);
    };
}

template <bool Const, typename... Vs>
consteval bool all_nothrow_iter_swappable() noexcept {
    return (...&& noexcept(ranges::iter_swap(
        std::declval<iterator_t<details::const_if<Const, Vs>> const&>(),
        std::declval<iterator_t<details::const_if<Const, Vs>> const&>())));
}

} // namespace details

template <input_range... Vs>
requires (... && view<Vs>) && (sizeof...(Vs) > 0) && details::concatable<Vs...>
template <bool Const>
class concat_view<Vs...>::iterator :
    public details::concat_view_iterator_category<Const, Vs...> {
private:
    using base_iter =
        details::variant_base<iterator_t<details::const_if<Const, Vs>>...>;
    friend concat_view;

    template <typename... Args>
    __RXX_HIDE_FROM_ABI explicit constexpr iterator(
        details::const_if<Const, concat_view>& parent,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<base_iter,
        Args...>)
    requires std::constructible_from<base_iter, Args...>
        : parent_{RXX_BUILTIN_addressof(parent)}
        , it_{std::forward<Args>(args)...} {}

    static consteval auto make_iterator_concept() noexcept {
        if constexpr (details::concat_is_random_access<Const, Vs...>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (details::concat_is_bidirectional<Const, Vs...>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (details::all_forward<Const, Vs...>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

public:
    using iterator_concept = decltype(make_iterator_concept());
    using value_type = details::concat_value_t<details::const_if<Const, Vs>...>;
    using difference_type =
        std::common_type_t<range_difference_t<details::const_if<Const, Vs>>...>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<base_iter>) = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires (Const && ... &&
                 std::convertible_to<iterator_t<Vs>, iterator_t<Vs const>>)
        : parent_{other.parent_}
        , it_{details::jump_table_for<base_iter>(
              [&]<size_t I>(details::size_constant<I>) {
                  return base_iter{std::in_place_index<I>,
                      std::move(other.it_).template value_ref<I>()};
              },
              other.it_.index())} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const noexcept((...&& noexcept(
        *std::declval<iterator_t<details::const_if<Const, Vs>> const&>())))
        -> decltype(auto) {
        using reference =
            details::concat_reference_t<details::const_if<Const, Vs>...>;
        return details::jump_table_for<base_iter>(
            [&]<size_t I>(details::size_constant<I>) -> reference {
                return *it_.template value_ref<I>();
            },
            it_.index());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type pos) const
        noexcept(noexcept(*((*this) + pos))) -> decltype(auto)
    requires details::concat_is_random_access<Const, Vs...>
    {
        return *((*this) + pos);
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        details::jump_table_for<base_iter>(
            [&]<size_t I>(details::size_constant<I>) {
                ++get_iter<I>();
                satisfy<I>();
            },
            it_.index());

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires details::all_forward<Const, Vs...>
    {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires details::concat_is_bidirectional<Const, Vs...>
    {
        details::jump_table_for<base_iter>(
            [&]<size_t I>(details::size_constant<I>) { prev<I>(); },
            it_.index());
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires details::concat_is_bidirectional<Const, Vs...>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type val)
    requires details::concat_is_random_access<Const, Vs...>
    {
        details::jump_table_for<base_iter>(
            [&]<size_t I>(details::size_constant<I>) {
                auto offset = get_iter<I>() - get_begin<I>();
                if (val > 0) {
                    advance_fwd<I>(offset, val);
                } else if (val < 0) {
                    advance_bwd<I>(offset, -val);
                }
            },
            it_.index());
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type val)
    requires details::concat_is_random_access<Const, Vs...>
    {
        *this += -val;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, std::default_sentinel_t) {
        return left.it_.index() == (sizeof...(Vs) - 1) &&
            left.it_.template value_ref<sizeof...(Vs) - 1>() ==
            left.template get_end<sizeof...(Vs) - 1>();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires (... &&
        std::equality_comparable<iterator_t<details::const_if<Const, Vs>>>)
    {
        return left.it_ == right.it_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<(iterator const& left, iterator const& right)
    requires (... && details::all_random_access<Const, Vs>)
    {
        return left.it_.index() < right.it_.index() ||
            (left.it_.index() == right.it_.index() &&
                details::jump_table_for<base_iter>(
                    [&]<size_t I>(details::size_constant<I>) {
                        return left.it_.template value_ref<I>() <
                            right.it_.template value_ref<I>();
                    },
                    left.it_.index()));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>(iterator const& left, iterator const& right)
    requires (... && details::all_random_access<Const, Vs>)
    {
        return right < left;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<=(
        iterator const& left, iterator const& right)
    requires (... && details::all_random_access<Const, Vs>)
    {
        return !(right < left);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>=(
        iterator const& left, iterator const& right)
    requires (... && details::all_random_access<Const, Vs>)
    {
        return !(left < right);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires (... && details::all_random_access<Const, Vs>)
    {
        return left <=> right;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& left, difference_type right)
    requires details::concat_is_random_access<Const, Vs...>
    {
        auto result = left;
        result += right;
        return result;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type left, iterator const& right)
    requires details::concat_is_random_access<Const, Vs...>
    {
        return right + left;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& left, difference_type right)
    requires details::concat_is_random_access<Const, Vs...>
    {
        auto result = left;
        result -= right;
        return result;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires details::concat_is_random_access<Const, Vs...>
    {
        // Split into 3 branches to reduce memory usage when compiling with GCC
        auto const cmp = (right.it_.index() < left.it_.index()) -
            (left.it_.index() < right.it_.index());
        if (cmp < 0) {
            return -(right - left);
        }

        if (cmp == 0) {
            return details::jump_table_for<base_iter>(
                [&]<size_t I>(details::size_constant<I>) -> difference_type {
                    return left.it_.template value_ref<I>() -
                        right.it_.template value_ref<I>();
                },
                left.it_.index());
        }

        return details::jump_table_for<base_iter>(
            [&]<size_t Ix>(details::size_constant<Ix>) -> difference_type {
                if constexpr (Ix != 0) {
                    return details::jump_table_for<base_iter>(
                        [&]<size_t Iy>
                        requires (Ix > Iy)
                        (details::size_constant<Iy>) -> difference_type {
                            auto const diff_y = ranges::distance(
                                right.it_.template value_ref<Iy>(),
                                right.template get_end<Iy>());
                            auto const diff_x =
                                ranges::distance(left.template get_begin<Ix>(),
                                    left.it_.template value_ref<Ix>());

                            difference_type result = 0;
                            [&]<size_t... Is>(std::index_sequence<Is...>) {
                                (...,
                                    (result +=
                                        ranges::size(left.template get_view<Is +
                                                     Iy + 1>())));
                            }(std::make_index_sequence<Ix - Iy - 1>{});

                            return diff_y + result + diff_x;
                        },
                        right.it_.index());
                } else {
                    RXX_BUILTIN_unreachable();
                }
            },
            left.it_.index());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, std::default_sentinel_t)
    requires (... &&
                 std::sized_sentinel_for<
                     sentinel_t<details::const_if<Const, Vs>>,
                     iterator_t<details::const_if<Const, Vs>>>) &&
        (details::all_but_first_sized_range<Const, Vs...>())
    {

        return details::jump_table_for<base_iter>(
            [&]<size_t Ix>(details::size_constant<Ix>) -> difference_type {
                auto const diff_x =
                    ranges::distance(left.it_.template value_ref<Ix>(),
                        left.template get_end<Ix>());

                difference_type result = 0;
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    (...,
                        (result += ranges::distance(
                             left.template get_view<Ix + 1 + Is>())));
                }(std::make_index_sequence<(sizeof...(Vs) - Ix - 1)>{});

                return -(diff_x + result);
            },
            left.it_.index());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        std::default_sentinel_t, iterator const& right)
    requires (... &&
                 std::sized_sentinel_for<
                     sentinel_t<details::const_if<Const, Vs>>,
                     iterator_t<details::const_if<Const, Vs>>>) &&
        (details::all_but_first_sized_range<Const, Vs...>())
    {
        return -(right - std::default_sentinel);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto iter_move(iterator const& self) noexcept((... &&
        (std::is_nothrow_invocable_v<decltype(ranges::iter_move),
             iterator_t<details::const_if<Const, Vs>> const&> &&
            std::is_nothrow_convertible_v<
                range_rvalue_reference_t<details::const_if<Const, Vs>>,
                details::concat_rvalue_reference_t<
                    details::const_if<Const, Vs>...>>))) {
        using Ref =
            details::concat_rvalue_reference_t<details::const_if<Const, Vs>...>;
        return details::jump_table_for<base_iter>(
            [&]<size_t I>(details::size_constant<I>) -> Ref {
                return ranges::iter_move(self.it_.template value_ref<I>());
            },
            self.it_.index());
    }

    __RXX_HIDE_FROM_ABI friend constexpr void iter_swap(iterator const& left,
        iterator const& right) noexcept(noexcept(ranges::swap(*left, *right)) &&
        details::all_nothrow_iter_swappable<Const, Vs...>())
    requires std::swappable_with<iter_reference_t<iterator>,
                 iter_reference_t<iterator>> &&
        (... &&
            std::indirectly_swappable<iterator_t<details::const_if<Const, Vs>>>)
    {
        details::jump_table_for<base_iter>(
            [&]<size_t Ix>(details::size_constant<Ix>) {
                details::jump_table_for<base_iter>(
                    [&]<size_t Iy>(details::size_constant<Iy>) {
                        if constexpr (
                            std::is_same_v<
                                details::template_element_t<Ix, base_iter>,
                                details::template_element_t<Iy, base_iter>>) {
                            ranges::iter_swap(left.it_.template value_ref<Ix>(),
                                right.it_.template value_ref<Iy>());
                        } else {
                            ranges::swap(*left.it_.template value_ref<Ix>(),
                                *right.it_.template value_ref<Iy>());
                        }
                    },
                    right.it_.index());
            },
            left.it_.index());
    }

private:
    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_iter() noexcept -> decltype(auto) {
        return it_.template value_ref<I>();
    }
    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_view() const noexcept -> decltype(auto) {
        return get_element<I>(parent_->views_);
    }
    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_begin() const
        noexcept(noexcept(ranges::begin(get_view<I>()))) {
        return ranges::begin(get_view<I>());
    }
    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto get_end() const
        noexcept(noexcept(ranges::end(get_view<I>()))) {
        return ranges::end(get_view<I>());
    }

    template <size_t I>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr auto to_underlying_diff_type(difference_type
            value) noexcept(std::is_nothrow_convertible_v<difference_type,
        std::iter_difference_t<details::template_element_t<I, base_iter>>>) {
        using Type = details::template_element_t<I, base_iter>;
        return static_cast<std::iter_difference_t<Type>>(value);
    }

    template <size_t I>
    __RXX_HIDE_FROM_ABI constexpr void satisfy() {
        if constexpr (I < sizeof...(Vs) - 1) {
            if (get_iter<I>() == get_end<I>()) {
                it_.template reinitialize_value<I + 1>(get_begin<I + 1>());
                satisfy<I + 1>();
            }
        }
    }

    template <size_t I>
    __RXX_HIDE_FROM_ABI constexpr void prev() {
        if constexpr (I == 0) {
            --get_iter<0>();
        } else if (get_iter<I>() == get_begin<I>()) {
            it_.template reinitialize_value<I - 1>(get_end<I - 1>());
            prev<I - 1>();
        } else {
            --get_iter<I>();
        }
    }

    template <size_t I>
    __RXX_HIDE_FROM_ABI constexpr void advance_fwd(
        difference_type offset, difference_type steps) {
        if constexpr (I == sizeof...(Vs) - 1) {
            get_iter<I>() += to_underlying_diff_type<I>(steps);
        } else {
            auto const n_size = ranges::distance(get_view<I>());
            if (offset + steps < n_size)
                get_iter<I>() += to_underlying_diff_type<I>(steps);
            else {
                it_.template reinitialize_value<I + 1>(get_begin<I + 1>());
                advance_fwd<I + 1>(0, offset + steps - n_size);
            }
        }
    }

    template <size_t I>
    __RXX_HIDE_FROM_ABI constexpr void advance_bwd(
        difference_type offset, difference_type steps) {
        if constexpr (I == 0) {
            get_iter<I>() -= to_underlying_diff_type<I>(steps);
        } else if (offset >= steps) {
            get_iter<I>() -= to_underlying_diff_type<I>(steps);
        } else {
            auto prev_size = ranges::distance(get_view<I - 1>());
            it_.template reinitialize_value<I - 1>(get_end<I - 1>());
            advance_bwd<I - 1>(prev_size, steps - offset);
        }
    }

    details::const_if<Const, concat_view>* parent_ = nullptr;
    base_iter it_;
};

namespace views {
namespace details {
struct concat_t {
    template <typename... Ts>
    requires requires { concat_view(std::declval<Ts>()...); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(Ts&&... args) RXX_CONST_CALL {
        return concat_view(std::forward<Ts>(args)...);
    }

    template <input_range R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& args) RXX_CONST_CALL {
        return __RXX ranges::views::all(std::forward<R>(args));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::concat_t concat{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
