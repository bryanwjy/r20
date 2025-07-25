// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/details/concat.h"
#include "rxx/details/const_if.h"
#include "rxx/details/non_propagating_cache.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/to_unsigned_like.h"
#include "rxx/details/variant_base.h"
#include "rxx/iterator.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/single_view.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/type_traits/common_reference.h"
#include "rxx/utility.h"

#include <cassert>
#include <compare>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {

template <typename R>
concept bidirectional_common = bidirectional_range<R> && common_range<R>;
} // namespace details

template <input_range V, forward_range P>
requires view<V> && view<P> && input_range<range_reference_t<V>> &&
    details::concatable<range_reference_t<V>, P>
class join_with_view : public view_interface<join_with_view<V, P>> {
    using InnerRange RXX_NODEBUG = range_reference_t<V>;

    template <bool Const>
    class iterator;
    template <bool Const>
    class sentinel;

    static consteval bool nothrow_begin() noexcept {
        if constexpr (forward_range<V>) {
            constexpr bool is_const = details::simple_view<V> &&
                std::is_reference_v<InnerRange> && details::simple_view<P>;
            return std::is_nothrow_constructible_v<iterator<is_const>,
                join_with_view&, iterator_t<V>>;
        } else {
            return std::is_nothrow_constructible_v<InnerRange, iterator_t<V>> &&
                std::is_nothrow_constructible_v<iterator<false>,
                    join_with_view&>;
        }
    }

    static consteval bool nothrow_end() noexcept {
        constexpr bool is_const =
            details::simple_view<V> && details::simple_view<P>;
        if constexpr (forward_range<V> && common_range<V> &&
            std::is_reference_v<InnerRange> && forward_range<InnerRange> &&
            common_range<InnerRange>) {
            return std::is_nothrow_constructible_v<iterator<is_const>,
                join_with_view&, sentinel_t<V>>;
        } else {
            return std::is_nothrow_constructible_v<sentinel<is_const>,
                join_with_view&>;
        }
    }

    static consteval bool nothrow_const_end() noexcept {
        using ConstInnerRange = range_reference_t<V const>;
        if constexpr (forward_range<ConstInnerRange> &&
            common_range<ConstInnerRange> && common_range<V const>) {
            return std::is_nothrow_constructible_v<iterator<true>,
                join_with_view const&, sentinel_t<V const>>;
        } else {
            return std::is_nothrow_constructible_v<sentinel<true>,
                join_with_view const&>;
        }
    }

public:
    __RXX_HIDE_FROM_ABI constexpr join_with_view() noexcept(
        std::is_nothrow_default_constructible_v<V> &&
        std::is_nothrow_default_constructible_v<P>)
    requires std::default_initializable<V> && std::default_initializable<P>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr join_with_view(
        V base, P pattern) noexcept(std::is_nothrow_move_constructible_v<V> &&
        std::is_nothrow_move_constructible_v<P>)
        : base_{std::move(base)}
        , pattern_{std::move(pattern)} {}

