// Copyright 2025 Bryan Wong
#pragma once

#include "rxx/config.h"

#include "rxx/access.h"
#include "rxx/algorithm/return_types.h"
#include "rxx/borrow_traits.h"
#include "rxx/concepts.h"
#include "rxx/iterator.h"

#include <concepts>
#include <utility>

RXX_DEFAULT_NAMESPACE_BEGIN
namespace ranges {

template <typename O, typename T>
using iota_result = out_value_result<O, T>;

namespace details {
struct iota_t {
    template <std::input_or_output_iterator O, std::sentinel_for<O> S,
        std::weakly_incrementable T>
    requires std::indirectly_writable<O, T const&>
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL constexpr iota_result<O, T> operator()(
        O first, S last, T value) RXX_CONST_CALL {
        while (first != last) {
            *first = std::as_const(value);
            ++first;
            ++value;
        }
        return {std::move(first), std::move(value)};
    }

    template <std::weakly_incrementable T, output_range<T const&> R>
    __RXX_HIDE_FROM_ABI
        RXX_STATIC_CALL constexpr iota_result<borrowed_iterator_t<R>, T>
        operator()(R&& r, T value) RXX_CONST_CALL {
        return (*this)(ranges::begin(r), ranges::end(r), std::move(value));
    }
};
} // namespace details

inline namespace cpo {
inline constexpr details::iota_t iota{};
}

} // namespace ranges
RXX_DEFAULT_NAMESPACE_END
