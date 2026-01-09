// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include "rxx/variant/fwd.h" // IWYU pragma: export

#include <exception>

RXX_DEFAULT_NAMESPACE_BEGIN

class bad_variant_access : public std::exception {
public:
    char const* what() const noexcept override { return "bad_variant_access"; }
};
RXX_DEFAULT_NAMESPACE_END
