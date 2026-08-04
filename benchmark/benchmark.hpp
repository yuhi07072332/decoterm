#ifndef DECOTERM_BENCHMARK_HPP
#define DECOTERM_BENCHMARK_HPP

#include <decoterm/style.hpp>
#include <decoterm/format.hpp>

#include <cstdint>
#include <chrono>
#include <concepts>
#include <string_view>

template <typename Fn>
inline auto measure(Fn&& fn, uint64_t i = 0) -> double {    // NOLINT
    using namespace std::chrono_literals;
    auto begin = std::chrono::steady_clock::now();
    if constexpr (std::invocable<Fn>) fn();
    else if constexpr (std::invocable<Fn, uint64_t>) fn(i);
    auto end = std::chrono::steady_clock::now();
    return (end - begin) / 1ns / 1000000.0;     //NOLINT
}

inline auto measure_avg(auto&& fn, uint64_t times) -> double {
    double avg = 0;
    for (uint64_t i = 0; i < times; ++i) {
        avg += measure(fn, i) / times;
    }
    return avg;
}

template <typename Fn>
    requires std::invocable<Fn> || std::invocable<Fn, uint64_t>
struct Entry {
    constexpr Entry(Fn fn)
        : fn(std::move(fn)) {}

    constexpr auto name(std::string name) -> Entry& {
        name_ = std::move(name);
        return *this;
    }

    Fn fn;
    std::string name_;
};

template <typename Fn>
inline void run_entry(uint64_t times, Entry<Fn>& entry) {
    double res = times == 1 ? measure(entry.fn) : measure_avg(entry.fn, times);
    std::print("{:24}:   n={:<8}   {:.6}ms\n", entry.name_, times, res);
}


template <typename... Entries>
inline void bench_mark(std::string_view name, uint64_t times = 1, Entries&&...entries /*NOLINT*/) {
    auto sfmt = deco::styled_fmt();
    sfmt.print(deco::bold | deco::fg(deco::bright_yellow), "BENCHMARK: {}", name)
        .print("\n\n");

    (run_entry(times, entries), ...);
    sfmt.print("\n");
}

#endif // !DECOTERM_BENCHMARK_HPPt
