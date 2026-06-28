#include <decoterm/decoterm.hpp>
#include <decoterm/terminal.hpp>

#include <print>

int main() {
    using namespace deco;
    Terminal term;
    std::print("color support: {}", 
               term.color_support() == ColorSupport::TrueColor ? "TrueColor" : "Color16");

}
