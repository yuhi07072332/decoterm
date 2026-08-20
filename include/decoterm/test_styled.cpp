#include "style.hpp"
#include "format.hpp"
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
    auto l = detail::store_styled_value(i);
    decltype(auto) lv = l.get();
    auto cl = detail::store_styled_value(ci);
    decltype(auto) clv = cl.get();

    // pointer
    auto rp = detail::store_styled_value(&i);
    auto p = detail::store_styled_value(pi);
    auto cp = detail::store_styled_value(cpi);

    // array / function
    auto sp = detail::store_styled_value("helloworld");
    auto ap = detail::store_styled_value(array);
    auto fp = detail::store_styled_value(main);

    // rvalue
    auto r = detail::store_styled_value(64);
    auto cr = detail::store_styled_value(std::move(ci));

    // other
    auto lc_lambda = detail::store_styled_value(const_lambda);
    auto rc_lambda = detail::store_styled_value([&]{ i++; });

    auto cref = detail::ConstRef(i);

    decltype(auto) unwrap1 = detail::unwrap(i);
    decltype(auto) unwrap2 = detail::unwrap(ci);
    decltype(auto) unwrap3 = detail::unwrap(cref);


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
    ) << '\n';

    auto sts = fg(blue)(
        31, i,
        (italic | fg(cyan))(f, "aaa"),
        ci
    );

    auto st2 = (fg(blue) | italic)("hello");

    std::cout << styled(fg(yellow), "aaa") << "bbb\n";
    std::cout << "and one more for the fans\n";

    std::println("{}", styled(bold, 65535));
    std::println("{}", styled(bold, detail::ConstRef(i)));
    std::println("{}", st);

    std::cout << "hello" << bold("world!");

    static_assert(std::is_default_constructible_v<detail::StyledFormatContext>);
    static_assert(std::is_default_constructible_v<std::formatter<Styled<Style, Styled<Style, int>>>>);
}
