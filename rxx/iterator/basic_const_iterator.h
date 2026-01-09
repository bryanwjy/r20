// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/iterator/iter_traits.h"
#include "rxx/utility/forward.h"
#include "rxx/utility/move.h"

#include <concepts>
#include <iterator>
#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

template <std::input_iterator It>
class basic_const_iterator;

namespace details {
template <typename It>
struct basic_const_iterator_category {};

template <std::forward_iterator It>
struct basic_const_iterator_category<It> {
    using iterator_category =
        typename std::iterator_traits<It>::iterator_category;
};

template <typename T>
inline constexpr bool is_const_iterator = false;
template <std::input_iterator It>
inline constexpr bool is_const_iterator<basic_const_iterator<It>> = true;

template <typename T>
concept not_const_iterator = !is_const_iterator<T>;

template <typename It>
concept constant_iterator = std::input_iterator<It> &&
    std::same_as<iter_const_reference_t<It>, iter_reference_t<It>>;

template <typename T, typename U>
concept different_from =
    !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
} // namespace details

template <std::input_iterator It>
class basic_const_iterator : public details::basic_const_iterator_category<It> {
    using reference = iter_const_reference_t<It>;
    using rvalue_reference = iter_const_rvalue_reference_t<It>;

    template <std::input_iterator>
    friend class basic_const_iterator;

    static consteval auto make_iterator_concept() noexcept {
        if constexpr (std::contiguous_iterator<It>) {
            return std::contiguous_iterator_tag{};
        } else if constexpr (std::random_access_iterator<It>) {
            return std::random_access_iterator_tag{};
        } else if constexpr (std::bidirectional_iterator<It>) {
            return std::bidirectional_iterator_tag{};
        } else if constexpr (std::forward_iterator<It>) {
            return std::forward_iterator_tag{};
        } else {
            return std::input_iterator_tag{};
        }
    }

public:
    using iterator_concept = decltype(make_iterator_concept());
    using value_type = std::iter_value_t<It>;
    using difference_type = std::iter_difference_t<It>;

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator() noexcept(
        std::is_nothrow_default_constructible_v<It>)
    requires std::default_initializable<It>
    = default;

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator(It other) noexcept(
        std::is_nothrow_move_constructible_v<It>)
        : current_{__RXX move(other)} {}

    template <std::convertible_to<It> U>
    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator(basic_const_iterator<U>
            other) noexcept(std::is_nothrow_constructible_v<It, U>)
        : current_{__RXX move(other.current_)} {}

