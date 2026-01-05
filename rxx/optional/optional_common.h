// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/const_if.h"
#include "rxx/functional/invoke_r.h"
#include "rxx/iterator.h"
#include "rxx/memory/construct_at.h"
#include "rxx/memory/destroy_at.h"
#include "rxx/optional/nullopt.h"
#include "rxx/optional/optional_abi.h" // IWYU pragma: always_keep
#include "rxx/type_traits/reference_constructs_from_temporary.h"
#include "rxx/utility.h"

#include <type_traits>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {
template <typename T>
inline constexpr bool is_optional_v = false;
template <typename T>
inline constexpr bool is_optional_v<T const> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<T volatile> = is_optional_v<T>;
template <typename T>
inline constexpr bool is_optional_v<T const volatile> = is_optional_v<T>;

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

template <typename T>
struct optional_iteration;

template <typename T, bool Const>
class optional_iterator {};

template <typename Opt, bool Const>
requires (!std::is_lvalue_reference_v<opt_arg_t<Opt>> ||
    (std::is_object_v<std::remove_reference_t<opt_arg_t<Opt>>> &&
        !std::is_unbounded_array_v<std::remove_reference_t<opt_arg_t<Opt>>>))
class optional_iterator<Opt, Const> {
    friend optional_iteration<Opt>;
    using T = opt_arg_t<Opt>;

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
        optional_iterator<T, Other> other) noexcept
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

template <typename T>
struct optional_iteration {
    void begin() const = delete;
    void end() const = delete;
};

