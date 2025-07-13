// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/movable_box.h"
#include "rxx/view_interface.h"

#include <concepts>
#include <type_traits>
#include <utility>

#if RXX_ENABLE_STD_INTEROP
#  include <ranges>
#endif

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <std::move_constructible T>
requires std::is_object_v<T>
class single_view : public view_interface<single_view<T>> {

public:
    __RXX_HIDE_FROM_ABI constexpr single_view() noexcept(
        std::is_nothrow_default_constructible_v<T>)
    requires std::default_initializable<T>
    = default;

#if RXX_ENABLE_STD_INTEROP
    __RXX_HIDE_FROM_ABI constexpr single_view(std::ranges::single_view<T> const&
            other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_{std::in_place, *other.data()} {};
#endif

    __RXX_HIDE_FROM_ABI explicit constexpr single_view(T const& value) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
    requires std::copy_constructible<T>
        : value_{value} {}

    __RXX_HIDE_FROM_ABI explicit constexpr single_view(T&& value) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : value_{std::move(value)} {}

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr single_view(std::in_place_t,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : value_{std::in_place, std::forward<Args>(args)...} {}

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr T* data() noexcept {
        return RXX_BUILTIN_addressof(*value_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const* data() const noexcept {
        return RXX_BUILTIN_addressof(*value_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr T* begin() noexcept {
        return data();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const* begin() const noexcept { return data(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr T* end() noexcept {
        return data() + 1;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const* end() const noexcept { return data() + 1; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr bool empty() noexcept { return false; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr size_t size() noexcept { return 1; }

private:
    details::movable_box<T> value_;
};

template <typename T>
single_view(T) -> single_view<T>;

namespace views {
namespace details {
struct single_t : __RXX ranges::details::adaptor_closure<single_t> {
    template <typename T>
    requires requires { single_view<std::decay_t<T>>(std::declval<T>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) RXX_STATIC_CALL constexpr auto
    operator()(T&& value) RXX_CONST_CALL noexcept(
        noexcept(single_view<std::decay_t<T>>(std::forward<T>(value)))) {
        return single_view<std::decay_t<T>>(std::forward<T>(value));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::single_t single{};
}
} // namespace views

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
