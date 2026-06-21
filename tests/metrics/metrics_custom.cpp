#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <fltx/f128_math.h>
#include <fltx/f256_math.h>
#include <fltx/io.h>

#include "metrics_case_output.h"
#include "metrics_config.h"
#include "metrics_f128_primary.h"
#include "metrics_f256_primary.h"
#include "metrics_reference.h"
#include "metrics_samples.h"

namespace bl::test::metrics::custom
{
    constexpr domain_id custom_domain{ "custom", domain_role::primary };

    template<class Float>
    struct profile;

    template<>
    struct profile<bl::f128>
    {
        using references = reference_types<bl::f128>;
        using fltx_type = references::fltx_type;
        using perfect_ref = references::perfect_ref;
        using competitor_ref = references::competitor_ref;
        using extra_competitor_ref = references::extra_competitor_ref;
        using sample_rng = f128_primary::sample_rng;

        static constexpr precision_type precision = precision_type::f128;
        static constexpr std::string_view precision_name = references::precision_name;
        static constexpr std::string_view competitor_name = references::competitor_name;
        static constexpr std::string_view extra_competitor_name = references::extra_competitor_name;
        static constexpr double required_bits = f128_primary::bits_90;

        [[nodiscard]] static std::size_t random_sample_count() noexcept
        {
            return f128_primary::random_sample_count();
        }

        [[nodiscard]] static value_sample make_runtime_value(std::string_view label, const fltx_type& value) noexcept
        {
            return f128_primary::make_runtime_value(label, value);
        }

        [[nodiscard]] static fltx_type make_fltx(const value_sample& value) noexcept
        {
            return f128_primary::make_fltx(value);
        }

        [[nodiscard]] static competitor_ref make_competitor(const value_sample& value)
        {
            return f128_primary::make_competitor(value);
        }

        [[nodiscard]] static extra_competitor_ref make_extra_competitor(const value_sample& value)
        {
            return f128_primary::make_extra_competitor(value);
        }

        [[nodiscard]] static perfect_ref make_perfect(const value_sample& value)
        {
            return f128_primary::make_perfect(value);
        }

        [[nodiscard]] static perfect_ref to_perfect(const fltx_type& value)
        {
            return f128_primary::to_perfect(value);
        }

        [[nodiscard]] static perfect_ref to_perfect(const competitor_ref& value)
        {
            return f128_primary::to_perfect(value);
        }

        [[nodiscard]] static perfect_ref to_perfect(const extra_competitor_ref& value)
        {
            return f128_primary::to_perfect(value);
        }

        [[nodiscard]] static perfect_ref target_expected(const perfect_ref& exact)
        {
            return f128_primary::target_expected(exact);
        }

        [[nodiscard]] static double target_ideal_bits_for(const perfect_ref& expected)
        {
            return f128_primary::target_ideal_bits_for(expected);
        }

        [[nodiscard]] static double finite_for_mean(double bits) noexcept
        {
            return f128_primary::finite_for_mean(bits);
        }

        [[nodiscard]] static fltx_type signed_log_value(sample_rng& rng, int min_exp, int max_exp) noexcept
        {
            return f128_primary::signed_log_value(rng, min_exp, max_exp);
        }

        template<class Values, class EvalFn>
        [[nodiscard]] static benchmark_result benchmark_unary_int_values(
            const Values& values,
            EvalFn eval,
            std::string_view operation)
        {
            return f128_primary::benchmark_unary_int_values(values, eval, operation);
        }
    };

    template<>
    struct profile<bl::f256>
    {
        using references = reference_types<bl::f256>;
        using fltx_type = references::fltx_type;
        using perfect_ref = references::perfect_ref;
        using competitor_ref = references::competitor_ref;
        using extra_competitor_ref = references::extra_competitor_ref;
        using sample_rng = f256_primary::sample_rng;

        static constexpr precision_type precision = precision_type::f256;
        static constexpr std::string_view precision_name = references::precision_name;
        static constexpr std::string_view competitor_name = references::competitor_name;
        static constexpr std::string_view extra_competitor_name = references::extra_competitor_name;
        static constexpr double required_bits = 180.0;

        [[nodiscard]] static std::size_t random_sample_count() noexcept
        {
            return f256_primary::random_sample_count();
        }

        [[nodiscard]] static value_sample make_runtime_value(std::string_view label, const fltx_type& value) noexcept
        {
            return f256_primary::make_runtime_value(label, value);
        }

