#include "benchmark.hpp"

#include <decoterm/output.hpp>

auto main(int argc, const char** argv) -> int {
    using namespace deco;

    std::ostringstream os;
    StyledOstream styled_os = styled_out(os).enable_context(false);
    StyledOstream styled_os_ctx = styled_out(os);

    Benchmark benchmark {
        Section { "StyledOstream",
            Entry("ostream raw escape code", [&]{  os << "\x1b[34m\x1b[m"; }),
            EntryCompare {
                Entry("ostream Style", [&]{ os << fg(blue) << reset; }),
                Entry("StyledOstream", [&]{ styled_os << fg(blue) << reset; }),
                Entry("StyledOs(w/ context)", [&]{ styled_os_ctx << fg(blue) << reset; })
            }
        },
    };

    parse_args(benchmark, argc, argv);
    benchmark.run();
    benchmark.print();
}
