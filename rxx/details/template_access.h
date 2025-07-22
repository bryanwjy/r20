// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

RXX_DEFAULT_NAMESPACE_BEGIN

namespace details {

template <typename...>
struct type_list {};

template <size_t I, typename List>
struct template_element {};

template <size_t I, typename List>
using template_element_t RXX_NODEBUG = typename template_element<I, List>::type;

template <template <typename...> class List, typename Head, typename... Tail>
struct template_element<0, List<Head, Tail...>> {
    using type RXX_NODEBUG = Head;
};

template <size_t I, template <typename...> class List, typename Head,
    typename... Tail>
struct template_element<I, List<Head, Tail...>> :
    template_element<I - 1, type_list<Tail...>> {};

template <typename List>
inline constexpr size_t template_size_v = 0;

template <template <typename...> class List, typename... Args>
inline constexpr size_t template_size_v<List<Args...>> = sizeof...(Args);

template <typename T, typename TList>
inline constexpr size_t template_index_v = static_cast<size_t>(-1);

template <typename T, template <typename...> class TList, typename... Tail>
inline constexpr size_t template_index_v<T, TList<T, Tail...>> = 0;

template <typename T, template <typename...> class TList, typename Head,
    typename... Tail>
inline constexpr size_t template_index_v<T, TList<Head, Tail...>> =
    1 + template_index_v<T, type_list<Tail...>>;

template <typename T, typename TList>
inline constexpr size_t template_count_v = 0;

template <typename T, template <typename...> class List, typename... Args>
inline constexpr size_t template_count_v<T, List<T, Args...>> =
    1 + template_count_v<T, type_list<Args...>>;

template <typename T, typename U, template <typename...> class List,
    typename... Args>
inline constexpr size_t template_count_v<T, List<U, Args...>> =
    template_count_v<T, type_list<Args...>>;

template <typename List>
struct template_size {};

template <template <typename...> class List, typename... Args>
struct template_size<List<Args...>> :
    std::integral_constant<size_t, template_size_v<List<Args...>>> {};

template <typename T, typename List>
struct template_count {};

template <typename T, template <typename...> class List, typename... Args>
struct template_count<T, List<Args...>> :
    std::integral_constant<size_t, template_count_v<T, List<Args...>>> {};

} // namespace details

RXX_DEFAULT_NAMESPACE_END
