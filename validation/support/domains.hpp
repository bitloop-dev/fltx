#ifndef FLTX_TESTS_SUPPORT_DOMAINS_INCLUDED
#define FLTX_TESTS_SUPPORT_DOMAINS_INCLUDED

#include "samples.hpp"

#include <fltx/config.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fltx::tests::domains
{
    struct domain
    {
        std::string name;
        std::uint64_t seed = default_seed;
        std::vector<sample> values;
    };

    struct binary_sample
    {
        sample lhs;
        sample rhs;
        bool preserve_nonzero = false;
    };

    struct binary_domain
    {
        std::string name;
        std::uint64_t seed = default_seed;
        std::vector<binary_sample> values;
    };

    enum class arithmetic_operation
    {
        add,
        subtract,
        multiply,
        divide
    };

    struct text_domain
    {
        std::string name;
        std::uint64_t seed = default_seed;
        std::vector<std::string> values;
    };

    [[nodiscard]] inline text_domain decimal_text(
        std::string name,
        std::size_t count,
        int significant_digits,
        int minimum_exponent,
        int maximum_exponent,
        std::uint64_t seed)
    {
        return {
            std::move(name),
            seed,
            decimal_strings(
                count,
                significant_digits,
                minimum_exponent,
                maximum_exponent,
                seed)
        };
    }

    [[nodiscard]] inline domain interval(
        std::string name,
        double low,
        double high,
        std::size_t count,
        std::uint64_t seed)
    {
        domain out{ std::move(name), seed, {} };
        out.values.reserve(count + 5);
        out.values.push_back(make_exact_sample(low, "lower endpoint"));
        out.values.push_back(make_exact_sample(high, "upper endpoint"));
        out.values.push_back(make_exact_sample((low + high) * 0.5, "midpoint"));

        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
            out.values.push_back(make_sample(rng.between(low, high)));
        return out;
    }

    [[nodiscard]] inline domain near_one(
        std::size_t count,
        std::uint64_t seed = default_seed ^ 0x01u)
    {
        return interval("near_one", 0.5, 1.5, count, seed);
    }

    [[nodiscard]] inline domain moderate(
        std::size_t count,
        double low = -8.0,
        double high = 8.0,
        std::uint64_t seed = default_seed ^ 0x02u)
    {
        return interval("moderate", low, high, count, seed);
    }

    [[nodiscard]] inline binary_domain arithmetic_general(
        arithmetic_operation operation,
        std::size_t count,
        int precision_bits,
        std::uint64_t seed = default_seed ^ 0x42u)
    {
        // One compact, deterministic corpus. Each 20-case block is 70%
        // representative log-random values, at least 20% broad-range values,
        // and 5% near the finite exponent limits. Add/subtract devote the
        // remaining 5% to subnormals; multiply/divide use another broad case
        // whose result remains finite. One representative case has an
        // operation-appropriate structured operand relationship. Keeping a
        // complete block even in smoke mode is the coverage contract.
        const std::size_t sample_count = std::max<std::size_t>(count, 20);
        binary_domain out{ "general", seed, {} };
        out.values.reserve(sample_count);
        random_bits rng(seed ^ static_cast<std::uint64_t>(operation));
        const bool additive = operation == arithmetic_operation::add ||
            operation == arithmetic_operation::subtract;

        const auto random_exponent = [&](int low, int high) {
            return low + static_cast<int>(
                rng.next() % static_cast<std::uint64_t>(high - low + 1));
        };
        const auto log_sample = [&](int low, int high, std::string label) {
            const int exponent = random_exponent(low, high);
            const double sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
            return make_sample(
                sign * std::ldexp(rng.between(0.5, 1.0), exponent),
                std::move(label));
        };
        const auto append_random_pair = [&](int low, int high, const char* stratum) {
            out.values.push_back({
                log_sample(low, high, std::string{stratum} + " lhs"),
                log_sample(low, high, std::string{stratum} + " rhs"),
                false
            });
        };

        static constexpr std::array<int, 8> extreme_exponents{
            -1021, -1000, -900, -500, 500, 900, 1000, 1021
        };
        static constexpr std::array<int, 4> multiplicative_extreme_exponents{
            -900, -500, 500, 900
        };
        for (std::size_t i = 0; i < sample_count; ++i)
        {
            const std::size_t slot = i % 20;
            if (slot == 0)
            {
                const int lhs_exponent = additive
                    ? extreme_exponents[
                        (i / 20 + rng.next()) % extreme_exponents.size()]
                    : multiplicative_extreme_exponents[
                        (i / 20 + rng.next()) %
                            multiplicative_extreme_exponents.size()];
                const int rhs_exponent = operation == arithmetic_operation::multiply
                    ? std::clamp(
                        -lhs_exponent + random_exponent(-80, 80),
                        -1021,
                        1021)
                    : lhs_exponent;
                const double lhs_sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                const double rhs_sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                out.values.push_back({
                    make_sample(
                        lhs_sign * std::ldexp(
                            rng.between(0.5, 0.74), lhs_exponent),
                        "extreme lhs"),
                    make_sample(
                        rhs_sign * std::ldexp(
                            rng.between(0.5, 0.74), rhs_exponent),
                        "extreme rhs"),
                    false
                });
            }
            else if (slot == 1)
            {
                if (!additive)
                {
                    append_random_pair(-400, 400, "broad");
                    continue;
                }
                const int lhs_exponent = random_exponent(-1073, -1022);
                const int rhs_exponent = random_exponent(-1073, -1022);
                const double lhs_sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                const double rhs_sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                out.values.push_back({
                    make_exact_sample(
                        lhs_sign * std::ldexp(rng.between(0.5, 1.0), lhs_exponent),
                        "subnormal lhs"),
                    make_exact_sample(
                        rhs_sign * std::ldexp(rng.between(0.5, 1.0), rhs_exponent),
                        "subnormal rhs"),
                    false
                });
            }
            else if (slot == 2)
            {
                sample lhs = log_sample(-80, 80, "structured lhs");
                sample rhs;
                if (operation == arithmetic_operation::multiply)
                {
                    rhs = make_sample(1.0 / lhs.limb[0], "balanced-product rhs");
                    out.values.push_back({std::move(lhs), std::move(rhs), false});
                    continue;
                }

                rhs = lhs;
                if (operation == arithmetic_operation::add)
                {
                    for (double& limb : rhs.limb)
                        limb = -limb;
                }
                const int maximum_offset_bits = std::max(80, precision_bits - 7);
                const int offset_bits = 65 + static_cast<int>(
                    rng.next() % static_cast<std::uint64_t>(
                        maximum_offset_bits - 64));
                const double direction = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                const std::size_t offset_limb = offset_bits <= 105
                    ? 1
                    : (offset_bits <= 159 ? 2 : 3);
                rhs.limb[offset_limb] += direction *
                    std::ldexp(std::abs(lhs.limb[0]), -offset_bits);
                rhs.label = operation == arithmetic_operation::divide
                    ? "near-equal divisor"
                    : "cancellation rhs";
                out.values.push_back({
                    std::move(lhs),
                    std::move(rhs),
                    operation == arithmetic_operation::add ||
                        operation == arithmetic_operation::subtract
                });
            }
            else if (slot == 3 || slot == 8 || slot == 13 || slot == 18)
            {
                append_random_pair(
                    additive ? -900 : -400,
                    additive ? 900 : 400,
                    "broad");
            }
            else
            {
                append_random_pair(-80, 80, "representative");
            }
        }
        return out;
    }

    [[nodiscard]] inline std::size_t binary_size(const domain& input) noexcept
    {
        return input.values.size();
    }

    [[nodiscard]] inline const sample& binary_lhs(
        const domain& input,
        std::size_t index) noexcept
    {
        return input.values[index];
    }

    [[nodiscard]] inline const sample& binary_rhs(
        const domain& input,
        std::size_t index) noexcept
    {
        return input.values[(index * 7 + 3) % input.values.size()];
    }

    [[nodiscard]] inline bool preserve_nonzero(
        const domain& input,
        std::size_t) noexcept
    {
        return input.name == "cancellation";
    }

    [[nodiscard]] inline std::size_t binary_size(
        const binary_domain& input) noexcept
    {
        return input.values.size();
    }

    [[nodiscard]] inline const sample& binary_lhs(
        const binary_domain& input,
        std::size_t index) noexcept
    {
        return input.values[index].lhs;
    }

    [[nodiscard]] inline const sample& binary_rhs(
        const binary_domain& input,
        std::size_t index) noexcept
    {
        return input.values[index].rhs;
    }

    [[nodiscard]] inline bool preserve_nonzero(
        const binary_domain& input,
        std::size_t index) noexcept
    {
        return input.values[index].preserve_nonzero;
    }

    [[nodiscard]] inline domain wide_exponent(
        std::size_t count,
        bool positive = false,
        int min_exponent = -900,
        int max_exponent = 900,
        std::uint64_t seed = default_seed ^ 0x03u)
    {
        domain out{ "wide_exponent", seed, {} };
        out.values.reserve(count + 4);
        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const int exponent = min_exponent +
                static_cast<int>(rng.next() % static_cast<std::uint64_t>(max_exponent - min_exponent + 1));
            const double sign = positive || (rng.next() & 1u) != 0 ? 1.0 : -1.0;
            out.values.push_back(make_sample(sign * std::ldexp(rng.between(0.5, 1.0), exponent)));
        }
        return out;
    }

    [[nodiscard]] inline domain extreme_finite(
        std::size_t count,
        bool positive = false,
        std::uint64_t seed = default_seed ^ 0x04u)
    {
        static constexpr std::array<int, 8> exponents{
            -1021, -1000, -900, -500, 500, 900, 1000, 1021
        };
        domain out{ "extreme_finite", seed, {} };
        out.values.reserve(count + exponents.size());
        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const int exponent = exponents[rng.next() % exponents.size()];
            const double sign = positive || (rng.next() & 1u) != 0 ? 1.0 : -1.0;
            out.values.push_back(make_sample(sign * std::ldexp(rng.between(0.5, 0.99), exponent)));
        }
        return out;
    }

    [[nodiscard]] inline domain boundary(
        std::size_t count,
        double low = -1.0,
        double high = 1.0,
        std::uint64_t seed = default_seed ^ 0x05u)
    {
        domain out{ "boundary", seed, {} };
        const std::array<double, 9> anchors{
            low, std::nextafter(low, high), -0.5, -0.0, 0.0,
            0.5, std::nextafter(high, low), high, (low + high) * 0.5
        };
        for (double value : anchors)
        {
            if (value >= low && value <= high)
                out.values.push_back(make_exact_sample(value));
        }

        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const double anchor = (rng.next() & 1u) != 0 ? low : high;
            const double inward = anchor == low ? 1.0 : -1.0;
            out.values.push_back(make_sample(anchor + inward * std::ldexp(rng.between(0.5, 1.0), -20)));
        }
        return out;
    }

    BL_PUSH_PRECISE;
    [[nodiscard]] inline domain near_equal_cancellation(
        std::size_t count,
        std::uint64_t seed = default_seed ^ 0x06u)
    {
        domain out{ "cancellation", seed, {} };
        out.values.reserve(count + 4);
        out.values.push_back(make_exact_sample(1.0, "one"));
        out.values.push_back(make_sample(std::nextafter(1.0, 2.0), "next double after one"));
        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const int exponent = 20 + static_cast<int>(rng.next() % 160u);
            const double delta = std::ldexp(rng.between(0.5, 1.0), -exponent);
            const double high = 1.0 + delta;
            const double low = delta - (high - 1.0);
            out.values.push_back({
                { high, low, 0.0, 0.0 },
                "one plus a low-limb delta"
            });
        }
        return out;
    }
    BL_POP_PRECISE;

    [[nodiscard]] inline domain near_zero_cancellation(
        std::size_t count,
        std::uint64_t seed = default_seed ^ 0x16u)
    {
        domain out{ "cancellation", seed, {} };
        out.values.reserve(count + 4);
        out.values.push_back(make_exact_sample(0x1p-20, "small positive"));
        out.values.push_back(make_exact_sample(-0x1p-20, "small negative"));

        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const int exponent = 20 + static_cast<int>(rng.next() % 880u);
            const double sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
            out.values.push_back(make_sample(
                sign * std::ldexp(rng.between(0.5, 1.0), -exponent)));
        }
        return out;
    }

    [[nodiscard]] inline domain argument_reduction_impl(
        std::size_t count,
        int max_exponent,
        std::uint64_t seed,
        std::size_t retained_limbs)
    {
        // Independent four-double expansion of pi/2. Scaling it by 2^n gives
        // exact integer multiples without first rounding the anchor to binary64.
        static constexpr std::array<double, 4> half_pi{
            1.5707963267948966,
            6.1232339957367660e-17,
            -1.4973849048591698e-33,
            5.5622711043168264e-50
        };

        domain out{ "argument_reduction", seed, {} };
        out.values.reserve(count + (retained_limbs == 2 ? 14 : 8));

        const auto append = [&](
            int exponent,
            std::size_t offset_limb,
            double offset,
            bool negative,
            const char* relation = nullptr)
        {
            sample value{};
            for (std::size_t limb = 0; limb < retained_limbs; ++limb)
                value.limb[limb] = std::ldexp(half_pi[limb], exponent);

            const double original = value.limb[offset_limb];
            value.limb[offset_limb] += offset;
            if (retained_limbs == 2 && value.limb[offset_limb] == original)
            {
                throw std::logic_error(
                    "f128 argument-reduction perturbation was rounded away");
            }

            if (negative)
            {
                for (double& limb : value.limb)
                    limb = -limb;
            }
            value.label =
                (relation == nullptr ? "near " : std::string{ relation } + ' ') +
                "2^" + std::to_string(exponent) + " * pi/2";
            out.values.push_back(std::move(value));
        };

        static constexpr std::array<int, 6> anchor_exponents{ 0, 20, 53, 56, 100, 159 };
        if (retained_limbs == 2)
        {
            // Keep the perturbation comfortably above the low limb's ulp while
            // remaining about 100 bits below the full value.
            for (int exponent : anchor_exponents)
            {
                if (exponent <= max_exponent)
                {
                    const double offset = std::ldexp(0.625, exponent - 100);
                    append(exponent, 1, -offset, false, "below");
                    append(exponent, 1, offset, false, "above");
                }
            }
        }
        else
        {
            for (std::size_t i = 0; i < anchor_exponents.size(); ++i)
            {
                const int exponent = anchor_exponents[i];
                if (exponent <= max_exponent)
                {
                    append(
                        exponent,
                        3,
                        std::ldexp(0.625, exponent - 172),
                        (i & 1u) != 0);
                }
            }
        }

        const auto append_anchor = [&](sample value) {
            for (std::size_t limb = retained_limbs; limb < value.limb.size(); ++limb)
                value.limb[limb] = 0.0;
            out.values.push_back(std::move(value));
        };

        if (max_exponent >= 56)
        {
            append_anchor({
                {
                    1.1318780403245504e+17,
                    4.4122550946403924,
                    -1.0804409825880908e-16,
                    4.0080387316375899e-33
                },
                "large even multiple anchor"
            });
        }
        if (max_exponent >= 53)
        {
            append_anchor({
                {
                    -1.4148475504056880e+16,
                    -0.55153188683004906,
                    1.3487244199107926e-17,
                    -4.9989771504159895e-34
                },
                "large negative multiple anchor"
            });
        }

        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const int exponent = static_cast<int>(
                rng.next() % static_cast<std::uint64_t>(max_exponent + 1));
            if (retained_limbs == 2)
            {
                static constexpr std::array<int, 3> offset_bits{ 64, 82, 100 };
                const double magnitude = rng.between(0.5, 1.0);
                const double sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                append(
                    exponent,
                    1,
                    std::ldexp(sign * magnitude, exponent - offset_bits[i % 3]),
                    (i & 1u) != 0);
            }
            else
            {
                const std::size_t offset_limb = 1 + i % 3;
                const int offset_bits =
                    offset_limb == 1 ? 64 : offset_limb == 2 ? 118 : 172;
                append(
                    exponent,
                    offset_limb,
                    std::ldexp(rng.between(-1.0, 1.0), exponent - offset_bits),
                    (i & 1u) != 0);
            }
        }
        return out;
    }

    [[nodiscard]] inline domain argument_reduction(
        std::size_t count,
        int max_exponent = 159,
        std::uint64_t seed = default_seed ^ 0x07u)
    {
        return argument_reduction_impl(count, max_exponent, seed, 4);
    }

    [[nodiscard]] inline domain argument_reduction_f128(
        std::size_t count,
        int max_exponent = 159,
        std::uint64_t seed = default_seed ^ 0x07u)
    {
        return argument_reduction_impl(count, max_exponent, seed, 2);
    }

    [[nodiscard]] inline domain subnormal(
        std::size_t count,
        std::uint64_t seed = default_seed ^ 0x08u)
    {
        domain out{ "subnormal", seed, {} };
        out.values.push_back(make_exact_sample(0.0, "+0"));
        out.values.push_back(make_exact_sample(-0.0, "-0"));
        out.values.push_back(make_exact_sample(std::numeric_limits<double>::denorm_min(), "denorm_min"));
        out.values.push_back(make_exact_sample(std::numeric_limits<double>::min(), "min normal"));
        random_bits rng(seed);
        for (std::size_t i = 0; i < count; ++i)
        {
            const double value = std::ldexp(
                rng.between(0.5, 1.0),
                -1073 + static_cast<int>(rng.next() % 53u));
            out.values.push_back(make_exact_sample((rng.next() & 1u) ? value : -value));
        }
        return out;
    }

    [[nodiscard]] inline domain atan2_extreme(
        std::uint64_t seed = default_seed ^ 0x09u)
    {
        domain out{ "extreme_finite", seed, {} };
        const double huge = 0x1.123456789abcp+900;
        const double tiny = 0x1.abcdef012345p-900;
        out.values = {
            make_sample(huge, "huge positive y"),
            make_sample(-huge, "huge negative y"),
            make_sample(tiny, "tiny positive x"),
            make_sample(-tiny, "tiny negative x"),
            make_sample(0x1.0000000000001p+1000, "near maximum y"),
            make_sample(-0x1.0000000000001p+1000, "near minimum y"),
            make_sample(0x1.0p-1000, "near minimum positive x"),
            make_sample(-0x1.0p-1000, "near minimum negative x")
        };
        return out;
    }

    [[nodiscard]] inline domain product_extreme(
        std::uint64_t seed = default_seed ^ 0x0au)
    {
        domain out{ "extreme_finite", seed, {} };
        const double huge = 0x1.23456789abcdep+900;
        const double tiny = 0x1.0fedcba987654p-900;
        const double large = 0x1.3456789abcdefp+500;
        const double small_value = 0x1.123456789abcdp-500;
        out.values = {
            make_sample(huge, "huge multiplicand"),
            make_sample(-huge, "negative huge multiplicand"),
            make_sample(tiny, "tiny multiplier"),
            make_sample(-tiny, "negative tiny multiplier"),
            make_sample(large, "large multiplicand"),
            make_sample(-large, "negative large multiplicand"),
            make_sample(small_value, "small multiplier"),
            make_sample(-small_value, "negative small multiplier")
        };
        return out;
    }

    [[nodiscard]] inline domain division_extreme(
        std::uint64_t seed = default_seed ^ 0x0bu)
    {
        domain out{ "extreme_finite", seed, {} };
        const double huge = 0x1.23456789abcdep+900;
        const double huge_peer = 0x1.0fedcba987654p+900;
        const double tiny = 0x1.3456789abcdefp-900;
        const double tiny_peer = 0x1.123456789abcdp-900;
        out.values = {
            make_sample(huge, "huge numerator"),
            make_sample(-huge, "negative huge numerator"),
            make_sample(huge_peer, "huge denominator"),
            make_sample(-huge_peer, "negative huge denominator"),
            make_sample(tiny, "tiny numerator"),
            make_sample(-tiny, "negative tiny numerator"),
            make_sample(tiny_peer, "tiny denominator"),
            make_sample(-tiny_peer, "negative tiny denominator")
        };
        return out;
    }

    [[nodiscard]] inline domain positive(domain input, double minimum = 0.0)
    {
        for (sample& value : input.values)
        {
            value.limb[0] = std::max(minimum, std::abs(value.limb[0]));
            value.limb[1] = std::abs(value.limb[1]);
            value.limb[2] = std::abs(value.limb[2]);
            value.limb[3] = std::abs(value.limb[3]);
        }
        return input;
    }

    [[nodiscard]] inline domain bounded(domain input, double low, double high)
    {
        const double width = high - low;
        for (sample& value : input.values)
        {
            const double normalized = std::fmod(std::abs(value.limb[0]), 1.0);
            value = make_sample(low + normalized * width, std::move(value.label));
        }
        return input;
    }
}

#endif
