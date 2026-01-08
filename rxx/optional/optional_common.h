// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/optional/fwd.h" // IWYU pragma: always_keep

#include "rxx/concepts/core_convertible_to.h"
#include "rxx/concepts/generatable.h"
#include "rxx/configuration/builtins.h"
#include "rxx/details/const_if.h"
#include "rxx/functional/invoke_r.h"
#include "rxx/iterator.h"
#include "rxx/memory/construct_at.h"
#include "rxx/memory/destroy_at.h"
#include "rxx/optional/bad_optional_access.h"
#include "rxx/optional/nullopt.h"
#include "rxx/type_traits/copy_cvref.h"
#include "rxx/type_traits/reference_constructs_from_temporary.h"
#include "rxx/utility.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename T, typename U>
__RXX_HIDE_FROM_ABI constexpr T make_from_union(
    bool has_value, U&& arg) noexcept(std::is_nothrow_constructible_v<T,
    std::in_place_t, decltype(std::declval<U>().get_value())>) {
    return has_value ? T(std::in_place, __RXX forward_like<U>(arg.get_value()))
                     : T(nullopt);
}

struct make_optional_barrier {};

struct dispatch_opt_t {
    __RXX_HIDE_FROM_ABI explicit constexpr dispatch_opt_t() noexcept = default;
};

__RXX_HIDE_FROM_ABI inline constexpr dispatch_opt_t dispatch_opt{};

template <typename T>
struct opt_arg;
template <typename T>
using opt_arg_t = typename opt_arg<T>::type;
template <template <typename> class Opt, typename T>
requires is_optional_v<Opt<T>>
struct opt_arg<Opt<T>> {
    using type = T;
};

template <typename T, typename U>
struct rebind_optional;
template <typename T, typename U>
using rebind_optional_t = typename rebind_optional<T, U>::type;
template <template <typename> class Opt, typename T, typename U>
requires is_optional_v<Opt<T>>
struct rebind_optional<Opt<T>, U> {
    using type = Opt<U>;
};

template <template <typename> class Opt, typename T>
struct optional_iteration {
    void begin() const noexcept = delete;
    void end() const noexcept = delete;
};

template <template <typename> class Opt, typename T, bool Const>
class optional_iterator {};

template <template <typename> class Opt, typename T, bool Const>
requires (!std::is_lvalue_reference_v<T> ||
    (std::is_object_v<std::remove_reference_t<T>> &&
        !std::is_unbounded_array_v<std::remove_reference_t<T>>))
class optional_iterator<Opt, T, Const> {
    friend optional_iteration<Opt, T>;

private:
    __RXX_HIDE_FROM_ABI constexpr optional_iterator(
        std::add_pointer_t<details::const_if<Const, T>> ptr) noexcept
        : ptr_(ptr) {}

public:
    using value_type = std::conditional_t<std::is_lvalue_reference_v<T>,
        std::remove_cvref_t<T>, std::remove_cv_t<T>>;
    using pointer = std::add_pointer_t<details::const_if<Const, T>>;
    using reference = details::const_if<Const, T>&;
    using difference_type = std::pointer_traits<pointer>::difference_type;
    using iterator_concept = std::contiguous_iterator_tag;