        [[nodiscard]] static fltx_type make_fltx(const value_sample& value) noexcept
        {
            return f256_primary::make_fltx(value);
        }

        [[nodiscard]] static competitor_ref make_competitor(const value_sample& value)
        {
            return f256_primary::make_competitor(value);
        }

        [[nodiscard]] static extra_competitor_ref make_extra_competitor(const value_sample& value)
        {
            return f256_primary::make_extra_competitor(value);
        }

        [[nodiscard]] static perfect_ref make_perfect(const value_sample& value)
        {
            return f256_primary::make_perfect(value);
        }

        [[nodiscard]] static perfect_ref to_perfect(const fltx_type& value)
        {
            return f256_primary::to_perfect(value);
        }

        [[nodiscard]] static perfect_ref to_perfect(const competitor_ref& value)
        {
            return f256_primary::to_perfect(value);
        }

        [[nodiscard]] static perfect_ref to_perfect(const extra_competitor_ref& value)
        {
            return f256_primary::to_perfect(value);
        }

        [[nodiscard]] static perfect_ref target_expected(const perfect_ref& exact)
        {
            return f256_primary::target_expected(exact);
        }

        [[nodiscard]] static double target_ideal_bits_for(const perfect_ref& expected)
        {
            return f256_primary::target_ideal_bits_for(expected);
        }

        [[nodiscard]] static double finite_for_mean(double bits) noexcept
        {
            return f256_primary::finite_for_mean(bits);
        }

        [[nodiscard]] static fltx_type signed_log_value(sample_rng& rng, int min_exp, int max_exp) noexcept
        {
            return f256_primary::signed_log_value(rng, min_exp, max_exp);
        }

        template<class Values, class EvalFn>
        [[nodiscard]] static benchmark_result benchmark_unary_int_values(
            const Values& values,
            EvalFn eval,
            std::string_view operation)
        {
            return f256_primary::benchmark_unary_int_values(values, eval, operation);
        }
    };

    template<class Float>
    [[nodiscard]] typename profile<Float>::perfect_ref pow10_reference(int exponent)
    {
        using perfect_ref = typename profile<Float>::perfect_ref;

        perfect_ref value = 1;
        perfect_ref base = 10;
        unsigned n = static_cast<unsigned>(exponent < 0 ? -exponent : exponent);
        while (n != 0)
        {
            if ((n & 1u) != 0)
                value *= base;
            n >>= 1u;
            if (n != 0)
                base *= base;
        }
        return exponent < 0 ? perfect_ref{ 1 } / value : value;
    }

    template<class Float>
    [[nodiscard]] typename profile<Float>::perfect_ref round_to_decimals_reference(
        const typename profile<Float>::perfect_ref& value,
        int decimals)
    {
        using perfect_ref = typename profile<Float>::perfect_ref;
        if (ref_is_nonfinite(value) || decimals <= 0)
            return value;

        const perfect_ref scale = pow10_reference<Float>(decimals);
        return round_nearest_even_integer_reference(value * scale) / scale;
    }

    template<class Float>
    [[nodiscard]] suite_id custom_suite(std::string_view operation) noexcept
    {
        return {
            profile<Float>::precision,
            operation_id{ operation, operation },
            custom_domain
        };
    }

    template<class Float>
    [[nodiscard]] metrics_record make_custom_record(std::string_view operation, bool competitors_supported = false)
    {
        metrics_record record{};
        record.suite = custom_suite<Float>(operation);
        record.competitor_name = profile<Float>::competitor_name;
        record.competitor_supported = competitors_enabled() && competitors_supported;
        record.competitor_special_values = special_support::unavailable;
        add_extra_competitor(record, profile<Float>::extra_competitor_name, competitors_supported);
        return record;
    }

    template<class Float>
    [[nodiscard]] unary_int_sample make_text_sample(std::string_view label, std::string_view text, int n)
    {
        using prof = profile<Float>;
        const auto value = bl::parse<typename prof::fltx_type>(text);
        return { label, prof::make_runtime_value(label, value), n };
    }

    template<class Float>
    [[nodiscard]] unary_int_sample make_value_sample(
        std::string_view label,
        const typename profile<Float>::fltx_type& value,
        int n)
    {
        return { label, profile<Float>::make_runtime_value(label, value), n };
    }

