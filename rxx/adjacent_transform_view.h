// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/adjacent_view.h"
#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/movable_box.h"
#include "rxx/details/referenceable.h"
#include "rxx/details/simple_view.h"
#include "rxx/primitives.h"
#include "rxx/zip_transform_view.h"

#include <compare>
#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

namespace details {

template <typename F, typename V, size_t... Is>
__RXX_HIDE_FROM_ABI consteval auto repeat_regular_invocable_impl(
    std::index_sequence<Is...>) noexcept {
    return std::regular_invocable<F, always_type<Is, V>...>;
}

template <typename F, typename V, size_t N>
concept repeat_regular_invocable =
    repeat_regular_invocable_impl<F, V>(std::make_index_sequence<N>{});

template <typename F, typename V, size_t... Is>
__RXX_HIDE_FROM_ABI auto repeat_invoke_result(
    std::index_sequence<Is...>) noexcept
    -> std::invoke_result<F, always_type<Is, V>...>;

template <typename F, typename V, size_t N>
using repeat_invoke_result_t RXX_NODEBUG =
    typename decltype(repeat_invoke_result<F, V>(
        std::make_index_sequence<N>{}))::type;

} // namespace details

template <std::ranges::forward_range V, std::move_constructible F, size_t N>
requires std::ranges::view<V> && (N > 0) && std::is_object_v<F> &&
    details::repeat_regular_invocable<F&, range_reference_t<V>, N> &&
    details::referenceable<
        details::repeat_invoke_result_t<F&, range_reference_t<V>, N>>
