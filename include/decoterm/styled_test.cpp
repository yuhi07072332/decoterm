#include "style.hpp"
#include "styled.hpp"

#include <iostream>
#include <print>

struct Immutable {
    bool value_;
    auto get() const -> bool { return value_; }
    void set() { value_ = !value_; }
};

int main() {
    int i = 30;
    const int ci = 45;
    volatile int vi = 60;
    const Immutable cim{true};

    auto r1 = deco::detail::Storage(i);
    auto r2 = deco::detail::Storage(ci);
    auto r3 = deco::detail::Storage(145);
    auto r4 = deco::detail::Storage(std::move(cim));

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
