#include "../include/decoterm/decoterm.hpp"

#include <iostream>
#include <cstdlib>
#include <new>

void* operator new(std::size_t size) {
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    std::cout << "[HEAP ALLOCATION] " << size << "bytes\n";
    return ptr;
}

int main() {
    using namespace deco;
    std::cout << (bold | italic | fg(green)) << "test\n";
    Style style_with_rgb = fg(rgb(111, 222, 101));
    Style big_style = bold | italic | underline | fg(rgb(123, 100, 255));
    std::cout << reset << style_with_rgb << "test2!!!\n";
    std::cout << reset << big_style << "test3!!!\n";
}
