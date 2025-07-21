// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/details/iterator_category_of.h"
#include "rxx/details/movable_box.h"
#include "rxx/details/referenceable.h"
#include "rxx/details/simple_view.h"
#include "rxx/iterator.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/empty_view.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/ranges/zip_view.h"
#include "rxx/tuple/apply.h"

#include <compare>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <std::move_constructible F, input_range... Views>
requires (... && view<Views>) &&
    (sizeof...(Views) > 0) && std::is_object_v<F> &&
    std::regular_invocable<F&, range_reference_t<Views>...> &&
    details::referenceable<
        std::invoke_result_t<F&, range_reference_t<Views>...>>
class zip_transform_view :
    public view_interface<zip_transform_view<F, Views...>> {
    using InnerView = zip_view<Views...>;
    template <bool Const>
    using ziperator = iterator_t<details::const_if<Const, InnerView>>;
    template <bool Const>
    using zentinel = sentinel_t<details::const_if<Const, InnerView>>;
    template <bool Const>
    using Base = details::const_if<Const, InnerView>;

    template <bool Const>
    class iterator;
    template <bool Const>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr zip_transform_view() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr zip_transform_view(F func, Views... views)
        : func_{std::move(func)}
        , zip_{std::move(views)...} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto begin() {
        return iterator<false>(*this, zip_.begin());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires range<InnerView const> &&
        std::regular_invocable<F const&, range_reference_t<Views const>...>
    {
        return iterator<true>(*this, zip_.begin());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto end() {
        if constexpr (common_range<InnerView>) {
            return iterator<false>(*this, zip_.end());
        } else {
            return sentinel<false>(zip_.end());
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires range<InnerView const> &&
        std::regular_invocable<F const&, range_reference_t<Views const>...>
    {
        if constexpr (common_range<InnerView const>) {
            return iterator<true>(*this, zip_.end());
        } else {
            return sentinel<true>(zip_.end());
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<InnerView>
    {
        return zip_.size();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<InnerView const>
    {
        return zip_.size();
    }

private:
    template <bool Const>
    struct iter_cat {};

    template <bool Const>
    requires forward_range<Base<Const>>
    struct iter_cat<Const> {
        using iterator_category = decltype([]() {
            using Result = std::invoke_result_t<details::const_if<Const, F>&,
                range_reference_t<details::const_if<Const, Views>>...>;

            if constexpr (!std::is_reference_v<Result>) {
                return std::input_iterator_tag{};
            } else if constexpr ((... &&
                                     std::derived_from<
                                         details::iterator_category_of<Const,
                                             Views>,
                                         std::random_access_iterator_tag>)) {
                return std::random_access_iterator_tag{};
            } else if constexpr ((... &&
                                     std::derived_from<
                                         details::iterator_category_of<Const,
                                             Views>,
                                         std::bidirectional_iterator_tag>)) {
                return std::bidirectional_iterator_tag{};
            } else if constexpr ((... &&
                                     std::derived_from<
                                         details::iterator_category_of<Const,
                                             Views>,
                                         std::forward_iterator_tag>)) {
                return std::forward_iterator_tag{};
            } else {
                return std::input_iterator_tag{};
            }
        }());
    };

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) details::movable_box<F> func_;
    InnerView zip_;
};

template <typename F, typename... Rs>
zip_transform_view(F, Rs&&...) -> zip_transform_view<F, views::all_t<Rs>...>;

template <std::move_constructible F, input_range... Views>
requires (... && view<Views>) &&
    (sizeof...(Views) > 0) && std::is_object_v<F> &&
    std::regular_invocable<F&, range_reference_t<Views>...> &&
    details::referenceable<
        std::invoke_result_t<F&, range_reference_t<Views>...>>
template <bool Const>
class zip_transform_view<F, Views...>::iterator :
    public zip_transform_view::iter_cat<Const> {

    friend class zip_transform_view;

    using Parent = details::const_if<Const, zip_transform_view>;

    __RXX_HIDE_FROM_ABI constexpr iterator(
        Parent& parent, ziperator<Const> inner) noexcept(std::
            is_nothrow_move_constructible_v<ziperator<Const>>)
        : parent_{RXX_BUILTIN_addressof(parent)}
        , inner_{std::move(inner)} {}

public:
    using iterator_concept = typename ziperator<Const>::iterator_concept;
    using value_type =
        std::remove_cvref_t<std::invoke_result_t<details::const_if<Const, F>&,
            range_reference_t<details::const_if<Const, Views>>...>>;
    using difference_type = range_difference_t<Base<Const>>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<ziperator<Const>>) = default;
    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_move_constructible_v<ziperator<Const>>)
    requires Const && std::convertible_to<ziperator<false>, ziperator<Const>>
        : parent_{other.parent_}
        , inner_{std::move(other.inner_)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator*() const {
        return apply(
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
    requires forward_range<Base<Const>>
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires bidirectional_range<Base<Const>>
    {
        --inner_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires bidirectional_range<Base<Const>>
    {
        auto previous = *this;
        --*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(difference_type offset)
    requires random_access_range<Base<Const>>
    {
        inner_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(difference_type offset)
    requires random_access_range<Base<Const>>
    {
        inner_ -= offset;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator[](difference_type offset) const
    requires random_access_range<Base<Const>>
    {
        return apply(
            [&]<typename... It>(It const&... iters) -> decltype(auto) {
                return std::invoke(
                    *parent_->func_, iters[iter_difference_t<It>(offset)]...);
            },
            get_current(inner_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires std::equality_comparable<ziperator<Const>>
    {
        return left.inner_ == right.inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto operator<=>(
        iterator const& left, iterator const& right)
    requires random_access_range<Base<Const>>
    {
        return left.inner_ <=> right.inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        iterator const& self, difference_type offset)
    requires random_access_range<Base<Const>>
    {
        return iterator(*self.parent_, self.inner_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(
        difference_type offset, iterator const& selft)
    requires random_access_range<Base<Const>>
    {
        return iterator(*selft.parent_, selft.inner_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator-(
        iterator const& selft, difference_type offset)
    requires random_access_range<Base<Const>>
    {
        return iterator(*selft.parent_, selft.inner_ - offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type operator-(
        iterator const& left, iterator const& right)
    requires std::sized_sentinel_for<ziperator<Const>, ziperator<Const>>
    {
        return left.inner_ - right.inner_;
    }

private:
    Parent* parent_;
    ziperator<Const> inner_;
};

template <std::move_constructible F, input_range... Views>
requires (... && view<Views>) &&
    (sizeof...(Views) > 0) && std::is_object_v<F> &&
    std::regular_invocable<F&, range_reference_t<Views>...> &&
    details::referenceable<
        std::invoke_result_t<F&, range_reference_t<Views>...>>
template <bool Const>
class zip_transform_view<F, Views...>::sentinel {

    friend class zip_transform_view;

    __RXX_HIDE_FROM_ABI explicit constexpr sentinel(
        zentinel<Const> inner) noexcept(std::
            is_nothrow_move_constructible_v<zentinel<Const>>)
        : inner_{std::move(inner)} {}

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<zentinel<Const>>) = default;

    template <bool OtherConst>
    requires std::sentinel_for<zentinel<Const>, ziperator<OtherConst>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool operator==(
        iterator<OtherConst> const& left, sentinel const& right) {
        return left.inner_ == right.inner_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<zentinel<Const>, ziperator<OtherConst>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, InnerView>>
    operator-(iterator<OtherConst> const& iter, sentinel const& end) {
        return iter.inner_ - end.inner_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<zentinel<Const>, ziperator<OtherConst>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, InnerView>>
    operator-(sentinel const& end, iterator<OtherConst> const& iter) {
        return end.inner_ - iter.inner_;
    }

private:
    zentinel<Const> inner_;
};

namespace views {
namespace details {
struct zip_transform_t {
    template <typename F>
    requires std::move_constructible<std::decay_t<F>> &&
        std::regular_invocable<std::decay_t<F>&> &&
        std::is_object_v<std::decay_t<std::invoke_result_t<std::decay_t<F>&>>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(F&&) RXX_CONST_CALL noexcept {
        return views::empty<
            std::decay_t<std::invoke_result_t<std::decay_t<F>&>>>;
    }

    template <typename F, typename... Rs>
    requires (sizeof...(Rs) > 0) && requires {
        zip_transform_view(std::declval<F>(), std::declval<Rs>()...);
    }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(F&& func, Rs&&... views) RXX_CONST_CALL noexcept(
        noexcept(zip_transform_view(std::declval<F>(), std::declval<Rs>()...)))
        -> decltype(zip_transform_view(
            std::declval<F>(), std::declval<Rs>()...)) {
        return zip_transform_view(
            std::forward<F>(func), std::forward<Rs>(views)...);
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::zip_transform_t zip_transform{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