    template<class Float>
    [[nodiscard]] std::vector<unary_int_sample> make_round_to_decimals_samples(std::size_t random_count)
    {
        using prof = profile<Float>;
        constexpr int digits = std::numeric_limits<typename prof::fltx_type>::digits10;
        std::vector<unary_int_sample> samples;
        samples.reserve(12 + random_count);

        samples.push_back(make_text_sample<Float>("unit", "1.23456789012345678901234567890123456789", std::min(2, digits)));
        samples.push_back(make_text_sample<Float>("negative", "-987654321.123456789123456789", std::min(6, digits)));
        samples.push_back(make_text_sample<Float>("tiny", "0.000000000000000000123456789123456789", std::min(25, digits)));
        samples.push_back(make_text_sample<Float>("large", "123456789012345.987654321987654321", std::min(3, digits)));
        samples.push_back(make_text_sample<Float>("tie-even", "2.125", std::min(2, digits)));
        samples.push_back(make_text_sample<Float>("tie-up", "2.135", std::min(2, digits)));
        samples.push_back(make_text_sample<Float>("carry", "9.999999999999999999999999999999999999", std::min(8, digits)));
        samples.push_back(make_text_sample<Float>("negative-carry", "-9.999999999999999999999999999999999999", std::min(8, digits)));

        typename prof::sample_rng rng{ 0x5c0570adec1a15ull };
        for (std::size_t i = 0; i < random_count; ++i)
        {
            const auto value = prof::signed_log_value(rng, -64, 64);
            const int decimals = rng.integer(0, digits);
            samples.push_back(make_value_sample<Float>("random", value, decimals));
        }

        return samples;
    }

    template<class Float>
    [[nodiscard]] std::vector<unary_int_sample> make_pow10_samples(std::size_t random_count)
    {
        using prof = profile<Float>;
        std::vector<unary_int_sample> samples;
        samples.reserve(11 + random_count);

        for (int exponent : { -300, -128, -32, -10, -1, 0, 1, 10, 32, 128, 300 })
            samples.push_back(make_value_sample<Float>("pow10", typename prof::fltx_type{ 0.0 }, exponent));

        typename prof::sample_rng rng{ 0x90a11c0570decadeull };
        for (std::size_t i = 0; i < random_count; ++i)
            samples.push_back(make_value_sample<Float>("random", typename prof::fltx_type{ 0.0 }, rng.integer(-300, 300)));

        return samples;
    }

    template<class T, class Samples, class MakeFn>
    [[nodiscard]] std::vector<unary_int_value<T>> make_values(const Samples& samples, MakeFn make_value)
    {
        std::vector<unary_int_value<T>> values;
        values.reserve(samples.size());
        for (const unary_int_sample& sample : samples)
            values.push_back({ make_value(sample.x), sample.n });
        return values;
    }

    template<class Float, class Samples>
    [[nodiscard]] std::vector<unary_int_value<typename profile<Float>::fltx_type>> make_fltx_values(const Samples& samples)
    {
        return make_values<typename profile<Float>::fltx_type>(samples, profile<Float>::make_fltx);
    }

    template<class Float, class Samples>
    [[nodiscard]] std::vector<unary_int_value<typename profile<Float>::competitor_ref>> make_competitor_values(const Samples& samples)
    {
        return make_values<typename profile<Float>::competitor_ref>(samples, profile<Float>::make_competitor);
    }

    template<class Float, class Samples>
    [[nodiscard]] std::vector<unary_int_value<typename profile<Float>::extra_competitor_ref>> make_extra_competitor_values(const Samples& samples)
    {
        return make_values<typename profile<Float>::extra_competitor_ref>(samples, profile<Float>::make_extra_competitor);
    }

