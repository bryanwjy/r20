/* Copyright 2023-2025 Bryan Wong */

#pragma once

#include "rxx/preprocessor/is_empty.h"
#include "rxx/preprocessor/paste.h"

/**
 * TODO Use VA_OPT in C++20
 */

#define RXX_EXPAND_VA_ARGS_1()
#define RXX_EXPAND_VA_ARGS_0(...) , __VA_ARGS__
#define RXX_EXPAND_VA_ARGS(...) \
    RXX_PASTE(                  \
        RXX_APPEND_IS_EMPTY(RXX_EXPAND_VA_ARGS_, __VA_ARGS__)(__VA_ARGS__))
