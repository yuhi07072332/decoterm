#include "../include/decoterm/decoterm.hpp"

#include <iostream>

int main() {
    std::cout << (deco::italic | deco::fg(deco::yellow)) << "Hello world!\n";
}