    template<class Float, class Samples, class Values, class EvalFn, class RefFn, class ToReferenceFn>
    [[nodiscard]] accuracy_result measure_custom_accuracy(
        std::string_view operation,
        double required_bits,
        bool enforce_required_bits,
        const Samples& samples,
        const Values& values,
        ToReferenceFn to_reference,
        EvalFn eval,
        RefFn reference)
    {
        using prof = profile<Float>;
        double total_bits = 0.0;
        double worst_bits = std::numeric_limits<double>::infinity();
        std::vector<double> domain_scores;
        domain_scores.reserve(samples.size());

        for (std::size_t index = 0; index < samples.size(); ++index)
        {
            const unary_int_sample& sample = samples[index];
            const typename prof::perfect_ref actual = to_reference(eval(values[index].x, values[index].n));
            const typename prof::perfect_ref expected =
                prof::target_expected(reference(prof::make_perfect(sample.x), sample.n));
            double bits = reference_matching_bits(actual, expected);

            INFO(operation << " sample '" << sample.label << "' matched " << bits << " bits");
            if (std::isnan(bits))
            {
                if (enforce_required_bits)
                    CHECK(!std::isnan(bits));
                bits = 0.0;
            }
            if (enforce_required_bits)
                CHECK(bits >= required_bits);

            worst_bits = std::min(worst_bits, bits);
            total_bits += prof::finite_for_mean(bits);
            domain_scores.push_back(domain_sample_score(bits, prof::target_ideal_bits_for(expected)));
        }

        return {
            worst_bits,
            total_bits / static_cast<double>(samples.size()),
            samples.size(),
            domain_score(std::move(domain_scores))
        };
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    [[nodiscard]] accuracy_result measure_custom_accuracy(
        std::string_view operation,
        double required_bits,
        bool enforce_required_bits,
        const Samples& samples,
        EvalFn eval,
        RefFn reference)
    {
        auto to_reference = [](const auto& value) { return profile<Float>::to_perfect(value); };
        return measure_custom_accuracy<Float>(
            operation,
            required_bits,
            enforce_required_bits,
            samples,
            make_fltx_values<Float>(samples),
            to_reference,
            eval,
            reference);
    }

    template<class Float, class EvalFn>
    [[nodiscard]] special_support measure_round_to_decimals_special_values(EvalFn eval)
    {
        special_support_accumulator support;
        const std::vector<unary_int_sample> samples = make_special_unary_int_samples();
        for (const unary_int_sample& sample : samples)
        {
            const bool has_inf = sample_has_inf(sample);
            const bool has_nan = sample_has_nan(sample);
            const auto actual = eval(profile<Float>::make_fltx(sample.x), sample.n);
            bool passed = true;
            if (has_nan)
                passed = bl::isnan(actual);
            else if (has_inf)
                passed = bl::isinf(actual) && bl::signbit(actual) == detail::fp::signbit(sample.x.hi);
            support.record(has_inf, has_nan, passed);
        }
        return support.result();
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    void run_custom_precision_case(
        std::string_view operation,
        const Samples& samples,
        EvalFn eval,
        RefFn reference,
        special_support special_values = special_support::unavailable)
    {
        auto record = make_custom_record<Float>(operation);
        record.fltx_special_values = special_values;
        record.fltx_accuracy =
            measure_custom_accuracy<Float>(operation, profile<Float>::required_bits, true, samples, eval, reference);
        write_metrics_case_report(
            std::string(profile<Float>::precision_name) + " custom precision: " + std::string(operation),
            record);
    }

    template<class Float, class Samples, class EvalFn>
    void run_custom_benchmark_case(std::string_view operation, const Samples& samples, EvalFn eval)
    {
        auto record = make_custom_record<Float>(operation);
        const auto values = make_fltx_values<Float>(samples);
        record.fltx_benchmark = profile<Float>::benchmark_unary_int_values(values, eval, operation);
        write_metrics_case_report(
            std::string(profile<Float>::precision_name) + " custom benchmark: " + std::string(operation),
            record);
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    void run_custom_domain_case(std::string_view operation, const Samples& samples, EvalFn eval, RefFn reference)
    {
        auto record = make_custom_record<Float>(operation);
        record.fltx_accuracy =
            measure_custom_accuracy<Float>(operation, profile<Float>::required_bits, false, samples, eval, reference);
        write_metrics_case_report(
            std::string(profile<Float>::precision_name) + " custom domain: " + std::string(operation),
            record);
    }

    template<class T>
    [[nodiscard]] T pow10_via_pow(int n)
    {
        using std::pow;
        return pow(T{ 10 }, n);
    }

    template<class T>
    [[nodiscard]] T pow10_via_npwr(int n)
    {
        return ::npwr(T{ 10 }, n);
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    void run_pow10_precision_case(const Samples& samples, EvalFn eval, RefFn reference)
    {
        using prof = profile<Float>;
        auto record = make_custom_record<Float>("pow10<T>", true);
        auto to_reference = [](const auto& value) { return prof::to_perfect(value); };

        record.fltx_accuracy =
            measure_custom_accuracy<Float>("pow10<T>", prof::required_bits, true, samples, eval, reference);

        if constexpr (!config::benchmark_only_fltx)
        {
            auto competitor_eval = [](const auto&, int n) { return pow10_via_pow<typename prof::competitor_ref>(n); };
            auto extra_eval = [](const auto&, int n) { return pow10_via_npwr<typename prof::extra_competitor_ref>(n); };

            record.competitor_accuracy = measure_custom_accuracy<Float>(
                "pow10<T>",
                prof::required_bits,
                false,
                samples,
                make_competitor_values<Float>(samples),
                to_reference,
                competitor_eval,
                reference);

            if (!record.extra_competitors.empty())
            {
                record.extra_competitors.front().accuracy = measure_custom_accuracy<Float>(
                    "pow10<T>",
                    prof::required_bits,
                    false,
                    samples,
                    make_extra_competitor_values<Float>(samples),
                    to_reference,
                    extra_eval,
                    reference);
            }
        }

        write_metrics_case_report(
            std::string(prof::precision_name) + " custom precision: pow10<T>",
            record);
    }

    template<class Float, class Samples, class EvalFn>
    void run_pow10_benchmark_case(const Samples& samples, EvalFn eval)
    {
        using prof = profile<Float>;
        auto record = make_custom_record<Float>("pow10<T>", true);
        record.fltx_benchmark =
            prof::benchmark_unary_int_values(make_fltx_values<Float>(samples), eval, "pow10<T>");

        if constexpr (!config::benchmark_only_fltx)
        {
            auto competitor_eval = [](const auto&, int n) { return pow10_via_pow<typename prof::competitor_ref>(n); };
            auto extra_eval = [](const auto&, int n) { return pow10_via_npwr<typename prof::extra_competitor_ref>(n); };

            record.competitor_benchmark = prof::benchmark_unary_int_values(
                make_competitor_values<Float>(samples),
                competitor_eval,
                "pow10<T>");

            if (!record.extra_competitors.empty())
            {
                record.extra_competitors.front().benchmark = prof::benchmark_unary_int_values(
                    make_extra_competitor_values<Float>(samples),
                    extra_eval,
                    "pow10<T>");
            }
        }

        write_metrics_case_report(
            std::string(prof::precision_name) + " custom benchmark: pow10<T>",
            record);
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    void run_pow10_domain_case(const Samples& samples, EvalFn eval, RefFn reference)
    {
        using prof = profile<Float>;
        auto record = make_custom_record<Float>("pow10<T>", true);
        auto to_reference = [](const auto& value) { return prof::to_perfect(value); };

        record.fltx_accuracy =
            measure_custom_accuracy<Float>("pow10<T>", prof::required_bits, false, samples, eval, reference);

        if constexpr (!config::benchmark_only_fltx)
        {
            auto competitor_eval = [](const auto&, int n) { return pow10_via_pow<typename prof::competitor_ref>(n); };
            auto extra_eval = [](const auto&, int n) { return pow10_via_npwr<typename prof::extra_competitor_ref>(n); };

            record.competitor_accuracy = measure_custom_accuracy<Float>(
                "pow10<T>",
                prof::required_bits,
                false,
                samples,
                make_competitor_values<Float>(samples),
                to_reference,
                competitor_eval,
                reference);

            if (!record.extra_competitors.empty())
            {
                record.extra_competitors.front().accuracy = measure_custom_accuracy<Float>(
                    "pow10<T>",
                    prof::required_bits,
                    false,
                    samples,
                    make_extra_competitor_values<Float>(samples),
                    to_reference,
                    extra_eval,
                    reference);
            }
        }

        write_metrics_case_report(
            std::string(prof::precision_name) + " custom domain: pow10<T>",
            record);
    }

    template<class Float>
    void run_round_to_decimals_precision()
    {
        auto eval = [](const auto& x, int n) { return bl::round_to_decimals(x, n); };
        auto reference = [](const auto& x, int n) { return round_to_decimals_reference<Float>(x, n); };
        const auto samples = make_round_to_decimals_samples<Float>(profile<Float>::random_sample_count());
        run_custom_precision_case<Float>(
            "round_to_decimals",
            samples,
            eval,
            reference,
            measure_round_to_decimals_special_values<Float>(eval));
    }

    template<class Float>
    void run_round_to_decimals_benchmark()
    {
        auto eval = [](const auto& x, int n) { return bl::round_to_decimals(x, n); };
        const auto samples = make_round_to_decimals_samples<Float>(profile<Float>::random_sample_count());
        run_custom_benchmark_case<Float>("round_to_decimals", samples, eval);
    }

    template<class Float>
    void run_round_to_decimals_domain()
    {
        auto eval = [](const auto& x, int n) { return bl::round_to_decimals(x, n); };
        auto reference = [](const auto& x, int n) { return round_to_decimals_reference<Float>(x, n); };
        const auto samples = make_round_to_decimals_samples<Float>(configured_domain_random_sample_count(profile<Float>::random_sample_count()));
        run_custom_domain_case<Float>("round_to_decimals", samples, eval, reference);
    }

    template<class Float>
    void run_pow10_precision()
    {
        auto eval = [](const auto&, int n) { return bl::pow10<typename profile<Float>::fltx_type>(n); };
        auto reference = [](const auto&, int n) { return pow10_reference<Float>(n); };
        const auto samples = make_pow10_samples<Float>(profile<Float>::random_sample_count());
        run_pow10_precision_case<Float>(samples, eval, reference);
    }

    template<class Float>
    void run_pow10_benchmark()
    {
        auto eval = [](const auto&, int n) { return bl::pow10<typename profile<Float>::fltx_type>(n); };
        const auto samples = make_pow10_samples<Float>(profile<Float>::random_sample_count());
        run_pow10_benchmark_case<Float>(samples, eval);
    }

    template<class Float>
    void run_pow10_domain()
    {
        auto eval = [](const auto&, int n) { return bl::pow10<typename profile<Float>::fltx_type>(n); };
        auto reference = [](const auto&, int n) { return pow10_reference<Float>(n); };
        const auto samples = make_pow10_samples<Float>(configured_domain_random_sample_count(profile<Float>::random_sample_count()));
        run_pow10_domain_case<Float>(samples, eval, reference);
    }
}

TEST_CASE("f128 custom round_to_decimals precision", "[metrics][custom][precision][accuracy][f128]")
{
    bl::test::metrics::custom::run_round_to_decimals_precision<bl::f128>();
}

TEST_CASE("f128 custom round_to_decimals benchmark", "[metrics][custom][bench][f128]")
{
    bl::test::metrics::custom::run_round_to_decimals_benchmark<bl::f128>();
}

TEST_CASE("f128 custom round_to_decimals domain", "[metrics][custom][domain][f128]")
{
    bl::test::metrics::custom::run_round_to_decimals_domain<bl::f128>();
}

TEST_CASE("f128 custom pow10 precision", "[metrics][custom][precision][accuracy][f128]")
{
    bl::test::metrics::custom::run_pow10_precision<bl::f128>();
}

TEST_CASE("f128 custom pow10 benchmark", "[metrics][custom][bench][f128]")
{
    bl::test::metrics::custom::run_pow10_benchmark<bl::f128>();
}

TEST_CASE("f128 custom pow10 domain", "[metrics][custom][domain][f128]")
{
    bl::test::metrics::custom::run_pow10_domain<bl::f128>();
}

TEST_CASE("f256 custom round_to_decimals precision", "[metrics][custom][precision][accuracy][f256]")
{
    bl::test::metrics::custom::run_round_to_decimals_precision<bl::f256>();
}

TEST_CASE("f256 custom round_to_decimals benchmark", "[metrics][custom][bench][f256]")
{
    bl::test::metrics::custom::run_round_to_decimals_benchmark<bl::f256>();
}

TEST_CASE("f256 custom round_to_decimals domain", "[metrics][custom][domain][f256]")
{
    bl::test::metrics::custom::run_round_to_decimals_domain<bl::f256>();
}

TEST_CASE("f256 custom pow10 precision", "[metrics][custom][precision][accuracy][f256]")
{
    bl::test::metrics::custom::run_pow10_precision<bl::f256>();
}

TEST_CASE("f256 custom pow10 benchmark", "[metrics][custom][bench][f256]")
{
    bl::test::metrics::custom::run_pow10_benchmark<bl::f256>();
}

TEST_CASE("f256 custom pow10 domain", "[metrics][custom][domain][f256]")
{
    bl::test::metrics::custom::run_pow10_domain<bl::f256>();
}
