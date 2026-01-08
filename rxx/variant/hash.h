// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h"

#include "rxx/functional/hash.h"
#include "rxx/type_traits/template_access.h"
#include "rxx/utility/jump_table.h"
#include "rxx/variant/get.h"

template <typename... Ts>
requires (... && std::semiregular<std::hash<std::remove_const_t<Ts>>>) &&
    (... &&
        requires(Ts const& val, std::hash<std::remove_const_t<Ts>> hasher) {
            { hasher(val) } -> std::same_as<size_t>;
        })
struct std::hash<__RXX variant<Ts...>> {
    __RXX_HIDE_FROM_ABI RXX_STATIC_CALL size_t operator()(
        __RXX variant<Ts...> const& var) RXX_CONST_CALL noexcept((... &&
        std::is_nothrow_invocable_v<std::hash<std::remove_cv_t<Ts>>,
            Ts const&>)) {
        using variant_type = __RXX variant<Ts...>;
        auto const result = var.valueless_by_exception()
            ? 299792458
            : __RXX iota_table<sizeof...(Ts)>(
                 []<size_t I>(std::integral_constant<size_t, I>,
                     variant_type const& var) {
                     using arugment_type = std::remove_cv_t<
                          __RXX template_element_t<I, variant_type>>;
                     return std::hash<arugment_type>()( __RXX get<I>(var));
                 },
                 var.index(), var);
        auto const index = std::hash<size_t>()(var.index());
        return result ^
            (index + static_cast<size_t>(0x9e3779b97f4a7c15ULL) +
                (result << 6) + (result >> 2));
    }
};