class adjacent_transform_view :
    public std::ranges::view_interface<adjacent_transform_view<V, F, N>> {
    using InnerView RXX_NODEBUG = adjacent_view<V, N>;
    template <bool Const>
    using inner_iterator RXX_NODEBUG =
        iterator_t<details::const_if<Const, InnerView>>;
    template <bool Const>
    using inner_sentinel RXX_NODEBUG =
        sentinel_t<details::const_if<Const, InnerView>>;

    template <bool Const>
    class iterator;
    template <bool Const>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr adjacent_transform_view() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr adjacent_transform_view(
        V base, F func) noexcept(std::is_nothrow_move_constructible_v<F> &&
        std::is_nothrow_move_constructible_v<InnerView>)
        : func_{std::move(func)}
        , inner_{std::move(base)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const& noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return inner_.base();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto base() && noexcept(std::is_nothrow_move_constructible_v<V>) {
        return std::move(inner_.base());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto begin() {
        return iterator<false>(*this, inner_.begin());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires std::ranges::range<InnerView const> &&
        details::repeat_regular_invocable<F const&, range_reference_t<V const>,
            N>
    {
        return iterator<true>(*this, inner_.begin());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto end() {
        if constexpr (std::ranges::common_range<InnerView>)
            return iterator<false>(*this, inner_.end());
        else
            return sentinel<false>(inner_.end());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires std::ranges::range<InnerView const> &&
        details::repeat_regular_invocable<F const&, range_reference_t<V const>,
            N>
    {
        if constexpr (std::ranges::common_range<InnerView const>)
            return iterator<true>(*this, inner_.end());
        else
            return sentinel<true>(inner_.end());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires std::ranges::sized_range<InnerView>
    {
        return inner_.size();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires std::ranges::sized_range<InnerView const>
    {
        return inner_.size();
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) details::movable_box<F> func_;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) adjacent_view<V, N> inner_;
};

template <std::ranges::forward_range V, std::move_constructible F, size_t N>
requires std::ranges::view<V> && (N > 0) && std::is_object_v<F> &&
    details::repeat_regular_invocable<F&, range_reference_t<V>, N> &&
    details::referenceable<
        details::repeat_invoke_result_t<F&, range_reference_t<V>, N>>
template <bool Const>
class adjacent_transform_view<V, F, N>::iterator {
    using Parent RXX_NODEBUG =
        details::const_if<Const, adjacent_transform_view>;
    using Base RXX_NODEBUG = details::const_if<Const, V>;

    friend class adjacent_transform_view;

    __RXX_HIDE_FROM_ABI constexpr iterator(
        Parent& parent, inner_iterator<Const> inner) noexcept(std::
            is_nothrow_move_constructible_v<inner_iterator<Const>>)
        : parent_{RXX_BUILTIN_addressof(parent)}
        , inner_{std::move(inner)} {}

public:
    using iterator_category = decltype([]() {
        using result_type =
            details::repeat_invoke_result_t<details::const_if<Const, F>&,
                range_reference_t<Base>, N>;
        using base_category =
            typename std::iterator_traits<iterator_t<Base>>::iterator_category;

        if constexpr (!std::is_reference_v<result_type>) {
            return std::input_iterator_tag{};
        } else if constexpr (std::derived_from<base_category,
                                 std::random_access_iterator_tag>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (std::derived_from<base_category,
                                 std::bidirectional_iterator_tag>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::derived_from<base_category,
                                 std::forward_iterator_tag>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());

    using iterator_concept = typename inner_iterator<Const>::iterator_concept;
    using value_type = std::remove_cvref_t<details::repeat_invoke_result_t<
        details::const_if<Const, F>&, range_reference_t<Base>, N>>;
    using difference_type = range_difference_t<Base>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<inner_iterator<Const>>) =
        default;
    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_move_constructible_v<inner_iterator<Const>>)
    requires Const &&
                 std::convertible_to<inner_iterator<false>,
                     inner_iterator<Const>>
        : parent_{other.parent_}
        , inner_{std::move(other.inner_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator*() const {
        return std::apply(
            [&](auto const&... iters) -> decltype(auto) {
                return std::invoke(*parent_->func_, *iters...);
            },
            get_current(inner_));
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        ++inner_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires std::ranges::forward_range<Base>
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires std::ranges::bidirectional_range<Base>
    {
        --inner_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires std::ranges::bidirectional_range<Base>
    {
        auto previous = *this;
        --*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        inner_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        inner_ -= offset;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator[](difference_type offset) const
    requires std::ranges::random_access_range<Base>
    {
        return std::apply(
            [&]<typename... It>(It const&... iters) -> decltype(auto) {
                return std::invoke(
                    *parent_->func_, iters[iter_difference_t<It>(offset)]...);
            },
            get_current(inner_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires std::equality_comparable<inner_iterator<Const>>
    {
        return left.inner_ == right.inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires std::ranges::random_access_range<Base>
    {
        return left.inner_ <=> right.inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& self, difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        return iterator(*self.parent_, self.inner_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type offset, iterator const& selft)
    requires std::ranges::random_access_range<Base>
    {
        return iterator(*selft.parent_, selft.inner_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& selft, difference_type offset)
    requires std::ranges::random_access_range<Base>
    {
        return iterator(*selft.parent_, selft.inner_ - offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires std::sized_sentinel_for<inner_iterator<Const>,
        inner_iterator<Const>>
    {
        return left.inner_ - right.inner_;
    }

private:
    Parent* parent_ = nullptr;
    inner_iterator<Const> inner_;
};

template <std::ranges::forward_range V, std::move_constructible F, size_t N>
requires std::ranges::view<V> && (N > 0) && std::is_object_v<F> &&
    details::repeat_regular_invocable<F&, range_reference_t<V>, N> &&
    details::referenceable<
        details::repeat_invoke_result_t<F&, range_reference_t<V>, N>>
template <bool Const>
class adjacent_transform_view<V, F, N>::sentinel {

    friend class adjacent_transform_view;

    __RXX_HIDE_FROM_ABI explicit constexpr sentinel(
        inner_sentinel<Const> inner) noexcept(std::
            is_nothrow_move_constructible_v<inner_sentinel<Const>>)
        : inner_{std::move(inner)} {}

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<inner_sentinel<Const>>) =
        default;

    template <bool OtherConst>
    requires std::sized_sentinel_for<inner_sentinel<Const>,
        inner_sentinel<OtherConst>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, InnerView>>
    operator-(iterator<OtherConst> const& iter, sentinel const& end) {
        return iter.inner_ - end.inner_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<inner_sentinel<Const>,
        inner_sentinel<OtherConst>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, InnerView>>
    operator-(sentinel const& end, iterator<OtherConst> const& iter) {
        return end.inner_ - iter.inner_;
    }

private:
    inner_sentinel<Const> inner_;
};

namespace views {
namespace details {

template <size_t N>
struct adjacent_transform_t
#if RXX_LIBSTDCXX
    :
    std::views::__adaptor::_RangeAdaptor<adjacent_transform_t<N>>
#endif
{

#if RXX_LIBSTDCXX
    using std::views::__adaptor::_RangeAdaptor<
        adjacent_transform_t>::operator();
    template <typename T>
    static constexpr bool _S_has_simple_extra_args = true;
    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX
    template <typename F>
    requires std::constructible_from<std::decay_t<F>, F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        F&& func) const
        noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
        return __RXX ranges::details::make_pipeable(
            [transformer = *this,
                func = std::forward<F>(func)]<std::ranges::viewable_range V>(
                V&& arg) mutable {
                return transformer(std::forward<V>(arg), std::forward<F>(func));
            });
    }
#elif RXX_MSVC_STL
    template <typename F>
    requires std::constructible_from<std::decay_t<F>, F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        F&& func) const
        noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
        return std::ranges::_Range_closure<adjacent_transform_t,
            std::decay_t<F>>{std::forward<F>(func)};
    }
#else
#  error "Unsupported"
#endif

    template <std::ranges::viewable_range V, typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator()(V&& arg, F&& func) const
        noexcept(noexcept(zip_transform(std::declval<F>())))
    requires (N == 0)
    {
        return zip_transform(std::forward<F>(func));
    }

    template <std::ranges::viewable_range V, typename F>
    requires requires {
        adjacent_transform_view<std::views::all_t<V>, std::decay_t<F>, N>(
            std::declval<V>(), std::declval<F>());
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto operator()(
        V&& arg, F&& func) const
        noexcept(noexcept(
            adjacent_transform_view<std::views::all_t<V>, std::decay_t<F>, N>(
                std::declval<V>(), std::declval<F>()))) {
        return adjacent_transform_view<std::views::all_t<V>, std::decay_t<F>,
            N>(std::forward<V>(arg), std::forward<F>(func));
    }
};
} // namespace details

inline namespace cpo {
template <size_t N>
inline constexpr details::adjacent_transform_t<N> adjacent_transform{};
inline constexpr details::adjacent_transform_t<2> pairwise_transform{};
} // namespace cpo
} // namespace views

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
