// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#if __has_include(<coroutine>)

#  define RXX_SUPPORTS_GENERATOR 1

#  include "rxx/details/variant_base.h"
#  include "rxx/ranges/elements_of.h"
#  include "rxx/ranges/primitives.h"
#  include "rxx/ranges/view_interface.h"
#  include "rxx/utility/exchange.h"

#  include <coroutine>
#  include <cstdint>
#  include <exception>
#  include <type_traits>

#  if RXX_LIBSTDCXX && __has_include(<bits/allocator.h>) \
      && __has_include(<bits/uses_allocator.h>) \
      && __has_include(<bits/alloc_traits.h>)
#    include <bits/alloc_traits.h>
#    include <bits/allocator.h>
#    include <bits/uses_allocator.h>
#  elif RXX_LIBCXX && __has_include(<__memory/allocator.h>) \
      && __has_include(<__memory/allocator_arg_t.h>) \
      && __has_include(<__memory/allocator_traits.h>)
#    include <__memory/allocator.h>
#    include <__memory/allocator_arg_t.h>
#    include <__memory/allocator_traits.h>
#  elif RXX_MSVC_STL && __has_include(<__msvc_iter_core.hpp>) && __has_include(<xmemory>)
#    include <__msvc_iter_core.hpp>
#    include <xmemory>
#  else
#    include <memory>
#  endif
RXX_STD_NAMESPACE_BEGIN
namespace pmr {
template <typename T>
class polymorphic_allocator;
}
RXX_STD_NAMESPACE_END

RXX_DEFAULT_NAMESPACE_BEGIN

template <typename Ref, typename V = void, typename Allocator = void>
class generator;

namespace details::generator {
template <typename Ref, typename V>
using value_for RXX_NODEBUG =
    std::conditional_t<std::is_void_v<V>, std::remove_cvref_t<Ref>, V>;
template <typename Ref, typename V>
using reference_for RXX_NODEBUG =
    std::conditional_t<std::is_void_v<V>, Ref&&, Ref>;
template <typename Ref, typename V>
using yielded_for RXX_NODEBUG =
    std::conditional_t<std::is_reference_v<reference_for<Ref, V>>,
        reference_for<Ref, V>, reference_for<Ref, V> const&>;

template <typename>
constexpr bool is_specialization = false;
template <typename Ref, typename V, typename A>
constexpr bool is_specialization<__RXX generator<Ref, V, A>> = true;

template <typename Y>
class promise_base {
    static_assert(std::is_reference_v<Y>);
    using qualified_value RXX_NODEBUG = std::remove_reference_t<Y>;
    using value_type RXX_NODEBUG = std::remove_cvref_t<Y>;
    using pointer RXX_NODEBUG = std::add_pointer_t<Y>;
    using handle_type RXX_NODEBUG = std::coroutine_handle<promise_base>;

    template <typename, typename, typename>
    friend class __RXX generator;
    template <typename>
    struct nested_awaiter;
    template <typename>
    friend struct nested_awaiter;
    struct element_awaiter;
    struct final_awaiter;
    class stack_descriptor;

public:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    std::suspend_always initial_suspend() const noexcept { return {}; }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    std::suspend_always yield_value(Y val) noexcept {
        bottom_value() = RXX_BUILTIN_addressof(val);
        return {};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    auto yield_value(qualified_value const& val) noexcept(
        std::is_nothrow_constructible_v<value_type, qualified_value const&>)
    requires (std::is_rvalue_reference_v<Y> &&
        std::constructible_from<value_type, qualified_value const&>)
    {
        return element_awaiter(value_type(val), bottom_value());
    }

    template <typename R2, typename V2, typename A2, typename Unused>
    requires std::same_as<yielded_for<R2, V2>, Y>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) auto yield_value(
        ranges::elements_of<__RXX generator<R2, V2, A2>&&, Unused>
            elements) noexcept {
        return nested_awaiter{std::move(elements.range)};
    }

