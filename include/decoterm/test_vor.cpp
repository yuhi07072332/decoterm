#include "style.hpp"

#include <iostream>
#include <type_traits>

#define ASSERT_TYPE(expr, t) static_assert(std::is_same_v<decltype(expr), t>);

void handle_iarr5(int (&arr)[5]) {
    for (int i : arr)
        std::cout << i << ' ';
    std::cout << '\n';
}

auto main() -> int {
    using namespace deco::detail;

    int i = 8;
    const int ci = 16;
    int& ri = i;
    const int& cri = i;
    int* pi = &i;
    const int* cpi = &ci;

    int arr[] = { 1, 2, 3, 4, 5 };
    const char* s = "hello, world";

    auto vorvi = ValueOrRef(32);
    auto vorvci = ValueOrRef(std::move(ci));
    ASSERT_TYPE(vorvi.get(), int &);
    ASSERT_TYPE(vorvci.get(), int &);

    auto vorri = ValueOrRef(i);
    auto vorrci = ValueOrRef(ci);
    ASSERT_TYPE(vorri.get(), int &);
    ASSERT_TYPE(vorrci.get(), const int &);

    auto vorrri = ValueOrRef(ri);
    auto vorrcri = ValueOrRef(cri);
    ASSERT_TYPE(vorrri.get(), int &);
    ASSERT_TYPE(vorrcri.get(), const int &);

    auto vorvpi = ValueOrRef(&i);
    auto vorrpi = ValueOrRef(pi);
    auto vorrcpi = ValueOrRef(cpi);
    ASSERT_TYPE(vorrpi.get(), int *&);
    ASSERT_TYPE(vorrcpi.get(), const int *&);

    auto vorarr = ValueOrRef(arr);
    auto vorf = ValueOrRef(handle_iarr5);
    auto vorvs = ValueOrRef("string");
    auto vorrs = ValueOrRef(s);

    vorf.get()(arr);
    ASSERT_TYPE(vorarr.get(), int (&)[5]);
    ASSERT_TYPE(vorrs.get(), const char *&);

    std::cout << vorvs.get() << '\n';
    std::cout << vorrs.get() << '\n';

    handle_iarr5(vorarr.get());
}
