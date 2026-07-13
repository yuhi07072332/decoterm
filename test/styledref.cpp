#include <decoterm/styled.hpp>
#include <iostream>

int main() {
    int a = 10;
    auto st1 = deco::styled(56, deco::bold);
    auto st2 = deco::styled(a, deco::bold);
    auto st3 = deco::styled(std::move(a), deco::bold);

    std::cout << st1.value << '\n';
    std::cout << st2.value << '\n';
    std::cout << st3.value << '\n';
}
