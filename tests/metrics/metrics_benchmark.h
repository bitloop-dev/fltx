#ifndef FLTX_TESTS_METRICS_BENCHMARK_INCLUDED
#define FLTX_TESTS_METRICS_BENCHMARK_INCLUDED

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>

#include "metrics_config.h"
#include "metrics_records.h"

namespace bl::test::metrics
{
    template<class TrialFn>
    [[nodiscard]] benchmark_result benchmark_trials(
        std::size_t sample_count,
        std::size_t repetitions,
        TrialFn&& trial)
    {
        if (sample_count == 0 || repetitions == 0)
            return {};

        const std::size_t iterations = repetitions * sample_count;

        for (std::size_t i = 0; i < config::benchmark_warmup_trials; ++i)
            trial();

        std::array<double, config::benchmark_timing_trials> ns{};
        for (double& value : ns)
        {
            const auto start = std::chrono::steady_clock::now();
            trial();
            const auto elapsed = std::chrono::steady_clock::now() - start;
            value = std::chrono::duration<double, std::nano>(elapsed).count()
                / static_cast<double>(iterations);
        }

        std::nth_element(ns.begin(), ns.begin() + ns.size() / 2, ns.end());
        return { ns[ns.size() / 2], iterations };
    }
}

#endif
