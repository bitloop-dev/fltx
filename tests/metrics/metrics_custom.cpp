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
        [[nodiscard]] static double cap_accuracy_bits(double bits) noexcept
        {
            return f128_primary::cap_accuracy_bits(bits);
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
        [[nodiscard]] static double cap_accuracy_bits(double bits) noexcept
        {
            return f256_primary::cap_accuracy_bits(bits);
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
    [[nodiscard]] typename profile<Float>::perfect_ref pow_integer_reference(
        const typename profile<Float>::perfect_ref& input_base,
        int exponent)
    {
        using perfect_ref = typename profile<Float>::perfect_ref;

        perfect_ref value = 1;
        perfect_ref base = input_base;
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
    [[nodiscard]] typename profile<Float>::perfect_ref pow10_reference(int exponent)
    {
        return pow_integer_reference<Float>(typename profile<Float>::perfect_ref{ 10 }, exponent);
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

    enum class special_pow_sample_set
    {
        exact_window,
        outside_window
    };

    struct special_pow_exponent_range
    {
        int min = 0;
        int max = 0;
    };

    struct special_pow_case
    {
        int base = 1;
        bool has_exact_window = false;
        bool emit_exact_window = true;
        bool enforce_exact_window_accuracy = true;
        bool exact_window_is_unbounded = false;
        bool has_outside_window = false;
        special_pow_exponent_range exact_window{};
        int sample_min = -512;
        int sample_max = 512;
        std::string exact_operation;
        std::string outside_operation;
    };

    inline constexpr int special_pow_nominal_sample_limit = 512;
    inline constexpr int ipow_integer_grid_max_abs_exponent = 300;
    inline constexpr std::string_view ipow_integer_grid_operation =
        "ipow<T>(bases[1,256], finite[-300,300])";

    template<class Float>
    struct special_pow_support_traits;

    template<>
    struct special_pow_support_traits<bl::f128>
    {
        static constexpr int pow10_min = detail::_f128::pow10_f128_min_exponent;
        static constexpr int pow10_max = detail::_f128::pow10_f128_max_exponent;
    };

    template<>
    struct special_pow_support_traits<bl::f256>
    {
        static constexpr int pow10_min = detail::_f256::pow10_f256_min_exponent;
        static constexpr int pow10_max = detail::_f256::pow10_f256_max_exponent;
    };

    [[nodiscard]] int finite_positive_pow_exponent_limit(int base) noexcept
    {
        if (base <= 1)
            return special_pow_nominal_sample_limit;

        const long double max_value = static_cast<long double>(std::numeric_limits<double>::max());
        const long double log_limit = std::log(max_value) / std::log(static_cast<long double>(base));
        if (!std::isfinite(log_limit) || log_limit < 0.0L)
            return 0;

        int limit = static_cast<int>(std::floor(log_limit));
        while (limit > 0 && std::pow(static_cast<long double>(base), limit) > max_value)
            --limit;
        while (limit < special_pow_nominal_sample_limit &&
               std::pow(static_cast<long double>(base), limit + 1) <= max_value)
        {
            ++limit;
        }

        return limit;
    }

    [[nodiscard]] int finite_negative_pow_exponent_limit(int base) noexcept
    {
        if (base <= 1)
            return special_pow_nominal_sample_limit;

        int limit = 0;
        while (limit < special_pow_nominal_sample_limit &&
               !detail::fp::integral_pow_reciprocal_underflows_binary64(
                   base,
                   static_cast<unsigned>(limit + 1)))
        {
            ++limit;
        }

        return limit;
    }

    [[nodiscard]] bool is_power_of_two(int value) noexcept
    {
        return value > 1 && (value & (value - 1)) == 0;
    }

    [[nodiscard]] int ceil_div_positive_denominator(int numerator, int denominator) noexcept
    {
        return numerator >= 0
            ? (numerator + denominator - 1) / denominator
            : numerator / denominator;
    }

    [[nodiscard]] int floor_div_positive_denominator(int numerator, int denominator) noexcept
    {
        return numerator >= 0
            ? numerator / denominator
            : -((-numerator + denominator - 1) / denominator);
    }

    template<class Float>
    [[nodiscard]] special_pow_case make_special_pow_case(int base)
    {
        special_pow_case pow_case{};
        pow_case.base = base;
        pow_case.sample_min = -special_pow_nominal_sample_limit;
        const int finite_positive_limit = finite_positive_pow_exponent_limit(base);
        pow_case.sample_max = std::max(
            0,
            std::min(special_pow_nominal_sample_limit, finite_positive_limit));

        if (base == 1)
        {
            pow_case.has_exact_window = true;
            pow_case.exact_window_is_unbounded = true;
            pow_case.exact_operation = "pow<T>(1, inside[all])";
            return pow_case;
        }

        if (is_power_of_two(base))
        {
            pow_case.has_exact_window = true;
            pow_case.exact_window_is_unbounded = true;
            pow_case.exact_operation = "pow<T>(" + std::to_string(base) + ", inside[ldexp])";
            return pow_case;
        }

        if (base == 10)
        {
            pow_case.has_exact_window = true;
            pow_case.has_outside_window = true;
            pow_case.exact_window = {
                special_pow_support_traits<Float>::pow10_min,
                special_pow_support_traits<Float>::pow10_max
            };
            pow_case.sample_min = std::min(pow_case.sample_min, pow_case.exact_window.min - 32);
            pow_case.sample_max = std::max(pow_case.sample_max, pow_case.exact_window.max + 32);
            pow_case.exact_operation =
                "pow<T>(" + std::to_string(base) + ", inside[" +
                std::to_string(pow_case.exact_window.min) + "," +
                std::to_string(pow_case.exact_window.max) + "])";
            pow_case.outside_operation =
                "pow<T>(" + std::to_string(base) + ", outside[" +
                std::to_string(pow_case.exact_window.min) + "," +
                std::to_string(pow_case.exact_window.max) + "])";
            return pow_case;
        }

        int pow2_count = 0;
        int pow5_count = 0;
        if (detail::fp::factor_power_of_two_five(static_cast<unsigned>(base), pow2_count, pow5_count))
        {
            const int finite_negative_limit = finite_negative_pow_exponent_limit(base);
            pow_case.sample_min = std::max(pow_case.sample_min, -finite_negative_limit);
            pow_case.sample_max = std::min(pow_case.sample_max, finite_positive_limit);
            pow_case.has_exact_window = true;
            pow_case.exact_window = {
                -finite_negative_limit,
                finite_positive_limit
            };
            pow_case.has_outside_window =
                pow_case.exact_window.min > pow_case.sample_min ||
                pow_case.exact_window.max < pow_case.sample_max;
            pow_case.exact_operation =
                "pow<T>(" + std::to_string(base) + ", inside[" +
                std::to_string(pow_case.exact_window.min) + "," +
                std::to_string(pow_case.exact_window.max) + "])";
            pow_case.outside_operation =
                "pow<T>(" + std::to_string(base) + ", outside[" +
                std::to_string(pow_case.exact_window.min) + "," +
                std::to_string(pow_case.exact_window.max) + "])";
            return pow_case;
        }

        pow_case.has_outside_window = true;
        pow_case.outside_operation = "pow<T>(" + std::to_string(base) + ", outside[fallback])";
        return pow_case;
    }

    template<class Float>
    [[nodiscard]] std::vector<special_pow_case> make_special_pow_cases()
    {
        std::vector<special_pow_case> cases;
        cases.reserve(257);
        for (int base = 1; base <= 256; ++base)
            cases.push_back(make_special_pow_case<Float>(base));

        cases.push_back(make_special_pow_case<Float>(65537));
        return cases;
    }

    template<class Float>
    [[nodiscard]] const std::vector<special_pow_case>& special_pow_cases()
    {
        static const std::vector<special_pow_case> cases = make_special_pow_cases<Float>();
        return cases;
    }

    [[nodiscard]] int finite_ipow_abs_exponent_limit(int base) noexcept
    {
        return std::min(ipow_integer_grid_max_abs_exponent, finite_positive_pow_exponent_limit(base));
    }

    template<class Float>
    [[nodiscard]] std::vector<unary_int_sample> make_ipow_integer_grid_samples()
    {
        using prof = profile<Float>;
        std::size_t sample_count = 0;
        for (int base = 1; base <= 256; ++base)
        {
            const int limit = finite_ipow_abs_exponent_limit(base);
            sample_count += static_cast<std::size_t>(limit * 2 + 1);
        }

        std::vector<unary_int_sample> samples;
        samples.reserve(sample_count);
        for (int base = 1; base <= 256; ++base)
        {
            const int limit = finite_ipow_abs_exponent_limit(base);
            const typename prof::fltx_type value{ base };
            for (int exponent = -limit; exponent <= limit; ++exponent)
                samples.push_back(make_value_sample<Float>("ipow finite grid", value, exponent));
        }

        return samples;
    }

    template<class Float>
    [[nodiscard]] std::vector<unary_int_sample> make_special_pow_samples(
        const special_pow_case& pow_case,
        special_pow_sample_set sample_set,
        std::size_t random_count)
    {
        using prof = profile<Float>;
        std::vector<unary_int_sample> samples;
        samples.reserve(14 + random_count);

        auto exponent_belongs_to_set = [&](int exponent) noexcept
        {
            if (sample_set == special_pow_sample_set::exact_window)
            {
                if (!pow_case.has_exact_window)
                    return false;
                if (pow_case.exact_window_is_unbounded)
                    return true;
                return exponent >= pow_case.exact_window.min && exponent <= pow_case.exact_window.max;
            }

            if (!pow_case.has_exact_window || pow_case.exact_window_is_unbounded)
                return true;

            return exponent < pow_case.exact_window.min || exponent > pow_case.exact_window.max;
        };

        auto add_sample = [&](std::string_view label, int exponent)
        {
            if (exponent < pow_case.sample_min || exponent > pow_case.sample_max)
                return;
            if (!exponent_belongs_to_set(exponent))
                return;

            const auto duplicate = std::find_if(
                samples.begin(),
                samples.end(),
                [exponent](const unary_int_sample& sample) { return sample.n == exponent; });
            if (duplicate == samples.end())
                samples.push_back(make_value_sample<Float>(label, typename prof::fltx_type{ pow_case.base }, exponent));
        };

        if (sample_set == special_pow_sample_set::exact_window)
        {
            const int low = pow_case.exact_window_is_unbounded ? pow_case.sample_min : pow_case.exact_window.min;
            const int high = pow_case.exact_window_is_unbounded ? pow_case.sample_max : pow_case.exact_window.max;
            add_sample("pow exact-window", low);
            for (int exponent : { -384, -323, -256, -128, -64, -32, -10, -1, 0, 1, 10, 32, 64, 128, 256, 308, 384 })
                add_sample("pow exact-window", exponent);
            add_sample("pow exact-window", high);
        }
        else if (pow_case.has_exact_window && !pow_case.exact_window_is_unbounded)
        {
            add_sample("pow outside-window", pow_case.sample_min);
            add_sample("pow outside-window", pow_case.exact_window.min - 1);
            add_sample("pow outside-window", pow_case.exact_window.min - 32);
            add_sample("pow outside-window", pow_case.exact_window.max + 1);
            add_sample("pow outside-window", pow_case.exact_window.max + 32);
            add_sample("pow outside-window", pow_case.sample_max);
        }
        else
        {
            add_sample("pow fallback", pow_case.sample_min);
            for (int exponent : { -384, -256, -128, -32, -1, 0, 1, 32, 128, 256, 384 })
                add_sample("pow fallback", exponent);
            add_sample("pow fallback", pow_case.sample_max);
        }

        typename prof::sample_rng rng{ 0x90a11c0570decadeull };
        for (std::size_t i = 0; i < random_count; ++i)
        {
            int exponent = 0;
            if (sample_set == special_pow_sample_set::outside_window &&
                pow_case.has_exact_window &&
                !pow_case.exact_window_is_unbounded)
            {
                const bool has_lower = pow_case.sample_min < pow_case.exact_window.min;
                const bool has_upper = pow_case.sample_max > pow_case.exact_window.max;
                if (has_lower && (!has_upper || rng.integer(0, 1) == 0))
                    exponent = rng.integer(pow_case.sample_min, pow_case.exact_window.min - 1);
                else
                    exponent = rng.integer(pow_case.exact_window.max + 1, pow_case.sample_max);
            }
            else if (sample_set == special_pow_sample_set::exact_window &&
                     pow_case.has_exact_window &&
                     !pow_case.exact_window_is_unbounded)
            {
                exponent = rng.integer(pow_case.exact_window.min, pow_case.exact_window.max);
            }
            else
            {
                exponent = rng.integer(pow_case.sample_min, pow_case.sample_max);
            }

            add_sample(
                sample_set == special_pow_sample_set::exact_window ? "random exact-window" : "random outside-window",
                exponent);
        }

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

            INFO(operation << " sample '" << sample.label << "' n=" << sample.n << " matched " << bits << " bits");
            if (std::isnan(bits))
            {
                if (enforce_required_bits)
                    CHECK(!std::isnan(bits));
                bits = 0.0;
            }
            if (enforce_required_bits)
                CHECK(bits >= required_bits);

            worst_bits = std::min(worst_bits, prof::cap_accuracy_bits(bits));
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
    [[nodiscard]] T pow_via_pow(int base, int n)
    {
        using std::pow;
        return pow(T{ base }, n);
    }

    template<class T>
    [[nodiscard]] T pow_via_npwr(int base, int n)
    {
        return ::npwr(T{ base }, n);
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    void run_special_pow_precision_case(
        std::string_view operation,
        int base,
        const Samples& samples,
        EvalFn eval,
        RefFn reference,
        bool enforce_required_bits)
    {
        using prof = profile<Float>;
        auto record = make_custom_record<Float>(operation, true);
        auto to_reference = [](const auto& value) { return prof::to_perfect(value); };

        record.fltx_accuracy =
            measure_custom_accuracy<Float>(operation, prof::required_bits, enforce_required_bits, samples, eval, reference);

        if constexpr (!config::benchmark_only_fltx)
        {
            auto competitor_eval = [base](const auto&, int n) { return pow_via_pow<typename prof::competitor_ref>(base, n); };
            auto extra_eval = [base](const auto&, int n) { return pow_via_npwr<typename prof::extra_competitor_ref>(base, n); };

            record.competitor_accuracy = measure_custom_accuracy<Float>(
                operation,
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
                    operation,
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
            std::string(prof::precision_name) + " custom precision: " + std::string(operation),
            record);
    }

    template<class Float, class Samples, class EvalFn>
    void run_special_pow_benchmark_case(std::string_view operation, int base, const Samples& samples, EvalFn eval)
    {
        using prof = profile<Float>;
        auto record = make_custom_record<Float>(operation, true);
        record.fltx_benchmark =
            prof::benchmark_unary_int_values(make_fltx_values<Float>(samples), eval, operation);

        if constexpr (!config::benchmark_only_fltx)
        {
            auto competitor_eval = [base](const auto&, int n) { return pow_via_pow<typename prof::competitor_ref>(base, n); };
            auto extra_eval = [base](const auto&, int n) { return pow_via_npwr<typename prof::extra_competitor_ref>(base, n); };

            record.competitor_benchmark = prof::benchmark_unary_int_values(
                make_competitor_values<Float>(samples),
                competitor_eval,
                operation);

            if (!record.extra_competitors.empty())
            {
                record.extra_competitors.front().benchmark = prof::benchmark_unary_int_values(
                    make_extra_competitor_values<Float>(samples),
                    extra_eval,
                    operation);
            }
        }

        write_metrics_case_report(
            std::string(prof::precision_name) + " custom benchmark: " + std::string(operation),
            record);
    }

    template<class Float, class Samples, class EvalFn, class RefFn>
    void run_special_pow_domain_case(std::string_view operation, int base, const Samples& samples, EvalFn eval, RefFn reference)
    {
        using prof = profile<Float>;
        auto record = make_custom_record<Float>(operation, true);
        auto to_reference = [](const auto& value) { return prof::to_perfect(value); };

        record.fltx_accuracy =
            measure_custom_accuracy<Float>(operation, prof::required_bits, false, samples, eval, reference);

        if constexpr (!config::benchmark_only_fltx)
        {
            auto competitor_eval = [base](const auto&, int n) { return pow_via_pow<typename prof::competitor_ref>(base, n); };
            auto extra_eval = [base](const auto&, int n) { return pow_via_npwr<typename prof::extra_competitor_ref>(base, n); };

            record.competitor_accuracy = measure_custom_accuracy<Float>(
                operation,
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
                    operation,
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
            std::string(prof::precision_name) + " custom domain: " + std::string(operation),
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
    void run_ipow_integer_grid_precision()
    {
        auto eval = [](const auto& x, int n) { return bl::ipow(x, n); };
        auto reference = [](const auto& x, int n) { return pow_integer_reference<Float>(x, n); };
        const auto samples = make_ipow_integer_grid_samples<Float>();
        run_custom_precision_case<Float>(ipow_integer_grid_operation, samples, eval, reference);
    }

    template<class Float>
    void run_ipow_integer_grid_benchmark()
    {
        auto eval = [](const auto& x, int n) { return bl::ipow(x, n); };
        const auto samples = make_ipow_integer_grid_samples<Float>();
        run_custom_benchmark_case<Float>(ipow_integer_grid_operation, samples, eval);
    }

    template<class Float>
    void run_ipow_integer_grid_domain()
    {
        auto eval = [](const auto& x, int n) { return bl::ipow(x, n); };
        auto reference = [](const auto& x, int n) { return pow_integer_reference<Float>(x, n); };
        const auto samples = make_ipow_integer_grid_samples<Float>();
        run_custom_domain_case<Float>(ipow_integer_grid_operation, samples, eval, reference);
    }

    template<class Float>
    void run_special_pow_precision()
    {
        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (!pow_case.has_exact_window || !pow_case.emit_exact_window)
                continue;

            const int base = pow_case.base;
            auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };
            auto reference = [base](const auto&, int n) {
                return pow_integer_reference<Float>(typename profile<Float>::perfect_ref{ base }, n);
            };

            const auto samples = make_special_pow_samples<Float>(
                pow_case,
                special_pow_sample_set::exact_window,
                profile<Float>::random_sample_count());
            run_special_pow_precision_case<Float>(
                pow_case.exact_operation,
                base,
                samples,
                eval,
                reference,
                pow_case.enforce_exact_window_accuracy);
        }

        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (!pow_case.has_outside_window)
                continue;

            const int base = pow_case.base;
            auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };
            auto reference = [base](const auto&, int n) {
                return pow_integer_reference<Float>(typename profile<Float>::perfect_ref{ base }, n);
            };

            const auto samples = make_special_pow_samples<Float>(
                pow_case,
                special_pow_sample_set::outside_window,
                profile<Float>::random_sample_count());
            run_special_pow_precision_case<Float>(pow_case.outside_operation, base, samples, eval, reference, false);
        }
    }

    template<class Float>
    void run_special_pow_benchmark()
    {
        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (!pow_case.has_exact_window || !pow_case.emit_exact_window)
                continue;

            const int base = pow_case.base;
            auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };

            const auto samples = make_special_pow_samples<Float>(
                pow_case,
                special_pow_sample_set::exact_window,
                profile<Float>::random_sample_count());
            run_special_pow_benchmark_case<Float>(pow_case.exact_operation, base, samples, eval);
        }

        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (!pow_case.has_outside_window)
                continue;

            const int base = pow_case.base;
            auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };

            const auto samples = make_special_pow_samples<Float>(
                pow_case,
                special_pow_sample_set::outside_window,
                profile<Float>::random_sample_count());
            run_special_pow_benchmark_case<Float>(pow_case.outside_operation, base, samples, eval);
        }
    }

    template<class Float>
    void run_special_pow_domain()
    {
        const std::size_t random_count = configured_domain_random_sample_count(profile<Float>::random_sample_count());

        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (!pow_case.has_exact_window || !pow_case.emit_exact_window)
                continue;

            const int base = pow_case.base;
            auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };
            auto reference = [base](const auto&, int n) {
                return pow_integer_reference<Float>(typename profile<Float>::perfect_ref{ base }, n);
            };

            const auto samples = make_special_pow_samples<Float>(
                pow_case,
                special_pow_sample_set::exact_window,
                random_count);
            run_special_pow_domain_case<Float>(pow_case.exact_operation, base, samples, eval, reference);
        }

        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (!pow_case.has_outside_window)
                continue;

            const int base = pow_case.base;
            auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };
            auto reference = [base](const auto&, int n) {
                return pow_integer_reference<Float>(typename profile<Float>::perfect_ref{ base }, n);
            };

            const auto samples = make_special_pow_samples<Float>(
                pow_case,
                special_pow_sample_set::outside_window,
                random_count);
            run_special_pow_domain_case<Float>(pow_case.outside_operation, base, samples, eval, reference);
        }
    }

    [[nodiscard]] inline bool special_pow_complete_mode() noexcept
    {
        if (!metrics_filter_has_explicit_phase())
            return metrics_filter_arguments().empty();

        return metrics_filter_phase_count() == 3;
    }

    [[nodiscard]] inline bool special_pow_filter_mentions_phase(std::string_view phase)
    {
        for (const std::string& filter : metrics_filter_arguments())
        {
            const std::string normalized = normalized_metrics_filter(filter);
            if (normalized.find("customspecialpow") != std::string::npos &&
                normalized.find(phase) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline bool special_pow_complete_mode_for_phase(std::string_view phase)
    {
        return special_pow_complete_mode() && !special_pow_filter_mentions_phase(phase);
    }

    template<class Float>
    void run_special_pow_complete_case(
        const special_pow_case& pow_case,
        special_pow_sample_set sample_set)
    {
        const int base = pow_case.base;
        const std::string& operation =
            sample_set == special_pow_sample_set::exact_window
                ? pow_case.exact_operation
                : pow_case.outside_operation;
        auto eval = [base](const auto&, int n) { return bl::pow(typename profile<Float>::fltx_type{ base }, n); };
        auto reference = [base](const auto&, int n) {
            return pow_integer_reference<Float>(typename profile<Float>::perfect_ref{ base }, n);
        };

        const auto precision_benchmark_samples = make_special_pow_samples<Float>(
            pow_case,
            sample_set,
            profile<Float>::random_sample_count());
        run_special_pow_precision_case<Float>(
            operation,
            base,
            precision_benchmark_samples,
            eval,
            reference,
            sample_set == special_pow_sample_set::exact_window && pow_case.enforce_exact_window_accuracy);
        run_special_pow_benchmark_case<Float>(operation, base, precision_benchmark_samples, eval);

        const auto domain_samples = make_special_pow_samples<Float>(
            pow_case,
            sample_set,
            configured_domain_random_sample_count(profile<Float>::random_sample_count()));
        run_special_pow_domain_case<Float>(operation, base, domain_samples, eval, reference);
    }

    template<class Float>
    void run_special_pow_complete()
    {
        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (pow_case.has_exact_window && pow_case.emit_exact_window)
                run_special_pow_complete_case<Float>(pow_case, special_pow_sample_set::exact_window);
        }

        for (const special_pow_case& pow_case : special_pow_cases<Float>())
        {
            if (pow_case.has_outside_window)
                run_special_pow_complete_case<Float>(pow_case, special_pow_sample_set::outside_window);
        }
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

TEST_CASE("f128 custom ipow integer grid precision", "[metrics][custom][precision][accuracy][ipow][f128]")
{
    if (!bl::test::metrics::metrics_case_phase_enabled("precision"))
    {
        SUCCEED("metrics precision phase not selected");
        return;
    }
    bl::test::metrics::custom::run_ipow_integer_grid_precision<bl::f128>();
}

TEST_CASE("f128 custom ipow integer grid benchmark", "[metrics][custom][bench][ipow][f128]")
{
    if (!bl::test::metrics::metrics_case_phase_enabled("bench"))
    {
        SUCCEED("metrics benchmark phase not selected");
        return;
    }
    bl::test::metrics::custom::run_ipow_integer_grid_benchmark<bl::f128>();
}

TEST_CASE("f128 custom ipow integer grid domain", "[metrics][custom][domain][ipow][f128]")
{
    if (!bl::test::metrics::metrics_case_phase_enabled("domain"))
    {
        SUCCEED("metrics domain phase not selected");
        return;
    }
    bl::test::metrics::custom::run_ipow_integer_grid_domain<bl::f128>();
}

TEST_CASE("f128 custom special pow precision", "[metrics][custom][precision][accuracy][f128]")
{
    if (bl::test::metrics::custom::special_pow_complete_mode_for_phase("precision"))
    {
        bl::test::metrics::custom::run_special_pow_complete<bl::f128>();
        return;
    }
    if (!bl::test::metrics::metrics_case_phase_enabled("precision"))
    {
        SUCCEED("metrics precision phase not selected");
        return;
    }
    bl::test::metrics::custom::run_special_pow_precision<bl::f128>();
}

TEST_CASE("f128 custom special pow benchmark", "[metrics][custom][bench][f128]")
{
    if (bl::test::metrics::custom::special_pow_complete_mode_for_phase("benchmark"))
    {
        SUCCEED("custom pow complete mode handled by precision test");
        return;
    }
    if (!bl::test::metrics::metrics_case_phase_enabled("bench"))
    {
        SUCCEED("metrics benchmark phase not selected");
        return;
    }
    bl::test::metrics::custom::run_special_pow_benchmark<bl::f128>();
}

TEST_CASE("f128 custom special pow domain", "[metrics][custom][domain][f128]")
{
    if (bl::test::metrics::custom::special_pow_complete_mode_for_phase("domain"))
    {
        SUCCEED("custom pow complete mode handled by precision test");
        return;
    }
    if (!bl::test::metrics::metrics_case_phase_enabled("domain"))
    {
        SUCCEED("metrics domain phase not selected");
        return;
    }
    bl::test::metrics::custom::run_special_pow_domain<bl::f128>();
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

TEST_CASE("f256 custom ipow integer grid precision", "[metrics][custom][precision][accuracy][ipow][f256]")
{
    if (!bl::test::metrics::metrics_case_phase_enabled("precision"))
    {
        SUCCEED("metrics precision phase not selected");
        return;
    }
    bl::test::metrics::custom::run_ipow_integer_grid_precision<bl::f256>();
}

TEST_CASE("f256 custom ipow integer grid benchmark", "[metrics][custom][bench][ipow][f256]")
{
    if (!bl::test::metrics::metrics_case_phase_enabled("bench"))
    {
        SUCCEED("metrics benchmark phase not selected");
        return;
    }
    bl::test::metrics::custom::run_ipow_integer_grid_benchmark<bl::f256>();
}

TEST_CASE("f256 custom ipow integer grid domain", "[metrics][custom][domain][ipow][f256]")
{
    if (!bl::test::metrics::metrics_case_phase_enabled("domain"))
    {
        SUCCEED("metrics domain phase not selected");
        return;
    }
    bl::test::metrics::custom::run_ipow_integer_grid_domain<bl::f256>();
}

TEST_CASE("f256 custom special pow precision", "[metrics][custom][precision][accuracy][f256]")
{
    if (bl::test::metrics::custom::special_pow_complete_mode_for_phase("precision"))
    {
        bl::test::metrics::custom::run_special_pow_complete<bl::f256>();
        return;
    }
    if (!bl::test::metrics::metrics_case_phase_enabled("precision"))
    {
        SUCCEED("metrics precision phase not selected");
        return;
    }
    bl::test::metrics::custom::run_special_pow_precision<bl::f256>();
}

TEST_CASE("f256 custom special pow benchmark", "[metrics][custom][bench][f256]")
{
    if (bl::test::metrics::custom::special_pow_complete_mode_for_phase("benchmark"))
    {
        SUCCEED("custom pow complete mode handled by precision test");
        return;
    }
    if (!bl::test::metrics::metrics_case_phase_enabled("bench"))
    {
        SUCCEED("metrics benchmark phase not selected");
        return;
    }
    bl::test::metrics::custom::run_special_pow_benchmark<bl::f256>();
}

TEST_CASE("f256 custom special pow domain", "[metrics][custom][domain][f256]")
{
    if (bl::test::metrics::custom::special_pow_complete_mode_for_phase("domain"))
    {
        SUCCEED("custom pow complete mode handled by precision test");
        return;
    }
    if (!bl::test::metrics::metrics_case_phase_enabled("domain"))
    {
        SUCCEED("metrics domain phase not selected");
        return;
    }
    bl::test::metrics::custom::run_special_pow_domain<bl::f256>();
}