template <typename Opt>
requires is_optional_v<Opt> && (!std::is_lvalue_reference_v<opt_arg_t<Opt>>)
struct optional_iteration<Opt> {
private:
    using T = opt_arg_t<Opt>;

public:
    using iterator = optional_iterator<Opt, false>;
    using const_iterator = optional_iterator<Opt, true>;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() noexcept {
        auto* self = static_cast<Opt*>(this);
        return iterator(RXX_BUILTIN_addressof(**self));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const noexcept {
        auto* self = static_cast<Opt const*>(this);
        return const_iterator(RXX_BUILTIN_addressof(**self));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() noexcept {
        auto* self = static_cast<Opt*>(this);
        return begin() + static_cast<int>(self->has_value());
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept {
        auto* self = static_cast<Opt const*>(this);
        return begin() + static_cast<int>(self->has_value());
    }

protected:
    __RXX_HIDE_FROM_ABI constexpr optional_iteration() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional_iteration() noexcept = default;
};

template <typename Opt>
requires is_optional_v<Opt> && std::is_lvalue_reference_v<opt_arg_t<Opt>> &&
    (std::is_object_v<std::remove_reference_t<opt_arg_t<Opt>>> &&
        !std::is_unbounded_array_v<std::remove_reference_t<opt_arg_t<Opt>>>)
struct optional_iteration<Opt> {
private:
    using T = std::remove_reference_t<opt_arg_t<Opt>>;

public:
    using iterator = optional_iterator<Opt, std::is_const_v<T>>;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto begin() const noexcept {
        auto* self = static_cast<Opt const*>(this);
        return iterator(
            self->has_value() ? RXX_BUILTIN_addressof(**self) : nullptr);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr auto end() const noexcept {
        auto* self = static_cast<Opt const*>(this);
        return begin() + static_cast<int>(self->has_value());
    }

protected:
    __RXX_HIDE_FROM_ABI constexpr optional_iteration() noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr ~optional_iteration() noexcept = default;
};

template <typename T, template <typename> class Union>
class optional_storage {
    template <typename, template <typename> class>
    friend class optional_storage;
    using union_type = Union<T>;
    static constexpr bool place_flag_in_tail =
        __RXX details::fits_in_tail_padding_v<union_type, bool>;
    static constexpr bool allow_external_overlap = !place_flag_in_tail;

    struct container {
        template <typename... Args>
        requires std::is_constructible_v<T, Args...>
        __RXX_HIDE_FROM_ABI explicit constexpr container(std::in_place_t tag,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
            Args...>)
            : union_(tag, tag, __RXX forward<Args>(args)...)
            , has_value_(true) {}

        template <typename F, typename... Args>
        requires std::is_constructible_v<union_type, generating_t, F, Args...>
        __RXX_HIDE_FROM_ABI explicit constexpr container(generating_t gen,
            F&& func,
            Args&&... args) noexcept(std::is_nothrow_constructible_v<union_type,
            generating_t, F, Args...>)
            : union_(std::in_place, gen, __RXX forward<F>(func),
                  __RXX forward<Args>(args)...)
            , has_value_(true) {}

        __RXX_HIDE_FROM_ABI explicit constexpr container(nullopt_t opt) noexcept
            : union_(std::in_place, opt)
            , has_value_(false) {}

        template <typename U>
        __RXX_HIDE_FROM_ABI explicit constexpr container(dispatch_opt_t,
            bool has_value,
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

        template <typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(std::in_place_t tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
            has_value_ = true;
        }

        template <typename... Args>
        __RXX_HIDE_FROM_ABI constexpr void
        construct_union(generating_t tag, Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>)
        requires (allow_external_overlap)
        {
            __RXX construct_at(RXX_BUILTIN_addressof(union_.data), tag,
                __RXX forward<Args>(args)...);
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

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(std::in_place_t tag,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : container_(tag, tag, __RXX forward<Args>(args)...) {}

    template <typename U, typename... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(std::in_place_t tag,
        std::initializer_list<U> list,
        Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
        std::initializer_list<U>&, Args...>)
        : container_(tag, tag, list, __RXX forward<Args>(args)...) {}

    template <typename F, typename... Args>
    requires generatable_from<T, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(generating_t tag,
        F&& func,
        Args&&... args) noexcept(nothrow_generatable_from<T, F, Args...>)
        : container_(std::in_place, tag, __RXX forward<F>(func),
              __RXX forward<Args>(args)...) {}

    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage const&) = delete;
    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage const&) noexcept
    requires std::is_trivially_copy_constructible_v<T> &&
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
        std::is_trivially_move_assignable_v<T> &&
        std::is_trivially_destructible_v<T>
    = default;
    __RXX_HIDE_FROM_ABI constexpr optional_storage&
    operator=(optional_storage&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_assignable_v<T> &&
        (!std::is_trivially_move_assignable_v<T> ||
            !std::is_trivially_destructible_v<T>)
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

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    __RXX_HIDE_FROM_ABI constexpr T& construct(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(
                ptr, std::in_place, __RXX forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                std::in_place, __RXX forward<Args>(args)...);
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
            __RXX construct_at(
                ptr, std::in_place, list, __RXX forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(
                std::in_place, list, __RXX forward<Args>(args)...);
        }
        return data_ref();
    }

    template <typename F, typename... Args>
    requires std::is_invocable_v<F, Args...> &&
        requires { T{std::invoke(std::declval<F>(), std::declval<Args>()...)}; }
    __RXX_HIDE_FROM_ABI constexpr T& dispatch_construct(F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...> &&
        noexcept(T{static_cast<std::invoke_result_t<F, Args...> (*)()>(0)()})) {
        if constexpr (place_flag_in_tail) {
            auto* ptr = RXX_BUILTIN_addressof(container_.data);
            __RXX destroy_at(ptr);
            __RXX construct_at(ptr, std::in_place, generating,
                __RXX forward<F>(func), __RXX forward<Args>(args)...);
        } else {
            container_.data.destroy_union();
            container_.data.construct_union(generating, __RXX forward<F>(func),
                __RXX forward<Args>(args)...);
        }
        return data_ref();
    }

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union>&&
            other) noexcept(std::is_nothrow_constructible_v<container,
        dispatch_opt_t, bool, Union<U>>)
    requires (allow_external_overlap)
        : container_(std::in_place, tag, other.engaged(),
              __RXX move(other.union_ref())) {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union> const&
            other) noexcept(std::is_nothrow_constructible_v<container,
        dispatch_opt_t, bool, Union<U> const&>)
    requires (allow_external_overlap)
        : container_(std::in_place, tag, other.engaged(), other.union_ref()) {}

    template <typename U>
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
    __RXX_HIDE_FROM_ABI constexpr optional_storage(dispatch_opt_t tag,
        optional_storage<U, Union> const&
            other) noexcept(noexcept(make_container(true,
        std::declval<Union<U> const&>())))
    requires (place_flag_in_tail)
        : container_(generating, [&]() {
            return make_container(other.engaged(), other.union_ref());
        }) {}

    RXX_ATTRIBUTE(NO_UNIQUE_ADDRESS)
    overlappable_if<allow_external_overlap, container> container_;
};

template <typename T, template <typename> class Unused>
class optional_storage<T&, Unused> {
#if RXX_SUPPORTS_reference_constructs_from_temporary
    template <typename, template <typename> class>
    friend class optional_storage;

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
        : data_(RXX_BUILTIN_addressof(arg)) {
        static_assert(!__RXX reference_constructs_from_temporary_v<T, U>);
    }

    template <typename F, typename... Args>
    requires std::is_invocable_r_v<T&, F, Args...>
    __RXX_HIDE_FROM_ABI explicit constexpr optional_storage(generating_t tag,
        F&& func,
        Args&&... args) noexcept(std::is_nothrow_invocable_r_v<T&, F, Args...>)
        : data_(RXX_BUILTIN_addressof(__RXX invoke_r<T&>(
              __RXX forward<F>(func), __RXX forward<Args>(args)...))) {
        static_assert(!__RXX reference_constructs_from_temporary_v<T,
            std::invoke_result_t<F, Args...>>);
    }

    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage const&) noexcept = default;
    __RXX_HIDE_FROM_ABI constexpr optional_storage& operator=(
        optional_storage&&) noexcept = default;

protected:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T& data_ref() const& noexcept { return *data_; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr T&& data_ref() const&& noexcept {
        return __RXX forward<T>(*data_);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD, ALWAYS_INLINE)
    constexpr bool engaged() const noexcept { return data_ != nullptr; }

    __RXX_HIDE_FROM_ABI constexpr void disengage() noexcept { data_ = nullptr; }

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr T& construct(U&& arg) noexcept {
        data_ = RXX_BUILTIN_addressof(arg);
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
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused> const& u) noexcept
        : data_(u.data_) {}

    template <typename U>
    __RXX_HIDE_FROM_ABI constexpr optional_storage(
        dispatch_opt_t, optional_storage<U, Unused>&& u) noexcept
        : data_(u.data_) {
        u.disengage();
    }

    T* data_;
#else
    static_assert(!std::is_reference_v<std::add_lvalue_reference_t<T>>);
#endif
};
} // namespace details

RXX_DEFAULT_NAMESPACE_END

#define RXX_SUPPORTS_OPTIONAL_REFERENCES \
    RXX_SUPPORTS_reference_constructs_from_temporary
