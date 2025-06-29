// Copyright 2025 Bryan Wong
// Adapted from GCC testsuite

// Copyright (C) 2020-2025 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include "rxx/stride_view.h"

#include "test_iterators.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <ranges>
#include <utility>

#define VERIFY assert

namespace ranges = std::ranges;
namespace views = std::views;

constexpr bool test01() {
    int x[] = {1, 2, 3, 4, 5, 6, 7};

    auto v2 = x | rxx::views::stride(2);
    auto const i0 = v2.begin(), i1 = v2.begin() + 1;
    VERIFY(i0 + 1 - 1 == i0);
    VERIFY(i0 != i1);
    VERIFY(i0 < i1);
    VERIFY(i0 <= i0);
    VERIFY(i0 >= i0);
    VERIFY(v2.end() > i1);
    VERIFY(i1 - i0 == 1);
    VERIFY(i0 - i1 == -1);
    VERIFY(v2.end() - i1 == 3);
    VERIFY(i1 - v2.end() == -3);
    auto i2 = v2.begin();
    i2 += 2;
    i2 -= -2;
    VERIFY(i2 == v2.end());
    VERIFY(ranges::size(v2) == 4);
    VERIFY(ranges::equal(v2, (int[]){1, 3, 5, 7}));
    VERIFY(ranges::equal(v2 | views::reverse, (int[]){7, 5, 3, 1}));
    VERIFY(v2.stride() == 2);

    auto v1 = x | rxx::views::stride(1);
    VERIFY(ranges::size(v1) == ranges::size(x));
    VERIFY(ranges::equal(v1, x));
    VERIFY(ranges::equal(v1 | views::reverse, x | views::reverse));
    VERIFY(v1.stride() == 1);

    auto v5 = x | rxx::views::stride(5);
    VERIFY(ranges::equal(v5, (int[]){1, 6}));
    VERIFY(ranges::equal(v5 | views::reverse, (int[]){6, 1}));
    VERIFY(v5.stride() == 5);

    auto v10 = x | rxx::views::stride(10);
    VERIFY(ranges::equal(v10, (int[]){1}));
    VERIFY(ranges::equal(v10 | views::reverse, (int[]){1}));
    VERIFY(v10.stride() == 10);

    return true;
}

template <typename container>
void test02() {
    int x[] = {1, 2, 3, 4, 5, 6, 7, 8};
    container rx(x);
    auto v = rx | rxx::views::stride(3);
    VERIFY(ranges::equal(v, (int[]){1, 4, 7}));
}

void test03() {
    // PR libstdc++/107313
    int x[] = {1, 2, 3, 4, 5};
    rxx::tests::test_input_range<int> rx(x);
    auto r = views::counted(rx.begin(), 4) | rxx::views::stride(2);
    auto i = r.begin();
    std::default_sentinel_t s = r.end();
    VERIFY(s != i);
    VERIFY(s - i == 2 && i - s == -2);
    ++i;
    VERIFY(s != i);
    VERIFY(s - i == 1 && i - s == -1);
    ++i;
    VERIFY(s == i);
    VERIFY(s - i == 0 && i - s == 0);
}

int main() {
    static_assert(test01());
    test02<rxx::tests::test_input_range<int>>();
    test02<rxx::tests::test_forward_range<int>>();
    test03();
}
