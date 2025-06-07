
#include "rxx/as_const_view.h"
#include "rxx/enumerate_view.h"
#include "rxx/repeat_view.h"
#include "rxx/take_view.h"
#include "rxx/zip_transform_view.h"
#include "rxx/zip_view.h"

#include <cstdio>

int main() {
    int a[]{0, 1, 2, 3, 4, 5};
    int b[]{6, 7, 8, 9, 10, 11};

    for (auto const val :
        rxx::views::zip_transform([](auto l, auto r) { return l + r; }, a, b)) {
        printf("%d\n", val);
    }

    for (auto const [idx, val] :
        a | rxx::views::as_const | rxx::views::enumerate) {
        printf("%ld: %d\n", idx, val);
    }

    for (auto const [idx, val] :
        rxx::views::repeat(std::views::all(a), 3) | rxx::views::enumerate) {
        for (auto const i : val) {
            printf("%d: %d\n", idx, i);
        }
    }

    for (auto val : rxx::views::repeat(37) | rxx::views::take(13)) {
        printf("%d\n", val);
    }

    return 0;
}
