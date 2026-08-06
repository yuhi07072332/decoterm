#include "benchmark.hpp"

#include <decoterm/output.hpp>

auto main(int argc, const char** argv) -> int {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os = StyledOstream(os);
    StyledOstream styled_os_ctx = styled_out(os);

    StyledFormat styled_fmt = StyledFormat().set_stream(os);
    StyledFormat styled_fmt_ctx = fstyled_out(os);

    Benchmark benchmark {
        Section {
            "StyledOstream",
            Entry("ostream raw escape code", [&] { os << "\x1b[34m\x1b[m"; }),
            EntryCompare {
                Entry("ostream Style", [&] { os << fg(blue) << reset; }),
                Entry("StyledOstream", [&] { styled_os << fg(blue) << reset; }),
                Entry("StyledOs(w/ context)",
                      [&] { styled_os_ctx << fg(blue) << reset; })}},

        Section {
            "StyledFormat",
            Entry("format raw escape code",
                  [&] { std::print(os, "\x1b[34m\x1b[m"); }),
            EntryCompare {
                Entry("format Style",
                      [&] { std::print(os, "{}", styled("", fg(blue))); }),
                Entry("StyledFormat",
                      [&] { styled_fmt.print("{}", styled("", fg(blue))); }),
                Entry("StyledFmt(w/ context)",
                      [&] {
                          styled_fmt_ctx.print("{}", styled("", fg(blue)));
                      })}},

    };

    parse_args(benchmark, argc, argv);
    benchmark.run();
    benchmark.print();
}
