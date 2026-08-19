#ifndef FLTX_TESTS_ACCURACY_RUNNER_INCLUDED
#define FLTX_TESTS_ACCURACY_RUNNER_INCLUDED

#include "../support/csv.hpp"
#include "../support/domains.hpp"
#include "../support/implementations.hpp"
#include "../support/mpfr.hpp"
#include "../support/source_identity.hpp"
#include "../support/thresholds.hpp"
#include "special_values.hpp"
#include "signed_zero.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <vector>

namespace fltx::tests::accuracy
{
    inline constexpr std::string_view schema_version = "6";
#if !defined(FLTX_TESTS_ENABLE_EXTERNAL_COMPARISONS) || \
    !FLTX_TESTS_ENABLE_EXTERNAL_COMPARISONS || \
    (defined(FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH) && \
     FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH)
    inline constexpr bool external_implementations_enabled = false;
#else
    inline constexpr bool external_implementations_enabled = true;
#endif
#if defined(FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH) && \
    FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH && \
    FLTX_ACCURACY_EXPECTS_SIMULATED_CONSTEVAL
    inline constexpr bool special_value_probes_enabled = false;
#else
    inline constexpr bool special_value_probes_enabled = true;
#endif

    struct options
    {
        std::string precision;
        std::string output;
        std::string run_id;
        std::string source_revision;
        std::string filter;
        std::string sample_mode = "custom";
        std::size_t samples = 64;
        bool advisory = false;
    };

    struct result
    {
        std::size_t samples = 0;
        double mean_bits = 0.0;
        double p01_bits = 0.0;
        double worst_bits = 0.0;
        std::string worst_input;
        std::string observed;
        std::string reference;
    };

    struct implementation_info
    {
        std::string id;
        std::string short_label;
        std::string label;
        std::string api;
        bool enabled = true;
    };

    [[nodiscard]] inline implementation_info describe(
        implementations::identity identity,
        std::string_view api)
    {
        return {
            std::string(identity.id),
            std::string(identity.short_label),
            std::string(identity.label),
            std::string(api.empty() ? identity.api : api),
            identity.enabled
        };
    }

    template<class Value, class Eval>
    struct implementation_spec
    {
        using value_type = Value;

        implementation_info info;
        Eval evaluate;
    };

    template<class Value, class Eval>
    [[nodiscard]] auto comparison(
        implementations::identity identity,
        std::string_view api,
        Eval evaluate)
    {
        implementation_info info = describe(identity, api);
        info.enabled = info.enabled && external_implementations_enabled;
        return implementation_spec<Value, Eval>{std::move(info), std::move(evaluate)};
    }

    [[nodiscard]] inline std::string number(double value)
    {
        if (mpfr::is_exact_score(value))
            return "inf";
        if (native_fp::is_inf(value))
            return value > 0 ? "inf" : "-inf";
        if (native_fp::is_nan(value))
            return "nan";
        std::ostringstream out;
        out << std::fixed << std::setprecision(6) << value;
        return out.str();
    }

    [[nodiscard]] inline std::string display_score(double value)
    {
        if (mpfr::is_exact_score(value))
            return "exact";
        if (native_fp::is_inf(value))
            return value > 0 ? "inf" : "-inf";
        if (native_fp::is_nan(value))
            return "nan";
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << value;
        return out.str();
    }

    template<class Float>
    class runner
    {
    public:
        runner(csv_writer& output, const options& settings)
            : output_(output), settings_(settings)
        {
        }

        template<class Eval, class Reference, class... Specs>
        void unary(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            unary_impl(
                group,
                operation,
                std::move(test_domains),
                std::move(evaluate),
                std::move(reference),
                false,
                std::move(comparisons)...);
        }

        template<class Eval, class Reference, class... Specs>
        void unary_exact(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            unary_impl(
                group,
                operation,
                std::move(test_domains),
                std::move(evaluate),
                std::move(reference),
                true,
                std::move(comparisons)...);
        }

        template<class Eval, class Reference, class... Specs>
        void binary(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            binary_impl(
                group,
                operation,
                std::move(test_domains),
                std::move(evaluate),
                std::move(reference),
                false,
                std::move(comparisons)...);
        }

