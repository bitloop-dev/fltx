#ifndef FLTX_TESTS_METRICS_RECORDS_INCLUDED
#define FLTX_TESTS_METRICS_RECORDS_INCLUDED

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string_view>
#include <vector>

#include "metrics_config.h"
#include "metrics_types.h"

namespace bl::test::metrics
{
    enum class special_support : unsigned
    {
        unavailable = 0,
        none        = 1u << 0,
        inf         = 1u << 1,
        nan         = 1u << 2,
        both        = inf | nan
    };

    [[nodiscard]] constexpr inline bool has_inf_support(special_support value) noexcept
    {
        return (static_cast<unsigned>(value) & static_cast<unsigned>(special_support::inf)) != 0;
    }

    [[nodiscard]] constexpr inline bool has_nan_support(special_support value) noexcept
    {
        return (static_cast<unsigned>(value) & static_cast<unsigned>(special_support::nan)) != 0;
    }

    [[nodiscard]] constexpr inline special_support make_special_support(bool inf, bool nan) noexcept
    {
        if (inf && nan)
            return special_support::both;
        if (inf)
            return special_support::inf;
        if (nan)
            return special_support::nan;
        return special_support::none;
    }

    [[nodiscard]] constexpr inline special_support merge_special_support(
        special_support lhs,
        special_support rhs) noexcept
    {
        if (lhs == special_support::unavailable)
            return rhs;
        if (rhs == special_support::unavailable)
            return lhs;
        return make_special_support(
            has_inf_support(lhs) || has_inf_support(rhs),
            has_nan_support(lhs) || has_nan_support(rhs));
    }

    struct special_support_accumulator
    {
        bool inf_seen = false;
        bool nan_seen = false;
        bool inf_ok = true;
        bool nan_ok = true;

        constexpr void record(bool has_inf, bool has_nan, bool passed) noexcept
        {
            if (has_inf)
            {
                inf_seen = true;
                inf_ok = inf_ok && passed;
            }
            if (has_nan)
            {
                nan_seen = true;
                nan_ok = nan_ok && passed;
            }
        }

        [[nodiscard]] constexpr special_support result() const noexcept
        {
            if (!inf_seen && !nan_seen)
                return special_support::unavailable;
            return make_special_support(inf_seen && inf_ok, nan_seen && nan_ok);
        }
    };

    struct accuracy_result
    {
        double worst_bits = std::numeric_limits<double>::infinity();
        double mean_bits = std::numeric_limits<double>::infinity();
        std::size_t sample_count = 0;
        double domain_score = 0.0;
    };

    struct benchmark_result
    {
        double ns_per_iter = 0.0;
        std::size_t iteration_count = 0;
    };

    struct competitor_result
    {
        std::string_view name;
        bool supported = !config::benchmark_only_fltx;
        accuracy_result accuracy;
        benchmark_result benchmark;
        special_support special_values = special_support::unavailable;
    };

    struct metrics_record
    {
        suite_id suite;
        accuracy_result fltx_accuracy;
        special_support fltx_special_values = special_support::unavailable;
        std::string_view competitor_name = "comp";
        bool competitor_supported = !config::benchmark_only_fltx;
        accuracy_result competitor_accuracy;
        special_support competitor_special_values = special_support::unavailable;
        benchmark_result fltx_benchmark;
        benchmark_result competitor_benchmark;
        std::vector<competitor_result> extra_competitors;
    };

    [[nodiscard]] constexpr inline bool competitors_enabled() noexcept
    {
        return !config::benchmark_only_fltx;
    }

    inline competitor_result& add_extra_competitor(
        metrics_record& record,
        std::string_view name,
        bool supported = true)
    {
        competitor_result& competitor = record.extra_competitors.emplace_back();
        competitor.name = name;
        competitor.supported = competitors_enabled() && supported;
        return competitor;
    }

    [[nodiscard]] inline double domain_sample_score(double bits, double ideal_bits) noexcept
    {
        if (std::isnan(bits))
            return 0.0;
        if (std::isinf(bits))
            return bits > 0.0 ? 1.0 : 0.0;
        if (ideal_bits <= 0.0)
            return bits > 0.0 ? 1.0 : 0.0;

        return std::clamp(bits / ideal_bits, 0.0, 1.0);
    }

    [[nodiscard]] inline double domain_score(std::vector<double> sample_scores)
    {
        if (sample_scores.empty())
            return 0.0;

        double total = 0.0;
        double worst = 1.0;
        for (double score : sample_scores)
        {
            total += score;
            worst = std::min(worst, score);
        }

        std::sort(sample_scores.begin(), sample_scores.end());
        const std::size_t lower_tail_index =
            std::min<std::size_t>(
                sample_scores.size() - 1,
                static_cast<std::size_t>(static_cast<double>(sample_scores.size() - 1) * 0.01));
        const double mean = total / static_cast<double>(sample_scores.size());
        const double lower_tail = sample_scores[lower_tail_index];

        return 100.0 * (mean * 0.50 + lower_tail * 0.30 + worst * 0.20);
    }
}

#endif
