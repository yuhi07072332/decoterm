#ifndef DECOTERM_BENCHMARK_HPP
#define DECOTERM_BENCHMARK_HPP

#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <tuple>
#include <vector>

#include "argparse.hpp"

// nanoseconds
using result_ns_t = double;

template <typename Fn>
concept benchmark_fn = std::invocable<Fn> || std::invocable<Fn, uint64_t>;

namespace detail {

template <benchmark_fn Fn>
inline void invoke_benchmark(Fn& fn, uint64_t i = 0) {
    if constexpr (std::invocable<Fn>) fn();
    else if constexpr (std::invocable<Fn, uint64_t>) fn(i);
}

inline auto measure(benchmark_fn auto& fn, uint64_t iterations) -> result_ns_t {
    const auto begin = std::chrono::steady_clock::now();
    for (uint64_t i = 0; i < iterations; ++i) {
        invoke_benchmark(fn, i);
    }
    const auto end = std::chrono::steady_clock::now();

    const double total =
        std::chrono::duration<double, std::nano>(end - begin).count();
    return total / static_cast<double>(iterations);
}

inline auto benchmark(benchmark_fn auto&& fn,
                      uint64_t iterations, // NOLINT
                      uint64_t rounds) -> result_ns_t { // NOLINT
    for (int i = 0; i < 10000; ++i)
        invoke_benchmark(fn, i);

    std::vector<result_ns_t> results(rounds);

    for (uint64_t i = 0; i < rounds; ++i) {
        results[i] = measure(fn, iterations);
    }

    std::ranges::sort(results);
    return results[rounds / 2];
}

} // namespace detail

// NOLINTBEGIN
struct BenchOption : argparse::Args {
    uint64_t& iterations =
        kwarg("n,iterations", "iterations per entry").set_default(100);
    uint64_t& rounds = kwarg("r,rounds", "rounds per entry").set_default(10);
};
// NOLINTEND

template <benchmark_fn Fn>
    requires std::invocable<Fn> || std::invocable<Fn, uint64_t>
struct Entry {
    constexpr Entry(std::string name, Fn fn)
        : fn(std::move(fn)),
          name(std::move(name)) {}

    void run(uint64_t iterations, uint64_t rounds) { // NOLINT
        result = detail::benchmark(fn, iterations, rounds);
    }

    void print() { std::print("{:24}:  {:8.2f}ns", name, *result); }

    Fn fn;
    std::string name;
    std::optional<result_ns_t> result;
};

template <typename E1, typename E2>
struct EntryCompare {
    EntryCompare(E1 lhs, E2 rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void run(uint64_t iterations, uint64_t rounds) { // NOLINT
        lhs.run(iterations, rounds);
        rhs.run(iterations, rounds);
    }

    void print() {
        std::print("┌ ");
        lhs.print();
        std::println();
        std::print("└ ");
        rhs.print();

        const double diff = *rhs.result - *lhs.result;
        const double diff_percent = diff / *lhs.result * 100;

        if (diff > 0) {
            std::print("  {}{:+.8f}ms ({:.1f}% slower){}",
                       deco::fg(deco::red),
                       diff / 10000000,
                       diff_percent,
                       deco::reset);
        } else if (diff < 0) {
            std::print("  {}{:+.8f}ms ({:.1f}% faster){}",
                       deco::fg(deco::green),
                       diff / 10000000,
                       diff_percent,
                       deco::reset);
        }
    }

    E1 lhs;
    E2 rhs;
};

template <typename... Entries>
struct Section {
    Section(std::string name, Entries... entries)
        : name_(std::move(name)),
          entries_(std::move(entries)...) {}

    constexpr auto get() -> std::tuple<Entries...>& { return entries_; }

    void run(uint64_t iterations, uint64_t rounds) { // NOLINT
        std::apply(
            [=](auto&... entries) { (entries.run(iterations, rounds), ...); },
            entries_);
    }

    void print() {
        auto sfmt = deco::styled_fmt();
        sfmt.print(
            deco::bold | deco::fg(deco::bright_yellow), "SECTION: {}", name_);

        sfmt.print("\n\n");

        std::apply(
            [](auto&... entries) {
                ((entries.print(), std::print("\n")), ...);
            },
            entries_);
        sfmt.print("\n");
    }

  private:
    std::string name_;
    std::tuple<Entries...> entries_;
};

template <typename... Sections>
struct Benchmark {
    Benchmark(Sections... sections) : sections(std::move(sections)...) {}

    void run() {
        std::apply(
            [this](auto&... sections) {
                (sections.run(iterations, rounds), ...);
            },
            sections);
    }

    void print() {
        std::print("{}iterations: {}{}\n",
                   deco::fg(deco::yellow) | deco::bold,
                   iterations,
                   deco::reset);
        std::print("rounds: {}\n\n", rounds);

        std::apply([](auto&... sections) { (sections.print(), ...); },
                   sections);
    }

    uint64_t iterations = 0;
    uint64_t rounds = 0;
    std::tuple<Sections...> sections;
};

template <typename Benchmark>
void parse_args(Benchmark& benchmark, int argc, const char** argv) {
    if (argc < 2) {
        std::print("{}NOTE: see '{} --help' for options.{}\n\n",
                   deco::fg(deco::bright_cyan) | deco::bold,
                   argv[0],
                   deco::reset);
    }

    auto option = argparse::parse<BenchOption>(argc, argv, true);
    benchmark.iterations = option.iterations;
    benchmark.rounds = option.rounds;
}

#endif // !DECOTERM_BENCHMARK_HPP
