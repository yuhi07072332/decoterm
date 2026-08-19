#include "style.hpp"
#include <iostream>


auto main() -> int {
    using namespace deco;
    int i = 16;
    const int ci = 32;
    float f = 3.14;
    int array[5] = { 1, 2, 3, 4, 5};
    int* pi = &i;
    const int* cpi = &i;

    auto const_lambda = [&i]{
        i = 16;
    };

    auto mutable_lambda = [=] mutable {
        i++;
    };

    // lvalue reference
    auto l = detail::make_styled_child(i);
    decltype(auto) lv = l.get();
    auto cl = detail::make_styled_child(ci);
    decltype(auto) clv = cl.get();

    // pointer
    auto p = detail::make_styled_child(pi);
    auto cp = detail::make_styled_child(cpi);
    auto sp = detail::make_styled_child("helloworld");
    auto ap = detail::make_styled_child(array);
    auto fp = detail::make_styled_child(main);

    // rvalue
    auto r = detail::make_styled_child(64);
    auto cr = detail::make_styled_child(std::move(ci));

    // other
    auto lc_lambda = detail::make_styled_child(const_lambda);
    auto rc_lambda = detail::make_styled_child([&]{ i++; });


    auto st = styled(fg(blue), 31, i, styled(italic | fg(cyan), f, "aaa"), ci);
    std::cout << st << '\n';
    std::cout << styled(
        bold,
        "one",
        styled(
            fg(green) | italic,
            "two",
            styled(
                fg(cyan) | underline,
                "three"
            ),
            "two"
        ),
        "one"
    );
}
