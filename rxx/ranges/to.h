// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/details/adaptor_closure.h"
#include "rxx/details/bind_back.h"
#include "rxx/ranges/concepts.h"
#include "rxx/ranges/primitives.h"
#include "rxx/ranges/ref_view.h"

#include <concepts>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN

namespace ranges {
namespace details {
template <typename Container>
constexpr bool reservable_container = sized_range<Container> &&
    requires(Container& container, range_size_t<Container> size) {
        container.reserve(size);
        { container.capacity() } -> std::same_as<decltype(size)>;
        { container.max_size() } -> std::same_as<decltype(size)>;
    };

template <typename Container, typename Ref>
constexpr bool container_appendable =
    requires(Container& container, Ref&& ref) {
        requires (
            requires { container.emplace_back(std::forward<Ref>(ref)); } ||
            requires { container.push_back(std::forward<Ref>(ref)); } ||
            requires {
                container.emplace(container.end(), std::forward<Ref>(ref));
            } ||
            requires {
                container.insert(container.end(), std::forward<Ref>(ref));
            });
    };

template <typename Container, typename Range>
concept try_non_recursive_conversion = !input_range<Container> ||
    std::convertible_to<range_reference_t<Range>, range_value_t<Container>>;

template <typename C, typename R, typename... Args>
concept constructible_from_iter_pair = common_range<R> &&
    requires {
        typename std::iterator_traits<iterator_t<R>>::iterator_category;
    } &&
    std::derived_from<
        typename std::iterator_traits<iterator_t<R>>::iterator_category,
        std::input_iterator_tag> &&
    std::constructible_from<C, iterator_t<R>, sentinel_t<R>, Args...>;

} // namespace details

template <typename C, input_range R, typename... Args>
requires (!view<C>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr C
    to(R&& range, Args&&... args) {
    static_assert(
        !std::is_const_v<C>, "The target container cannot be const-qualified");
    static_assert(!std::is_volatile_v<C>,
        "The target container cannot be volatile-qualified");
    static_assert(std::is_class_v<C> || std::is_union_v<C>,
        "The target must be a class type or union type");

    if constexpr (details::try_non_recursive_conversion<C, R>) {
        // Case 1 -- construct directly from the given range.
        if constexpr (std::constructible_from<C, R, Args...>) {
            return C(std::forward<R>(range), std::forward<Args>(args)...);
        }
#if RXX_CXX23
        // Case 2 -- construct using the `from_range_t` tagged constructor.
        else if constexpr (std::constructible_from<C, std::from_range_t, R,
                               Args...>) {
            return C(std::from_range, std::forward<R>(range),
                std::forward<Args>(args)...);
        }
#endif

        // Case 3 -- construct from a begin-end iterator pair.
        else if constexpr (details::constructible_from_iter_pair<C, R,
                               Args...>) {
            return C(ranges::begin(range), ranges::end(range),
                std::forward<Args>(args)...);
        }

        // Case 4 -- default-construct (or construct from the extra arguments)
        // and insert, reserving the size if possible.
        else {
            static_assert(std::constructible_from<C, Args...> &&
                    details::container_appendable<C, range_reference_t<R>>,
                "ranges::to: unable to convert to the given container type.");

            C result(std::forward<Args>(args)...);
            if constexpr (sized_range<R> && details::reservable_container<C>) {
                result.reserve(
                    static_cast<range_size_t<C>>(ranges::size(range)));
            }

            for (auto&& ref : range) {
                using RefType = decltype(ref);
                if constexpr ( //
                    requires {
                        result.emplace_back(std::declval<RefType>());
                    }) {
                    result.emplace_back(std::forward<RefType>(ref));
                } else if constexpr ( //
                    requires { result.push_back(std::declval<RefType>()); }) {
                    result.push_back(std::forward<RefType>(ref));
                } else if constexpr ( //
                    requires {
                        result.emplace(result.end(), std::declval<RefType>());
                    }) {
                    result.emplace(result.end(), std::forward<RefType>(ref));
                } else {
                    static_assert(requires {
                        result.insert(result.end(), std::declval<RefType>());
                    });
                    result.insert(result.end(), std::forward<RefType>(ref));
                }
            }
            return result;
        }
    } else {
        static_assert(input_range<range_reference_t<R>>,
            "ranges::to: unable to convert to the given container type.");
        return ranges::to<C>(
            ref_view(range) | std::views::transform([]<typename T>(T&& item) {
                return ranges::to<range_value_t<C>>(std::forward<T>(item));
            }),
            std::forward<Args>(args)...);
    }
}

namespace details {
template <typename R>
struct minimal_input_iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = range_value_t<R>;
    using difference_type = ptrdiff_t;
    using pointer = std::add_pointer_t<range_reference_t<R>>;
    using reference = range_reference_t<R>;

    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    reference operator*() const noexcept;
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    pointer operator->() const noexcept;
    __RXX_HIDE_FROM_ABI minimal_input_iterator& operator++();
    __RXX_HIDE_FROM_ABI minimal_input_iterator operator++(int);
    RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
    bool operator==(minimal_input_iterator const&) const noexcept;
};

template <template <typename...> class C, input_range R, typename... Args>
struct template_deducer {
    using type = typename decltype([]() {
        using Iter = minimal_input_iterator<R>;

        // Case 1 -- can construct directly from the given range.
        if constexpr (requires {
                          C(std::declval<R>(), std::declval<Args>()...);
                      }) {
            using ResultType = decltype( //
                C(std::declval<R>(), std::declval<Args>()...));
            return std::type_identity<ResultType>{};
        }
        // Case 2 -- can construct from the given range using the
        // `from_range_t` tagged constructor.
#if RXX_CXX23
        else if constexpr ( //
            requires {
                C(from_range, std::declval<R>(), std::declval<Args>()...);
            }) {
            using ResultType = //
                decltype(C(
                    from_range, std::declval<R>(), std::declval<Args>()...));
            return std::type_identity<ResultType>{};

        }
#endif
        // Case 3 -- can construct from a begin-end iterator pair.
        else {
            static_assert(
                requires {
                    C(std::declval<Iter>(), std::declval<Iter>(),
                        std::declval<Args>()...);
                },
                "ranges::to: unable to deduce the container type from the "
                "template template argument.");
            using ResultType = decltype(C(std::declval<Iter>(),
                std::declval<Iter>(), std::declval<Args>()...));
            return std::type_identity<ResultType>{};
        }
    }())::type;
};
} // namespace details

template <template <typename...> class C, input_range R, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto to(R&& range, Args&&... args) {
    using deduced_expr =
        typename details::template_deducer<C, R, Args...>::type;
    return ranges::to<deduced_expr>(
        std::forward<R>(range), std::forward<Args>(args)...);
}

template <typename C, typename... Args>
requires (!view<C>)
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD) constexpr auto to(Args&&... args) {
    static_assert(!std::is_const_v<C>,
        "The target container cannot be const-qualified, please remove the "
        "const");
    static_assert(!std::is_volatile_v<C>,
        "The target container cannot be volatile-qualified, please remove the "
        "volatile");
    static_assert(std::is_class_v<C> || std::is_union_v<C>,
        "The target must be a class type or union type");

    return details::make_pipeable(details::bind_back(
        []<input_range R, typename... Tail>(R&& range, Tail&&... tail)
            RXX_STATIC_CALL
        requires requires { //
            ranges::to<C>(std::forward<R>(range), std::forward<Tail>(tail)...);
        }
        {
            return ranges::to<C>(
                std::forward<R>(range), std::forward<Tail>(tail)...);
        },
        std::forward<Args>(args)...));
}

template <template <typename...> class C, typename... Args>
RXX_ATTRIBUTES(_HIDE_FROM_ABI, NODISCARD)
constexpr auto to(Args&&... args) {

    using details::template_deducer;
    return details::make_pipeable(details::bind_back(
        []<input_range R, typename... Tail,
            typename D = typename template_deducer<C, R, Tail...>::type>(
            R && ranges,
            Tail && ... tail) RXX_STATIC_CALL requires requires { //
            ranges::to<D>(std::forward<R>(ranges), std::forward<Tail>(tail)...);
        } {
            return ranges::to<D>(
                std::forward<R>(ranges), std::forward<Tail>(tail)...);
        },
        std::forward<Args>(args)...));
}

} // namespace ranges

RXX_DEFAULT_NAMESPACE_END
