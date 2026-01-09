// Copyright 2025 Bryan Wong

#pragma once

#include "rxx/config.h"

#include <exception>

RXX_DEFAULT_NAMESPACE_BEGIN

class bad_optional_access : public std::exception {
public:
    char const* what() const noexcept override { return "bad_optional_access"; }
};

RXX_DEFAULT_NAMESPACE_END
