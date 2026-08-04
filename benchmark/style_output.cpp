#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <cstdint>
#include <iostream>
#include <print>
#include <sstream>

#include "benchmark.hpp"

auto main(int argc, const char** argv) -> int {
    using namespace deco;
    const uint64_t times = argc == 2 ? std::stoi(argv[1]) : 1;

    std::ostringstream os = std::ostringstream();

    bench_mark(
        "terminal color output",
        times,
        Entry([&] { os << "\x1b[34m\x1b[m"; }).name("ostream/raw escape"),
        Entry([&] { os << fg(blue) << reset; }).name("ostream/Style output"),
        Entry([&] {
            std::print(os, "\x1b[34m\x1b[m");
        }).name("format/raw escape"),
        Entry([&] {
            std::print(os, "{}", fg(blue));
        }).name("format/Style output"));

    bench_mark("fixed RGB color output",
               times,
               Entry([&] {
                   os << "\x1b[48;2;156;234;108m\x1b[;m";
               }).name("ostream/raw escape"),
               Entry([&] {
                   os << bg(rgb(156, 234, 108));
               }).name("ostream/Style output"),
               Entry([&] {
                   std::print(os, "\x1b[48;2;156;234;108m\x1b[;m");
               }).name("format/raw escape"),
               Entry([&] {
                   std::print(os, "{}", bg(rgb(156, 234, 108)));
               }).name("format/Style output"));

    bench_mark("dynamic RGB color output",
               times,
               Entry([&](uint64_t i) {
                   os << "\x1b[48;2;" << (i & 255) << ';' << ((i >> 8) & 255)
                      << ';' << ((i >> 16) & 255) << "m\x1b[;m";
               }).name("ostream/raw escape"),
               Entry([&](uint64_t i) {
                   os << bg(rgb(i & 255, (i >> 8) & 255, (i >> 16) & 255))
                      << reset;
               }).name("ostream/Style output"),
               Entry([&](uint64_t i) {
                   std::print(os,
                              "\x1b[48;2;{};{};{}m\x1b[;m",
                              i & 255,
                              (i >> 8) & 225,
                              (i >> 16) & 255);
               }).name("format/raw escape"),
               Entry([&](uint64_t i) {
                   std::print(os,
                              "{}{}",
                              bg(rgb(i & 255, (i >> 8) & 255, (i >> 16) & 255)),
                              reset);
               }).name("format/Style output"));
}
