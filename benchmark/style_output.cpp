#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <cstdint>
#include <iostream>
#include <ostream>
#include <print>
#include <streambuf>

#include "benchmark.hpp"

class void_streambuf final : public std::streambuf {
  protected:
    auto overflow(int_type ch) -> int_type override {
        return traits_type::not_eof(ch);
    }

    auto xsputn(const char_type*, std::streamsize count)
        -> std::streamsize override {
        return count;
    }
};

class void_ostream final : public std::ostream {
  public:
    void_ostream() : std::ostream(&buffer_) {} // NOLINT
  private:
    void_streambuf buffer_;
};

auto main(int argc, const char** argv) -> int {
    using namespace deco;

    void_ostream os;

    Benchmark benchlist {
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

    parse_args(benchlist, argc, argv);

    benchlist.run();
    benchlist.print();
}