    template <details::different_from<basic_const_iterator> T>
    requires std::convertible_to<T, It>
    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator(T&& other) noexcept(
        std::is_nothrow_constructible_v<It, T>)
        : current_{__RXX forward<T>(other)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr It base() const& noexcept(
        std::is_nothrow_copy_constructible_v<It>)
    requires std::copy_constructible<It>
    {
        return current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr It base() && noexcept(std::is_nothrow_move_constructible_v<It>) {
        return __RXX move(current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr reference operator*() const
        noexcept(noexcept(static_cast<reference>(*current_))) {
        return static_cast<reference>(*current_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto const* operator->() const
        noexcept(std::contiguous_iterator<It> || noexcept(*current_))
    requires std::is_lvalue_reference_v<iter_reference_t<It>> &&
        std::same_as<std::remove_cvref_t<iter_reference_t<It>>, value_type>
    {
        if constexpr (std::contiguous_iterator<It>) {
            return std::to_address(current_);
        } else {
            return RXX_BUILTIN_addressof(*current_);
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr reference operator[](difference_type n) const
        noexcept(noexcept(static_cast<reference>(current_[n])))
    requires std::random_access_iterator<It>
    {
        return static_cast<reference>(current_[n]);
    }

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator& operator++() noexcept(
        noexcept(++current_)) {
        ++current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr void operator++(int) noexcept(
        noexcept(++current_)) {
        ++current_;
    }

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator operator++(int) noexcept(
        noexcept(++*this) &&
        std::is_nothrow_copy_constructible_v<basic_const_iterator>)
    requires std::forward_iterator<It>
    {
        auto prev = *this;
        ++*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator& operator--() noexcept(
        noexcept(--current_))
    requires std::bidirectional_iterator<It>
    {
        --current_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator operator--(int) noexcept(
        noexcept(--*this) &&
        std::is_nothrow_copy_constructible_v<basic_const_iterator>)
    requires std::bidirectional_iterator<It>
    {
        auto prev = *this;
        --*this;
        return prev;
    }

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator& operator+=(
        difference_type offset) noexcept(noexcept(current_ += offset))
    requires std::random_access_iterator<It>
    {
        current_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr basic_const_iterator& operator-=(
        difference_type offset) noexcept(noexcept(current_ -= offset))
    requires std::random_access_iterator<It>
    {
        current_ -= offset;
        return *this;
    }

    template <std::sentinel_for<It> S>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator==(S const& other) const
        noexcept(noexcept(current_ == other)) {
        return current_ == other;
    }

    template <details::not_const_iterator Ot>
    requires details::constant_iterator<Ot> && std::convertible_to<It, Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr operator Ot() const& noexcept(
        noexcept(static_cast<Ot>(current_))) {
        return static_cast<Ot>(current_);
    }

    template <details::not_const_iterator Ot>
    requires details::constant_iterator<Ot> && std::convertible_to<It, Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr operator Ot() && noexcept(
        noexcept(static_cast<Ot>(__RXX move(current_)))) {
        return static_cast<Ot>(__RXX move(current_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator<(basic_const_iterator const& other) const
        noexcept(noexcept(current_ < other.current_))
    requires std::random_access_iterator<It>
    {
        return current_ < other.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator>(basic_const_iterator const& other) const
        noexcept(noexcept(current_ > other.current_))
    requires std::random_access_iterator<It>
    {
        return current_ > other.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator<=(basic_const_iterator const& other) const
        noexcept(noexcept(current_ <= other.current_))
    requires std::random_access_iterator<It>
    {
        return current_ <= other.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator>=(basic_const_iterator const& other) const
        noexcept(noexcept(current_ >= other.current_))
    requires std::random_access_iterator<It>
    {
        return current_ >= other.current_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator<=>(basic_const_iterator const& other) const
        noexcept(noexcept(current_ <=> other.current_))
    requires std::random_access_iterator<It> && std::three_way_comparable<It>
    {
        return current_ <=> other.current_;
    }

    template <details::different_from<basic_const_iterator> Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator<(Ot const& other) const
        noexcept(noexcept(current_ < other))
    requires std::random_access_iterator<It> &&
        std::totally_ordered_with<It, Ot>
    {
        return current_ < other;
    }

    template <details::different_from<basic_const_iterator> Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator>(Ot const& other) const
        noexcept(noexcept(current_ > other))
    requires std::random_access_iterator<It> &&
        std::totally_ordered_with<It, Ot>
    {
        return current_ > other;
    }

    template <details::different_from<basic_const_iterator> Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator<=(Ot const& other) const
        noexcept(noexcept(current_ <= other))
    requires std::random_access_iterator<It> &&
        std::totally_ordered_with<It, Ot>
    {
        return current_ <= other;
    }

    template <details::different_from<basic_const_iterator> Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator>=(Ot const& other) const
        noexcept(noexcept(current_ >= other))
    requires std::random_access_iterator<It> &&
        std::totally_ordered_with<It, Ot>
    {
        return current_ >= other;
    }

    template <details::different_from<basic_const_iterator> Ot>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator<=>(Ot const& other) const
        noexcept(noexcept(current_ <=> other))
    requires std::random_access_iterator<It> &&
        std::totally_ordered_with<It, Ot> &&
        std::three_way_comparable_with<It, Ot>
    {
        return current_ <=> other;
    }

#if RXX_COMPILER_GCC | RXX_COMPILER_CLANG_AT_LEAST(21, 0, 0)
    // Infinite meta-recursion fix for GCC
    template <details::not_const_iterator L, std::same_as<It> It2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator<(L const& left, basic_const_iterator<It2> const& right) noexcept(
        noexcept(left < right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left < right.current_;
    }

    template <details::not_const_iterator L, std::same_as<It> It2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator>(L const& left, basic_const_iterator<It2> const& right) noexcept(
        noexcept(left > right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left > right.current_;
    }

    template <details::not_const_iterator L, std::same_as<It> It2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator<=(L const& left, basic_const_iterator<It2> const& right) noexcept(
        noexcept(left <= right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left <= right.current_;
    }

    template <details::not_const_iterator L, std::same_as<It> It2>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator>=(L const& left, basic_const_iterator<It2> const& right) noexcept(
        noexcept(left >= right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left >= right.current_;
    }

#else
    template <details::not_const_iterator L>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator<(L const& left, basic_const_iterator const& right) noexcept(
        noexcept(left < right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left < right.current_;
    }

    template <details::not_const_iterator L>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator>(L const& left, basic_const_iterator const& right) noexcept(
        noexcept(left > right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left > right.current_;
    }

    template <details::not_const_iterator L>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator<=(L const& left, basic_const_iterator const& right) noexcept(
        noexcept(left <= right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left <= right.current_;
    }

    template <details::not_const_iterator L>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr bool
    operator>=(L const& left, basic_const_iterator const& right) noexcept(
        noexcept(left >= right.current_))
    requires std::random_access_iterator<It> && std::totally_ordered_with<It, L>
    {
        return left >= right.current_;
    }
#endif

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr basic_const_iterator operator+(
        basic_const_iterator const& self,
        difference_type
            offset) noexcept(noexcept(basic_const_iterator(self.current_ +
        offset)))
    requires std::random_access_iterator<It>
    {
        return basic_const_iterator(self.current_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr basic_const_iterator operator+(difference_type offset,
        basic_const_iterator const&
            self) noexcept(noexcept(basic_const_iterator(self.current_ +
        offset)))
    requires std::random_access_iterator<It>
    {
        return basic_const_iterator(self.current_ + offset);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr basic_const_iterator operator-(
        basic_const_iterator const& self,
        difference_type
            offset) noexcept(noexcept(basic_const_iterator(self.current_ -
        offset)))
    requires std::random_access_iterator<It>
    {
        return basic_const_iterator(self.current_ - offset);
    }

    template <std::sized_sentinel_for<It> S>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr difference_type operator-(S const& other) const
        noexcept(noexcept(current_ - other)) {
        return current_ - other;
    }

#if RXX_COMPILER_GCC | RXX_COMPILER_CLANG_AT_LEAST(21, 0, 0)
    // Infinite meta-recursion fix for GCC
    template <details::not_const_iterator S, std::same_as<It> It2>
    requires std::sized_sentinel_for<S, It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type
    operator-(S const& left, basic_const_iterator<It2> const& right) noexcept(
        noexcept(left - right.current_)) {
        return left - right.current_;
    }

#else
    template <details::not_const_iterator S>
    requires std::sized_sentinel_for<S, It>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr difference_type
    operator-(S const& left, basic_const_iterator const& right) noexcept(
        noexcept(left - right.current_)) {
        return left - right.current_;
    }
#endif
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend constexpr rvalue_reference
    iter_move(basic_const_iterator const& self) noexcept(noexcept(
        static_cast<rvalue_reference>(std::ranges::iter_move(self.current_)))) {
        return static_cast<rvalue_reference>(
            std::ranges::iter_move(self.current_));
    }

private:
    It current_{};
};

template <std::input_iterator I>
using const_iterator = std::conditional_t<details::constant_iterator<I>, I,
    basic_const_iterator<I>>;

namespace details {
template <typename S>
struct const_sentinel {
    using type = S;
};

template <std::input_iterator S>
struct const_sentinel<S> {
    using type = const_iterator<S>;
};
} // namespace details

template <std::semiregular S>
using const_sentinel = typename details::const_sentinel<S>::type;

template <std::input_iterator I>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr const_iterator<I> make_const_iterator(I it) noexcept(
    std::is_nothrow_convertible_v<I, const_iterator<I>>) {
    return it;
}
template <std::semiregular S>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr const_sentinel<S> make_const_sentinel(S s) noexcept(
    std::is_nothrow_convertible_v<S, const_sentinel<S>>) {
    return s;
}

RXX_DEFAULT_NAMESPACE_END

template <typename T, std::common_with<T> U>
requires std::input_iterator<std::common_type_t<T, U>>
struct std::common_type<__RXX basic_const_iterator<T>, U> {
    using type RXX_NODEBUG =
        __RXX basic_const_iterator<std::common_type_t<T, U>>;
};

template <typename T, std::common_with<T> U>
requires std::input_iterator<std::common_type_t<T, U>>
struct std::common_type<U, __RXX basic_const_iterator<T>> {
    using type RXX_NODEBUG =
        __RXX basic_const_iterator<std::common_type_t<T, U>>;
};

template <typename T, std::common_with<T> U>
requires std::input_iterator<std::common_type_t<T, U>>
struct std::common_type<__RXX basic_const_iterator<T>,
    __RXX basic_const_iterator<U>> {
    using type RXX_NODEBUG =
        __RXX basic_const_iterator<std::common_type_t<T, U>>;
};