    template <typename R2, typename V2, typename A2, typename Unused>
    requires std::same_as<yielded_for<R2, V2>, Y>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) auto yield_value(
        ranges::elements_of<__RXX generator<R2, V2, A2>&, Unused>
            elements) noexcept {
        return nested_awaiter{std::move(elements.range)};
    }

    template <ranges::input_range R, typename Alloc>
    requires std::convertible_to<ranges::range_reference_t<R>, Y>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) auto yield_value(
        ranges::elements_of<R, Alloc> elements) {
        auto executor = [](::std::allocator_arg_t, Alloc,
                            ranges::iterator_t<R> it,
                            ranges::sentinel_t<R> sentinel)
            -> __RXX generator<Y, ranges::range_value_t<R>, Alloc> {
            for (; it != sentinel; ++it) {
                co_yield static_cast<Y>(*it);
            }
        };
        return yield_value(ranges::elements_of(
            executor(::std::allocator_arg, elements.allocator,
                ranges::begin(elements.range), ranges::end(elements.range))));
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    final_awaiter final_suspend() noexcept { return {}; }

    __RXX_HIDE_FROM_ABI void unhandled_exception() {
        // Active coroutines are always at the top of the stack.
        // If the active coroutine is also the bottom, then it is alone
        if (auto const alone_in_stack = nested_.is_bottom()) {
            RXX_RETHROW("Unhandled exception");
        } else {
            exception_ = std::current_exception();
        }
    }

    void await_transform() = delete;
    __RXX_HIDE_FROM_ABI void return_void() const noexcept {}

private:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) pointer& bottom_value() noexcept {
        return nested_.bottom_value(*this);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) pointer& value() noexcept {
        return nested_.value(*this);
    }

    stack_descriptor nested_;
    std::exception_ptr exception_;
};

template <typename Y>
class promise_base<Y>::stack_descriptor {
    struct stack_frame {
        handle_type bottom;
        handle_type parent;
    };