    __RXX_HIDE_FROM_ABI constexpr optional_iterator() noexcept : ptr_() {}
    __RXX_HIDE_FROM_ABI constexpr optional_iterator(
        optional_iterator const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional_iterator(
        optional_iterator&&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional_iterator& operator=(
        optional_iterator const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional_iterator& operator=(
        optional_iterator&&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional_iterator() noexcept = default;

    template <bool Other>
    requires (!Other && Const)
    __RXX_HIDE_FROM_ABI constexpr optional_iterator(
        optional_iterator<Opt, T, Other> other) noexcept
        : ptr_(other.ptr_) {}

    RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
    constexpr reference operator*() const noexcept { return *ptr_; }

    RXX_ATTRIBUTES(NODISCARD, ALWAYS_INLINE, _HIDE_FROM_ABI)
    constexpr pointer operator->() const noexcept { return ptr_; }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    friend inline constexpr optional_iterator operator+(
        difference_type offset, optional_iterator const& it) noexcept {
        return optional_iterator(it.ptr_ + offset);
    }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    constexpr auto operator+(difference_type offset) const noexcept {
        return optional_iterator(ptr_ + offset);
    }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    constexpr auto operator-(difference_type offset) const noexcept {
        return optional_iterator(ptr_ - offset);
    }

    RXX_ATTRIBUTES(NODISCARD, _HIDE_FROM_ABI)
    friend constexpr difference_type operator-(optional_iterator const& left,
        optional_iterator const& right) noexcept {
        return left.ptr_ - right.ptr_;
    }

    __RXX_HIDE_FROM_ABI
    constexpr optional_iterator& operator+=(difference_type offset) noexcept {
        ptr_ += offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr optional_iterator& operator-=(difference_type offset) noexcept {
        ptr_ -= offset;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr optional_iterator& operator++() noexcept {
        ++ptr_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr optional_iterator operator++(int) noexcept {
        optional_iterator before = *this;
        ++ptr_;
        return before;
    }

    __RXX_HIDE_FROM_ABI
    constexpr optional_iterator& operator--() noexcept {
        --ptr_;
        return *this;
    }

    __RXX_HIDE_FROM_ABI
    constexpr optional_iterator operator--(int) noexcept {
        auto before = *this;
        --ptr_;
        return before;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr reference operator[](difference_type offset) const noexcept {
        return ptr_[offset];
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool operator==(optional_iterator const& right) const noexcept {
        return ptr_ == right.ptr_;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto operator<=>(optional_iterator const& right) const noexcept {
        return ptr_ <=> right.ptr_;
    }

private:
    pointer ptr_;
};

template <template <typename> class Opt, typename T>
requires is_optional_v<Opt<T>> && (!std::is_reference_v<T>)
struct optional_iteration<Opt, T> {
    using iterator = optional_iterator<Opt, T, false>;
    using const_iterator = optional_iterator<Opt, T, true>;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() noexcept {
        auto* self = static_cast<Opt<T>*>(this);
        return iterator(RXX_BUILTIN_addressof(**self));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const noexcept {
        auto* self = static_cast<Opt<T> const*>(this);
        return const_iterator(RXX_BUILTIN_addressof(**self));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() noexcept {
        auto* self = static_cast<Opt<T>*>(this);
        return begin() + static_cast<int>(self->has_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept {
        auto* self = static_cast<Opt<T> const*>(this);
        return begin() + static_cast<int>(self->has_value());
    }

protected:
    __RXX_HIDE_FROM_ABI constexpr optional_iteration() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional_iteration() noexcept = default;
};

template <template <typename> class Opt, typename T>
requires is_optional_v<Opt<T>> &&
    (std::is_object_v<T> && !std::is_unbounded_array_v<T>)
struct optional_iteration<Opt, T&> {
    using iterator = optional_iterator<Opt, T&, std::is_const_v<T>>;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const noexcept {
        auto* self = static_cast<Opt<T&> const*>(this);
        return iterator(
            self->has_value() ? RXX_BUILTIN_addressof(**self) : nullptr);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept {
        auto* self = static_cast<Opt<T&> const*>(this);
        return begin() + static_cast<int>(self->has_value());
    }

protected:
    __RXX_HIDE_FROM_ABI constexpr optional_iteration() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional_iteration() noexcept = default;
};

template <typename T, template <typename> class Union>
class optional_storage {
    static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t> &&
        !std::is_same_v<std::remove_cv_t<T>, __RXX nullopt_t>);
    static_assert(std::is_destructible_v<T> && !std::is_rvalue_reference_v<T> &&
        !std::is_array_v<T>);
    template <typename, template <typename> class>
    friend class optional_storage;
    using union_type = Union<T>;
    static constexpr bool place_flag_in_tail =
        __RXX details::fits_in_tail_padding_v<union_type, bool>;
    static constexpr bool allow_external_overlap = !place_flag_in_tail;

    struct container {
        template <std::same_as<std::in_place_t> Tag, typename... Args>
        requires std::is_constructible_v<T, Args...>
        __RXX_HIDE_FROM_ABI explicit constexpr container(
            Tag tag, Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
            Args...>)
            : union_(std::in_place, tag, __RXX forward<Args>(args)...)
            , has_value_(true) {}

        template <std::same_as<generating_t> Tag, typename F, typename... Args>
        requires std::is_constructible_v<union_type, generating_t, F, Args...>
        __RXX_HIDE_FROM_ABI explicit constexpr container(Tag gen, F&& func,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<union_type,
            generating_t, F, Args...>)
            : union_(std::in_place, gen, __RXX forward<F>(func),
                  __RXX forward<Args>(args)...)
            , has_value_(true) {}

        __RXX_HIDE_FROM_ABI explicit constexpr container(nullopt_t opt) noexcept
            : union_(std::in_place, opt)
            , has_value_(false) {}

        template <std::same_as<dispatch_opt_t> Tag, typename U>
        __RXX_HIDE_FROM_ABI explicit constexpr container(Tag, bool has_value,
            U&& u) noexcept(noexcept(make_from_union<union_type>(has_value,
            std::declval<U>())))
        requires (allow_external_overlap)
            : union_(generating,
                  [&]() {
                      return make_from_union<union_type>(
                          has_value, __RXX forward<U>(u));
                  })
            , has_value_(has_value) {}

        __RXX_HIDE_FROM_ABI constexpr container(container const&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container(container const&) noexcept
        requires std::is_copy_constructible_v<T> &&
            std::is_trivially_copy_constructible_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container(container&&) noexcept
        requires std::is_move_constructible_v<T> &&
            std::is_trivially_move_constructible_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container const&) noexcept
        requires std::is_copy_assignable_v<T> &&
            std::is_trivially_copy_assignable_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(
            container&&) = delete;
        __RXX_HIDE_FROM_ABI constexpr container& operator=(container&&) noexcept
        requires std::is_move_assignable_v<T> &&
            std::is_trivially_move_assignable_v<T>
        = default;

        __RXX_HIDE_FROM_ABI constexpr ~container() noexcept
        requires std::is_trivially_destructible_v<T>
        = default;
        __RXX_HIDE_FROM_ABI constexpr ~container() noexcept {
            destroy_member();
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires allow_external_overlap && std::is_trivially_destructible_v<T>
        {
            __RXX destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        __RXX_HIDE_FROM_ABI constexpr void destroy_union() noexcept
        requires allow_external_overlap &&
            (!std::is_trivially_destructible_v<T>)
        {
            destroy_member();
            __RXX destroy_at(RXX_BUILTIN_addressof(union_.data));
        }

        template <std::same_as<std::in_place_t> Tag, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(Tag tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
            has_value_ = true;
        }

        template <std::same_as<generating_t> Tag, typename F, typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(Tag tag, F&& func, Args&&... args) noexcept(
            std::is_nothrow_invocable_r_v<T, F, Args...>)
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<F>(func), __RXX forward<Args>(args)...);
            has_value_ = true;
        }

        __RXX_HIDE_FROM_ABI inline constexpr void construct_union(
            nullopt_t tag) noexcept
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag);
            has_value_ = false;
        }

        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS)
        overlappable_if<place_flag_in_tail, union_type> union_;
        RXX_ATTRIBUTES(NO_UNIQUE_ADDRESS) bool has_value_;

    private:
        __RXX_HIDE_FROM_ABI constexpr void destroy_member() noexcept {
            if (has_value_) {
                __RXX destroy_at(
                    RXX_BUILTIN_addressof(union_.data.get_value()));
            }
        }
    };

    template <typename U>
    __RXX_HIDE_FROM_ABI static constexpr container make_container(
        bool has_value,
        U&& item) noexcept(std::is_nothrow_constructible_v<container,
        std::in_place_t, decltype(std::declval<U>().get_value())>)
    requires (place_flag_in_tail)
    {
        return has_value
            ? container(std::in_place, __RXX forward_like<U>(item.get_value()))
            : container(nullopt);
    }

public:
    __RXX_HIDE_FROM_ABI constexpr ~optional_storage() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr optional_storage() noexcept
        : container_(std::in_place, nullopt) {}

    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        optional_storage const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        optional_storage const&) noexcept
    requires std::is_trivially_copy_constructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_storage(optional_storage const&
            other) noexcept(std::is_nothrow_copy_constructible_v<T>)
    requires (std::is_copy_constructible_v<T> &&
        !std::is_trivially_copy_constructible_v<T>)
        : optional_storage(dispatch_opt, other) {}

    __RXX_HIDE_FROM_ABI constexpr optional_storage(optional_storage&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_storage(optional_storage&&) noexcept
    requires std::is_trivially_move_constructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_storage(optional_storage&&
            other) noexcept(std::is_nothrow_move_constructible_v<T>)
    requires (std::is_move_constructible_v<T> &&
        !std::is_trivially_move_constructible_v<T>)
        : optional_storage(dispatch_opt, __RXX move(other)) {}

    template <std::same_as<std::in_place_t> Tag, typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(Tag tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : container_(tag, tag, __RXX forward<Args>(args)...) {}

    template <std::same_as<generating_t> Tag, typename F, typename... Args>
    requires generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(Tag tag, F&& func,
        Args&&... args) noexcept(nothrow_generatable_from<T, F, Args...>)
        : container_(std::in_place, tag, __RXX forward<F>(func),
              __RXX forward<Args>(args)...) {}

    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage const&) noexcept
    requires std::is_copy_assignable_v<T> &&
        std::is_trivially_copy_constructible_v<T> &&
        std::is_trivially_copy_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;

    __RXX_HIDE_FROM_ABI constexpr optional_storage&
    operator=(optional_storage const& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> &&
        std::is_nothrow_copy_constructible_v<T>)
    requires std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T> &&
        (!(std::is_trivially_copy_constructible_v<T> &&
            std::is_trivially_copy_assignable_v<T> &&
            std::is_trivially_destructible_v<T>))
    {
        if (this == RXX_BUILTIN_addressof(other)) {
            return *this;
        }

        if (this->engaged() != other.engaged()) {
            if (other.engaged()) {
                this->construct(other.data_ref());
            } else {
                this->disengage();
            }
        } else if (other.engaged()) {
            this->data_ref() = other.data_ref();
        }

        return *this;
    }

    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage&&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage&&) noexcept
    requires std::is_move_assignable_v<T> &&
        std::is_trivially_move_constructible_v<T> &&
        std::is_trivially_move_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr optional_storage&
    operator=(optional_storage&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_assignable_v<T> && std::is_move_constructible_v<T> &&
        (!(std::is_trivially_move_constructible_v<T> &&
            std::is_trivially_move_assignable_v<T> &&
            std::is_trivially_destructible_v<T>))
    {
        if (this->engaged() != other.engaged()) {
            if (other.engaged()) {
                this->construct(__RXX move(other.data_ref()));
            } else {
                this->disengage();
            }
        } else if (other.engaged()) {
            this->data_ref() = __RXX move(other.data_ref());
        }

        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& value() const& {
        if (!engaged()) {
            RXX_THROW(bad_optional_access());
        }

        return data_ref();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& value() &
    requires (!std::is_reference_v<T>)
    {
        if (!engaged()) {
            RXX_THROW(bad_optional_access());
        }

        return data_ref();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) value() const&& {
        if (!engaged()) {
            RXX_THROW(bad_optional_access());
        }

        if constexpr (std::is_reference_v<T>) {
            return __RXX forward<T>(data_ref());
        } else {
            return __RXX move(data_ref());
        }
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T&& value() && {
        if (!engaged()) {
            RXX_THROW(bad_optional_access());
        }

        return __RXX move(data_ref());
    }

    template <typename U = std::remove_cv_t<T>>
    requires (!std::is_function_v<T> && !std::is_array_v<T>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) const& noexcept {
        static_assert(
            std::is_copy_constructible_v<T> && std::is_convertible_v<U&&, T>);
        return engaged() ? data_ref()
                         : static_cast<T>(__RXX forward<U>(default_value));
    }

    template <typename U = std::remove_cv_t<T>>
    requires (!std::is_function_v<T> && !std::is_array_v<T>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T value_or(U&& default_value) && noexcept {
        static_assert(
            std::is_move_constructible_v<T> && std::is_convertible_v<U&&, T>);
        return engaged() ? __RXX move(data_ref())
                         : static_cast<T>(__RXX forward<U>(default_value));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T const& operator*() const& noexcept { return data_ref(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& operator*() & noexcept { return data_ref(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr decltype(auto) operator*() const&& noexcept {
        return __RXX move(data_ref());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T&& operator*() && noexcept { return __RXX move(data_ref()); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::add_pointer_t<T const> operator->() const noexcept {
        return RXX_BUILTIN_addressof(data_ref());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::add_pointer_t<T> operator->() noexcept {
        return RXX_BUILTIN_addressof(data_ref());
    }

protected:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto data_ref() const& noexcept -> decltype(auto) {
        return (union_ref().get_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto data_ref() & noexcept -> decltype(auto) {
        return (union_ref().get_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto data_ref() const&& noexcept -> decltype(auto) {
        return __RXX move(union_ref().get_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto data_ref() && noexcept -> decltype(auto) {
        return __RXX move(union_ref().get_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr bool engaged() const noexcept {
        return container_.data.has_value_;
    }

    template <typename U>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE)
    constexpr void assign_value(U&& data) noexcept(
        std::is_nothrow_assignable_v<T&, U>) {
        data_ref() = __RXX forward<U>(data);
    }

    __RXX_HIDE_FROM_ABI constexpr void disengage() noexcept {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(ptr, __RXX nullopt);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(__RXX nullopt);
        }
    }

    __RXX_HIDE_FROM_ABI constexpr void swap(optional_storage& other) noexcept(
        std::is_nothrow_swappable_v<T> &&
        std::is_nothrow_move_constructible_v<T>) {
        if (other.engaged() != this->engaged()) {
            if (other.engaged()) {
                this->construct(__RXX move(other.data_ref()));
                other.disengage();
            } else {
                other.construct(__RXX move(this->data_ref()));
                this->disengage();
            }
        } else if (this->engaged()) {
            __RXX ranges::swap(other.data_ref(), this->data_ref());
        }
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& construct(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            RXX_TRY {
                __RXX construct_at(
                    ptr, std::in_place, __RXX forward<Args>(args)...);
            } RXX_CATCH(...) {
                if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
                    RXX_BUILTIN_unreachable();
                } else {
                    __RXX construct_at(ptr, __RXX nullopt);
                    RXX_RETHROW();
                }
            }
        } else {
            container_.data.destroy_union();
            RXX_TRY {
                container_.data.construct_union(
                    std::in_place, __RXX forward<Args>(args)...);
            } RXX_CATCH(...) {
                if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
                    RXX_BUILTIN_unreachable();
                } else {
                    container_.data.construct_union(__RXX nullopt);
                    RXX_RETHROW();
                }
            }
        }
        return data_ref();
    }

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& construct(std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            RXX_TRY {
                __RXX construct_at(
                    ptr, std::in_place, list, __RXX forward<Args>(args)...);
            } RXX_CATCH(...) {
                if constexpr (std::is_nothrow_constructible_v<T,
                                  std::initializer_list<U>&, Args...>) {
                    RXX_BUILTIN_unreachable();
                } else {
                    __RXX construct_at(ptr, __RXX nullopt);
                    RXX_RETHROW();
                }
            }
        } else {
            container_.data.destroy_union();
            RXX_TRY {
                container_.data.construct_union(
                    std::in_place, list, __RXX forward<Args>(args)...);
            } RXX_CATCH(...) {
                if constexpr (std::is_nothrow_constructible_v<T,
                                  std::initializer_list<U>&, Args...>) {
                    RXX_BUILTIN_unreachable();
                } else {
                    container_.data.construct_union(__RXX nullopt);
                    RXX_RETHROW();
                }
            }
        }
        return data_ref();
    }

    template <typename F, typename... Args>
    requires generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& dispatch_construct(F&& func,
        Args&&... args) noexcept(nothrow_generatable_from<T, F, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            RXX_TRY {
                __RXX construct_at(ptr, generating, __RXX forward<F>(func),
                    __RXX forward<Args>(args)...);
            } RXX_CATCH(...) {
                if constexpr (nothrow_generatable_from<T, F, Args...>) {
                    RXX_BUILTIN_unreachable();
                } else {
                    __RXX construct_at(ptr, __RXX nullopt);
                    RXX_RETHROW();
                }
            }
        } else {
            container_.data.destroy_union();
            RXX_TRY {
                container_.data.construct_union(generating,
                    __RXX forward<F>(func), __RXX forward<Args>(args)...);
            } RXX_CATCH(...) {
                if constexpr (nothrow_generatable_from<T, F, Args...>) {
                    RXX_BUILTIN_unreachable();
                } else {
                    container_.data.construct_union(__RXX nullopt);
                    RXX_RETHROW();
                }
            }
        }
        return data_ref();
    }

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>)
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union>&&
            other) noexcept(std::is_nothrow_constructible_v<container,
        dispatch_opt_t, bool, Union<U>>)
    requires (allow_external_overlap)
        : container_(std::in_place, tag, other.engaged(),
              __RXX move(other.union_ref())) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>)
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union> const&
            other) noexcept(std::is_nothrow_constructible_v<container,
        dispatch_opt_t, bool, Union<U> const&>)
    requires (allow_external_overlap)
        : container_(std::in_place, tag, other.engaged(), other.union_ref()) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>)
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union>&&
            other) noexcept(noexcept(make_container(true,
        std::declval<Union<U>>())))
    requires (place_flag_in_tail)
        : container_(generating, [&]() {
            return make_container(
                other.engaged(), __RXX move(other.union_ref()));
        }) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>)
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union> const&
            other) noexcept(noexcept(make_container(true,
        std::declval<Union<U> const&>())))
    requires (place_flag_in_tail)
        : container_(generating, [&]() {
            return make_container(other.engaged(), other.union_ref());
        }) {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union> const&
            other) noexcept(std::is_nothrow_constructible_v<T, U>)
        : container_(std::in_place, nullopt) {
        if (other.engaged()) {
            construct(other.data_ref());
        }
    }

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() const& noexcept -> decltype(auto) {
        return (container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() & noexcept -> decltype(auto) {
        return (container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() const&& noexcept -> decltype(auto) {
        return __RXX move(container_.data.union_.data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr auto union_ref() && noexcept -> decltype(auto) {
        return __RXX move(container_.data.union_.data);
    }

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};

template <typename T, template <typename> class Unused>
class optional_storage<T&, Unused> {
#if RXX_SUPPORTS_reference_constructs_from_temporary
    template <typename, template <typename> class>
    friend class optional_storage;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE, NODISCARD)
    static constexpr T* addressof(T& arg) { return RXX_BUILTIN_addressof(arg); }

public:
    __RXX_HIDE_FROM_ABI constexpr ~optional_storage() noexcept = default;

    __RXX_HIDE_FROM_ABI constexpr optional_storage() noexcept
        : data_(nullptr) {}
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        optional_storage const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        optional_storage&&) noexcept = default;

    template <typename U>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(
        std::in_place_t tag, U&& arg) noexcept
        : data_(addressof(arg)) {
        static_assert(!__RXX reference_constructs_from_temporary_v<T&, U>);
    }

    template <typename F, typename... Args>
    requires std::is_invocable_r_v<T&, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(generating_t tag,
        F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<T&, F, Args...>)
        : data_(RXX_BUILTIN_addressof(__RXX invoke_r<T&>(
              __RXX forward<F>(func), __RXX forward<Args>(args)...))) {
        static_assert(!__RXX reference_constructs_from_temporary_v<T&,
            std::invoke_result_t<F, Args...>>);
    }

    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage&&) noexcept = default;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& value() const {
        if (!engaged()) {
            RXX_THROW(bad_optional_access());
        }

        return data_ref();
    }

    template <typename U = std::remove_cv_t<T>>
    requires (!std::is_array_v<T> && !std::is_function_v<T>)
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::decay_t<T> value_or(U&& default_value) const {
        using unqualified = std::remove_cv_t<T>;

        static_assert(std::is_convertible_v<T&, unqualified>,
            "optional<T&>::value_or: T& must be move constructible");
        static_assert(std::is_convertible_v<U, unqualified>,
            "optional<T&>::value_or: U must be convertible to T&");
        return engaged() ? data_ref() : __RXX forward<U>(default_value);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& operator*() const noexcept { return data_ref(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T* operator->() const noexcept {
        return RXX_BUILTIN_addressof(data_ref());
    }

protected:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T& data_ref() const& noexcept { return *data_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T&& data_ref() const&& noexcept {
        return __RXX forward<T>(*data_);
    }

    template <typename U>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, ALWAYS_INLINE)
    constexpr void assign_value(U&& data) noexcept {
        data_ = addressof(data);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr bool engaged() const noexcept { return data_ != nullptr; }

    __RXX_HIDE_FROM_ABI constexpr void disengage() noexcept { data_ = nullptr; }

    __RXX_HIDE_FROM_ABI constexpr void swap(optional_storage& other) noexcept {
        if (other.engaged() != this->engaged()) {
            if (other.engaged()) {
                this->data_ = __RXX exchange(other.data_, nullptr);
            } else {
                other.data_ = __RXX exchange(this->data_, nullptr);
            }
        } else if (this->engaged()) {
            this->data_ = __RXX exchange(other.data_, this->data_);
        }
    }

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr T& construct(U&& arg) noexcept {
        data_ = addressof(__RXX forward<U>(arg));
        return data_ref();
    }

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...>
    __RXX_HIDE_FROM_ABI constexpr T&
    dispatch_construct(F&& func, Args&&... args) noexcept(
        std::is_nothrow_invocable_r_v<T&, F, Args...>) {
        static_assert(!__RXX reference_constructs_from_temporary_v<T,
            std::invoke_result_t<F, Args...>>);
        data_ = RXX_BUILTIN_addressof(__RXX invoke_r<T&>(
            __RXX forward<F>(func), __RXX forward<Args>(args)...));
        return data_ref();
    }

    template <typename U>
    requires std::is_lvalue_reference_v<U>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused> const& u) noexcept
        : data_(u.data_) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) && core_convertible_to<U&, T&>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused>& u) noexcept
        : data_(u.engaged() ? addressof(u.data_ref()) : nullptr) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) &&
        core_convertible_to<U const&, T&>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused> const& u) noexcept
        : data_(u.engaged() ? addressof(u.data_ref()) : nullptr) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) && core_convertible_to<U, T&>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused>&& u) noexcept
        : data_(u.engaged() ? addressof(static_cast<U&&>(u.data_ref()))
                            : nullptr) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) &&
        core_convertible_to<U const, T&>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused> const&& u) noexcept
        : data_(u.engaged() ? addressof(static_cast<U const&&>(u.data_ref()))
                            : nullptr) {}

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) && core_convertible_to<U&, T&> &&
        __RXX reference_constructs_from_temporary_v<T&, U&>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused>& u) noexcept = delete;

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) &&
        core_convertible_to<U const&, T&> &&
        __RXX reference_constructs_from_temporary_v<T&, U const&>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused> const& u) noexcept = delete;

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) && core_convertible_to<U, T&> &&
        __RXX reference_constructs_from_temporary_v<T&, U>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused>&& u) noexcept = delete;

    template <typename U>
    requires (!std::is_lvalue_reference_v<U>) &&
        core_convertible_to<U const, T&> &&
        __RXX reference_constructs_from_temporary_v<T&, U const>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t,
        optional_storage<U, Unused> const&& u) noexcept = delete;

    T* data_;
#else
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr T& value() const = delete;

    template <typename U = std::remove_cv_t<T>>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr std::remove_cv_t<T> value_or(U&& default_value) const = delete;
    static_assert(!std::is_reference_v<std::add_lvalue_reference_t<T>>);
#endif
};

template <template <typename> class Opt, typename T>
struct optional_monads {
    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) & {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(is_optional_v<result_type>);
        if (static_cast<Opt<T>*>(this)->has_value()) {
            return std::invoke(
                __RXX forward<F>(func), **static_cast<Opt<T>*>(this));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const& {
        using result_type =
            std::remove_cvref_t<std::invoke_result_t<F, T const&>>;
        static_assert(details::is_optional_v<result_type>);
        if (static_cast<Opt<T> const*>(this)->has_value()) {
            return std::invoke(
                __RXX forward<F>(func), **static_cast<Opt<T> const*>(this));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) && {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T>>;
        static_assert(details::is_optional_v<result_type>);
        if (static_cast<Opt<T>*>(this)->has_value()) {
            return std::invoke(__RXX forward<F>(func),
                __RXX move(**static_cast<Opt<T>*>(this)));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const&& {
        using result_type =
            std::remove_cvref_t<std::invoke_result_t<F, T const>>;
        static_assert(details::is_optional_v<result_type>);
        if (static_cast<Opt<T> const*>(this)->has_value()) {
            return std::invoke(__RXX forward<F>(func),
                __RXX move(**static_cast<Opt<T> const*>(this)));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) & {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        using result_opt = Opt<result_type>;
        if (static_cast<Opt<T>*>(this)->has_value()) {
            return result_opt(generating, __RXX forward<F>(func),
                **static_cast<Opt<T>*>(this));
        } else {
            return result_opt();
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const& {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T const&>>;
        using result_opt = Opt<result_type>;
        if (static_cast<Opt<T> const*>(this)->has_value()) {
            return result_opt(generating, __RXX forward<F>(func),
                **static_cast<Opt<T> const*>(this));
        } else {
            return result_opt();
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) && {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T>>;
        using result_opt = Opt<result_type>;
        if (static_cast<Opt<T>*>(this)->has_value()) {
            return result_opt(generating, __RXX forward<F>(func),
                __RXX move(**static_cast<Opt<T>*>(this)));
        } else {
            return result_opt();
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const&& {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T const>>;
        using result_opt = Opt<result_type>;
        if (static_cast<Opt<T> const*>(this)->has_value()) {
            return result_opt(generating, __RXX forward<F>(func),
                __RXX move(**static_cast<Opt<T> const*>(this)));
        } else {
            return result_opt();
        }
    }

    template <std::invocable F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr Opt<T> or_else(F&& func) &&
    requires std::is_move_constructible_v<T>
    {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(std::same_as<Opt<T>, result_type>);
        if (static_cast<Opt<T>*>(this)->has_value()) {
            return __RXX move(*static_cast<Opt<T>*>(this));
        } else {
            return std::invoke(__RXX forward<F>(func));
        }
    }

    template <std::invocable F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr Opt<T> or_else(F&& func) const&
    requires std::is_copy_constructible_v<T>
    {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(std::same_as<Opt<T>, result_type>);
        if (static_cast<Opt<T> const*>(this)->has_value()) {
            return *static_cast<Opt<T> const*>(this);
        } else {
            return std::invoke(__RXX forward<F>(func));
        }
    }
};

template <template <typename> class Opt, typename T>
struct optional_monads<Opt, T&> {
    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto and_then(F&& func) const {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F, T&>>;
        static_assert(is_optional_v<result_type>);
        if (static_cast<Opt<T&> const*>(this)->has_value()) {
            return std::invoke(
                __RXX forward<F>(func), **static_cast<Opt<T&> const*>(this));
        } else {
            return result_type{};
        }
    }

    template <typename F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto transform(F&& func) const {
        using result_type = std::remove_cv_t<std::invoke_result_t<F, T&>>;
        using result_opt = Opt<result_type>;
        if (static_cast<Opt<T&> const*>(this)->has_value()) {
            return result_opt(generating, __RXX forward<F>(func),
                **static_cast<Opt<T&> const*>(this));
        } else {
            return result_opt();
        }
    }

    template <std::invocable F>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr Opt<T&> or_else(F&& func) const {
        using result_type = std::remove_cvref_t<std::invoke_result_t<F>>;
        static_assert(std::same_as<Opt<T&>, result_type>);
        if (static_cast<Opt<T&> const*>(this)->has_value()) {
            return **static_cast<Opt<T&> const*>(this);
        } else {
            return std::invoke(__RXX forward<F>(func));
        }
    }
};

template <typename T, typename Opt>
concept optional_constructible_from = is_optional_v<std::remove_cvref_t<Opt>> &&
    !std::same_as<T, opt_arg_t<std::remove_cvref_t<Opt>>> &&
    std::is_constructible_v<T,
        copy_cvref_t<Opt, opt_arg_t<std::remove_cvref_t<Opt>>>>;

template <typename T, typename Opt>
concept optional_ref_constructible_from =
    is_optional_v<std::remove_cvref_t<Opt>> &&
    !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<Opt>> &&
    !std::same_as<T, opt_arg_t<std::remove_cvref_t<Opt>>> &&
    std::is_constructible_v<T,
        copy_cvref_t<Opt, opt_arg_t<std::remove_cvref_t<Opt>>>>;

template <typename T, typename Opt>
concept optional_convertible = is_optional_v<Opt> && requires {
    requires !std::is_constructible_v<T, Opt&>;
    requires !std::is_constructible_v<T, Opt const&>;
    requires !std::is_constructible_v<T, Opt&&>;
    requires !std::is_constructible_v<T, Opt const&&>;
    requires !std::is_convertible_v<Opt&, T>;
    requires !std::is_convertible_v<Opt const&, T>;
    requires !std::is_convertible_v<Opt&&, T>;
    requires !std::is_convertible_v<Opt const&&, T>;
};

template <typename T, typename Opt>
concept valid_optional_ctor =
    is_optional_v<std::remove_cvref_t<Opt>> && requires {
        requires !std::is_lvalue_reference_v<T>;
        requires optional_constructible_from<T, Opt> &&
            is_optional_v<rebind_optional_t<std::remove_cvref_t<Opt>, T>> &&
            (std::same_as<std::remove_cv_t<T>, bool> ||
                optional_convertible<T, std::remove_cvref_t<Opt>>);
    } || requires {
        requires std::is_lvalue_reference_v<T>;
        requires optional_ref_constructible_from<T, Opt>;
    };

template <typename U, template <typename> class Opt, typename T>
concept assignable_from_optional_of = is_optional_v<Opt<T>> && requires {
    requires optional_convertible<T, Opt<U>>;
    requires !std::is_assignable_v<T&, Opt<U>&>;
    requires !std::is_assignable_v<T&, Opt<U> const&>;
    requires !std::is_assignable_v<T&, Opt<U>&&>;
    requires !std::is_assignable_v<T&, Opt<U> const&&>;
};

template <typename U, template <typename> class Opt, typename T>
concept value_convertible_to_optional = is_optional_v<Opt<T>> &&
    (!std::same_as<std::remove_cvref_t<U>, std::in_place_t> &&
        !std::same_as<std::remove_cvref_t<U>, Opt<T>> &&
        (!std::same_as<std::remove_cv_t<T>, bool> ||
            !is_optional_v<std::remove_cvref_t<U>>)) &&
    std::is_constructible_v<T, U>;

template <typename T>
concept optional_value =
    (std::is_destructible_v<T> && !std::is_rvalue_reference_v<T> &&
        !std::is_array_v<T>)
#if RXX_SUPPORTS_reference_constructs_from_temporary
    || std::is_lvalue_reference_v<T>
#endif
    ;

#if RXX_SUPPORTS_reference_constructs_from_temporary
template <typename U, template <typename> class Opt, typename T>
concept delete_optional_reference_specialization =
    std::is_lvalue_reference_v<T> &&
    __RXX reference_constructs_from_temporary_v<T, U>;
#endif
} // namespace details

RXX_DEFAULT_NAMESPACE_END
