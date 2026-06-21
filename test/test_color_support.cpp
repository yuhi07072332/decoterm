#define DECOTERM_NO_FORMAT
#include <decoterm/decoterm.hpp>
#include <print>

int main() {
    using namespace deco;
    std::print("color support: {}", 
               terminal.color_support() == ColorSupport::TrueColor ? "TrueColor" : "Color16");

}
