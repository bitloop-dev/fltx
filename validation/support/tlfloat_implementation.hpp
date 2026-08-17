#pragma once

#include "implementations.hpp"

#ifndef FLTX_METRICS_HAS_TLFLOAT
#define FLTX_METRICS_HAS_TLFLOAT 0
#endif

#if FLTX_METRICS_HAS_TLFLOAT
#include "tlfloat_support.hpp"
#endif

#include <array>
#include <bit>
#include <cstdint>

namespace fltx::tests::implementations
{
#if FLTX_METRICS_HAS_TLFLOAT
    using tl_f128 = tlfloat::Quad;
    using tl_f256 = tlfloat::Octuple;
#else
    using tl_f128 = double;
    using tl_f256 = double;
#endif

    inline constexpr identity tlfloat_f128{"tlfloat", "tlquad", "TLFloat Quad", "TLFloat",
                                           FLTX_METRICS_HAS_TLFLOAT != 0};
    inline constexpr identity tlfloat_f256{"tlfloat", "tloct", "TLFloat Octuple", "TLFloat",
                                           FLTX_METRICS_HAS_TLFLOAT != 0};

    template<class Float>
    inline constexpr identity tlfloat_identity =
        std::is_same_v<Float, bl::f128> ? tlfloat_f128 : tlfloat_f256;

#if FLTX_METRICS_HAS_TLFLOAT
    template<std::size_t N>
    [[nodiscard]] mpfr::real tlfloat_to_real(const std::array<std::uint64_t, N>& words,
                                             unsigned exponent_bits, unsigned fraction_bits)
    {
        const unsigned top_fraction_bits = fraction_bits - static_cast<unsigned>((N - 1) * 64);
        const std::uint64_t exponent_mask = (std::uint64_t{1} << exponent_bits) - 1;
        const std::uint64_t fraction_mask = (std::uint64_t{1} << top_fraction_bits) - 1;
        const std::uint64_t exponent = (words.back() >> top_fraction_bits) & exponent_mask;
        const bool negative = ((words.back() >> (top_fraction_bits + exponent_bits)) & 1) != 0;

        bool zero = (words.back() & fraction_mask) == 0;
        for (std::size_t i = 0; i + 1 < N; ++i)
            zero = zero && words[i] == 0;
        if (exponent == exponent_mask)
        {
            if (!zero)
                return mpfr::real{"nan"};
            return mpfr::real{negative ? "-inf" : "inf"};
        }
        if (exponent == 0 && zero)
            return mpfr::real{negative ? -0.0 : 0.0};

        const long long bias = (std::int64_t{1} << (exponent_bits - 1)) - 1;
        const long long binary_exponent =
            exponent == 0 ? 1 - bias : static_cast<long long>(exponent) - bias;

        mpfr::real significand{words.back() & fraction_mask};
        for (std::size_t i = N - 1; i-- > 0;)
        {
            using boost::multiprecision::ldexp;
            significand = ldexp(significand, 64) + mpfr::real{words[i]};
        }
        if (exponent != 0)
        {
            using boost::multiprecision::ldexp;
            significand += ldexp(mpfr::real{1}, static_cast<int>(fraction_bits));
        }
        using boost::multiprecision::ldexp;
        const mpfr::real out =
            ldexp(significand, static_cast<int>(binary_exponent - fraction_bits));
        return negative ? -out : out;
    }

    template<std::size_t N>
    [[nodiscard]] bool tlfloat_is_finite(const std::array<std::uint64_t, N>& words,
                                         unsigned exponent_bits, unsigned fraction_bits) noexcept
    {
        const unsigned top_fraction_bits = fraction_bits - static_cast<unsigned>((N - 1) * 64);
        const std::uint64_t exponent_mask = (std::uint64_t{1} << exponent_bits) - 1;
        return ((words.back() >> top_fraction_bits) & exponent_mask) != exponent_mask;
    }

    template<std::size_t N>
    [[nodiscard]] bool tlfloat_sign_bit(const std::array<std::uint64_t, N>& words,
                                        unsigned exponent_bits, unsigned fraction_bits) noexcept
    {
        const unsigned top_fraction_bits = fraction_bits - static_cast<unsigned>((N - 1) * 64);
        return ((words.back() >> (top_fraction_bits + exponent_bits)) & 1) != 0;
    }

