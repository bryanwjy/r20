// Copyright 2025 Bryan Wong

#include "rxx/adjacent_transform_view.h"
#include "rxx/adjacent_view.h"
#include "rxx/as_const_view.h"
#include "rxx/drop_view.h"
#include "rxx/enumerate_view.h"
#include "rxx/repeat_view.h"
#include "rxx/take_view.h"
#include "rxx/zip_transform_view.h"
#include "rxx/zip_view.h"

#include <cstdio>

int main() {
    int a[]{0, 1, 2, 3, 4, 5};
    int b[]{6, 7, 8, 9, 10, 11};

    puts("zip_transform");
    for (auto const val :
        rxx::views::zip_transform([](auto l, auto r) { return l + r; }, a, b)) {
        printf("%d\n", val);
    }

    puts("const enumerate");
    for (auto const [idx, val] :
        a | rxx::views::as_const | rxx::views::enumerate) {
        printf("%ld: %d\n", idx, val);
    }

    puts("adjacent pair");
    for (auto const [first, second] : a | rxx::views::adjacent<2>) {
        printf("{%d, %d}\n", first, second);
    }

    puts("transfor adjacent pair");
    for (auto const val : a |
            rxx::views::adjacent_transform<2>(
                [](auto l, auto r) { return l + r; })) {
        printf("{%d}\n", val);
    }

    puts("adjacent triple");
    for (auto const [first, second, third] : a | rxx::views::adjacent<3>) {
        printf("{%d, %d, %d}\n", first, second, third);
    }

    puts("repeat, enumerate");
    for (auto const [idx, val] :
        rxx::views::repeat(std::views::all(a), 3) | rxx::views::enumerate) {
        for (auto const i : val) {
            printf("%d: %d\n", idx, i);
        }
    }

    puts("repeat, take");
    for (auto val : rxx::views::repeat(37) | rxx::views::take(13)) {
        printf("%d\n", val);
    }

    puts("repeat, drop");
    for (auto val : rxx::views::repeat(23, 15) | rxx::views::drop(13)) {
        printf("%d\n", val);
    }

    return 0;
}
