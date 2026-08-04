#ifndef DECOTERM_BENCHMARK_HPP
#define DECOTERM_BENCHMARK_HPP

#include <decoterm/format.hpp>
#include <decoterm/style.hpp>

#include <chrono>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>

#include "argparse.hpp"

namespace detail {

template <typename Fn>
inline auto measure(Fn&& fn, uint64_t i = 0) -> double { // NOLINT
    using namespace std::chrono_literals;
    auto begin = std::chrono::steady_clock::now();
    if constexpr (std::invocable<Fn>) fn();
    else if constexpr (std::invocable<Fn, uint64_t>) fn(i);
    auto end = std::chrono::steady_clock::now();
    return (end - begin) / 1ns / 1000000.0; // NOLINT
}

inline auto measure_avg(auto&& fn, uint64_t times) -> double {
    double avg = 0;
    for (uint64_t i = 0; i < times; ++i) {
        avg += measure(fn, i) / times;
    }
    return avg;
}

} // namespace detail

// NOLINTBEGIN
struct BenchOption : argparse::Args {
    uint64_t& repeat_per_entry =
        kwarg("n,repeat-per-entry", "repeat times per entry").set_default(1);
    uint64_t& repeat_per_bench =
        kwarg("r,repeat-per-bench", "repeat times per benchmark")
            .set_default(1);
};
// NOLINTEND

template <typename Fn>
    requires std::invocable<Fn> || std::invocable<Fn, uint64_t>
struct Entry {
    constexpr Entry(std::string name, Fn fn)
        : fn_(std::move(fn)),
          name_(std::move(name)) {}

    constexpr auto repeat(uint64_t times) -> Entry& {
        times_per_run_ = times;
        return *this;
    }
    constexpr auto repeat_times() -> std::optional<uint64_t>& {
        return times_per_run_;
    }

    constexpr auto fn() const -> Fn& { return fn_; }
    constexpr auto name() const -> std::string_view { return name_; }
    constexpr auto result() const -> double { return result_; }

    void run() {
        times_per_run_ = times_per_run_.value_or(1);
        run_count_++;
        result_ += (result_ / run_count_)
                   + (detail::measure_avg(fn_, *times_per_run_) / run_count_);
    }

    void print() {
        std::print("{:24}:   n={:<6} repeat={:<6}   {:.8f}ms",
                   name_,
                   *times_per_run_,
                   run_count_,
                   result_);
        if (*times_per_run_ > 1 || run_count_ > 1)
            std::print("{}",
                       deco::styled(" (average)", deco::fg(deco::yellow)));
    }

  private:
    Fn fn_;
    std::string name_;
    std::optional<uint64_t> times_per_run_;

    // result
    bool is_avg_ = false;
    uint64_t run_count_ = 0;
    double result_ = 0;
};

template <typename E1, typename E2>
struct EntryCompare {
    EntryCompare(E1 lhs, E2 rhs) : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    constexpr auto repeat(uint64_t times) -> EntryCompare& {
        times_ = times;
        return *this;
    }

    constexpr auto repeat_times() -> std::optional<uint64_t>& { return times_; }

    void run() {
        lhs_.repeat(times_.value_or(1));
        rhs_.repeat(times_.value_or(1));
        lhs_.run();
        rhs_.run();
    }

    void print() {
        std::print("┌ ");
        lhs_.print();
        std::println();
        std::print("└ ");
        rhs_.print();

        double diff = rhs_.result() - lhs_.result();
        if (diff > 0) {
            std::print("  {}{:.1f}% slower{}",
                       deco::fg(deco::red),
                       diff / lhs_.result() * 100,
                       deco::reset);
        } else if (diff < 0) {
            std::print("  {}{:.1f}% faster{}",
                       deco::fg(deco::green),
                       -diff / lhs_.result() * 100,
                       deco::reset);
        }
    }

  private:
    E1 lhs_;
    E2 rhs_;

    std::optional<uint64_t> times_;
};

template <typename... Entries>
struct Benchmark {
    Benchmark(std::string name, Entries... entries)
        : name_(std::move(name)),
          entries_(std::move(entries)...) {}

    constexpr auto get() -> std::tuple<Entries...>& { return entries_; }

    constexpr auto repeat(uint64_t times) -> Benchmark& {
        times_ = times;
        return *this;
    }

    constexpr auto repeat_per_entry(uint64_t times) -> Benchmark& {
        std::apply(
            [times](auto&... entries) {
                ((entries.repeat_times() =
                      entries.repeat_times().value_or(times)),
                 ...);
            },
            entries_);
        return *this;
    }

    void run() {
        for (uint64_t i = 0; i < times_; ++i)
            std::apply([](auto&... entries) { (entries.run(), ...); },
                       entries_);
    }

    void print() {
        auto sfmt = deco::styled_fmt();
        sfmt.print(deco::bold | deco::fg(deco::bright_yellow),
                   "BENCHMARK: {}",
                   name_)
            .print("\n\n");

        std::apply(
            [](auto&... entries) {
                ((entries.print(), std::print("\n")), ...);
            },
            entries_);
        sfmt.print("\n");
    }

  private:
    std::string name_;
    uint64_t times_ = 1;
    std::tuple<Entries...> entries_;
};

template <typename T> struct is_benchmark : std::false_type {};
template <typename... Ts> struct is_benchmark<Benchmark<Ts...>>
    : std::true_type {};
template <typename T>
concept benchmark = is_benchmark<T>::value;

template <benchmark... Benchmarks>
struct BenchmarkList {
    BenchmarkList(Benchmarks... benchmarks)
        : benchmarks_(std::move(benchmarks)...) {}

    constexpr auto get() -> std::tuple<Benchmarks...>& { return benchmarks_; }

    void run() {
        std::apply([](auto&... benchmarks) { (benchmarks.run(), ...); },
                   benchmarks_);
    }

    void print() {
        std::apply([](auto&... benchmarks) { (benchmarks.print(), ...); },
                   benchmarks_);
    }

  private:
    std::tuple<Benchmarks...> benchmarks_;
};

template <typename BenchmarkList>
void parse_args(BenchmarkList& benchlist, int argc, const char** argv) {
    if (argc < 2) {
        std::print("{}NOTE: see '{} --help' for options.{}\n\n",
                   deco::fg(deco::bright_cyan) | deco::bold,
                   argv[0],
                   deco::reset);
    }

    auto option = argparse::parse<BenchOption>(argc, argv, true);
    std::apply(
        [&option](auto&... benchmarks) {
            ((benchmarks.repeat(option.repeat_per_bench),
              benchmarks.repeat_per_entry(option.repeat_per_entry)),
             ...);
        },
        benchlist.get());
}

#endif // !DECOTERM_BENCHMARK_HPP
