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

#include "rxx/slide_view.h"

#include "rxx/join_view.h"
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
    auto v1 = std::array{1, 2} | rxx::views::slide(1);
    auto const i0 = v1.begin(), i1 = v1.begin() + 1;
    VERIFY(i0 + 1 - 1 == i0);
    VERIFY(i0 < i1);
    VERIFY(i1 < v1.end());
    VERIFY(i1 - i0 == 1);
    VERIFY(i0 - i1 == -1);
    VERIFY(v1.end() - i1 == 1);
    VERIFY(i1 - v1.end() == -1);
    VERIFY(ranges::equal(std::move(v1) | rxx::views::join, (int[]){1, 2}));

    int x[] = {1, 2, 3, 4};
    auto v2 = x | rxx::views::slide(2);
    auto i2 = v2.begin();
    i2 += 2;
    i2 -= -1;
    VERIFY(i2 == v2.end());
    VERIFY(ranges::size(v2) == 3);
    VERIFY(ranges::size(std::as_const(v2)) == 3);
    VERIFY(ranges::equal(v2,
        (std::initializer_list<int>[]){
            {1, 2},
            {2, 3},
            {3, 4}
    },
        ranges::equal));

    int y[] = {1, 2, 3, 4, 5};
    auto const v3 = y | rxx::views::slide(3);
    VERIFY(ranges::size(v3) == 3);
    for (unsigned i = 0; i < ranges::size(x); i++) {
        VERIFY(&v3[i][0] == &y[i] + 0);
        VERIFY(&v3[i][1] == &y[i] + 1);
        VERIFY(&v3[i][2] == &y[i] + 2);
    }

    // LWG 3848 - slide_view etc missing base accessor
    static_assert(sizeof(decltype(v3.base())) > 0);

    auto const v5 = y | rxx::views::slide(5);
    VERIFY(ranges::size(v5) == 1);
    VERIFY(ranges::equal(v5 | rxx::views::join, y));

    auto const v6 = y | rxx::views::slide(6);
    VERIFY(ranges::empty(v6));

    return true;
}

constexpr bool test02() {
    using rxx::tests::test_forward_range;
    using rxx::tests::test_input_range;
    using rxx::tests::test_random_access_range;

    using ty1 = rxx::ranges::slide_view<views::all_t<test_forward_range<int>>>;
    static_assert(ranges::forward_range<ty1>);
    static_assert(!ranges::bidirectional_range<ty1>);
    static_assert(!ranges::sized_range<ty1>);

    using ty2 =
        rxx::ranges::slide_view<views::all_t<test_random_access_range<int>>>;
    static_assert(ranges::random_access_range<ty2>);
    static_assert(ranges::sized_range<ty2>);

    return true;
}

constexpr bool test03() {
    auto v = views::iota(0, 4) | views::filter([](auto) { return true; }) |
        rxx::views::slide(2);
    using ty = decltype(v);
    static_assert(ranges::forward_range<ty>);
    static_assert(ranges::common_range<ty>);
    static_assert(!ranges::sized_range<ty>);
    VERIFY(v.begin() == v.begin());
    VERIFY(v.begin() != v.end());
    VERIFY(ranges::next(v.begin(), 3) == v.end());
    auto it = v.begin();
    ++it;
    it++;
    VERIFY(ranges::next(it) == v.end());
    it--;
    --it;
    VERIFY(it == v.begin());

    return true;
}

int main() {
    static_assert(test01());
    static_assert(test02());
    static_assert(test03());
}