    template <input_range R>
    requires std::constructible_from<V, views::all_t<R>> &&
                 std::constructible_from<P,
                     single_view<range_value_t<InnerRange>>>
    __RXX_HIDE_FROM_ABI explicit constexpr join_with_view(
        R&& base, range_value_t<InnerRange> pattern)
        : base_{views::all(std::forward<R>(base))}
        , pattern_{views::single(std::move(pattern))} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const& noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return base_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() && noexcept(std::is_nothrow_move_constructible_v<V>) {
        return std::move(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() noexcept(nothrow_begin()) {
        if constexpr (forward_range<V>) {
            constexpr bool is_const = details::simple_view<V> &&
                std::is_reference_v<InnerRange> && details::simple_view<P>;
            return iterator<is_const>{*this, __RXX ranges::begin(base_)};
        } else {
            outer_.emplace(__RXX ranges::begin(base_));
            return iterator<false>{*this};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
        noexcept(std::is_nothrow_constructible_v<iterator<true>,
            join_with_view const&, iterator_t<V const>>)
    requires forward_range<V const> && forward_range<P const> &&
        std::is_reference_v<range_reference_t<V const>> &&
        input_range<range_reference_t<V const>> &&
        details::concatable<range_reference_t<V const>, P const>
    {
        return iterator<true>{*this, __RXX ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() noexcept(nothrow_end()) {
        constexpr bool is_const =
            details::simple_view<V> && details::simple_view<P>;
        if constexpr (forward_range<V> && common_range<V> &&
            std::is_reference_v<InnerRange> && forward_range<InnerRange> &&
            common_range<InnerRange>) {
            return iterator<is_const>{*this, __RXX ranges::end(base_)};
        } else {
            return sentinel<is_const>{*this};
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept(nothrow_const_end())
    requires forward_range<V const> && forward_range<P const> &&
        std::is_reference_v<range_reference_t<V const>> &&
        input_range<range_reference_t<V const>> &&
        details::concatable<range_reference_t<V const>, P const>
    {
        using ConstInnerRange = range_reference_t<V const>;
        if constexpr (forward_range<ConstInnerRange> &&
            common_range<ConstInnerRange> && common_range<V const>) {
            return iterator<true>{*this, __RXX ranges::end(base_)};
        } else {
            return sentinel<true>{*this};
        }
    }

private:
    using OuterItType RXX_NODEBUG = details::non_propagating_cache<
        std::conditional_t<forward_range<V>, void, iterator_t<V>>>;
    using InnerType RXX_NODEBUG =
        details::non_propagating_cache<std::remove_cv_t<InnerRange>>;

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_{};
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) OuterItType outer_;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) InnerType inner_;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) P pattern_{};
};

template <typename R, typename P>
join_with_view(R&&, P&&) -> join_with_view<views::all_t<R>, views::all_t<P>>;

template <input_range R>
join_with_view(R&&, range_value_t<range_reference_t<R>>)
    -> join_with_view<views::all_t<R>,
        single_view<range_value_t<range_reference_t<R>>>>;

namespace details {

template <bool Const, typename V, typename P>
struct join_with_view_iterator_category {
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    using InnerBase RXX_NODEBUG = range_reference_t<Base>;
    using PatternBase RXX_NODEBUG = details::const_if<Const, P>;
    using OuterIter RXX_NODEBUG = iterator_t<Base>;
    using InnerIter RXX_NODEBUG = iterator_t<InnerBase>;
    using PatternIter RXX_NODEBUG = iterator_t<PatternBase>;
};

template <bool Const, typename V, typename P>
requires std::is_reference_v<range_reference_t<details::const_if<Const, V>>> &&
    forward_range<details::const_if<Const, V>> &&
    forward_range<range_reference_t<details::const_if<Const, V>>>
struct join_with_view_iterator_category<Const, V, P> {
private:
    static consteval auto make_iterator_category() noexcept {
        using OuterC RXX_NODEBUG =
            typename std::iterator_traits<OuterIter>::iterator_category;
        using InnerC RXX_NODEBUG =
            typename std::iterator_traits<InnerIter>::iterator_category;
        using PatternC RXX_NODEBUG =
            typename std::iterator_traits<PatternIter>::iterator_category;

        if constexpr (!std::is_reference_v<
                          common_reference_t<range_reference_t<InnerBase>,
                              range_reference_t<PatternBase>>>) {
            return std::input_iterator_tag{};
        } else if constexpr (std::derived_from<OuterC,
                                 std::bidirectional_iterator_tag> &&
            std::derived_from<InnerC, std::bidirectional_iterator_tag> &&
            std::derived_from<PatternC, std::bidirectional_iterator_tag> &&
            common_range<InnerBase> && common_range<PatternBase>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::derived_from<OuterC,
                                 std::forward_iterator_tag> &&
            std::derived_from<InnerC, std::forward_iterator_tag> &&
            std::derived_from<PatternC, std::forward_iterator_tag>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

public:
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    using InnerBase RXX_NODEBUG = range_reference_t<Base>;
    using PatternBase RXX_NODEBUG = details::const_if<Const, P>;
    using OuterIter RXX_NODEBUG = iterator_t<Base>;
    using InnerIter RXX_NODEBUG = iterator_t<InnerBase>;
    using PatternIter RXX_NODEBUG = iterator_t<PatternBase>;
    using iterator_category = decltype(make_iterator_category());
};

} // namespace details

template <input_range V, forward_range P>
requires view<V> && view<P> && input_range<range_reference_t<V>> &&
    details::concatable<range_reference_t<V>, P>
template <bool Const>
class join_with_view<V, P>::iterator :
    public details::join_with_view_iterator_category<Const, V, P> {

    friend join_with_view;
    using category_base RXX_NODEBUG =
        details::join_with_view_iterator_category<Const, V, P>;

    using Parent RXX_NODEBUG = details::const_if<Const, join_with_view>;

    using typename category_base::Base;
    using typename category_base::InnerBase;
    using typename category_base::InnerIter;
    using typename category_base::OuterIter;
    using typename category_base::PatternBase;
    using typename category_base::PatternIter;

    static_assert(!Const || forward_range<Base>,
        "Const can only be true when Base models forward_range.");

    __RXX_HIDE_FROM_ABI constexpr OuterIter& get_outer() noexcept {
        if constexpr (forward_range<Base>) {
            return outer_;
        } else {
            return *parent_->outer_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr OuterIter const& get_outer() const noexcept {
        if constexpr (forward_range<Base>) {
            return outer_;
        } else {
            return *parent_->outer_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr auto& update_inner() noexcept(
        noexcept(InnerRange(*get_outer()))) {
        if constexpr (std::is_reference_v<InnerBase>) {
            return []<typename U>(U&& ref) -> U& {
                return static_cast<U&>(ref);
            }(*get_outer());
        } else {
            return parent_->inner_.emplace_deref(get_outer());
        }
    }

    __RXX_HIDE_FROM_ABI constexpr auto& get_inner() noexcept(
        !std::is_reference_v<InnerBase> || noexcept(*get_outer())) {
        if constexpr (std::is_reference_v<InnerBase>) {
            return []<typename U>(U&& ref) -> U& {
                return static_cast<U&>(ref);
            }(*get_outer());
        } else {
            return *parent_->inner_;
        }
    }

    __RXX_HIDE_FROM_ABI constexpr void satisfy() {
        while (true) {
            if (inner_.index() == 0) {
                if (inner_.template value_ref<0>() !=
                    __RXX ranges::end(parent_->pattern_)) {
                    break;
                }
                inner_.template reinitialize_value<1>(
                    __RXX ranges::begin(update_inner()));
            } else {
                if (inner_.template value_ref<1>() !=
                    __RXX ranges::end(get_inner())) {
                    break;
                }

                if (++get_outer() == __RXX ranges::end(parent_->base_)) {
                    if constexpr (std::is_reference_v<InnerBase>) {
                        inner_.template reinitialize_value<0>();
                    }

                    break;
                }

                inner_.template reinitialize_value<0>(
                    __RXX ranges::begin(parent_->pattern_));
            }
        }
    }

    __RXX_HIDE_FROM_ABI constexpr iterator(Parent& parent, OuterIter outer)
    requires forward_range<Base>
        : parent_{RXX_BUILTIN_addressof(parent)}
        , outer_{std::move(outer)} {
        if (get_outer() != __RXX ranges::end(parent_->base_)) {
            inner_.template reinitialize_value<1>(
                __RXX ranges::begin(update_inner()));
            satisfy();
        }
    }

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(Parent& parent)
    requires (!forward_range<Base>)
        : parent_{RXX_BUILTIN_addressof(parent)} {
        if (get_outer() != __RXX ranges::end(parent_->base_)) {
            inner_.template reinitialize_value<1>(
                __RXX ranges::begin(update_inner()));
            satisfy();
        }
    }

    static consteval auto make_iterator_concept() noexcept {
        if constexpr (std::is_reference_v<InnerBase> &&
            bidirectional_range<Base> &&
            details::bidirectional_common<PatternBase> &&
            details::bidirectional_common<InnerBase>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::is_reference_v<InnerBase> &&
            forward_range<Base> && forward_range<InnerBase>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

public:
    using iterator_concept = decltype(make_iterator_concept());
    using value_type =
        std::common_type_t<iter_value_t<InnerIter>, iter_value_t<PatternIter>>;
    using difference_type = std::common_type_t<iter_difference_t<OuterIter>,
        iter_difference_t<InnerIter>, iter_difference_t<PatternIter>>;

    __RXX_HIDE_FROM_ABI constexpr iterator() = default;

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other)
    requires Const && std::convertible_to<iterator_t<V>, OuterIter> &&
                 std::convertible_to<iterator_t<InnerRange>, InnerIter> &&
                 std::convertible_to<iterator_t<P>, PatternIter>
        : parent_{other.parent_}
        , outer_{std::move(other.outer_)}
        , inner_{[&]() {
            if (other.inner_.index() == 0) {
                return InnerType{std::in_place_index<0>,
                    std::move(other.inner_.template value_ref<0>())};
            } else {
                return InnerType{std::in_place_index<1>,
                    std::move(other.inner_.template value_ref<1>())};
            }
        }()} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr common_reference_t<iter_reference_t<InnerIter>,
        iter_reference_t<PatternIter>>
    operator*() const
        noexcept(noexcept(*std::declval<PatternIter const&>()) && noexcept(
            *std::declval<InnerIter const&>())) {
        if (inner_.index() == 0) {
            return *inner_.template value_ref<0>();
        } else {
            return *inner_.template value_ref<1>();
        }
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() {
        if (inner_.index() == 0) {
            ++inner_.template value_ref<0>();
        } else {
            ++inner_.template value_ref<1>();
        }

        satisfy();
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) { ++*this; }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int)
    requires std::is_reference_v<InnerBase> && forward_range<Base> &&
        forward_range<InnerBase>
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--()
    requires std::is_reference_v<InnerBase> &&
        bidirectional_range<Base> && details::bidirectional_common<InnerBase> &&
        details::bidirectional_common<PatternBase>
    {
        if (outer_ == __RXX ranges::end(parent_->base_)) {
            inner_.template reinitialize_value<1>(__RXX ranges::end(*--outer_));
        }

        while (true) {
            if (inner_.index() == 0) {
                if (inner_.template value_ref<0>() ==
                    __RXX ranges::begin(parent_->pattern_)) {
                    inner_.template reinitialize_value<1>(
                        __RXX ranges::end(*--outer_));
                } else {
                    break;
                }
            } else {
                if (inner_.template value_ref<1>() ==
                    __RXX ranges::begin(*outer_)) {
                    inner_.template reinitialize_value<0>(
                        __RXX ranges::end(parent_->pattern_));
                } else {
                    break;
                }
            }
        }

        if (inner_.index() == 0) {
            --inner_.template value_ref<0>();
        } else {
            --inner_.template value_ref<1>();
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int)
    requires std::is_reference_v<InnerBase> &&
        bidirectional_range<Base> && details::bidirectional_common<InnerBase> &&
        details::bidirectional_common<PatternBase>
    {
        auto previous = *this;
        --*this;
        return previous;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator==(
        iterator const& left, iterator const& right)
    requires std::is_reference_v<InnerBase> &&
        forward_range<Base> && std::equality_comparable<InnerIter>
    {
        return left.outer_ == right.outer_ && left.inner_ == right.inner_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr common_reference_t<iter_rvalue_reference_t<InnerIter>,
        iter_rvalue_reference_t<PatternIter>>
    iter_move(iterator const& iter) noexcept(noexcept(ranges::iter_move(
        std::declval<PatternIter const&>())) && noexcept(ranges::
            iter_move(std::declval<InnerIter const&>()))) {
        switch (iter.inner_.index()) {
        case 0:
            return ranges::iter_move(iter.inner_.template value_ref<0>());
        default:
            return ranges::iter_move(iter.inner_.template value_ref<1>());
        }
    }

    __RXX_HIDE_FROM_ABI friend constexpr void
    iter_swap(iterator const& left, iterator const& right) noexcept(
        noexcept(ranges::iter_swap(std::declval<InnerIter const&>(),
            std::declval<InnerIter const&>())) && noexcept(ranges::
                iter_swap(std::declval<PatternIter const&>(),
                    std::declval<PatternIter const&>())) && noexcept(std::
                ranges::iter_swap(std::declval<InnerIter const&>(),
                    std::declval<PatternIter const&>())))
    requires std::indirectly_swappable<InnerIter, InnerIter> &&
        std::indirectly_swappable<PatternIter, PatternIter> &&
        std::indirectly_swappable<InnerIter, PatternIter>
    {
        if (left.inner_.index() == 0) {
            if (right.inner_.index() == 0) {
                ranges::iter_swap(left.inner_.template value_ref<0>(),
                    right.inner_.template value_ref<0>());
            } else {
                ranges::iter_swap(left.inner_.template value_ref<0>(),
                    right.inner_.template value_ref<1>());
            }
        } else {
            if (right.inner_.index() == 0) {
                ranges::iter_swap(left.inner_.template value_ref<1>(),
                    right.inner_.template value_ref<0>());
            } else {
                ranges::iter_swap(left.inner_.template value_ref<1>(),
                    right.inner_.template value_ref<1>());
            }
        }
    }

private:
    struct nothing_t {};
    using OuterType RXX_NODEBUG =
        std::conditional_t<forward_range<Base>, OuterIter, nothing_t>;
    using InnerType RXX_NODEBUG = details::variant_base<PatternIter, InnerIter>;
    Parent* parent_ = nullptr;
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) OuterType outer_{};
    InnerType inner_;
};

template <input_range V, forward_range P>
requires view<V> && view<P> && input_range<range_reference_t<V>> &&
    details::concatable<range_reference_t<V>, P>
template <bool Const>
class join_with_view<V, P>::sentinel {
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    using Parent RXX_NODEBUG = details::const_if<Const, join_with_view>;

    template <bool>
    friend class sentinel;
    friend join_with_view;

    __RXX_HIDE_FROM_ABI explicit constexpr sentinel(Parent& parent)
        : end_(__RXX ranges::end(parent.base_)) {}

    template <bool OtherConst>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr decltype(auto)
        iter_outer(iterator<OtherConst> const& iter) noexcept {
        return iter.get_outer();
    }

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<sentinel_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other)
    requires Const && std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : end_(std::move(other.end_)) {}

    template <bool OtherConst>
    requires std::sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool operator==(
        iterator<OtherConst> const& left, sentinel const& right) {
        return iter_outer(left) == right.end_;
    }

private:
    sentinel_t<Base> end_{};
};

namespace views {
namespace details {
struct join_with_t : ranges::details::adaptor_non_closure<join_with_t> {
    template <typename R, typename P>
    requires requires { join_with_view(std::declval<R>(), std::declval<P>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& range, P&& pattern) RXX_CONST_CALL noexcept(
        noexcept(join_with_view(std::declval<R>(), std::declval<P>()))) {
        return join_with_view(std::forward<R>(range), std::forward<P>(pattern));
    }

#if RXX_LIBSTDCXX
    using ranges::details::adaptor_non_closure<join_with_t>::operator();
    template <typename T>
    static constexpr bool _S_has_simple_extra_args =
        std::is_scalar_v<T> || (view<T> && std::copy_constructible<T>);
    static constexpr int _S_arity = 2;
#elif RXX_LIBCXX | RXX_MSVC_STL
    template <typename D>
    requires std::constructible_from<std::decay_t<D>, D>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(D&& delimiter) RXX_CONST_CALL
        noexcept(std::is_nothrow_constructible_v<std::decay_t<D>, D>) {
        return __RXX ranges::details::make_pipeable(
            __RXX ranges::details::set_arity<2>(*this),
            std::forward<D>(delimiter));
    }
#else
#  error "Unsupported"
#endif
};
} // namespace details

inline namespace cpo {
inline constexpr details::join_with_t join_with{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