    [[nodiscard]] inline timing_components observe(const tlfloat::Quad& value) noexcept
    {
        const auto words = std::bit_cast<std::array<std::uint64_t, 2>>(value);
        return {static_cast<double>(words[0]), static_cast<double>(words[1]), 0.0, 0.0};
    }

    [[nodiscard]] inline timing_components observe(const tlfloat::Octuple& value) noexcept
    {
        const auto words = std::bit_cast<std::array<std::uint64_t, 4>>(value);
        return {static_cast<double>(words[0]), static_cast<double>(words[1]),
                static_cast<double>(words[2]), static_cast<double>(words[3])};
    }

    template<> struct value_traits<tlfloat::Quad>
    {
        static constexpr double nominal_bits = 113.0;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            using boost::multiprecision::ldexp;
            static const mpfr::real value =
                ldexp(mpfr::real{1}, -16494);
            return value;
        }

        [[nodiscard]] static tlfloat::Quad from_sample(const sample& value)
        {
            if (!native_fp::is_finite(value.limb[0]))
            {
                if (native_fp::is_nan(value.limb[0]))
                    return tlfloat::Quad{"nan"};
                return tlfloat::Quad{
                    native_fp::sign_bit(value.limb[0]) ? "-inf" : "inf"};
            }
            if (value.limb[0] == 0.0 && value.limb[1] == 0.0)
            {
                return tlfloat::Quad{native_fp::sign_bit(value.limb[0]) ? "-0" : "0"};
            }
            return tlfloat::Quad{value.limb[0]} + tlfloat::Quad{value.limb[1]};
        }

        [[nodiscard]] static mpfr::real to_real(const tlfloat::Quad& value)
        {
            return tlfloat_to_real(std::bit_cast<std::array<std::uint64_t, 2>>(value), 15, 112);
        }

        [[nodiscard]] static bool is_finite(const tlfloat::Quad& value)
        {
            return tlfloat_is_finite(std::bit_cast<std::array<std::uint64_t, 2>>(value), 15, 112);
        }

        [[nodiscard]] static bool sign_bit(const tlfloat::Quad& value)
        {
            return tlfloat_sign_bit(std::bit_cast<std::array<std::uint64_t, 2>>(value), 15, 112);
        }
    };

    template<> struct value_traits<tlfloat::Octuple>
    {
        static constexpr double nominal_bits = 237.0;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            using boost::multiprecision::ldexp;
            static const mpfr::real value =
                ldexp(mpfr::real{1}, -262378);
            return value;
        }

        [[nodiscard]] static tlfloat::Octuple from_sample(const sample& value)
        {
            if (!native_fp::is_finite(value.limb[0]))
            {
                if (native_fp::is_nan(value.limb[0]))
                    return tlfloat::Octuple{"nan"};
                return tlfloat::Octuple{
                    native_fp::sign_bit(value.limb[0]) ? "-inf" : "inf"};
            }
            if (value.limb[0] == 0.0 && value.limb[1] == 0.0 && value.limb[2] == 0.0 &&
                value.limb[3] == 0.0)
            {
                return tlfloat::Octuple{native_fp::sign_bit(value.limb[0]) ? "-0" : "0"};
            }
            return tlfloat::Octuple{value.limb[0]} + tlfloat::Octuple{value.limb[1]} +
                   tlfloat::Octuple{value.limb[2]} + tlfloat::Octuple{value.limb[3]};
        }

        [[nodiscard]] static mpfr::real to_real(const tlfloat::Octuple& value)
        {
            return tlfloat_to_real(std::bit_cast<std::array<std::uint64_t, 4>>(value), 19, 236);
        }

        [[nodiscard]] static bool is_finite(const tlfloat::Octuple& value)
        {
            return tlfloat_is_finite(std::bit_cast<std::array<std::uint64_t, 4>>(value), 19, 236);
        }

        [[nodiscard]] static bool sign_bit(const tlfloat::Octuple& value)
        {
            return tlfloat_sign_bit(std::bit_cast<std::array<std::uint64_t, 4>>(value), 19, 236);
        }
    };
#endif
} // namespace fltx::tests::implementations
