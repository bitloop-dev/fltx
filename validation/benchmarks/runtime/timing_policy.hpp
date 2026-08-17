#ifndef FLTX_TESTS_BENCHMARK_TIMING_POLICY_INCLUDED
#define FLTX_TESTS_BENCHMARK_TIMING_POLICY_INCLUDED

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string_view>
#include <utility>

namespace fltx::tests::benchmark::timing_policy
{
    inline constexpr std::string_view identity = "adaptive-v2";
    inline constexpr double fast_threshold_ns = 20.0;
    inline constexpr double slow_threshold_ns = 10'000.0;
    inline constexpr double fast_trial_target_ns = 15'000'000.0;
    inline constexpr double normal_trial_target_ns = 8'000'000.0;
    inline constexpr double maximum_row_ns = 5'000'000'000.0;
    inline constexpr std::size_t fast_trials = 7;
    inline constexpr std::size_t ordinary_trials = 3;

    struct trial_policy
    {
        std::size_t trials;
        double target_trial_ns;
    };

    struct calibration
    {
        std::size_t batches;
        double elapsed_ns;
        std::size_t calls;
    };

    template<class Measure>
    void for_each_measurement_primary_first(
        std::size_t measurement_count,
        Measure&& measure)
    {
        for (std::size_t index = 0; index < measurement_count; ++index)
            measure(index);
    }

    [[nodiscard]] inline trial_policy select(double ns_per_iteration) noexcept
    {
        if (ns_per_iteration < fast_threshold_ns)
            return { fast_trials, fast_trial_target_ns };
        if (ns_per_iteration < slow_threshold_ns)
            return { ordinary_trials, normal_trial_target_ns };
        return { ordinary_trials, 0.0 };
    }

    [[nodiscard]] inline std::size_t fitting_trials(
        std::size_t requested,
        double estimated_trial_ns) noexcept
    {
        if (requested == 0 || estimated_trial_ns <= 0.0)
            return 1;
        while (
            requested > 1 &&
            static_cast<double>(requested) * estimated_trial_ns > maximum_row_ns)
        {
            requested -= 2;
        }
        return std::max<std::size_t>(1, requested);
    }

    template<class Measure>
    [[nodiscard]] calibration calibrate_after_pilot(
        Measure&& measure,
        double pilot_elapsed_ns,
        double target_ns,
        std::size_t maximum_batches)
    {
        std::size_t batches = 1;
        double elapsed = pilot_elapsed_ns;
        std::size_t calls = 1;
        if (target_ns <= 0.0)
            return { batches, elapsed, calls };

        for (int attempt = 1; attempt < 5; ++attempt)
        {
            if (elapsed >= target_ns || batches == maximum_batches)
                break;
            const double scale = target_ns / std::max(elapsed, 1.0);
            const auto requested = static_cast<std::size_t>(
                std::ceil(static_cast<double>(batches) * scale * 1.1));
            batches = std::min(
                maximum_batches,
                std::max(batches + 1, requested));
            elapsed = measure(batches);
            ++calls;
        }
        return { batches, elapsed, calls };
    }

    template<class Measure>
    [[nodiscard]] calibration calibrate_once(
        Measure&& measure,
        double target_ns,
        std::size_t maximum_batches)
    {
        const double pilot_elapsed_ns = measure(1);
        return calibrate_after_pilot(
            std::forward<Measure>(measure),
            pilot_elapsed_ns,
            target_ns,
            maximum_batches);
    }
}

#endif
