#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <cstdint>
#include <iostream>
#include <print>
#include <sstream>

#include "benchmark.hpp"

auto main(int argc, const char** argv) -> int {
    using namespace deco;

    std::ostringstream os = std::ostringstream();

    BenchmarkList benchlist {
        Benchmark {
            "terminal color output",
            EntryCompare(
                Entry("ostream/raw escape", [&] { os << "\x1b[34m\x1b[m"; }),
                Entry("ostream/Style output",
                      [&] { os << fg(blue) << reset; })),
            EntryCompare(Entry("format/raw escape",
                               [&] { std::print(os, "\x1b[34m\x1b[m"); }),
                         Entry("format/Style output",
                               [&] { std::print(os, "{}", fg(blue)); }))},
        Benchmark {
            "fixed RGB color output",
            EntryCompare(Entry("ostream/raw escape",
                               [&] { os << "\x1b[48;2;156;234;108m\x1b[;m"; }),
                         Entry("ostream/Style output",
                               [&] {
                                   constexpr deco::Style s =
                                       bg(rgb(156, 234, 108));
                                   os << s;
                               })),
            EntryCompare(
                Entry("format/raw escape",
                      [&] { std::print(os, "\x1b[48;2;156;234;108m\x1b[;m"); }),
                Entry("format/Style output",
                      [&] {
                          constexpr deco::Style s = bg(rgb(156, 234, 108));
                          std::print(os, "{}", s);
                      }))},
        Benchmark {"dynamic RGB color output",
                   EntryCompare(Entry("ostream/raw escape",
                                      [&](uint64_t i) {
                                          os << "\x1b[48;2;" << (i & 255) << ';'
                                             << ((i >> 8) & 255) << ';'
                                             << ((i >> 16) & 255) << "m\x1b[;m";
                                      }),
                                Entry("ostream/Style output",
                                      [&](uint64_t i) {
                                          os << bg(rgb(i & 255,
                                                       (i >> 8) & 255,
                                                       (i >> 16) & 255))
                                             << reset;
                                      })),
                   EntryCompare(Entry("format/raw escape",
                                      [&](uint64_t i) {
                                          std::print(
                                              os,
                                              "\x1b[48;2;{};{};{}m\x1b[;m",
                                              i & 255,
                                              (i >> 8) & 225,
                                              (i >> 16) & 255);
                                      }),
                                Entry("format/Style output",
                                      [&](uint64_t i) {
                                          std::print(os,
                                                     "{}{}",
                                                     bg(rgb(i & 255,
                                                            (i >> 8) & 255,
                                                            (i >> 16) & 255)),
                                                     reset);
                                      }))},
    };

    parse_args(benchlist, argc, argv);

    benchlist.run();
    benchlist.print();
}
