#include "runner.hpp"
#include "../support/config_banner.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef FLTX_ACCURACY_EXPECTS_SIMULATED_CONSTEVAL
#error "accuracy runners must declare their expected consteval execution mode"
#endif

namespace
{
    [[nodiscard]] bool describes_build(int argc, char** argv) noexcept
    {
        return argc == 2 && std::string_view{ argv[1] } == "--describe";
    }

    using high_precision_real = boost::multiprecision::number<
        boost::multiprecision::mpfr_float_backend<800>,
        boost::multiprecision::et_off>;

    template<class Evaluate>
    [[nodiscard]] bool stable_reference(
        const char* input,
        Evaluate evaluate)
    {
        using fltx::tests::mpfr::real;
        const real low = evaluate(real{ input });
        const real projected = real{ evaluate(high_precision_real{ input }) };

        using boost::multiprecision::abs;
        using boost::multiprecision::ldexp;
        const real scale = std::max(abs(projected), real{ 1 });
        return abs(low - projected) <= ldexp(scale, -1200);
    }

    void check_oracle_precision()
    {
        const bool stable =
            stable_reference(
                "1.23456789012345678901234567890123456789e100",
                [](const auto& value) {
                    using boost::multiprecision::sin;
                    return sin(value);
                }) &&
            stable_reference(
                "0.375",
                [](const auto& value) {
                    using boost::multiprecision::exp;
                    return exp(value);
                }) &&
            stable_reference(
                "1e-80",
                [](const auto& value) {
                    using boost::multiprecision::log1p;
                    return log1p(value);
                }) &&
            stable_reference(
                "3.99221356",
                [](const auto& value) {
                    using boost::multiprecision::erfc;
                    return erfc(value);
                });

        if (!stable)
            throw std::logic_error(
                "400-digit MPFR references changed at 800-digit precision");
    }

    void check_accuracy_metric()
    {
        using fltx::tests::mpfr::real;
        using fltx::tests::mpfr::sign_bit;

        if (!sign_bit(fltx::tests::mpfr::traits<bl::f128>::to_real(
                bl::f128_s{ -0.0, 0.0 })) ||
            !sign_bit(fltx::tests::mpfr::traits<bl::f256>::to_real(
                bl::f256_s{ -0.0, 0.0, 0.0, 0.0 })))
        {
            throw std::logic_error(
                "MPFR expansion conversion did not preserve negative zero");
        }

        const real resolution = fltx::tests::mpfr::absolute_resolution<bl::f128>();
        const auto bits = [&](const real& observed, const real& reference) {
            return fltx::tests::mpfr::resolution_adjusted_bits(
                observed, reference, resolution, 106.0);
        };

        if (bits(real{ 0 }, resolution) != 0.0 ||
            bits(-resolution, resolution) != 0.0 ||
            !std::isinf(bits(real{ 0 }, resolution / 4)) ||
            bits(real{ 0 }, -resolution / 4) != 0.0 ||
            !std::isinf(bits(real{ -0.0 }, -resolution / 4)) ||
            !std::isinf(bits(resolution, resolution)))
        {
            throw std::logic_error("accuracy metric failed its zero/resolution self-check");
        }
    }

    [[nodiscard]] BL_NO_INLINE bool runtime_execution_mode_is_consteval() noexcept
    {
        // Keep the query in a non-constexpr call frame: an argument-free direct
        // query may itself be constant-folded by an optimizing compiler.
        return bl::detail::is_constant_evaluated();
    }

    void check_execution_mode()
    {
        constexpr bool expected =
            FLTX_ACCURACY_EXPECTS_SIMULATED_CONSTEVAL != 0;
        const bool actual = runtime_execution_mode_is_consteval();
        if (actual != expected)
        {
            throw std::logic_error(
                expected
                    ? "fixed accuracy runner did not enter simulated consteval mode"
                    : "runtime accuracy runner unexpectedly entered simulated consteval mode");
        }
    }

    [[nodiscard]] fltx::tests::accuracy::options parse_arguments(int argc, char** argv)
    {
        fltx::tests::accuracy::options out;
        bool sample_mode_set = false;
        bool samples_set = false;
        for (int i = 1; i < argc; ++i)
        {
            const std::string argument = argv[i];
            auto value = [&]() -> std::string {
                if (++i >= argc)
                    throw std::runtime_error("missing value after " + argument);
                return argv[i];
            };

            if (argument == "--precision")
                out.precision = value();
            else if (argument == "--output")
                out.output = value();
            else if (argument == "--run-id")
                out.run_id = value();
            else if (argument == "--source-revision")
                out.source_revision = value();
            else if (argument == "--filter")
                out.filter = value();
            else if (argument == "--samples")
            {
                out.samples = static_cast<std::size_t>(std::stoull(value()));
                samples_set = true;
            }
            else if (argument == "--sample-mode")
            {
                out.sample_mode = value();
                sample_mode_set = true;
                if (out.sample_mode != "smoke" && out.sample_mode != "standard" &&
                    out.sample_mode != "full")
                    throw std::runtime_error(
                        "--sample-mode must be smoke, standard or full");
            }
            else if (argument == "--help")
            {
                std::cout
                    << "fltx_accuracy --precision f32|f64|f128|f256 --output FILE "
                       "--run-id ID --source-revision REV "
                       "[--sample-mode smoke|standard|full] [--samples N] "
                       "[--filter TEXT]\n"
                       "fltx_accuracy --describe\n";
                std::exit(0);
            }
            else
                throw std::runtime_error("unknown argument: " + argument);
        }

        if (out.precision != "f32" && out.precision != "f64" &&
            out.precision != "f128" && out.precision != "f256")
            throw std::runtime_error("--precision must be f32, f64, f128 or f256");
        if (sample_mode_set && !samples_set)
            out.samples = out.sample_mode == "smoke"
                ? 12
                : out.sample_mode == "standard" ? 4096 : 65536;
        else if (!sample_mode_set && samples_set)
            out.sample_mode = "custom";
        if (out.output.empty() || out.run_id.empty() || out.source_revision.empty())
            throw std::runtime_error("--output, --run-id and --source-revision are required");
        if (out.samples == 0)
            throw std::runtime_error("--samples must be greater than zero");
        return out;
    }
}

int main(int argc, char** argv)
{
    try
    {
        check_execution_mode();
        if (describes_build(argc, argv))
        {
            fltx::tests::support::print_config_banner("accuracy");
            return 0;
        }

        const auto settings = parse_arguments(argc, argv);
        check_accuracy_metric();
        check_oracle_precision();
        fltx::tests::support::print_config_banner(
            "accuracy",
            settings.sample_mode.c_str(),
            settings.samples);
        fltx::tests::csv_writer output(settings.output);
        output.row({
            "schema_version", "run_id", "source_revision", "source_fingerprint",
            "precision", "group", "operation",
            "implementation", "implementation_short", "implementation_label",
            "api", "domain", "samples", "seed",
            "mean_bits", "p01_bits", "worst_bits", "required_worst_bits",
            "margin_bits", "pass", "special_support",
            "worst_input", "observed", "reference"
        });

        int failures = 0;
        if (settings.precision == "f32")
            failures = fltx::tests::accuracy::run_f32(output, settings);
        else if (settings.precision == "f64")
            failures = fltx::tests::accuracy::run_f64(output, settings);
        else if (settings.precision == "f128")
            failures = fltx::tests::accuracy::run_f128(output, settings);
        else
            failures = fltx::tests::accuracy::run_f256(output, settings);
        if (failures != 0)
            return 1;

        output.finish();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "fltx_accuracy: " << error.what() << '\n';
        return 2;
    }
}
