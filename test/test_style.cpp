#include "../include/decoterm/decoterm.hpp"

#include <iostream>

int main() {
    using namespace deco;
    std::cout << (bold | italic | fg(Color::Green)) << "test";
}