        template<class Eval, class Reference, class... Specs>
        void binary_exact(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            binary_impl(
                group,
                operation,
                std::move(test_domains),
                std::move(evaluate),
                std::move(reference),
                true,
                std::move(comparisons)...);
        }

        template<class Eval, class Reference, class... Specs>
        void binary_pairs(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::binary_domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::binary_domain& domain : test_domains)
            {
                measure_binary<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    false,
                    true);
                (measure_binary_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference,
                    false), ...);
            }
        }

        template<class Eval, class Reference, class... Specs>
        void predicate(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::domain& domain : test_domains)
            {
                measure_predicate<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    true);
                (measure_predicate_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference), ...);
            }
        }

        template<class Eval, class Reference, class... Specs>
        void unary_pair(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            unary_pair_impl(
                group,
                operation,
                std::move(test_domains),
                std::move(evaluate),
                std::move(reference),
                false,
                std::move(comparisons)...);
        }

        template<class Eval, class Reference, class... Specs>
        void unary_pair_exact(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            unary_pair_impl(
                group,
                operation,
                std::move(test_domains),
                std::move(evaluate),
                std::move(reference),
                true,
                std::move(comparisons)...);
        }

        template<class Eval, class Reference, class... Specs>
        void binary_pair(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::domain& domain : test_domains)
            {
                measure_binary_pair<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    true);
                (measure_binary_pair_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference), ...);
            }
        }

        template<class Eval, class Reference, class... Specs>
        void ternary(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::ternary_domain> test_domains,
            Eval evaluate,
            Reference reference,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::ternary_domain& domain : test_domains)
            {
                measure_ternary<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    true);
                (measure_ternary_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference), ...);
            }
        }

        template<class Eval, class... Specs>
        void parse(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::text_domain> test_domains,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::text_domain& domain : test_domains)
            {
                std::vector<mpfr::real> expected;
                expected.reserve(domain.values.size());
                for (const std::string& text : domain.values)
                    expected.emplace_back(text);

                measure_parse<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    expected,
                    true);
                (measure_parse_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    expected), ...);
            }
        }

        template<class Eval, class... Specs>
        void format(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::domain& domain : test_domains)
            {
                measure_format<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    true);
                (measure_format_spec(
                    group,
                    operation,
                    domain,
                    comparisons), ...);
            }
        }

        [[nodiscard]] int failures() const noexcept
        {
            return failures_;
        }

        void require_complete(std::size_t expected_rows) const
        {
            if (fltx_rows_ == 0)
                throw std::runtime_error("accuracy filter matched no operations");
            if (settings_.filter.empty() && fltx_rows_ != expected_rows)
            {
                throw std::runtime_error(
                    "accuracy inventory mismatch: expected " +
                    std::to_string(expected_rows) + " fltx rows, wrote " +
                    std::to_string(fltx_rows_));
            }
        }

    private:
        template<class Eval, class Reference, class... Specs>
        void unary_impl(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            bool require_exact,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::domain& domain : test_domains)
            {
                measure_unary<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    require_exact,
                    true);
                (measure_unary_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference,
                    require_exact), ...);
            }
        }

        template<class Eval, class Reference, class... Specs>
        void binary_impl(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            bool require_exact,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::domain& domain : test_domains)
            {
                measure_binary<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    require_exact,
                    true);
                (measure_binary_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference,
                    require_exact), ...);
            }
        }

        template<class Eval, class Reference, class... Specs>
        void unary_pair_impl(
            std::string_view group,
            std::string_view operation,
            std::vector<domains::domain> test_domains,
            Eval evaluate,
            Reference reference,
            bool require_exact,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            for (const domains::domain& domain : test_domains)
            {
                measure_unary_pair<Float>(
                    primary_info(operation),
                    group,
                    operation,
                    domain,
                    evaluate,
                    reference,
                    require_exact,
                    true);
                (measure_unary_pair_spec(
                    group,
                    operation,
                    domain,
                    comparisons,
                    reference,
                    require_exact), ...);
            }
        }

        [[nodiscard]] implementation_info primary_info(
            std::string_view operation) const
        {
            return describe(
                implementations::primary_identity<Float>,
                "bl::" + std::string(operation));
        }

        template<class Spec, class Reference>
        void measure_unary_spec(
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            const Spec& spec,
            Reference reference,
            bool require_exact)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_unary<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        reference,
                        require_exact,
                        false);
                }
            }
        }

        template<class Domain, class Spec, class Reference>
        void measure_binary_spec(
            std::string_view group,
            std::string_view operation,
            const Domain& domain,
            const Spec& spec,
            Reference reference,
            bool require_exact)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_binary<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        reference,
                        require_exact,
                        false);
                }
            }
        }

        template<class Spec, class Reference>
        void measure_predicate_spec(
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            const Spec& spec,
            Reference reference)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_predicate<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        reference,
                        false);
                }
            }
        }

        template<class Spec, class Reference>
        void measure_unary_pair_spec(
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            const Spec& spec,
            Reference reference,
            bool require_exact)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_unary_pair<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        reference,
                        require_exact,
                        false);
                }
            }
        }

        template<class Spec, class Reference>
        void measure_binary_pair_spec(
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            const Spec& spec,
            Reference reference)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_binary_pair<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        reference,
                        false);
                }
            }
        }

        template<class Spec, class Reference>
        void measure_ternary_spec(
            std::string_view group,
            std::string_view operation,
            const domains::ternary_domain& domain,
            const Spec& spec,
            Reference reference)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_ternary<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        reference,
                        false);
                }
            }
        }

        template<class Spec>
        void measure_parse_spec(
            std::string_view group,
            std::string_view operation,
            const domains::text_domain& domain,
            const Spec& spec,
            const std::vector<mpfr::real>& expected)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_parse<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        expected,
                        false);
                }
            }
        }

        template<class Spec>
        void measure_format_spec(
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            const Spec& spec)
        {
            if constexpr (external_implementations_enabled)
            {
                if (spec.info.enabled)
                {
                    measure_format<typename Spec::value_type>(
                        spec.info,
                        group,
                        operation,
                        domain,
                        spec.evaluate,
                        false);
                }
            }
        }

        template<class Value>
        [[nodiscard]] static Value project_to_value(const mpfr::real& value)
        {
            return implementations::value_traits<Value>::from_sample(
                mpfr::sample_from_real<Float>(value));
        }

        template<class Value>
        [[nodiscard]] static mpfr::real native_step(const mpfr::real& value)
        {
            using boost::multiprecision::ldexp;
            const int exponent = value == 0 ? 1 : mpfr::ilogb(value) + 1;
            return ldexp(
                mpfr::real{ 1 },
                exponent - static_cast<int>(
                    implementations::value_traits<Value>::nominal_bits));
        }

        template<class Value, class Eval, class Reference>
        void measure_unary(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            Eval evaluate,
            Reference reference,
            bool require_exact,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::unary<Value>(operation, evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::unary<Value>(operation, evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size());
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const Value input =
                    implementations::value_traits<Value>::from_sample(
                        domain.values[i]);
                require_finite(input, operation, domain.name);
                const Value value(evaluate(input));
                const mpfr::real expected = reference(
                    implementations::value_traits<Value>::to_real(input));
                add_sample(
                    value,
                    expected,
                    require_exact,
                    describe(domain.values[i]),
                    measured,
                    bits);
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value, class Domain, class Eval, class Reference>
        void measure_binary(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const Domain& domain,
            Eval evaluate,
            Reference reference,
            bool require_exact,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::binary<Value>(operation, evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::binary<Value>(operation, evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domains::binary_size(domain));
            for (std::size_t i = 0; i < domains::binary_size(domain); ++i)
            {
                const sample& lhs_sample = domains::binary_lhs(domain, i);
                const sample& rhs_sample = domains::binary_rhs(domain, i);
                const Value lhs =
                    implementations::value_traits<Value>::from_sample(lhs_sample);
                Value rhs =
                    implementations::value_traits<Value>::from_sample(rhs_sample);
                require_finite(lhs, operation, domain.name);
                require_finite(rhs, operation, domain.name);
                const mpfr::real lhs_real =
                    implementations::value_traits<Value>::to_real(lhs);
                mpfr::real rhs_real =
                    implementations::value_traits<Value>::to_real(rhs);
                mpfr::real expected = reference(lhs_real, rhs_real);
                std::string input_description =
                    describe(lhs_sample) + " ; " + describe(rhs_sample);

                // A cancellation input can become two identical operands when
                // projected to a narrower implementation. In that case, retain
                // the intended ordering but move the right operand to a native
                // representable neighbour. The MPFR oracle still uses the exact
                // converted values that the implementation receives.
                if (domains::preserve_nonzero(domain, i) &&
                    (operation == "add" || operation == "subtract" ||
                     operation == "fdim"))
                {
                    const mpfr::real raw_lhs =
                        mpfr::input_real<Float>(lhs_sample);
                    const mpfr::real raw_rhs =
                        mpfr::input_real<Float>(rhs_sample);
                    const mpfr::real raw_expected = reference(raw_lhs, raw_rhs);
                    if (raw_expected != 0 && expected == 0 && raw_lhs != raw_rhs)
                    {
                        bool adapted = false;
                        mpfr::real step = native_step<Value>(rhs_real);
                        const int direction = operation == "add"
                            ? (raw_expected > 0 ? 1 : -1)
                            : (raw_expected > 0 ? -1 : 1);
                        const mpfr::real candidate_base = operation == "add"
                            ? -lhs_real
                            : lhs_real;
                        for (int attempt = 0; attempt < 16; ++attempt)
                        {
                            Value candidate = project_to_value<Value>(
                                candidate_base + direction * step);
                            const mpfr::real candidate_real =
                                implementations::value_traits<Value>::to_real(
                                    candidate);
                            const mpfr::real candidate_expected =
                                reference(lhs_real, candidate_real);
                            if (candidate_real != lhs_real &&
                                candidate_expected != 0 &&
                                ((candidate_expected > 0) ==
                                 (raw_expected > 0)))
                            {
                                rhs = std::move(candidate);
                                rhs_real = candidate_real;
                                expected = candidate_expected;
                                input_description +=
                                    " [right operand adapted to native precision]";
                                adapted = true;
                                break;
                            }
                            step *= 2;
                        }
                        if (!adapted)
                        {
                            throw std::runtime_error(
                                "unable to preserve nonzero additive cancellation "
                                "after native input conversion");
                        }
                    }
                }

                const Value value(evaluate(lhs, rhs));
                add_sample(
                    value,
                    expected,
                    require_exact,
                    std::move(input_description),
                    measured,
                    bits);
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value, class Eval, class Reference>
        void measure_predicate(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            Eval evaluate,
            Reference reference,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] {
                    return special_values::predicate<Value>(
                        operation, evaluate);
                });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size() * 2);
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const sample& lhs_sample = domain.values[i];
                const sample& rhs_sample =
                    domain.values[(i * 7 + 3) % domain.values.size()];
                const Value lhs =
                    implementations::value_traits<Value>::from_sample(lhs_sample);
                const Value rhs =
                    implementations::value_traits<Value>::from_sample(rhs_sample);
                require_finite(lhs, operation, domain.name);
                require_finite(rhs, operation, domain.name);
                const mpfr::real lhs_real =
                    implementations::value_traits<Value>::to_real(lhs);

                const auto add_case = [&](const Value& right,
                                          std::string input) {
                    const bool observed =
                        static_cast<bool>(evaluate(lhs, right));
                    const bool expected = static_cast<bool>(reference(
                        lhs_real,
                        implementations::value_traits<Value>::to_real(right)));
                    const double sample_bits = observed == expected
                        ? mpfr::exact_score()
                        : 0.0;
                    bits.push_back(sample_bits);
                    if (bits.size() == 1 ||
                        sample_bits < measured.worst_bits)
                    {
                        measured.worst_bits = sample_bits;
                        measured.worst_input = std::move(input);
                        measured.observed = observed ? "true" : "false";
                        measured.reference = expected ? "true" : "false";
                    }
                };

                add_case(
                    rhs,
                    describe(lhs_sample) + " ; " + describe(rhs_sample));
                add_case(
                    lhs,
                    describe(lhs_sample) + " ; same value");
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, "-", gated);
        }

        template<class Value, class Eval, class Reference>
        void measure_unary_pair(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            Eval evaluate,
            Reference reference,
            bool require_exact,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::unary_pair<Value>(operation, evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::unary_pair<Value>(operation, evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size());
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const Value input =
                    implementations::value_traits<Value>::from_sample(
                        domain.values[i]);
                require_finite(input, operation, domain.name);
                const auto [first_raw, second_raw] = evaluate(input);
                const Value first(first_raw);
                const Value second(second_raw);
                const auto expected = reference(
                    implementations::value_traits<Value>::to_real(input));
                const double first_bits =
                    score(first, expected.first, require_exact);
                const double second_bits =
                    score(second, expected.second, require_exact);
                add_pair_sample(
                    first,
                    second,
                    expected.first,
                    expected.second,
                    std::min(first_bits, second_bits),
                    describe(domain.values[i]),
                    measured,
                    bits);
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value, class Eval, class Reference>
        void measure_binary_pair(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            Eval evaluate,
            Reference reference,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::binary_pair<Value>(operation, evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::binary_pair<Value>(operation, evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size());
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const sample& lhs_sample = domain.values[i];
                const sample& rhs_sample =
                    domain.values[(i * 7 + 3) % domain.values.size()];
                const Value lhs =
                    implementations::value_traits<Value>::from_sample(lhs_sample);
                const Value rhs =
                    implementations::value_traits<Value>::from_sample(rhs_sample);
                require_finite(lhs, operation, domain.name);
                require_finite(rhs, operation, domain.name);
                const auto [first_raw, second_raw] = evaluate(lhs, rhs);
                const Value first(first_raw);
                const Value second(second_raw);
                const auto expected = reference(
                    implementations::value_traits<Value>::to_real(lhs),
                    implementations::value_traits<Value>::to_real(rhs));
                const double sample_bits = std::min(
                    score(first, expected.first, false),
                    score(second, expected.second, false));
                add_pair_sample(
                    first,
                    second,
                    expected.first,
                    expected.second,
                    sample_bits,
                    describe(lhs_sample) + " ; " + describe(rhs_sample),
                    measured,
                    bits);
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value, class Eval, class Reference>
        void measure_ternary(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::ternary_domain& domain,
            Eval evaluate,
            Reference reference,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::ternary<Value>(operation, evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::ternary<Value>(operation, evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size());
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const domains::ternary_sample& input = domain.values[i];
                const sample& x_sample = input.x;
                const sample& y_sample = input.y;
                Value x =
                    implementations::value_traits<Value>::from_sample(x_sample);
                Value y =
                    implementations::value_traits<Value>::from_sample(y_sample);
                sample z_sample = input.z;
                Value z =
                    implementations::value_traits<Value>::from_sample(z_sample);
                require_finite(x, operation, domain.name);
                require_finite(y, operation, domain.name);
                require_finite(z, operation, domain.name);
                mpfr::real x_real =
                    implementations::value_traits<Value>::to_real(x);
                mpfr::real y_real =
                    implementations::value_traits<Value>::to_real(y);
                mpfr::real z_real =
                    implementations::value_traits<Value>::to_real(z);
                mpfr::real expected;
                std::string input_description =
                    describe(x_sample) + " ; " + describe(y_sample) + " ; " +
                    describe(z_sample);

                if (operation == "fma" && domain.name == "cancellation")
                {
                    const mpfr::real raw_x =
                        mpfr::input_real<Float>(x_sample);
                    const mpfr::real raw_y =
                        mpfr::input_real<Float>(y_sample);
                    const sample raw_z_sample = mpfr::sample_from_real<Float>(
                        -(raw_x * raw_y), "product cancellation");
                    const mpfr::real raw_expected = reference(
                        raw_x,
                        raw_y,
                        mpfr::input_real<Float>(raw_z_sample));
                    const auto set_cancelling_z = [&] {
                        const mpfr::real target = -(x_real * y_real);
                        z_sample = mpfr::sample_from_real<Float>(
                            target, "native product cancellation");
                        z = implementations::value_traits<Value>::from_sample(
                            z_sample);
                        z_real =
                            implementations::value_traits<Value>::to_real(z);
                        expected = reference(x_real, y_real, z_real);
                    };

                    set_cancelling_z();
                    bool adapted_operands = false;
                    if (raw_expected != 0 && expected == 0)
                    {
                        const mpfr::real base_x = x_real;
                        const mpfr::real base_y = y_real;
                        const mpfr::real x_step = native_step<Value>(base_x);
                        const mpfr::real y_step = native_step<Value>(base_y);
                        for (int attempt = 1; attempt <= 16; ++attempt)
                        {
                            Value candidate_x = project_to_value<Value>(
                                base_x + attempt * x_step);
                            Value candidate_y = project_to_value<Value>(
                                base_y + (2 * attempt + 1) * y_step);
                            x = std::move(candidate_x);
                            y = std::move(candidate_y);
                            x_real =
                                implementations::value_traits<Value>::to_real(x);
                            y_real =
                                implementations::value_traits<Value>::to_real(y);
                            set_cancelling_z();
                            if (expected != 0)
                            {
                                adapted_operands = true;
                                break;
                            }
                        }
                        if (expected == 0)
                        {
                            throw std::runtime_error(
                                "unable to preserve nonzero fma cancellation "
                                "after native input conversion");
                        }
                    }
                    input_description =
                        describe(x_sample) + " ; " + describe(y_sample) +
                        " ; native product cancellation";
                    if (adapted_operands)
                    {
                        input_description +=
                            " [operands adapted to native precision]";
                    }
                }
                else
                {
                    expected = reference(x_real, y_real, z_real);
                }

                const Value value(evaluate(x, y, z));
                add_sample(
                    value,
                    expected,
                    false,
                    std::move(input_description),
                    measured,
                    bits);
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value, class Eval>
        void measure_parse(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::text_domain& domain,
            Eval evaluate,
            const std::vector<mpfr::real>& expected,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::parse<Value>(evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::parse<Value>(evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size());
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const Value value(evaluate(domain.values[i]));
                add_sample(
                    value,
                    expected[i],
                    false,
                    domain.values[i],
                    measured,
                    bits);
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value, class Eval>
        void measure_format(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const domains::domain& domain,
            Eval evaluate,
            bool gated)
        {
            const std::string special = cached_special(
                info, operation, gated,
                [&] { return special_values::format<Value>(evaluate); });
            const std::string zero = cached_signed_zero(
                info, operation,
                [&] { return signed_zero::format<Value>(evaluate); });
            result measured{};
            std::vector<double> bits;
            bits.reserve(domain.values.size());
            for (std::size_t i = 0; i < domain.values.size(); ++i)
            {
                const Value input =
                    implementations::value_traits<Value>::from_sample(
                        domain.values[i]);
                require_finite(input, operation, domain.name);
                const std::string text = evaluate(input);

                // MPFR parses the formatter output directly. This deliberately
                // avoids every library parser, including fltx's, so the row
                // measures only the numeric information retained by formatting.
                const mpfr::real observed{text};
                const mpfr::real expected =
                    implementations::value_traits<Value>::to_real(input);
                const double sample_bits =
                    normalized_bits<Value>(mpfr::resolution_adjusted_bits(
                        observed,
                        expected,
                        implementations::value_traits<Value>::absolute_resolution(),
                        implementations::value_traits<Value>::nominal_bits));
                bits.push_back(sample_bits);
                if (bits.size() == 1 || sample_bits < measured.worst_bits)
                {
                    measured.worst_bits = sample_bits;
                    measured.worst_input = describe(domain.values[i]);
                    measured.observed = text;
                    measured.reference = mpfr::text(expected);
                }
            }
            finish<Value>(
                info, group, operation, domain, measured, bits,
                special, zero, gated);
        }

        template<class Value>
        [[nodiscard]] double score(
            const Value& value,
            const mpfr::real& expected,
            bool require_exact) const
        {
            const mpfr::real observed =
                implementations::value_traits<Value>::to_real(value);
            if (!require_exact)
            {
                return normalized_bits<Value>(mpfr::resolution_adjusted_bits(
                    observed,
                    expected,
                    implementations::value_traits<Value>::absolute_resolution(),
                    implementations::value_traits<Value>::nominal_bits));
            }

            bool matches = false;
            if (mpfr::is_nan(expected))
                matches = mpfr::is_nan(observed);
            else if (mpfr::is_inf(expected))
            {
                matches =
                    mpfr::is_inf(observed) &&
                    mpfr::sign_bit(observed) == mpfr::sign_bit(expected);
            }
            else
            {
                matches = observed == expected;
            }
            return matches
                ? mpfr::exact_score()
                : 0.0;
        }

        template<class Value>
        void add_sample(
            const Value& value,
            const mpfr::real& expected,
            bool require_exact,
            std::string input,
            result& measured,
            std::vector<double>& bits)
        {
            const mpfr::real observed =
                implementations::value_traits<Value>::to_real(value);
            const double sample_bits = score(value, expected, require_exact);
            bits.push_back(sample_bits);
            if (bits.size() == 1 || sample_bits < measured.worst_bits)
            {
                measured.worst_bits = sample_bits;
                measured.worst_input = std::move(input);
                measured.observed = mpfr::text(observed);
                measured.reference = mpfr::text(expected);
            }
        }

        template<class First, class Second>
        void add_pair_sample(
            const First& first,
            const Second& second,
            const mpfr::real& expected_first,
            const mpfr::real& expected_second,
            double sample_bits,
            std::string input,
            result& measured,
            std::vector<double>& bits)
        {
            bits.push_back(sample_bits);
            if (bits.size() == 1 || sample_bits < measured.worst_bits)
            {
                measured.worst_bits = sample_bits;
                measured.worst_input = std::move(input);
                measured.observed =
                    mpfr::text(
                        implementations::value_traits<First>::to_real(first)) +
                    " ; " +
                    mpfr::text(
                        implementations::value_traits<Second>::to_real(second));
                measured.reference =
                    mpfr::text(expected_first) + " ; " +
                    mpfr::text(expected_second);
            }
        }

        template<class Value>
        static void require_finite(
            const Value& value,
            std::string_view operation,
            std::string_view domain)
        {
            if (!implementations::value_traits<Value>::is_finite(value))
            {
                throw std::runtime_error(
                    "non-finite input escaped into accuracy row " +
                    std::string(operation) + "/" + std::string(domain));
            }
        }

        [[nodiscard]] bool selected(std::string_view operation) const
        {
            return settings_.filter.empty() ||
                   operation.find(settings_.filter) != std::string_view::npos;
        }

        template<class Value>
        [[nodiscard]] static double normalized_bits(double bits)
        {
            const double ceiling =
                implementations::value_traits<Value>::nominal_bits + 32.0;
            if (native_fp::is_nan(bits))
                return 0.0;
            if (mpfr::is_exact_score(bits))
                return bits;
            return std::clamp(bits, 0.0, ceiling);
        }

        template<class Value>
        [[nodiscard]] static double mean_bits(double bits)
        {
            return mpfr::is_exact_score(bits)
                ? implementations::value_traits<Value>::nominal_bits + 32.0
                : bits;
        }

        template<class Probe>
        [[nodiscard]] std::string cached_special(
            const implementation_info& info,
            std::string_view operation,
            bool gated,
            Probe probe)
        {
            if constexpr (special_value_probes_enabled)
                return cached_special_enabled(
                    info, operation, gated, std::move(probe));
            else
                return "-";
        }

        template<class Probe>
        [[nodiscard]] std::string cached_special_enabled(
            const implementation_info& info,
            std::string_view operation,
            bool gated,
            Probe probe)
        {
            const std::string key =
                info.id + '\n' + std::string(operation);
            if (const auto found = special_support_.find(key);
                found != special_support_.end())
                return found->second;

            // These dd_real functions enter finite-only reduction/Newton code
            // for non-finite inputs and can access invalid intermediate state.
            // Record that unsupported behavior without letting an
            // informational comparison terminate the complete run.
            const bool unsafe_ddreal_special =
                operation == "tan" || operation == "atan" ||
                operation == "atan2" || operation == "asin" ||
                operation == "acos";
            const std::string category =
                info.id == "qdpp" &&
                implementations::precision_name<Float> == "f128" &&
                unsafe_ddreal_special
                    ? "No"
                    : probe().category();
            special_support_.emplace(key, category);
            #if defined(FLTX_FAST_MATH)
            const bool relaxed_basic_arithmetic =
                info.id == "fltx" &&
                (implementations::precision_name<Float> == "f128" ||
                 implementations::precision_name<Float> == "f256") &&
                (operation == "add" || operation == "subtract" ||
                 operation == "multiply" || operation == "divide");
            #else
            constexpr bool relaxed_basic_arithmetic = false;
            #endif
            if (gated && !relaxed_basic_arithmetic &&
                category != "-" && category != "Both")
            {
                ++failures_;
                std::cerr
                    << "special-value contract failed: "
                    << implementations::precision_name<Float>
                    << " fltx " << operation
                    << " supports " << category << " only\n";
            }
            return category;
        }

        template<class Probe>
        [[nodiscard]] std::string cached_signed_zero(
            const implementation_info& info,
            std::string_view operation,
            Probe probe)
        {
            const std::string key = info.id + '\n' + std::string(operation);
            if (const auto found = signed_zero_support_.find(key);
                found != signed_zero_support_.end())
            {
                return found->second;
            }
            const std::string category = probe().category();
            signed_zero_support_.emplace(key, category);
            return category;
        }

        template<class Value, class Domain>
        void finish(
            const implementation_info& info,
            std::string_view group,
            std::string_view operation,
            const Domain& domain,
            result& measured,
            std::vector<double>& bits,
            std::string_view special,
            std::string_view zero,
            bool gated)
        {
            if (bits.empty())
                throw std::runtime_error("accuracy domain produced no samples");

            double total = 0.0;
            bool all_exact = true;
            for (const double value : bits)
            {
                total += mean_bits<Value>(value);
                all_exact =
                    all_exact && mpfr::is_exact_score(value);
            }
            std::sort(bits.begin(), bits.end());
            measured.samples = bits.size();
            measured.mean_bits = all_exact
                ? mpfr::exact_score()
                : total / static_cast<double>(bits.size());
            measured.p01_bits = bits[
                std::min(
                    bits.size() - 1,
                    static_cast<std::size_t>((bits.size() - 1) * 0.01))];

            std::string required;
            std::string margin;
            std::string passed;
            if (gated)
            {
                const double minimum = thresholds::required(
                    mpfr::traits<Float>::precision,
                    operation,
                    domain.name);
                const double safety = measured.worst_bits - minimum;
                const bool ok = safety >= 0.0;
                required = number(minimum);
                margin = number(safety);
                passed = ok ? "yes" : "no";
                failures_ += ok ? 0 : 1;
                ++fltx_rows_;
            }

            output_.row({
                std::string(schema_version),
                settings_.run_id,
                settings_.source_revision,
                fltx::tests::support::source_fingerprint,
                std::string(implementations::precision_name<Float>),
                std::string(group),
                std::string(operation),
                info.id,
                info.short_label,
                info.label,
                info.api,
                domain.name,
                std::to_string(measured.samples),
                std::to_string(domain.seed),
                number(measured.mean_bits),
                number(measured.p01_bits),
                number(measured.worst_bits),
                required,
                margin,
                passed,
                std::string(special),
                std::string(zero),
                measured.worst_input,
                measured.observed,
                measured.reference
            });

            std::cout << std::left
                      << std::setw(8)
                      << implementations::precision_name<Float>
                      << std::setw(9)
                      << info.short_label
                      << std::setw(32)
                      << operation
                      << std::setw(20)
                      << domain.name
                      << "mean "
                      << std::setw(9)
                      << display_score(measured.mean_bits)
                      << "worst "
                      << std::setw(9)
                      << display_score(measured.worst_bits);
            if (gated)
                std::cout << (passed == "yes" ? "PASS" : "FAIL");
            else
                std::cout << "info";
            std::cout << '\n' << std::flush;
        }

        csv_writer& output_;
        const options& settings_;
        int failures_ = 0;
        std::size_t fltx_rows_ = 0;
        std::unordered_map<std::string, std::string> special_support_;
        std::unordered_map<std::string, std::string> signed_zero_support_;
    };

    int run_f128(csv_writer& output, const options& settings);
    int run_f256(csv_writer& output, const options& settings);
    int run_f32(csv_writer& output, const options& settings);
    int run_f64(csv_writer& output, const options& settings);
}

#endif
