// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/const_if.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/get_element.h"
#include "rxx/ranges/primitives.h"
#include "rxx/tuple.h"

#include <concepts>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

namespace details {
template <typename T, size_t I>
concept has_tuple_element = tuple_like<T> && I < std::tuple_size<T>::value;

template <typename T, size_t I>
concept returnable_element = std::is_reference_v<T> ||
    std::move_constructible<std::tuple_element_t<I, T>>;

} // namespace details

template <input_range V, size_t I>
requires view<V> && details::has_tuple_element<range_value_t<V>, I> &&
    details::has_tuple_element<std::remove_reference_t<range_reference_t<V>>,
        I> &&
    details::returnable_element<range_reference_t<V>, I>
class elements_view : public view_interface<elements_view<V, I>> {
    template <bool>
    class iterator;
    template <bool>
    class sentinel;

public:
    __RXX_HIDE_FROM_ABI constexpr elements_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI constexpr elements_view(V base) noexcept(
        std::is_nothrow_move_constructible_v<V>)
        : base_{std::move(base)} {}

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
    constexpr auto begin() noexcept(
        std::is_nothrow_constructible_v<iterator<false>,
            iterator_t<V>>&& noexcept(ranges::begin(base_)))
    requires (!details::simple_view<V>)
    {
        return iterator<false>{ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
        noexcept(std::is_nothrow_constructible_v<iterator<true>,
            iterator_t<V const>>&& noexcept(ranges::begin(base_)))
    requires range<V const>
    {
        return iterator<true>{ranges::begin(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() noexcept(
        std::is_nothrow_constructible_v<sentinel<false>,
            sentinel_t<V>>&& noexcept(ranges::end(base_)))
    requires (!details::simple_view<V> && !common_range<V>)
    {
        return sentinel<false>{ranges::end(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() noexcept(
        std::is_nothrow_constructible_v<iterator<false>,
            sentinel_t<V>>&& noexcept(ranges::end(base_)))
    requires (!details::simple_view<V> && common_range<V>)
    {
        return iterator<false>{ranges::end(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
        noexcept(std::is_nothrow_constructible_v<sentinel<true>,
            sentinel_t<V>>&& noexcept(ranges::end(base_)))
    requires range<V const>
    {
        return sentinel<true>{ranges::end(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
        noexcept(std::is_nothrow_constructible_v<iterator<true>,
            sentinel_t<V>>&& noexcept(ranges::end(base_)))
    requires common_range<V const>
    {
        return iterator<true>{ranges::end(base_)};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() noexcept(noexcept(ranges::size(base_)))
    requires sized_range<V>
    {
        return ranges::size(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const noexcept(noexcept(ranges::size(base_)))
    requires sized_range<V const>
    {
        return ranges::size(base_);
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) V base_{};
};

namespace details {
template <typename, size_t>
struct elements_view_iterator_category {};

template <forward_range V, size_t I>
struct elements_view_iterator_category<V, I> {
    using iterator_category = decltype([]() {
        using Result = decltype(get_element<I>(*std::declval<iterator_t<V>>()));
        using Category =
            typename std::iterator_traits<iterator_t<V>>::iterator_category;

        if constexpr (!std::is_lvalue_reference_v<Result>) {
            return std::input_iterator_tag{};
        } else if constexpr (std::derived_from<Category,
                                 std::random_access_iterator_tag>) {
            return std::random_access_iterator_tag{};
        } else {
            return Category{};
        }
    }());
};

} // namespace details

template <input_range V, size_t I>
requires view<V> && details::has_tuple_element<range_value_t<V>, I> &&
    details::has_tuple_element<std::remove_reference_t<range_reference_t<V>>,
        I> &&
    details::returnable_element<range_reference_t<V>, I>
template <bool Const>
class elements_view<V, I>::iterator :
    details::elements_view_iterator_category<V, I> {
    friend elements_view;
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr decltype(auto) get(iterator_t<Base> const& iter) noexcept(
        []() {
            if constexpr (std::is_reference_v<range_reference_t<Base>>) {
                return noexcept(ranges::get_element<I>(*iter));
            } else {
                using Element = std::remove_cv_t<
                    std::tuple_element_t<I, range_reference_t<Base>>>;
                return noexcept(
                    static_cast<Element>(ranges::get_element<I>(*iter)));
            }
        }()) {
        if constexpr (std::is_reference_v<range_reference_t<Base>>) {
            return ranges::get_element<I>(*iter);
        } else {
            using Element = std::remove_cv_t<
                std::tuple_element_t<I, range_reference_t<Base>>>;
            auto result = static_cast<Element>(ranges::get_element<I>(*iter));
            return result;
        }
    }

public:
    using iterator_concept = decltype([]() {
        if constexpr (random_access_range<Base>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (bidirectional_range<Base>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (forward_range<Base>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }());
    using value_type =
        std::remove_cvref_t<std::tuple_element_t<I, range_value_t<Base>>>;
    using difference_type = range_difference_t<Base>;

    __RXX_HIDE_FROM_ABI constexpr iterator() noexcept(
        std::is_nothrow_default_constructible_v<iterator_t<Base>>)
    requires std::default_initializable<iterator_t<Base>>
    = default;

    __RXX_HIDE_FROM_ABI constexpr explicit iterator(
        iterator_t<Base> current) noexcept(std::
            is_nothrow_move_constructible_v<iterator_t<Base>>)
        : current_(std::move(current)) {}

    __RXX_HIDE_FROM_ABI constexpr iterator(iterator<!Const> other) noexcept(
        std::is_nothrow_convertible_v<iterator_t<V>, iterator_t<Base>>)
    requires Const && std::convertible_to<iterator_t<V>, iterator_t<Base>>
        : current_(std::move(other.current_)) {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> const& base() const& noexcept {
        return current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr iterator_t<Base> base() && noexcept(
        std::is_nothrow_move_constructible_v<iterator_t<Base>>) {
        return std::move(current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator*() const noexcept(noexcept(get(current_)))
        -> decltype(auto) {
        return get(current_);
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator++() noexcept(
        noexcept(++current_)) {
        ++current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) noexcept(
        noexcept(++current_)) {
        ++current_;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator++(int) noexcept(
        std::is_nothrow_copy_constructible_v<iterator_t<Base>>&& noexcept(
            ++current_))
    requires forward_range<Base>
    {
        auto prev = *this;
        ++current_;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator--() noexcept(
        noexcept(--current_))
    requires bidirectional_range<Base>
    {
        --current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator operator--(int) noexcept(
        std::is_nothrow_copy_constructible_v<iterator_t<Base>>&& noexcept(
            --current_))
    requires bidirectional_range<Base>
    {
        auto prev = *this;
        --current_;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator+=(
        difference_type offset) noexcept(noexcept(current_ += offset))
    requires random_access_range<Base>
    {
        current_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr iterator& operator-=(
        difference_type offset) noexcept(noexcept(current_ -= offset))
    requires random_access_range<Base>
    {
        current_ -= offset;
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator[](difference_type offset) const
        noexcept(noexcept(get(current_ + offset))) -> decltype(auto)
    requires random_access_range<Base>
    {
        return get(current_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator==(iterator const& left, iterator const& right) noexcept(
        noexcept(std::declval<iterator_t<Base> const&>() ==
            std::declval<iterator_t<Base> const&>()))
    requires std::equality_comparable<iterator_t<Base>>
    {
        return left.current_ == right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator<(iterator const& left, iterator const& right) noexcept(
        noexcept(std::declval<iterator_t<Base> const&>() <
            std::declval<iterator_t<Base> const&>()))
    requires random_access_range<Base>
    {
        return left.current_ < right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>(iterator const& left,
        iterator const& right) noexcept(noexcept(right < left))
    requires random_access_range<Base>
    {
        return right < left;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator<=(iterator const& left,
        iterator const& right) noexcept(noexcept(right < left))
    requires random_access_range<Base>
    {
        return !(right < left);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool operator>=(iterator const& left,
        iterator const& right) noexcept(noexcept(left < right))
    requires random_access_range<Base>
    {
        return !(left < right);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr auto
    operator<=>(iterator const& left, iterator const& right) noexcept(
        noexcept(std::declval<iterator_t<Base> const&>() <=>
            std::declval<iterator_t<Base> const&>()))
    requires random_access_range<Base> &&
        std::three_way_comparable<iterator_t<Base>>
    {
        return left.current_ <=> right.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator
    operator+(iterator const& iter, difference_type offset) noexcept(
        std::is_nothrow_copy_constructible_v<iterator_t<Base>>&& noexcept(
            std::declval<iterator&>() += offset))
    requires random_access_range<Base>
    {
        return iterator{iter} += offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator operator+(difference_type offset,
        iterator const& iter) noexcept(noexcept(iter + offset))
    requires random_access_range<Base>
    {
        return iter + offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr iterator
    operator-(iterator const& iter, difference_type offset) noexcept(
        std::is_nothrow_copy_constructible_v<iterator_t<Base>>&& noexcept(
            std::declval<iterator&>() -= offset))
    requires random_access_range<Base>
    {
        return iterator{iter} -= offset;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type
    operator-(iterator const& left, iterator const& right) noexcept(
        noexcept(left.current_ - right.current_))
    requires std::sized_sentinel_for<iterator_t<Base>, iterator_t<Base>>
    {
        return left.current_ - right.current_;
    }

private:
    iterator_t<Base> current_{};
};

template <input_range V, size_t I>
requires view<V> && details::has_tuple_element<range_value_t<V>, I> &&
    details::has_tuple_element<std::remove_reference_t<range_reference_t<V>>,
        I> &&
    details::returnable_element<range_reference_t<V>, I>
template <bool Const>
class elements_view<V, I>::sentinel {
    friend elements_view;
    using Base RXX_NODEBUG = details::const_if<Const, V>;
    template <bool Any>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr auto get_current(iterator<Any> const& iter) noexcept
        -> decltype(auto) {
        return (iter.current_);
    }

public:
    __RXX_HIDE_FROM_ABI constexpr sentinel() noexcept(
        std::is_nothrow_default_constructible_v<sentinel_t<Base>>) = default;

    __RXX_HIDE_FROM_ABI constexpr explicit sentinel(
        sentinel_t<Base> end) noexcept(std::
            is_nothrow_move_constructible_v<sentinel_t<Base>>)
        : end_(std::move(end)) {}

    __RXX_HIDE_FROM_ABI constexpr sentinel(sentinel<!Const> other) noexcept(
        std::is_nothrow_convertible_v<sentinel_t<V>, sentinel_t<Base>>)
    requires Const && std::convertible_to<sentinel_t<V>, sentinel_t<Base>>
        : end_(std::move(other.end_)) {}

    __RXX_HIDE_FROM_ABI constexpr sentinel_t<Base> base() const
        noexcept(std::is_nothrow_copy_constructible_v<sentinel_t<Base>>) {
        return end_;
    }

    template <bool OtherConst>
    requires std::sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) friend constexpr bool
    operator==(iterator<OtherConst> const& iter, sentinel const& sent) noexcept(
        noexcept(get_current(iter) == sent.end_)) {
        return get_current(iter) == sent.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, V>>
    operator-(iterator<OtherConst> const& iter, sentinel const& sent) noexcept(
        noexcept(get_current(iter) - sent.end_)) {
        return get_current(iter) - sent.end_;
    }

    template <bool OtherConst>
    requires std::sized_sentinel_for<sentinel_t<Base>,
        iterator_t<details::const_if<OtherConst, V>>>
    RXX_ATTRIBUTES(
        _HIDE_FROM_ABI, NODISCARD) friend constexpr range_difference_t<details::
            const_if<OtherConst, V>>
    operator-(sentinel const& sent, iterator<OtherConst> const& iter) noexcept(
        noexcept(sent.end_ - get_current(iter))) {
        return sent.end_ - get_current(iter);
    }

private:
    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS) sentinel_t<Base> end_{};
};

template <typename R>
using keys_view = elements_view<R, 0>;
template <typename R>
using values_view = elements_view<R, 1>;

namespace views {
namespace details {

template <size_t N>
struct elements_t : ranges::details::adaptor_closure<elements_t<N>> {
    template <typename R>
    requires requires { elements_view<all_t<R>, N>{std::declval<R>()}; }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(R&& range) RXX_CONST_CALL
        noexcept(noexcept(elements_view<all_t<R>, N>(std::forward<R>(range))))
            -> decltype(elements_view<all_t<R>, N>(std::forward<R>(range))) {
        return elements_view<all_t<R>, N>(std::forward<R>(range));
    }

#if RXX_LIBSTDCXX
    static constexpr bool _S_has_simple_call_op = true;
#endif
};

} // namespace details

inline namespace cpo {
template <size_t I>
inline constexpr details::elements_t<I> elements{};
inline constexpr details::elements_t<0> keys{};
inline constexpr details::elements_t<1> values{};
} // namespace cpo
} // namespace views

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END

template <typename T, size_t I>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::elements_view<T, I>> =
        std::ranges::enable_borrowed_range<T>;
