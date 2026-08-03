#ifndef DECOTERM_BENCHMARK_HPP
#define DECOTERM_BENCHMARK_HPP

#include <decoterm/style.hpp>

#include <chrono>
#include <concepts>
#include <string_view>

inline auto measure(auto&& fn) -> double {
    using namespace std::chrono_literals;
    auto begin = std::chrono::steady_clock::now();
    fn();
    auto end = std::chrono::steady_clock::now();
    return (end - begin) / 1ns / 1000000.0;     //NOLINT
}

inline auto measure_avg(auto&& fn, long long times) -> double {
    double avg = 0;
    for (long long i = 0; i < times; ++i) {
        avg += measure(fn) / times;
    }
    return avg;
}

template <std::invocable<void> Fn>
struct Entry {
    constexpr Entry(Fn fn)
        : fn(std::move(fn)) {}

    constexpr auto times(int times) -> Entry& { times_ = times; return *this; }

    Fn fn;
    int times_ = 1;
};

template <typename... Entries>
inline void bench_mark(std::string_view name, Entries&&...entries) {
}

#endif // !DECOTERM_BENCHMARK_HPPt
