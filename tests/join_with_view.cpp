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

#include "rxx/join_with_view.h"

#include "test_iterators.h"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#define VERIFY assert

namespace ranges = std::ranges;
namespace views = std::ranges::views;
using namespace std::literals;

constexpr bool test01() {
    std::string_view rs[] = {"hello", "world"};
    auto v = rs | rxx::views::join_with(' ');
    VERIFY(ranges::equal(v | views::split(' '), rs, ranges::equal));
    auto i = v.begin(), j = v.begin();
    VERIFY(i == j);
    ++i;
    i++;
    VERIFY(i != j);
    VERIFY(*i == 'l');
    --i;
    i--;
    VERIFY(*i == 'h');
    return true;
}

constexpr bool test02() {
    std::string_view rs[] = {"the", "quick", "brown", "fox"};
    auto v = rs | views::transform([](auto x) { return x; }) |
        views::filter([](auto) { return true; });
    VERIFY(ranges::equal(
        v | rxx::views::join_with(views::empty<char>), "thequickbrownfox"sv));
    VERIFY(
        ranges::equal(v | rxx::views::join_with('-'), "the-quick-brown-fox"sv));
    VERIFY(ranges::equal(
        v | rxx::views::join_with("--"sv), "the--quick--brown--fox"sv));
    VERIFY(ranges::empty(views::empty<int[3]> | rxx::views::join_with(0)));
    VERIFY(ranges::equal(
        views::single(std::array{42}) | rxx::views::join_with(0), (int[]){42}));
    return true;
}

constexpr bool test03() {
    using rxx::tests::test_bidirectional_range;
    using rxx::tests::test_forward_range;
    using rxx::tests::test_input_range;

    using ty1 = rxx::ranges::join_with_view<
        views::all_t<test_input_range<test_input_range<int>>>,
        views::all_t<test_forward_range<int>>>;
    static_assert(ranges::input_range<ty1>);
    static_assert(!ranges::forward_range<ty1>);
    static_assert(!ranges::common_range<ty1>);

    using ty2 = rxx::ranges::join_with_view<
        views::all_t<test_forward_range<test_forward_range<int>>>,
        views::all_t<test_forward_range<int>>>;
    static_assert(ranges::forward_range<ty2>);
    static_assert(!ranges::bidirectional_range<ty2>);
    static_assert(!ranges::common_range<ty2>);

    using ty3 = rxx::ranges::join_with_view<
        views::all_t<std::array<std::string_view, 3>>, std::string_view>;
    static_assert(ranges::bidirectional_range<ty3>);
    static_assert(!ranges::random_access_range<ty3>);
    static_assert(ranges::common_range<ty3>);

    return true;
}

constexpr bool test04() {
    std::string rs[] = {"a", "", "b", "", "c"};
    auto v = rs | rxx::views::join_with(' ');
    VERIFY(ranges::equal(v, "a  b  c"sv));
    auto i = v.begin();
    auto j = ranges::next(i, 3);
    ranges::iter_swap(i, j);
    *j = ranges::iter_move(i);
    VERIFY(ranges::equal(v, "b  b  c"sv));
    return true;
}

#include <iostream>

int main() {
    // std::string_view rs[] = {"the", "quick", "brown", "fox"};
    // auto v = rs | views::transform([](auto x) { return x; }) |
    //     views::filter([](auto) { return true; });
    // for (auto val : v | rxx::views::join_with(views::empty<char>)) {
    //     std::cout << val << std::endl;
    // }
    static_assert(test01());
    static_assert(test02());
    static_assert(test03());
    static_assert(test04());
}
