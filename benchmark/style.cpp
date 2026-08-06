#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <cstdint>
#include <iostream>
#include <ostream>
#include <print>
#include <sstream>

#include "benchmark.hpp"

auto make_benchmark(std::ostream& os) {
    using namespace deco;
    return Benchmark {
        Section {"terminal color output",
                 EntryCompare(Entry("ostream/raw escape",
                                    [&] { os << "\x1b[34m\x1b[m"; }),
                              Entry("ostream/Style output",
                                    [&] { os << fg(blue) << reset; })),
                 EntryCompare(Entry("format/raw escape",
                                    [&] { std::print(os, "\x1b[34m\x1b[m"); }),
                              Entry("format/Style output",
                                    [&] { std::print(os, "{}", fg(blue)); }))},
        Section {
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
        Section {"dynamic RGB color output",
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
                                        std::print(os,
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
        Section {"style composition",
                 EntryCompare(Entry("Style",
                                    [&] {
                                        os << (fg(rgb(111, 122, 133))
                                               | bg(rgb(32, 64, 128)) | bold
                                               | italic | underline | dim
                                               | strikethrough | blink
                                               | invert);
                                    }),
                              Entry("constexpr Style", [&] {
                                  constexpr deco::Style s =
                                      fg(rgb(111, 122, 133))
                                      | bg(rgb(32, 64, 128)) | bold | italic
                                      | underline | dim | strikethrough | blink
                                      | invert;
                                  os << s;
                              }))}};
}

auto main(int argc, const char** argv) -> int {
    using namespace deco;

    void_ostream vo;
    std::ostringstream oss;

    Benchmark bench_vo = make_benchmark(vo);
    Benchmark bench_oss = make_benchmark(oss);
    Benchmark bench_cout = make_benchmark(std::cout);

    parse_args(bench_oss, argc, argv);
    parse_args(bench_vo, argc, argv);
    parse_args(bench_cout, argc, argv);


    bench_vo.run();
    bench_oss.run();
    bench_cout.run();

    std::cout << reset;
    std::cout << styled("[BENCHMARK: using void_ostream]\n", fg(cyan) | bold);
    bench_vo.print();

    std::cout << styled("[BENCHMARK: using ostring_stream]\n", fg(cyan) | bold);
    bench_oss.print();

    std::cout << styled("[BENCHMARK: using std::cout]\n", fg(cyan) | bold);
    bench_cout.print();

}
