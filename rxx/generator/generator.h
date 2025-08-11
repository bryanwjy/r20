// Copyright 2025 Bryan Wong

#include "rxx/config.h"

#include "rxx/ranges/view_interface.h"

#include <type_traits>

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
class erased_promise;

} // namespace details::generator

template <typename Ref, typename V, typename Allocator>
class generator : public ranges::view_interface<generator<Ref, V, Allocator>> {

    using value RXX_NODEBUG = details::generator::value_for<Ref, V>;
    using reference RXX_NODEBUG = details::generator::reference_for<Ref, V>;
    using RRef RXX_NODEBUG = std::conditional_t<std::is_reference_v<reference>,
        std::remove_reference_t<reference>&&, reference>;
    using alloc_traits = std::allocator_traits<Allocator>;
    using allocator_type = Allocator;

    static_assert(std::is_pointer_v<typename alloc_traits::pointer>);
    static_assert(!std::is_const_v<value> && !std::is_volatile_v<value> &&
        std::is_object_v<value>);
    static_assert(std::is_reference_v<reference> ||
        !std::is_const_v<reference> && !std::is_volatile_v<reference> &&
            std::is_object_v<reference> && std::copy_constructible<reference>);
    static_assert(std::common_reference_with<reference&&, value&> &&
        std::common_reference_with<reference&&, RRef&&> &&
        std::common_reference_with<RRef&&, value const&>);

public:
    using yielded RXX_NODEBUG = details::generator::yielded_for<Ref, V>;
    class promise_type;
    class iterator;

private:
    std::coroutine_handle<promise_type> coroutine_;
    bool active_ = false;
};
RXX_DEFAULT_NAMESPACE_END
