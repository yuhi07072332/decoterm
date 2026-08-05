#ifndef DECOTERM_BENCHMARK_HPP
#define DECOTERM_BENCHMARK_HPP

#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <algorithm>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
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
                      uint64_t iterations,              // NOLINT
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
    bool& disable_transition = flag(
        "disable-transition", "disable running transition between entries");
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

    void print() { std::print("{:24}  {:8.2f}ns", name, *result); }

    Fn fn;
    std::string name;
    std::optional<result_ns_t> result;
};

template <typename E1, typename E2>
struct EntryCompare {
    EntryCompare(E1 lhs, E2 rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void run(uint64_t iterations,
             uint64_t rounds,
             std::function<void()>* transition_fn_ = nullptr) { // NOLINT
        lhs.run(iterations, rounds);
        if (transition_fn_ && *transition_fn_) transition_fn_->operator()();
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

template <typename T>
struct is_entry_compare : std::false_type {};

template <typename T, typename U>
struct is_entry_compare<EntryCompare<T, U>> : std::true_type {};

template <typename... Entries>
struct Section {
    Section(std::string name, Entries... entries)
        : name_(std::move(name)),
          entries_(std::move(entries)...) {}

    constexpr auto transition(std::function<void()> fn) -> Section& {
        transition_fn_ = std::move(fn);
        return *this;
    }

    constexpr auto get() -> std::tuple<Entries...>& { return entries_; }

    void run(uint64_t iterations,
             uint64_t rounds,
             bool enable_transition = true) { // NOLINT
        std::apply(
            [&, this](Entries&... entries) {
                (run_entry(entries, iterations, rounds, enable_transition), ...);
            },
            entries_);
    }

    void print() {
        auto sfmt = deco::styled_fmt();
        sfmt.print(deco::bold | deco::fg(deco::bright_yellow),
                   "SECTION{}: {}",
                   number_ ? std::format("[{}]", *number_) : "",
                   name_);

        sfmt.print("\n\n");

        std::apply(
            [](Entries&... entries) {
                ((entries.print(), std::print("\n")), ...);
            },
            entries_);
        sfmt.print("\n");
    }

  private:
    template <typename...>
    friend class Benchmark;

    template <typename Entry>
    void run_entry(Entry& entry,
                   uint64_t iterations,
                   uint64_t rounds,
                   bool enable_transition) { // NOLINT
        if constexpr (is_entry_compare<std::remove_cvref_t<Entry>>::value) {
            if (transition_fn_ && enable_transition)
                entry.run(iterations, rounds, &transition_fn_);
            else entry.run(iterations, rounds);
        } else {
            entry.run(iterations, rounds);
        }
        if (enable_transition && transition_fn_) transition_fn_();
    }

    constexpr void set_number(int n) { number_ = n; }

    std::function<void()> transition_fn_;
    std::optional<int> number_;
    std::string name_;
    std::tuple<Entries...> entries_;
};

template <typename... Sections>
struct Benchmark {
    static constexpr std::size_t N = sizeof...(Sections);

    constexpr Benchmark(Sections... sections) // NOLINT
        : sections(std::move(sections)...) {
        [this]<std::size_t... Is>(std::index_sequence<Is...>) constexpr {
            (std::get<Is>(this->sections).set_number(Is), ...);
        }(std::make_index_sequence<sizeof...(Sections)>());
    }

    void run() {
        std::apply(
            [this](Sections&... sections) constexpr {
                    (sections.run(iterations, rounds), ...);
            },
            sections);
    }

    void print() {
        if (!enable_transition)
            std::print("{}transition disabled{}\n",
                       deco::fg(deco::cyan) | deco::bold,
                       deco::reset);

        std::println();

        std::print("{}iterations: {}{}\n",
                   deco::fg(deco::yellow),
                   iterations,
                   deco::reset);
        std::print("rounds: {}\n", rounds);

        std::println();

        std::apply(
            [](Sections&... sections) constexpr { (sections.print(), ...); },
            sections);
    }

    uint64_t iterations = 0;
    uint64_t rounds = 0;
    bool enable_transition = true;
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

    try {
        auto option = argparse::parse<BenchOption>(argc, argv, true);
        benchmark.iterations = option.iterations;
        benchmark.rounds = option.rounds;
        benchmark.enable_transition = !option.disable_transition;
    } catch (std::exception& e) {
        std::cerr << deco::styled("error: ", deco::fg(deco::red) | deco::bold)
                  << e.what() << '\n';
        std::exit(1);
    }
}

#endif // !DECOTERM_BENCHMARK_HPP
