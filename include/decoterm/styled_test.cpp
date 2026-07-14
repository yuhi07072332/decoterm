#include "style.hpp"
#include "styled.hpp"

#include <iostream>
#include <print>

int main() {
    int i = 30;
    const int ci = 45;
    volatile int vi = 60;

    auto r1 = deco::detail::ConstRef(i);

    constexpr auto s1 = deco::styled(128, deco::bold);
    auto s2 = deco::styled(i, deco::bold);
    auto s3 = deco::styled(ci, deco::bold);

    std::println("{}", *r1);
    std::println("{}", *r2);
    std::println("{}", *r3);
    std::println("{}", *r4);
    std::println("{}", *r5);
    std::println("{}", *r6);
}
