// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/simple_view.h"
#include "rxx/details/view_traits.h"
#include "rxx/ranges/access.h"
#include "rxx/ranges/all.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/empty_view.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/ref_view.h"
#include "rxx/ranges/view_interface.h"
#include "rxx/utility.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {

template <view V>
requires input_range<V>
class as_const_view : public view_interface<as_const_view<V>> {
public:
    __RXX_HIDE_FROM_ABI constexpr as_const_view() noexcept(
        std::is_nothrow_default_constructible_v<V>)
    requires std::default_initializable<V>
    = default;

    __RXX_HIDE_FROM_ABI explicit constexpr as_const_view(V base) noexcept(
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
        return __RXX ranges::cbegin(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const
    requires range<V const>
    {
        return __RXX ranges::cbegin(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end()
    requires (!details::simple_view<V>)
    {
        return __RXX ranges::cend(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const
    requires range<V const>
    {
        return __RXX ranges::cend(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size()
    requires sized_range<V>
    {
        return __RXX ranges::size(base_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto size() const
    requires sized_range<V const>
    {
        return __RXX ranges::size(base_);
    }

private:
    V base_{};
};

template <typename R>
as_const_view(R&&) -> as_const_view<views::all_t<R>>;

namespace views {
namespace details {
template <typename T>
inline constexpr bool is_constable_ref_view = false;

template <typename R>
inline constexpr bool is_constable_ref_view<ref_view<R>> =
    constant_range<R const>;
#if RXX_SUPPORTS_OPTIONAL_REFERENCES
template <typename T>
inline constexpr bool is_optional_ref = false;
template <typename T>
inline constexpr bool is_optional_ref<__RXX nua::optional<T&>> = true;
template <typename T>
inline constexpr bool is_optional_ref<__RXX gcc::optional<T&>> = true;
template <template <typename> class Opt, typename T>
inline constexpr bool is_optional_ref<Opt<T&>> =
    requires(Opt<T&>* ptr) { ptr->~optional(); };

template <typename>
struct as_const_ref_optional;

template <typename T>
using as_const_ref_optional_t = typename as_const_ref_optional<T>::type;

template <typename T>
struct as_const_ref_optional<__RXX gcc::optional<T&>> {
    using type = __RXX gcc::optional<T const&>;
};
template <typename T>
struct as_const_ref_optional<__RXX nua::optional<T&>> {
    using type = __RXX nua::optional<T const&>;
};
template <template <typename> class Opt, typename T>
struct as_const_ref_optional<Opt<T&>> {
    using type = Opt<T const&>;
};
#endif

struct as_const_t : __RXX ranges::details::adaptor_closure<as_const_t> {
    template <viewable_range R>
    requires requires { as_const_view(std::declval<R>()); }
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    RXX_STATIC_CALL constexpr auto operator()(R&& arg) RXX_CONST_CALL
        noexcept(noexcept(as_const_view(std::declval<R>()))) {
        using Type = std::remove_cvref_t<R>;
        using Element = std::remove_reference_t<range_reference_t<R>>;
        if constexpr (constant_range<all_t<R>>) {
            return views::all(__RXX forward<R>(arg));
        } else if constexpr (__RXX ranges::details::is_empty_view<Type>) {
            return views::empty<Element const>;
        }
#if RXX_SUPPORTS_OPTIONAL_REFERENCES
        else if constexpr (is_optional_ref<Type>) {
            return as_const_ref_optional_t<Type>(arg);
        }
#endif
        else if constexpr (__RXX ranges::details::is_span<Type>) {
            return std::span<Element const, Type::extent>(
                __RXX forward<R>(arg));
        } else if constexpr (is_constable_ref_view<R>) {
            return ref_view(std::as_const(__RXX forward<R>(arg).base()));
        } else if constexpr (std::is_lvalue_reference_v<R> &&
            constant_range<Type const> && !view<Type>) {
            return ref_view(static_cast<Type const&>(arg));
        } else {
            return as_const_view(__RXX forward<R>(arg));
        }
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::as_const_t as_const{};
}
} // namespace views
} // namespace ranges

RXX_DEFAULT_NAMESPACE_END

template <typename R>
inline constexpr bool
    std::ranges::enable_borrowed_range<__RXX ranges::as_const_view<R>> =
        std::ranges::enable_borrowed_range<R>;
