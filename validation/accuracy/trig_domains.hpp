#ifndef FLTX_TESTS_ACCURACY_TRIG_DOMAINS_INCLUDED
#define FLTX_TESTS_ACCURACY_TRIG_DOMAINS_INCLUDED

#include "../support/domains.hpp"
#include "../support/mpfr.hpp"

#include <boost/math/constants/constants.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace fltx::tests::accuracy::trig_domains
{
    template<class Float>
    [[nodiscard]] domains::domain quadrant_boundaries(
        std::size_t count,
        int max_multiplier_exponent = 159,
        std::uint64_t seed = default_seed ^ 0x37u)
    {
        using mpfr::real;
        using boost::multiprecision::abs;
        using boost::multiprecision::floor;
        using boost::multiprecision::ldexp;

        static const real half_pi =
            boost::math::constants::pi<real>() / 2;
        static constexpr std::array<int, 3> f128_offset_bits{
            64, 82, 100
        };
        static constexpr std::array<int, 4> f256_offset_bits{
            64, 118, 172, 204
        };

        domains::domain out{ "quadrant_boundaries", seed, {} };
        out.values.reserve(count + 219);

        const auto perturbation = [&](const real& anchor, int offset_bits) {
            const real scale = anchor == 0 ? half_pi : abs(anchor);
            return ldexp(scale, -offset_bits);
        };
        const auto converted = [&](const real& value, std::string label) {
            return mpfr::sample_from_real<Float>(value, std::move(label));
        };
        const auto append_neighborhood = [&](
            const real& multiplier,
            const std::string& label)
        {
            const real anchor = multiplier * half_pi;
            constexpr int deepest_offset =
                std::is_same_v<Float, bl::f128>
                ? f128_offset_bits.back()
                : f256_offset_bits.back();
            const real delta = perturbation(anchor, deepest_offset);
            sample below = converted(anchor - delta, "below " + label);
            sample nearest = converted(anchor, "nearest " + label);
            sample above = converted(anchor + delta, "above " + label);
            if (!(
                mpfr::input_real<Float>(below) <
                mpfr::input_real<Float>(nearest) &&
                mpfr::input_real<Float>(nearest) <
                mpfr::input_real<Float>(above)
            ))
            {
                throw std::logic_error(
                    "quadrant-boundary neighborhood collapsed");
            }
            out.values.push_back(std::move(below));
            out.values.push_back(std::move(nearest));
            out.values.push_back(std::move(above));
        };

        for (int multiplier = -16; multiplier <= 16; ++multiplier)
        {
            append_neighborhood(
                real{ multiplier },
                std::to_string(multiplier) + " * pi/2");
        }

        static constexpr std::array<int, 5> large_exponents{
            20, 53, 56, 100, 159
        };
        static constexpr std::array<int, 4> multiplier_offsets{
            -3, -1, 1, 3
        };
        for (const int exponent : large_exponents)
        {
            if (exponent > max_multiplier_exponent)
                continue;
            for (const int offset : multiplier_offsets)
            {
                const real multiplier =
                    ldexp(real{ 1 }, exponent) + offset;
                const std::string label =
                    "(2^" + std::to_string(exponent) +
                    (offset < 0 ? " - " : " + ") +
                    std::to_string(offset < 0 ? -offset : offset) +
                    ") * pi/2";
                append_neighborhood(multiplier, label);
                append_neighborhood(-multiplier, "-" + label);
            }
        }

        random_bits rng(seed);
        for (std::size_t index = 0; index < count; ++index)
        {
            const int exponent = static_cast<int>(
                rng.next() %
                static_cast<std::uint64_t>(max_multiplier_exponent + 1));
            const int coefficient_bits = std::min(exponent + 1, 32);
            const std::uint64_t mask =
                (std::uint64_t{ 1 } << coefficient_bits) - 1;
            const std::uint64_t coefficient =
                (rng.next() & mask) |
                (std::uint64_t{ 1 } << (coefficient_bits - 1));
            const int shift = exponent - (coefficient_bits - 1);
            real multiplier = ldexp(real{ coefficient }, shift);

            // Make the quotient class explicit. This exercises all four
            // quadrants even when the large part is highly divisible by four.
            const unsigned quadrant = static_cast<unsigned>(index % 4);
            multiplier = floor(multiplier / 4) * 4 + quadrant;
            if (multiplier == 0)
                multiplier = 4 + quadrant;
            if (((index / 4) & 1u) != 0)
                multiplier = -multiplier;

            int offset_bits;
            if constexpr (std::is_same_v<Float, bl::f128>)
            {
                offset_bits =
                    f128_offset_bits[index % f128_offset_bits.size()];
            }
            else
            {
                offset_bits =
                    f256_offset_bits[index % f256_offset_bits.size()];
            }

            const real anchor = multiplier * half_pi;
            const int relation = static_cast<int>(index % 3) - 1;
            const real value = (
                relation == 0
                ? anchor
                : anchor + relation * perturbation(anchor, offset_bits)
            );
            const std::string relation_label =
                relation < 0 ? "below " :
                relation > 0 ? "above " : "nearest ";
            out.values.push_back(converted(
                value,
                relation_label +
                "random k*pi/2, exponent " +
                std::to_string(exponent) +
                ", quadrant " +
                std::to_string(quadrant) +
                ", offset bits " +
                std::to_string(offset_bits)));
        }
        return out;
    }
}

#endif