    struct bottom_frame {
        handle_type top;
        pointer value = nullptr;
    };
    using stack_type RXX_NODEBUG =
        __RXX ranges::details::variant_base<bottom_frame, stack_frame>;

public:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) bool is_bottom() const noexcept {
        return stack_.index() != template_index_v<stack_frame, stack_type>;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) handle_type& top() noexcept {
        if (stack_.index() == template_index_v<stack_frame, stack_type>) {
            constexpr auto idx = template_index_v<stack_frame, stack_type>;
            return stack_.template value_ref<idx>()
                .bottom.promise()
                .nested_.top();
        } else {
            constexpr auto idx = template_index_v<bottom_frame, stack_type>;
            assert(stack_.index() == idx);
            return stack_.template value_ref<idx>().top;
        }
    }

    __RXX_HIDE_FROM_ABI void push(
        handle_type current, handle_type subyield) noexcept {
        assert(&current.promise().nested_ == this);
        assert(this->top() == current);

        subyield.promise().nested_.jump_in(current, subyield);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    std::coroutine_handle<> pop() noexcept {

        if (stack_.index() == template_index_v<stack_frame, stack_type>) {
            constexpr auto idx = template_index_v<stack_frame, stack_type>;
            auto handle = this->top() = stack_.template value_ref<idx>().parent;
            return handle;
        }

        return std::noop_coroutine();
    }

    __RXX_HIDE_FROM_ABI void jump_in(
        handle_type others, handle_type latest) noexcept {
        assert(&latest.promise().nested_ == this);
        assert(this->is_bottom());
        assert(!this->top());

        auto& others_nested = others.promise().nested_;
        others_nested.top() = latest;

        auto new_bottom = others;
        if (others_nested.stack_.index() ==
            template_index_v<stack_frame, stack_type>) {
            constexpr auto idx = template_index_v<stack_frame, stack_type>;
            new_bottom = others_nested.stack_.template value_ref<idx>().bottom;
        }

        this->stack_ = stack_frame{.bottom = new_bottom, .parent = others};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    pointer& bottom_value(promise_base& current [[maybe_unused]]) noexcept {
        assert(&current.nested_ == this);

        if (stack_.index() == template_index_v<bottom_frame, stack_type>) {
            constexpr auto idx = template_index_v<bottom_frame, stack_type>;
            return stack_.template value_ref<idx>().value;
        }

        constexpr auto idx = template_index_v<stack_frame, stack_type>;
        assert(stack_.index() == idx);
        auto& promise = stack_.template value_ref<idx>().bottom.promise();
        return promise.nested_.value(promise);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    pointer& value(promise_base& current [[maybe_unused]]) noexcept {
        assert(&current.nested_ == this);
        assert((stack_.index() == template_index_v<bottom_frame, stack_type>));
        constexpr auto idx = template_index_v<bottom_frame, stack_type>;
        return stack_.template value_ref<idx>().value;
    }

private:
    stack_type stack_;
};

template <typename Y>
struct promise_base<Y>::final_awaiter {
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    bool await_ready() const noexcept { return false; }

    template <typename P>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    auto await_suspend(std::coroutine_handle<P> handle) noexcept {
        // static_assert(
        //     std::is_pointer_interconvertible_base_of_v<promise_base, P>);
        return handle.promise().nested_.pop();
    }

    __RXX_HIDE_FROM_ABI void await_resume() noexcept {}
};

template <typename Y>
struct promise_base<Y>::element_awaiter {
    value_type value;
    pointer& bottom_value;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool await_ready() const noexcept { return false; }

    template <typename P>
    __RXX_HIDE_FROM_ABI void await_suspend(std::coroutine_handle<P>) noexcept {
        // static_assert(
        //     std::is_pointer_interconvertible_base_of_v<promise_base, P>);
        bottom_value = RXX_BUILTIN_addressof(value);
    }

    __RXX_HIDE_FROM_ABI constexpr void await_resume() const noexcept {}
};

template <typename Y>
template <typename Generator>
struct promise_base<Y>::nested_awaiter {
    static_assert(is_specialization<Generator>);
    static_assert(std::same_as<typename Generator::yielded, Y>);

    __RXX_HIDE_FROM_ABI nested_awaiter(Generator generator) noexcept
        : generator(std::move(generator)) {
        this->generator.activate();
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    constexpr bool await_ready() const noexcept { return false; }

    template <typename P>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<P> handle) noexcept {
        // static_assert(
        //     std::is_pointer_interconvertible_base_of_v<promise_base, P>);

        auto current = handle_type::from_address(handle.address());
        auto subyield =
            handle_type::from_address(this->generator.coroutine_.address());
        handle.promise().nested_.push(current, subyield);
        return subyield;
    }

    __RXX_HIDE_FROM_ABI void await_resume() {
        if (auto exception = generator.coroutine_.promise().exception_) {
            std::rethrow_exception(exception);
        }
    }

    Generator generator;
};

template <typename A>
concept stateless_allocator =
    (std::allocator_traits<A>::is_always_equal::value &&
        std::default_initializable<A>);

struct allocation_block {
    static constexpr auto alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
    alignas(alignment) char data[alignment];

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static constexpr auto count(size_t size) noexcept {
        constexpr auto block = sizeof(allocation_block);
        return (size + block - 1) / block;
    }
};

template <typename A, typename U>
struct rebind {};
template <template <typename...> typename Alloc, typename U, typename T,
    typename... Args>
struct rebind<Alloc<T, Args...>, U> {
    using type = Alloc<U, Args...>;
};
template <typename A, typename U>
requires requires { typename A::template rebind<U>::other; }
struct rebind<A, U> {
    using type = typename A::template rebind<U>::other;
};
template <typename A, typename U>
using rebind_t RXX_NODEBUG = typename rebind<A, U>::type;

template <typename A>
class promise_allocator {
    using allocator_type RXX_NODEBUG = rebind_t<A, allocation_block>;
    using alloc_traits RXX_NODEBUG = std::allocator_traits<allocator_type>;
    static_assert(std::is_pointer_v<typename alloc_traits::pointer>);

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static allocator_type* allocator_address(
        uintptr_t const ptr, size_t const size) noexcept {
        static_assert(!stateless_allocator<allocator_type>);
        auto const address = (ptr + size + alignof(allocator_type) - 1) &
            ~(alignof(allocator_type) - 1);
        return reinterpret_cast<allocator_type*>(address);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static auto allocation_size_bytes(size_t size) noexcept {
        // overallocate to account for padding, no need to be exact
        // allocation_block::count will round up
        return size + alignof(allocator_type) + sizeof(allocator_type);
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static void* allocate(allocator_type alloc, size_t size_bytes) {
        if constexpr (stateless_allocator<allocator_type>) {
            return alloc.allocate(allocation_block::count(size_bytes));
        } else {
            auto const block_count =
                allocation_block::count(allocation_size_bytes(size_bytes));
            auto* const ptr = alloc.allocate(block_count);
            auto const address = reinterpret_cast<uintptr_t>(ptr);
            auto* alloc_ptr = allocator_address(address, size_bytes);
            ::new (alloc_ptr) allocator_type(std::move(alloc));
            return ptr;
        }
    }

public:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    void* operator new(size_t size)
    requires std::default_initializable<allocator_type>
    {
        return allocate({}, size);
    }

    template <typename A2, typename... Args>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    void* operator new(
        size_t size, std::allocator_arg_t, A2 const& alloc, Args const&...) {
        static_assert(std::convertible_to<A2 const&, A>);
        return allocate(allocator_type(A(alloc)), size);
    }

    template <typename Self, typename A2, typename... Args>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    void* operator new(size_t size, Self const&, std::allocator_arg_t,
        const A2& alloc, Args const&...) {
        static_assert(std::convertible_to<const A2&, A>);
        return allocate(allocator_type(A(alloc)), size);
    }

    __RXX_HIDE_FROM_ABI void operator delete(void* ptr, size_t count) noexcept {
        if constexpr (stateless_allocator<allocator_type>) {
            allocator_type alloc;
            return alloc.deallocate(reinterpret_cast<allocation_block*>(ptr),
                allocation_block::count(count));
        } else {
            auto const block_count =
                allocation_block::count(allocation_size_bytes(count));
            auto const address = reinterpret_cast<uintptr_t>(ptr);
            auto* alloc_ptr = allocator_address(address, count);
            allocator_type alloc(std::move(*alloc_ptr));
            alloc_ptr->~allocator_type();
            alloc.deallocate(
                reinterpret_cast<allocation_block*>(ptr), block_count);
        }
    }
};

template <>
class promise_allocator<void> {
    using deallocator RXX_NODEBUG = void (*)(void*, size_t);

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static deallocator* deallocator_address(
        uintptr_t const ptr, size_t const size) noexcept {
        constexpr auto alignment = alignof(deallocator);
        auto const after_frame = ptr + size;
        auto const aligned_ptr =
            ((after_frame + alignment - 1) / alignment) * alignment;
        return reinterpret_cast<deallocator*>(aligned_ptr);
    }

    template <typename A>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static A* allocator_address(uintptr_t const ptr, size_t const size) noexcept
    requires (!stateless_allocator<A>)
    {
        constexpr auto alignment = alignof(A);
        auto* const deallocator = deallocator_address(ptr, size);
        auto const dealloc_ptr =
            reinterpret_cast<uintptr_t>(deallocator) + sizeof(deallocator);
        auto const aligned_ptr =
            ((dealloc_ptr + alignment - 1) / alignment) * alignment;
        return reinterpret_cast<A*>(aligned_ptr);
    }

    template <typename A>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static auto allocation_size_bytes(size_t size) noexcept {
        // Aligned storage of [ Coroutine frame | Deallocator | Allocator ]
        if constexpr (std::is_void_v<A>) {
            constexpr auto alignment = alignof(deallocator);
            constexpr auto tail_size = alignment + sizeof(deallocator);
            return size + tail_size;
        } else {
            constexpr auto alignment = alignof(A) + alignof(deallocator);
            constexpr auto tail_size =
                alignment + sizeof(A) + sizeof(deallocator);
            return size + tail_size;
        }
    }

    template <typename A>
    __RXX_HIDE_FROM_ABI static void deallocate(
        void* ptr, size_t size) noexcept {
        auto const total_size = allocation_size_bytes<A>(size);
        auto const block_count = allocation_block::count(total_size);

        if constexpr (stateless_allocator<A>) {
            A{}.deallocate(
                reinterpret_cast<allocation_block*>(ptr), block_count);
        } else {
            auto const address = reinterpret_cast<uintptr_t>(ptr);
            auto const alloc_ptr = allocator_address<A>(address, size);
            A alloc(std::move(*alloc_ptr));
            alloc_ptr->~A();
            alloc.deallocate(
                reinterpret_cast<allocation_block*>(ptr), block_count);
        }
    }

    template <typename A>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    static void* allocate(A const& alloc_src, size_t size) {
        using allocator_type = rebind_t<A, allocation_block>;
        using alloc_traits = std::allocator_traits<allocator_type>;

        static_assert(std::is_pointer_v<typename alloc_traits::pointer>,
            "Must use allocators for true pointers with generators");

        auto const alloc_size = allocation_size_bytes<allocator_type>(size);
        auto const block_count = allocation_block::count(alloc_size);
        auto alloc = static_cast<allocator_type>(alloc_src);
        void* ptr = alloc.allocate(block_count);
        auto const address = reinterpret_cast<uintptr_t>(ptr);
        *deallocator_address(address, size) = &deallocate<allocator_type>;
        if constexpr (!stateless_allocator<allocator_type>) {
            auto alloc_ptr = allocator_address<allocator_type>(address, size);
            ::new (alloc_ptr) allocator_type(std::move(alloc));
        }
        return ptr;
    }

public:
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) void* operator new(size_t size) {
        auto const total_size = allocation_size_bytes<void>(size);
        auto ptr = ::operator new(total_size);
        auto const address = reinterpret_cast<uintptr_t>(ptr);
        *deallocator_address(address, size) = +[](void* ptr, size_t size) {
            ::operator delete(ptr, allocation_size_bytes<void>(size));
        };
        return ptr;
    }

    template <typename A, typename... Args>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    void* operator new(
        size_t size, std::allocator_arg_t, A const& alloc, Args const&...) {
        return allocate(alloc, size);
    }

    template <typename Self, typename A, typename... Args>
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    void* operator new(size_t size, Self const&, std::allocator_arg_t,
        A const& alloc, Args const&...) {
        return allocate(alloc, size);
    }

    __RXX_HIDE_FROM_ABI void operator delete(void* ptr, size_t size) noexcept {
        auto const address = reinterpret_cast<uintptr_t>(ptr);
        auto deallocator_func = *deallocator_address(address, size);
        deallocator_func(ptr, size);
    }
};

} // namespace details::generator

template <typename Ref, typename V, typename Allocator>
class generator : public ranges::view_interface<generator<Ref, V, Allocator>> {

    using value RXX_NODEBUG = details::generator::value_for<Ref, V>;
    using reference RXX_NODEBUG = details::generator::reference_for<Ref, V>;
    using RRef RXX_NODEBUG = std::conditional_t<std::is_reference_v<reference>,
        std::remove_reference_t<reference>&&, reference>;

    static_assert(!std::is_const_v<value> && !std::is_volatile_v<value> &&
        std::is_object_v<value>);
    static_assert(std::is_reference_v<reference> ||
        !std::is_const_v<reference> && !std::is_volatile_v<reference> &&
            std::is_object_v<reference> && std::copy_constructible<reference>);
    static_assert(std::common_reference_with<reference&&, value&> &&
        std::common_reference_with<reference&&, RRef&&> &&
        std::common_reference_with<RRef&&, value const&>);

    class iterator;

public:
    using yielded RXX_NODEBUG = details::generator::yielded_for<Ref, V>;
    class promise_type :
        public details::generator::promise_base<yielded>,
        public details::generator::promise_allocator<Allocator> {
    public:
        RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
        generator get_return_object() noexcept {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    // static_assert(std::is_pointer_interconvertible_base_of_v<
    //     details::generator::promise_base<yielded>, promise_type>);

    __RXX_HIDE_FROM_ABI generator(generator const&) = delete;

    __RXX_HIDE_FROM_ABI generator(generator&& other) noexcept
        : coroutine_(exchange(other.coroutine_, nullptr))
        , active_(exchange(other.active_, false)) {}
    __RXX_HIDE_FROM_ABI ~generator() {
        if (auto& instance = this->coroutine_) {
            instance.destroy();
        }
    }

    __RXX_HIDE_FROM_ABI generator& operator=(generator other) noexcept {
        swap(other.coroutine_, this->coroutine_);
        this->active_ = exchange(other.active_, this->active_);
        return *this;
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) iterator begin() {
        this->activate();
        auto handle = base_handle::from_promise(coroutine_.promise());
        handle.promise().nested_.top() = handle;
        return {handle};
    }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }

private:
    friend details::generator::promise_base<yielded>;
    using base_handle RXX_NODEBUG =
        std::coroutine_handle<details::generator::promise_base<yielded>>;

    __RXX_HIDE_FROM_ABI generator(
        std::coroutine_handle<promise_type> coroutine) noexcept
        : coroutine_{std::move(coroutine)} {}

    __RXX_HIDE_FROM_ABI void activate() noexcept {
        assert(!this->active_);
        this->active_ = true;
    }

    std::coroutine_handle<promise_type> coroutine_;
    bool active_ = false;
};

template <typename Ref, typename V, typename Allocator>
class generator<Ref, V, Allocator>::iterator {
    friend generator;

    __RXX_HIDE_FROM_ABI iterator(base_handle instance) : coroutine_{instance} {
        this->next();
    }

    __RXX_HIDE_FROM_ABI void next() {
        auto& top = this->coroutine_.promise().nested_.top();
        top.resume();
    }

public:
    using value_type RXX_NODEBUG = value;
    using difference_type RXX_NODEBUG = ptrdiff_t;
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    friend bool operator==(
        iterator const& it, std::default_sentinel_t) noexcept {
        return it.coroutine_.done();
    }

    friend generator;

    __RXX_HIDE_FROM_ABI iterator(iterator&& other) noexcept
        : coroutine_(exchange(other.coroutine_, {})) {}

    __RXX_HIDE_FROM_ABI iterator& operator=(iterator&& other) noexcept {
        this->coroutine_ = exchange(other.coroutine_, {});
        return *this;
    }

    __RXX_HIDE_FROM_ABI iterator& operator++() {
        next();
        return *this;
    }

    __RXX_HIDE_FROM_ABI void operator++(int) { this->operator++(); }

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    reference operator*() const
        noexcept(std::is_nothrow_move_constructible_v<reference>) {
        auto& promise = this->coroutine_.promise();
        return static_cast<reference>(*promise.value());
    }

private:
    base_handle coroutine_;
};

namespace pmr {
template <typename Ref, typename V = void>
using generator =
    __RXX generator<Ref, V, std::pmr::polymorphic_allocator<std::byte>>;
}

RXX_DEFAULT_NAMESPACE_END
#else
#  define RXX_SUPPORTS_GENERATOR 0
#endif
