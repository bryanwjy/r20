// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/simple_view.h"
#include "rxx/iterator.h"
#include "rxx/iterator/move_iterator.h"
#include "rxx/iterator/move_sentinel.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/utility.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <view V>
requires input_range<V>
class as_rvalue_view : public view_interface<as_rvalue_view<V>> {
public:
    __RXX_HIDE_FROM_ABI constexpr as_rvalue_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI constexpr explicit as_rvalue_view(V base) noexcept(
        std::is_nothrow_move_constructible_v<V>)
        : base_{__RXX move(base)} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() const& noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires std::copy_constructible<V>
    {
        return base_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr V base() && noexcept(std::is_nothrow_move_constructible_v<V>) {
        return __RXX move(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin()
    requires (!details::simple_view<V>)
    {
        return move_iterator(__RXX ranges::begin(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires range<V const>
    {
        return move_iterator(__RXX ranges::begin(base_));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        if constexpr (common_range<V>) {
            return move_iterator(__RXX ranges::end(base_));
        } else {
            return move_sentinel(__RXX ranges::end(base_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires range<V const>
    {
        if constexpr (common_range<V const>) {
            return move_iterator(__RXX ranges::end(base_));
        } else {
            return move_sentinel(__RXX ranges::end(base_));
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return ranges::size(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return ranges::size(base_);
    }

private:
    V base_{};
};

template <typename R>
as_rvalue_view(R&&) -> as_rvalue_view<views::all_t<R>>;

namespace views {
namespace details {

struct as_rvalue_t : __RXX ranges::details::adaptor_closure<as_rvalue_t> {
    template <typename R>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& range) RXX_CONST_CALL
        noexcept(noexcept(as_rvalue_view(__RXX forward<R>(range))))
            -> decltype(as_rvalue_view(std::declval<R>())) {
        return as_rvalue_view(__RXX forward<R>(range));
    }

    template <typename R>
    requires std::same_as<range_rvalue_reference_t<R>, range_reference_t<R>> &&
        requires { views::all(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& range) RXX_CONST_CALL
        noexcept(noexcept(views::all(std::declval<R>()))) {
        return views::all(__RXX forward<R>(range));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::as_rvalue_t as_rvalue{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename R>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::as_rvalue_view<R>> =
        std::ranges::enable_borrowed_range<R>;
