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

#include "rxx/zip_view.h"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>
#define VERIFY assert

namespace ranges = std::ranges;
namespace views = std::ranges::views;

constexpr bool test01() {
    static_assert(ranges::empty(rxx::views::zip()));
    static_assert(ranges::empty(views::empty<int>));

    auto z1 = rxx::views::zip(std::array{1, 2});
    auto const i0 = z1.begin(), i1 = z1.begin() + 1;
    VERIFY(i0 + 1 - 1 == i0);
    VERIFY(i0 < i1);
    VERIFY(i1 < z1.end());
    VERIFY(i1 - i0 == 1);
    VERIFY(i0 - i1 == -1);
    VERIFY(z1.end() - i1 == 1);
    VERIFY(i1 - z1.end() == -1);
    ranges::iter_swap(i0, i1);
    VERIFY(ranges::equal(std::move(z1) | views::keys, (int[]){2, 1}));

    auto z2 = rxx::views::zip(std::array{1, 2}, std::array{3, 4, 5});
    auto i2 = z2.begin();
    i2 += 1;
    i2 -= -1;
    VERIFY(i2 == z2.end());
    VERIFY(ranges::size(z2) == 2);
    VERIFY(ranges::size(std::as_const(z2)) == 2);
    VERIFY(std::get<0>(z2[0]) == 1 && std::get<1>(z2[0]) == 3);
    VERIFY(std::get<0>(z2[1]) == 2 && std::get<1>(z2[1]) == 4);
    for (auto const [x, y] : z2) {
        VERIFY(y - x == 2);
        std::swap(x, y);
    }

    int x[2] = {1, 2}, y[2] = {3, 4}, z[2] = {5, 6};
    auto const z3 = rxx::views::zip(x, y, z);
    VERIFY(ranges::size(z3) == 2);
    for (int i = 0; i < ranges::size(x); i++) {
        VERIFY(&std::get<0>(z3[i]) == &x[i]);
        VERIFY(&std::get<1>(z3[i]) == &y[i]);
        VERIFY(&std::get<2>(z3[i]) == &z[i]);
    }

    return true;
}

constexpr bool test02() {
    // using __gnu_test::test_forward_range;
    // using __gnu_test::test_input_range;
    // using __gnu_test::test_random_access_range;

    // using ty1 = ranges::zip_view<views::all_t<test_forward_range<int>>,
    //     views::all_t<test_random_access_range<int>>>;
    // static_assert(ranges::forward_range<ty1>);
    // static_assert(!ranges::random_access_range<ty1>);
    // static_assert(!ranges::sized_range<ty1>);

    // using ty2 = ranges::zip_view<views::all_t<test_forward_range<int>>,
    //     views::all_t<test_input_range<int>>,
    //     views::all_t<test_forward_range<int>>>;
    // static_assert(ranges::input_range<ty2>);
    // static_assert(!ranges::forward_range<ty2>);
    // static_assert(!ranges::sized_range<ty2>);

    return true;
}

constexpr bool test03() {
    int u[] = {1, 2, 3, 4}, v[] = {4, 5, 6}, w[] = {7, 8, 9, 10};
    auto z =
        rxx::views::zip(u | views::filter([](auto) { return true; }), v, w);
    using ty = decltype(z);
    static_assert(ranges::forward_range<ty>);
    static_assert(!ranges::common_range<ty>);
    static_assert(!ranges::sized_range<ty>);
    VERIFY(z.begin() == z.begin());
    VERIFY(z.begin() != z.end());
    VERIFY(ranges::next(z.begin(), 3) == z.end());
    auto it = z.begin();
    ++it;
    it++;
    it--;
    --it;
    VERIFY(it == z.begin());

    return true;
}

constexpr bool test04() {
    // PR libstdc++/106766
#if __SIZEOF_INT128__ && 0
    auto r = rxx::views::zip(views::iota(__int128(0), __int128(1)));
#else
    auto r = rxx::views::zip(views::iota(0ll, 1ll));
#endif
    auto i = r.begin();
    auto s = r.end();
    VERIFY(s - i == 1);
    VERIFY(i + 1 - i == 1);

    return true;
}

constexpr bool test05() {
#if RXX_CXX23
    // This requies const assignment operator on tuple
    // PR libstdc++/109203
    int x[] = {1, 1, 2};
    int y[] = {2, 1, 3};
    auto r = rxx::views::zip(x, y);
    ranges::sort(r);
#endif

    return true;
}

int main() {
    static_assert(test01());
    static_assert(test02());
    static_assert(test03());
    static_assert(test04());
    static_assert(test05());
}
