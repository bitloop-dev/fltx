#ifndef FLTX_TESTS_BENCHMARK_SAMPLES_INCLUDED
#define FLTX_TESTS_BENCHMARK_SAMPLES_INCLUDED

#include "../../support/samples.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace fltx::tests::benchmark::samples
{
    struct input
    {
        sample x;
        sample y;
        sample z;
    };

    [[nodiscard]] inline double signed_log(
        random_bits& rng,
        int minimum_exponent,
        int maximum_exponent)
    {
        const int span = maximum_exponent - minimum_exponent + 1;
        const int exponent = minimum_exponent +
            static_cast<int>(rng.next() % static_cast<std::uint64_t>(span));
        const double sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
        return sign * std::ldexp(rng.between(1.0, 2.0), exponent);
    }

    [[nodiscard]] inline double positive_log(
        random_bits& rng,
        int minimum_exponent,
        int maximum_exponent)
    {
        return std::abs(signed_log(rng, minimum_exponent, maximum_exponent));
    }

    [[nodiscard]] inline bool uses_large_adaptive_corpus(
        std::string_view operation)
    {
        constexpr std::string_view operations[] = {
            "add", "subtract", "multiply", "divide",
            "equal", "not_equal", "less", "less_equal", "greater",
            "greater_equal", "abs", "fabs", "fmin", "fmax", "fdim",
            "copysign", "fma", "modf", "ldexp", "scalbn", "scalbln",
            "frexp", "ilogb", "logb", "nextafter", "nexttoward",
            "floor", "ceil", "trunc", "round", "roundeven", "sqrt", "sqr",
        };
        return std::find(
            std::begin(operations),
            std::end(operations),
            operation) != std::end(operations);
    }

    [[nodiscard]] inline std::size_t count_for(
        std::string_view operation,
        std::size_t base_count,
        std::string_view sample_mode = "custom")
    {
        const bool adaptive = sample_mode == "small" || sample_mode == "standard";
        if (
            adaptive &&
            (operation == "erf" || operation == "erfc" ||
             operation == "lgamma" || operation == "tgamma"))
        {
            return std::max<std::size_t>(1, base_count / 32);
        }

        if (adaptive && uses_large_adaptive_corpus(operation))
            return base_count * 10;

        if (operation == "lround" || operation == "llround")
            return std::min<std::size_t>(base_count, 8192);

        if (operation == "fmod" || operation == "remainder" || operation == "remquo")
            return std::max<std::size_t>(1, base_count / 2);

        if (operation == "erf" || operation == "erfc" ||
            operation == "lgamma" || operation == "tgamma" ||
            operation == "asin" || operation == "acos" ||
            operation == "atan" || operation == "atan2" ||
            operation == "asinh" || operation == "acosh" ||
            operation == "atanh" || operation == "pow" ||
            operation == "cbrt" || operation == "sin" ||
            operation == "cos" || operation == "sincos" ||
            operation == "tan" || operation == "exp" ||
            operation == "exp2" || operation == "expm1" ||
            operation == "log" || operation == "log2" ||
            operation == "log10" || operation == "log1p" ||
            operation == "sinh" || operation == "cosh" ||
            operation == "tanh")
        {
            return std::max<std::size_t>(1, base_count / 4);
        }

        return base_count;
    }

    [[nodiscard]] inline std::vector<input> make(
        std::string_view operation,
        std::size_t base_count,
        std::string_view sample_mode = "custom")
    {
        const std::size_t count = count_for(operation, base_count, sample_mode);
        std::vector<input> out;
        out.reserve(count);
        random_bits rng(default_seed ^ 0xb3f4219c5d7a0e61ull);

        const auto append = [&](double x, double y = 0.0, double z = 0.0) {
            out.push_back({ make_sample(x), make_sample(y), make_sample(z) });
        };

        for (std::size_t index = 0; index < count; ++index)
        {
            if (operation == "sin" || operation == "cos" || operation == "sincos" ||
                operation == "tan")
            {
                double x = rng.between(-0x1p16, 0x1p16);
                if ((index & 1u) != 0)
                    x += 1.57079632679489661923;
                append(x);
            }
            else if (operation == "exp")
                append(rng.between(-40.0, 40.0));
            else if (operation == "exp2")
                append(rng.between(-128.0, 128.0));
            else if (operation == "expm1")
                append(rng.between(-20.0, 20.0));
            else if (operation == "log1p")
                append(rng.between(-0.95, 20.0));
            else if (operation == "asin" || operation == "acos")
                append(rng.between(-1.0, 1.0));
            else if (operation == "atanh")
                append(rng.between(-0.999, 0.999));
            else if (operation == "acosh")
                append(1.0 + positive_log(rng, -20, 20));
            else if (operation == "sinh" ||
                     operation == "cosh" || operation == "tanh")
                append(rng.between(-40.0, 40.0));
            else if (operation == "asinh")
                append(signed_log(rng, -40, 40));
            else if (operation == "erf" || operation == "erfc")
                append(rng.between(-6.0, 6.0));
            else if (operation == "lgamma" || operation == "tgamma")
            {
                const double magnitude = rng.between(0.125, 35.0);
                const double x = index % 5 == 4 ? -magnitude - 0.25 : magnitude;
                append(x);
            }
            else if (operation == "pow")
            {
                switch (index % 5)
                {
                case 0: append(positive_log(rng, -12, 12), rng.between(-6.0, 6.0)); break;
                case 1: append(rng.between(0.875, 1.125), rng.between(-256.0, 256.0)); break;
                case 2: append(positive_log(rng, -32, 32), rng.between(-2.0, 2.0)); break;
                case 3: append(rng.between(0.125, 64.0), static_cast<double>(static_cast<int>(rng.next() % 33) - 16)); break;
                default: append(-positive_log(rng, -12, 12), static_cast<double>(static_cast<int>(rng.next() % 33) - 16)); break;
                }
            }
            else if (operation == "fma" || operation == "product_sum")
            {
                const double x = signed_log(rng, -24, 24);
                const double y = signed_log(rng, -24, 24);
                const double product = x * y;
                double z = 0.0;
                switch (index % 4)
                {
                case 0: z = signed_log(rng, -48, 48); break;
                case 1: z = -product + std::ldexp(product, -12); break;
                case 2: z = -0.5 * product; break;
                default: z = signed_log(rng, -12, 12); break;
                }
                append(x, y, z);
            }
            else if (operation == "floor" || operation == "ceil" ||
                     operation == "trunc" || operation == "round" ||
                     operation == "roundeven" || operation == "lround" ||
                     operation == "llround" || operation == "modf")
            {
                switch (index % 4)
                {
                case 0: append(rng.between(-1.0e6, 1.0e6)); break;
                case 1:
                    append(static_cast<double>(
                        static_cast<int>(rng.next() % 2'000'001) - 1'000'000) + 0.5);
                    break;
                case 2:
                    append(static_cast<double>(
                        static_cast<int>(rng.next() % 2'000'001) - 1'000'000));
                    break;
                default: append(signed_log(rng, -20, 29)); break;
                }
            }
            else if (operation == "fmod" || operation == "remainder" ||
                     operation == "remquo")
            {
                append(signed_log(rng, -40, 40), signed_log(rng, -16, 16));
            }
            else if (operation == "sqrt" || operation == "recip" ||
                     operation == "log" || operation == "log2" ||
                     operation == "log10" || operation == "ilogb" ||
                     operation == "logb")
            {
                append(positive_log(rng, -80, 80));
            }
            else if (operation == "hypot")
                append(signed_log(rng, -80, 80), signed_log(rng, -80, 80));
            else if (operation == "equal" || operation == "not_equal" ||
                     operation == "less" || operation == "less_equal" ||
                     operation == "greater" || operation == "greater_equal")
            {
                const double x = signed_log(rng, -80, 80);
                const double y = index % 5 == 0 ? x :
                    index % 5 == 1 ? std::nextafter(
                        x, std::numeric_limits<double>::infinity()) :
                    index % 5 == 2 ? std::nextafter(
                        x, -std::numeric_limits<double>::infinity()) :
                    index % 5 == 3 ? -x : signed_log(rng, -80, 80);
                append(x, y);
            }
            else if (operation == "divide" || operation == "atan2")
            {
                append(signed_log(rng, -80, 80), signed_log(rng, -80, 80));
            }
            else if (operation == "add" || operation == "subtract" ||
                     operation == "multiply" || operation == "fmin" ||
                     operation == "fmax" || operation == "fdim" ||
                     operation == "copysign")
            {
                const double x = signed_log(rng, -80, 80);
                const double y = index % 4 == 1 ? -x :
                    index % 4 == 2 ? x : signed_log(rng, -80, 80);
                append(x, y);
            }
            else if (operation == "cbrt" || operation == "atan" ||
                     operation == "abs" || operation == "fabs" ||
                     operation == "sqr" || operation == "ipow" ||
                     operation == "frexp" || operation == "nextafter" ||
                     operation == "nexttoward")
            {
                append(signed_log(rng, -80, 80), signed_log(rng, -80, 80));
            }
            else
            {
                append(rng.between(-4.0, 4.0), rng.between(-4.0, 4.0),
                    rng.between(-4.0, 4.0));
            }
        }

        return out;
    }

    [[nodiscard]] inline std::vector<std::string> decimal_strings(
        std::size_t count,
        int significant_digits)
    {
        return fltx::tests::decimal_strings(
            count,
            significant_digits,
            -80,
            80,
            default_seed ^ 0x5c7b61e29a40fd83ull);
    }
}

#endif
