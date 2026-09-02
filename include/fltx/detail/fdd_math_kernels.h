/**
 * fltx/detail/fdd_math_kernels.h - dd math kernels and helper algorithms.
 *
 * Low-level dd helper logic used by grouped math implementations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_DETAIL_MATH_KERNELS_INCLUDED
#define FDD_DETAIL_MATH_KERNELS_INCLUDED
#include "fltx/detail/common_decimal.h"
#include "fltx/detail/fdd_declarations.h"

namespace bl {

namespace detail::_dd // primitives and kernels
{
    // Algorithmic convergence tolerance. This intentionally remains the
    // former 2^-106 value and is independent of the public nominal epsilon.
    inline constexpr fdd_s convergence_epsilon{ 0x1p-106, 0.0 };

    using detail::fp::signbit;
    using detail::fp::fabs;
    using detail::fp::floor;
    using detail::fp::ceil;
    using detail::fp::double_integer_is_odd;
    using detail::fp::fmod;
    using detail::fp::sqrt_seed;
    using detail::fp::round_nearest_even_value;
    using detail::fp::two_diff_precise;
    using detail::fp::abs_double_is_power_of_two;
    using detail::fp::frexp_exponent_limb;
    using detail::fp::ldexp_limb;

    [[nodiscard]] BL_FORCE_INLINE constexpr bool is_subnormal_limb(double value) noexcept
    {
        constexpr std::uint64_t magnitude_mask = 0x7fffffffffffffffull;
        constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
        const std::uint64_t magnitude =
            std::bit_cast<std::uint64_t>(value) & magnitude_mask;
        return magnitude != 0 && (magnitude & exponent_mask) == 0;
    }

    BL_FORCE_INLINE constexpr int ilogb_finite_fast(const fdd_s& x) noexcept
    {
        const double hi = x.hi != 0.0 ? x.hi : x.lo;
        const double abs_hi = absd(hi);
        int exponent = detail::fp::frexp_exponent(abs_hi) - 1;

        if (x.hi != 0.0 && x.lo != 0.0 && signbit(x.hi) != signbit(x.lo) &&
            abs_double_is_power_of_two(abs_hi))
        {
            --exponent;
        }

        return exponent;
    }

    // scaling helpers
    BL_FORCE_INLINE constexpr bool ldexp_normal_limb(double value, int exponent, double& out) noexcept
    {
        if (exponent == 0)
        {
            out = value;
            return true;
        }

        constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
        constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
        constexpr std::uint64_t sign_mask     = 0x8000000000000000ull;

        const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
        if ((bits & ~sign_mask) == 0u)
        {
            out = value;
            return true;
        }

        const std::uint32_t exponent_bits =
            static_cast<std::uint32_t>((bits & exponent_mask) >> 52);
        if (exponent_bits == 0u || exponent_bits == 0x7ffu)
            return false;

        const int scaled_exponent = static_cast<int>(exponent_bits) + exponent;
        if (scaled_exponent <= 0 || scaled_exponent >= 0x7ff)
            return false;

        out = std::bit_cast<double>(
            (bits & (sign_mask | fraction_mask)) |
            (static_cast<std::uint64_t>(scaled_exponent) << 52));
        return true;
    }

    BL_FORCE_INLINE constexpr bool ldexp_fast_normal(const fdd_s& value, int exponent, fdd_s& out) noexcept
    {
        if (exponent == 0)
        {
            out = value;
            return true;
        }

        double hi{};
        double lo{};
        if (!ldexp_normal_limb(value.hi, exponent, hi) ||
            !ldexp_normal_limb(value.lo, exponent, lo))
        {
            return false;
        }

        out = fdd_s{ hi, lo };
        return true;
    }

    BL_FORCE_INLINE constexpr fdd_s ldexp_terms(const fdd_s& value, int exponent) noexcept
    {
#if BL_FP_BARRIER_ACTIVE
        double hi = detail::fp::ldexp(value.hi, exponent);  BL_FP_BARRIER(hi);
        double lo = detail::fp::ldexp(value.lo, exponent);  BL_FP_BARRIER(lo);
        if (is_subnormal_limb(lo))
            return { hi, lo };
        return renorm(hi, lo);
#else
        return renorm(
            ldexp_limb(value.hi, exponent),
            ldexp_limb(value.lo, exponent));
#endif
    }

    BL_FORCE_INLINE constexpr fdd_s _ldexp(const fdd_s& x, int e)
    {
        fdd_s fast{};
        if (ldexp_fast_normal(x, e, fast))
            return fast;

        if (bl::detail::is_constant_evaluated())
        {
            return renorm(
                detail::fp::ldexp(x.hi, e),
                detail::fp::ldexp(x.lo, e)
            );
        }

#if BL_FP_BARRIER_ACTIVE
        double hi = detail::fp::ldexp(x.hi, e);  BL_FP_BARRIER(hi);
        double lo = detail::fp::ldexp(x.lo, e);  BL_FP_BARRIER(lo);
        if (is_subnormal_limb(lo))
            return { hi, lo };
        return renorm(hi, lo);
#else
        return renorm(
            std::ldexp(x.hi, e),
            std::ldexp(x.lo, e)
        );
#endif
    }

    struct fmod_u128
    {
        std::uint64_t lo = 0;
        std::uint64_t hi = 0;
    };

    struct exact_dyadic_fmod
    {
        bool neg = false;
        int exp2 = 0;
        fmod_u128 mant{};
    };

    struct dd_decimal_traits
    {
        using value_type = fdd_s;
        static constexpr int limb_count = 2;
        static constexpr int significand_bits = 106;
        static constexpr int conversion_significand_bits = 53 * (limb_count + 1);
        static constexpr int max_binary_exponent = 1023;
        static constexpr int min_binary_exponent = -1074;

        static constexpr double limb(const value_type& x, int index) noexcept
        {
            return index == 0 ? x.hi : x.lo;
        }

        static constexpr value_type zero(bool neg = false) noexcept
        {
            return neg ? value_type{ -0.0, 0.0 } : value_type{ 0.0, 0.0 };
        }

        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

        static constexpr value_type pack_from_significand(
            const detail::exact_decimal::biguint& q,
            int e2,
            bool neg) noexcept
        {
            const int bits = q.bit_length();
            if (bits <= 0)
                return zero(neg);

            if (bits <= 53)
            {
                const std::uint64_t c0 = q.get_bits(0, bits);
                value_type out{ c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - (bits - 1)) : 0.0, 0.0 };
                return neg ? -out : out;
            }

            if (bits <= significand_bits)
            {
                const int lo_width = bits - 53;
                const std::uint64_t c1 = q.get_bits(0, lo_width);
                const std::uint64_t c0 = q.get_bits(lo_width, 53);

                const double hi = c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - 52) : 0.0;
                const double lo = c1 ? detail::fp::ldexp(static_cast<double>(c1), e2 - (bits - 1)) : 0.0;
                value_type out = renorm(hi, lo);
                return neg ? -out : out;
            }

            const int lo_width = bits - significand_bits;
            const std::uint64_t c2 = q.get_bits(0, lo_width);
            const std::uint64_t c1 = q.get_bits(lo_width, 53);
            const std::uint64_t c0 = q.get_bits(bits - 53, 53);

            const double hi = c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - 52) : 0.0;
            const double mid = c1 ? detail::fp::ldexp(static_cast<double>(c1), e2 - 105) : 0.0;
            const double lo = c2 ? detail::fp::ldexp(static_cast<double>(c2), e2 - (bits - 1)) : 0.0;

            const fdd_s tail = renorm(mid, lo);
            fdd_s out = renorm(hi, tail.hi);
            out = renorm(out.hi, out.lo + tail.lo);
            return neg ? -out : out;
        }
    };

    // exact integer helpers
    BL_FORCE_INLINE constexpr bool fmod_u128_is_zero(const fmod_u128& value)
    {
        return value.lo == 0 && value.hi == 0;
    }

    BL_FORCE_INLINE constexpr bool fmod_u128_is_odd(const fmod_u128& value)
    {
        return (value.lo & 1u) != 0;
    }

    BL_FORCE_INLINE constexpr int fmod_u128_compare(const fmod_u128& a, const fmod_u128& b)
    {
        if (a.hi < b.hi) return -1;
        if (a.hi > b.hi) return 1;
        if (a.lo < b.lo) return -1;
        if (a.lo > b.lo) return 1;
        return 0;
    }

    BL_FORCE_INLINE constexpr int fmod_u128_bit_length(const fmod_u128& value)
    {
        if (value.hi != 0)
            return 128 - static_cast<int>(std::countl_zero(value.hi));
        if (value.lo != 0)
            return 64 - static_cast<int>(std::countl_zero(value.lo));
        return 0;
    }

    BL_FORCE_INLINE constexpr int fmod_u128_trailing_zero_bits(const fmod_u128& value)
    {
        if (value.lo != 0)
            return static_cast<int>(std::countr_zero(value.lo));
        if (value.hi != 0)
            return 64 + static_cast<int>(std::countr_zero(value.hi));
        return 0;
    }

    BL_FORCE_INLINE constexpr bool fmod_u128_get_bit(const fmod_u128& value, int index)
    {
        if (index < 0 || index >= 128)
            return false;
        if (index < 64)
            return ((value.lo >> index) & 1u) != 0;
        return ((value.hi >> (index - 64)) & 1u) != 0;
    }

    BL_FORCE_INLINE constexpr std::uint64_t fmod_u128_get_bits(const fmod_u128& value, int start, int count)
    {
        std::uint64_t out = 0;
        for (int i = 0; i < count; ++i)
        {
            if (fmod_u128_get_bit(value, start + i))
                out |= (std::uint64_t{ 1 } << i);
        }
        return out;
    }

    BL_FORCE_INLINE constexpr bool fmod_u128_any_low_bits_set(const fmod_u128& value, int count)
    {
        if (count <= 0)
            return false;

        if (count >= 64)
        {
            if (value.lo != 0)
                return true;
            count -= 64;
            if (count >= 64)
                return value.hi != 0;
            return (value.hi & ((std::uint64_t{ 1 } << count) - 1u)) != 0;
        }

        return (value.lo & ((std::uint64_t{ 1 } << count) - 1u)) != 0;
    }

    BL_FORCE_INLINE constexpr void fmod_u128_add_inplace(fmod_u128& a, const fmod_u128& b)
    {
        const std::uint64_t old_lo = a.lo;
        a.lo += b.lo;
        a.hi += b.hi + (a.lo < old_lo ? 1u : 0u);
    }

    BL_FORCE_INLINE constexpr void fmod_u128_add_small(fmod_u128& a, std::uint32_t value)
    {
        const std::uint64_t old_lo = a.lo;
        a.lo += value;
        if (a.lo < old_lo)
            ++a.hi;
    }

    BL_FORCE_INLINE constexpr void fmod_u128_sub_inplace(fmod_u128& a, const fmod_u128& b)
    {
        const std::uint64_t borrow = (a.lo < b.lo) ? 1u : 0u;
        a.lo -= b.lo;
        a.hi -= b.hi + borrow;
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_shl_bits(fmod_u128 value, int bits)
    {
        if (bits <= 0 || fmod_u128_is_zero(value))
            return value;
        if (bits >= 128)
            return {};
        if (bits >= 64)
            return { 0, value.lo << (bits - 64) };

        return {
            value.lo << bits,
            (value.hi << bits) | (value.lo >> (64 - bits))
        };
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_shr_bits(fmod_u128 value, int bits)
    {
        if (bits <= 0 || fmod_u128_is_zero(value))
            return value;
        if (bits >= 128)
            return {};
        if (bits >= 64)
            return { value.hi >> (bits - 64), 0 };

        return {
            (value.lo >> bits) | (value.hi << (64 - bits)),
            value.hi >> bits
        };
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_shl1(fmod_u128 value)
    {
        return { value.lo << 1, (value.hi << 1) | (value.lo >> 63) };
    }

    BL_FORCE_INLINE constexpr bool fmod_u128_shift_exceeds_capacity(const fmod_u128& value, int bits)
    {
        return bits > 0 && !fmod_u128_is_zero(value) &&
            fmod_u128_bit_length(value) + bits > 128;
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_mod_shift_subtract(fmod_u128 numerator, const fmod_u128& denominator)
    {
        if (fmod_u128_is_zero(denominator))
            return {};
        if (fmod_u128_compare(numerator, denominator) < 0)
            return numerator;

        int shift = fmod_u128_bit_length(numerator) - fmod_u128_bit_length(denominator);
        fmod_u128 shifted = fmod_u128_shl_bits(denominator, shift);

        for (; shift >= 0; --shift)
        {
            if (fmod_u128_compare(numerator, shifted) >= 0)
                fmod_u128_sub_inplace(numerator, shifted);
            shifted = fmod_u128_shr_bits(shifted, 1);
        }

        return numerator;
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_mod_shift_subtract_with_quotient_mod(
        fmod_u128 numerator,
        const fmod_u128& denominator,
        std::uint64_t& quotient_mod)
    {
        quotient_mod = 0;
        if (fmod_u128_is_zero(denominator))
            return {};
        if (fmod_u128_compare(numerator, denominator) < 0)
            return numerator;

        int shift = fmod_u128_bit_length(numerator) - fmod_u128_bit_length(denominator);
        fmod_u128 shifted = fmod_u128_shl_bits(denominator, shift);

        for (; shift >= 0; --shift)
        {
            if (fmod_u128_compare(numerator, shifted) >= 0)
            {
                fmod_u128_sub_inplace(numerator, shifted);
                if (shift < 31)
                    quotient_mod |= std::uint64_t{ 1 } << shift;
            }
            shifted = fmod_u128_shr_bits(shifted, 1);
        }

        return numerator;
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_double_mod(fmod_u128 value, const fmod_u128& modulus)
    {
        const bool overflow = (value.hi >> 63) != 0u;
        value = fmod_u128_shl1(value);
        if (overflow || fmod_u128_compare(value, modulus) >= 0)
            fmod_u128_sub_inplace(value, modulus);
        return value;
    }

    BL_FORCE_INLINE constexpr fmod_u128 fmod_u128_double_mod_with_quotient_bit(
        fmod_u128 value,
        const fmod_u128& modulus,
        std::uint64_t& bit)
    {
        const bool overflow = (value.hi >> 63) != 0u;
        value = fmod_u128_shl1(value);
        if (overflow || fmod_u128_compare(value, modulus) >= 0)
        {
            fmod_u128_sub_inplace(value, modulus);
            bit = 1;
        }
        else
        {
            bit = 0;
        }
        return value;
    }

    // exact fmod conversion
    BL_FORCE_INLINE constexpr exact_dyadic_fmod exact_from_double_fmod(double value)
    {
        exact_dyadic_fmod out;
        if (value == 0.0)
            return out;

        int exponent = 0;
        bool neg = false;
        const std::uint64_t mantissa = detail::exact_decimal::decompose_double_mantissa(value, exponent, neg);
        if (mantissa == 0)
            return out;

        out.neg = neg;
        out.exp2 = exponent;
        out.mant.lo = mantissa;
        return out;
    }

    BL_FORCE_INLINE constexpr void normalize_exact_dyadic_fmod(exact_dyadic_fmod& value)
    {
        if (fmod_u128_is_zero(value.mant))
        {
            value.neg = false;
            value.exp2 = 0;
            return;
        }

        const int tz = fmod_u128_trailing_zero_bits(value.mant);
        if (tz != 0)
        {
            value.mant = fmod_u128_shr_bits(value.mant, tz);
            value.exp2 += tz;
        }
    }

    BL_FORCE_INLINE constexpr exact_dyadic_fmod exact_from_dd_fmod(const fdd_s& value)
    {
        exact_dyadic_fmod hi = exact_from_double_fmod(value.hi);
        exact_dyadic_fmod lo = exact_from_double_fmod(value.lo);

        if (fmod_u128_is_zero(hi.mant))
            return lo;
        if (fmod_u128_is_zero(lo.mant))
            return hi;

        const int common_exp = (hi.exp2 < lo.exp2) ? hi.exp2 : lo.exp2;
        const fmod_u128 hi_scaled = fmod_u128_shl_bits(hi.mant, hi.exp2 - common_exp);
        const fmod_u128 lo_scaled = fmod_u128_shl_bits(lo.mant, lo.exp2 - common_exp);

        exact_dyadic_fmod out;
        out.exp2 = common_exp;

        if (hi.neg == lo.neg)
        {
            out.neg = hi.neg;
            out.mant = hi_scaled;
            fmod_u128_add_inplace(out.mant, lo_scaled);
        }
        else
        {
            const int cmp = fmod_u128_compare(hi_scaled, lo_scaled);
            if (cmp >= 0)
            {
                out.neg = hi.neg;
                out.mant = hi_scaled;
                fmod_u128_sub_inplace(out.mant, lo_scaled);
            }
            else
            {
                out.neg = lo.neg;
                out.mant = lo_scaled;
                fmod_u128_sub_inplace(out.mant, hi_scaled);
            }
        }

        normalize_exact_dyadic_fmod(out);
        return out;
    }

    // fmod kernels
    BL_FORCE_INLINE constexpr int fmod_append_expansion_term(double* terms, int count, double value) noexcept
    {
        if (value != 0.0)
            terms[count++] = value;
        return count;
    }

    BL_FORCE_INLINE constexpr int fmod_compress_expansion_zeroelim(int elen, const double* e, double* h) noexcept
    {
        if (elen <= 0)
            return 0;

        double g[16]{};
        double q = e[elen - 1];
        for (int i = elen - 2; i >= 0; --i)
        {
            double q_new{};
            double low{};
            two_sum_precise(q, e[i], q_new, low);
            q = q_new;
            g[i + 1] = low;
        }
        g[0] = q;

        int hindex = 0;
        q = g[0];
        for (int i = 1; i < elen; ++i)
        {
            double q_new{};
            double low{};
            two_sum_precise(q, g[i], q_new, low);
            if (low != 0.0)
                h[hindex++] = low;
            q = q_new;
        }

        if (q != 0.0 || hindex == 0)
            h[hindex++] = q;
        return hindex;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s fmod_add_scalar_precise(fdd_s value, double scalar) noexcept
    {
        double s0{};
        double e0{};
        two_sum_precise(value.hi, scalar, s0, e0);

        double s1{};
        double e1{};
        two_sum_precise(value.lo, e0, s1, e1);

        return renorm(s0, s1 + e1);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s fmod_from_expansion_fast(const double* terms, int count) noexcept
    {
        if (count <= 0)
            return {};

        double compressed[16]{};
        const int compressed_count = fmod_compress_expansion_zeroelim(count, terms, compressed);

        fdd_s sum{};
        for (int i = 0; i < compressed_count; ++i)
            sum = fmod_add_scalar_precise(sum, compressed[i]);

        return sum;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s fmod_from_expansion_direct(const double* terms, int count) noexcept
    {
        fdd_s sum{};
        for (int i = 0; i < count; ++i)
            sum = fmod_add_scalar_precise(sum, terms[i]);
        return sum;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s fmod_sub_mul_scalar_compact(
        const fdd_s& r,
        const fdd_s& b,
        double q) noexcept
    {
        double p0{};
        double e0{};
        two_prod_precise(b.hi, q, p0, e0);

        double p1{};
        double e1{};
        two_prod_precise(b.lo, q, p1, e1);

        double s{};
        double t{};
        two_diff_precise(r.hi, p0, s, t);

        fdd_s out{ s, t };
        out = fmod_add_scalar_precise(out, r.lo);
        out = fmod_add_scalar_precise(out, -e0);
        out = fmod_add_scalar_precise(out, -p1);
        out = fmod_add_scalar_precise(out, -e1);
        return out;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s fmod_sub_mul_scalar_expansion(
        const fdd_s& r,
        const fdd_s& b,
        double q) noexcept
    {
        double p0{};
        double e0{};
        two_prod_precise(b.hi, q, p0, e0);

        double p1{};
        double e1{};
        two_prod_precise(b.lo, q, p1, e1);

        double s{};
        double t{};
        two_diff_precise(r.hi, p0, s, t);

        double terms[6]{};
        int count = 0;
        count = fmod_append_expansion_term(terms, count, -e1);
        count = fmod_append_expansion_term(terms, count, -e0);
        count = fmod_append_expansion_term(terms, count, -p1);
        count = fmod_append_expansion_term(terms, count, r.lo);
        count = fmod_append_expansion_term(terms, count, t);
        count = fmod_append_expansion_term(terms, count, s);

        if (q < 0x1p47)
            return fmod_from_expansion_direct(terms, count);

        return fmod_from_expansion_fast(terms, count);
    }

    BL_FORCE_INLINE constexpr bool fmod_normalize_remainder(fdd_s& r, const fdd_s& modulus) noexcept
    {
        for (int i = 0; i < 4; ++i)
        {
            if (r < 0.0)
            {
                r = add_finite_inline(r, modulus);
                continue;
            }

            if (r >= modulus)
            {
                r = sub_finite_inline(r, modulus);
                continue;
            }

            return true;
        }

        return r >= 0.0 && r < modulus;
    }

    BL_FORCE_INLINE constexpr bool fmod_normalize_remainder_with_quotient(
        fdd_s& r,
        const fdd_s& modulus,
        std::uint64_t& quotient) noexcept
    {
        for (int i = 0; i < 4; ++i)
        {
            if (r < 0.0)
            {
                r = add_finite_inline(r, modulus);
                if (quotient == 0u)
                    return false;
                --quotient;
                continue;
            }

            if (r >= modulus)
            {
                r = sub_finite_inline(r, modulus);
                ++quotient;
                continue;
            }

            return true;
        }

        return r >= 0.0 && r < modulus;
    }

    BL_FORCE_INLINE constexpr int fmod_compare_remainder_to_half(const fdd_s& r_abs, const fdd_s& half) noexcept
    {
        const fdd_s delta = sub_finite_inline(r_abs, half);
        if (iszero(delta))
            return 0;
        return delta < 0.0 ? -1 : 1;
    }

    BL_FORCE_INLINE constexpr bool fmod_fast_small_quotient_abs_with_quotient(
        const fdd_s& ax,
        const fdd_s& ay,
        fdd_s& out,
        std::uint64_t& quotient,
        bool allow_compact_residual = false) noexcept
    {
        if (!(ay.hi > 0.0) || detail::fp::isinf_or_nan(ay.hi) || !(ax >= ay))
            return false;

        const double q = detail::fp::trunc(ax.hi / ay.hi);
        if (!(q > 0.0) || q >= 0x1p53)
            return false;

        quotient = static_cast<std::uint64_t>(q);
        if (q >= 0x1p50)
            return false;

        if (q < 0x1p48 && abs_double_is_power_of_two(q))
        {
            std::uint64_t cheap_quotient = quotient;
            fdd_s r = sub_finite_inline(ax, mul_pwr2_inline(ay, q));
            if (fmod_normalize_remainder_with_quotient(r, ay, cheap_quotient))
            {
                const fdd_s edge_slack = mul_double_product_inline(ay, 0x1p-44);
                const fdd_s half = mul_double_product_inline(ay, 0.5);
                const fdd_s distance_to_half = mag(sub_finite_inline(r, half));

                if (r > edge_slack &&
                    sub_finite_inline(ay, r) > edge_slack &&
                    distance_to_half > edge_slack)
                {
                    out = r;
                    quotient = cheap_quotient;
                    return true;
                }
            }
        }

        if (allow_compact_residual)
        {
            std::uint64_t compact_quotient = quotient;
            fdd_s r = fmod_sub_mul_scalar_compact(ax, ay, q);
            if (fmod_normalize_remainder_with_quotient(r, ay, compact_quotient))
            {
                const fdd_s edge_slack = mul_double_product_inline(ay, 0x1p-80);
                const fdd_s half = mul_double_product_inline(ay, 0.5);
                if (r <= edge_slack ||
                    sub_finite_inline(ay, r) <= edge_slack ||
                    mag(sub_finite_inline(r, half)) <= edge_slack)
                {
                    return false;
                }

                out = r;
                quotient = compact_quotient;
                return true;
            }
        }

        fdd_s r = fmod_sub_mul_scalar_expansion(ax, ay, q);
        if (!fmod_normalize_remainder_with_quotient(r, ay, quotient))
            return false;

        const fdd_s edge_slack = mul_double_product_inline(ay, 0x1p-80);
        if (r <= edge_slack || sub_finite_inline(ay, r) <= edge_slack)
            return false;

        if (allow_compact_residual)
        {
            const fdd_s half = mul_double_product_inline(ay, 0.5);
            if (mag(sub_finite_inline(r, half)) <= edge_slack)
                return false;
        }

        out = r;
        return true;
    }

    BL_FORCE_INLINE constexpr bool fmod_fast_small_quotient_abs(
        const fdd_s& ax,
        const fdd_s& ay,
        fdd_s& out) noexcept
    {
        std::uint64_t quotient{};
        return fmod_fast_small_quotient_abs_with_quotient(ax, ay, out, quotient);
    }

    BL_FORCE_INLINE constexpr fdd_s exact_dyadic_to_dd_fmod(const fmod_u128& coeff, int exp2, bool neg)
    {
        if (fmod_u128_is_zero(coeff))
            return neg ? fdd_s{ -0.0, 0.0 } : fdd_s{ 0.0, 0.0 };

        int ratio_exp = fmod_u128_bit_length(coeff) - 1;
        fmod_u128 q = coeff;

        if (ratio_exp > 105)
        {
            const int right_shift = ratio_exp - 105;
            const bool round_bit = fmod_u128_get_bit(q, right_shift - 1);
            const bool sticky    = fmod_u128_any_low_bits_set(q, right_shift - 1);

            q = fmod_u128_shr_bits(q, right_shift);

            if (round_bit && (sticky || fmod_u128_is_odd(q)))
                fmod_u128_add_small(q, 1u);

            if (fmod_u128_bit_length(q) > 106)
            {
                q = fmod_u128_shr_bits(q, 1);
                ++ratio_exp;
            }
        }
        else if (ratio_exp < 105)
        {
            q = fmod_u128_shl_bits(q, 105 - ratio_exp);
        }

        const int e2 = exp2 + ratio_exp;
        if (e2 > 1023)
            return neg ? -std::numeric_limits<fdd_s>::infinity() : std::numeric_limits<fdd_s>::infinity();
        if (e2 < -1074)
            return neg ? fdd_s{ -0.0, 0.0 } : fdd_s{ 0.0, 0.0 };

        const std::uint64_t c1 = fmod_u128_get_bits(q, 0, 53);
        const std::uint64_t c0 = fmod_u128_get_bits(q, 53, 53);
        const double hi = c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - 52) : 0.0;
        const double lo = c1 ? detail::fp::ldexp(static_cast<double>(c1), e2 - 105) : 0.0;

        fdd_s out = renorm(hi, lo);
        return neg ? -out : out;
    }

    BL_MSVC_NOINLINE constexpr fdd_s exact_dyadic_to_dd_fmod_big(
        detail::exact_decimal::biguint q,
        int exp2,
        bool neg)
    {
        return detail::exact_decimal::exact_binary_integer_to_value<dd_decimal_traits>(q, exp2, neg);
    }

    BL_MSVC_NOINLINE constexpr fdd_s fmod_exact_biguint(const fdd_s& x, const fdd_s& y)
    {
        detail::exact_decimal::biguint mx;
        detail::exact_decimal::biguint my;
        int ex = 0;
        int ey = 0;
        bool unused_neg = false;

        if (!detail::exact_decimal::exact_binary_components<dd_decimal_traits>(mag(x), mx, ex, unused_neg) ||
            !detail::exact_decimal::exact_binary_components<dd_decimal_traits>(mag(y), my, ey, unused_neg))
        {
            return detail::_dd::signed_zero(signbit(x.hi));
        }

        detail::exact_decimal::biguint remainder;
        int out_exp = 0;

        if (ex < ey)
        {
            const int shift = ey - ex;
            if (detail::exact_decimal::compare_shifted(mx, my, shift) < 0)
            {
                remainder = mx;
            }
            else
            {
                detail::exact_decimal::biguint denominator = my;
                denominator.shl_bits(shift);
                detail::exact_decimal::mod_shift_subtract(mx, denominator, remainder);
            }
            out_exp = ex;
        }
        else
        {
            const int shift = ex - ey;
            mx.shl_bits(shift);
            detail::exact_decimal::mod_shift_subtract(mx, my, remainder);
            out_exp = ey;
        }

        fdd_s out = exact_dyadic_to_dd_fmod_big(remainder, out_exp, signbit(x));
        if (iszero(out))
            return detail::_dd::signed_zero(signbit(x.hi));
        return out;
    }

    BL_MSVC_NOINLINE constexpr bool fmod_exact_candidate_quotient_abs(
        const fdd_s& ax,
        const fdd_s& ay,
        std::uint64_t quotient,
        fdd_s& out)
    {
        if (quotient == 0)
            return false;

        detail::exact_decimal::biguint mx;
        detail::exact_decimal::biguint my;
        int ex = 0;
        int ey = 0;
        bool unused_neg = false;

        if (!detail::exact_decimal::exact_binary_components<dd_decimal_traits>(ax, mx, ex, unused_neg) ||
            !detail::exact_decimal::exact_binary_components<dd_decimal_traits>(ay, my, ey, unused_neg))
        {
            return false;
        }

        const int common_exp = (ex < ey) ? ex : ey;

        detail::exact_decimal::biguint numerator = mx;
        numerator.shl_bits(ex - common_exp);

        detail::exact_decimal::biguint product = detail::exact_decimal::mul_small_u64_big(my, quotient);
        product.shl_bits(ey - common_exp);

        detail::exact_decimal::biguint modulus = my;
        modulus.shl_bits(ey - common_exp);

        detail::exact_decimal::biguint remainder;
        bool remainder_negative = false;
        const int cmp = numerator.compare(product);
        if (cmp >= 0)
        {
            remainder = numerator;
            remainder.sub_inplace(product);
        }
        else
        {
            remainder = product;
            remainder.sub_inplace(numerator);
            remainder_negative = true;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (remainder_negative)
            {
                const int mag_cmp = remainder.compare(modulus);
                if (mag_cmp <= 0)
                {
                    detail::exact_decimal::biguint adjusted = modulus;
                    adjusted.sub_inplace(remainder);
                    remainder = adjusted;
                    remainder_negative = false;
                }
                else
                {
                    remainder.sub_inplace(modulus);
                }
                continue;
            }

            if (remainder.compare(modulus) >= 0)
            {
                remainder.sub_inplace(modulus);
                continue;
            }

            out = exact_dyadic_to_dd_fmod_big(remainder, common_exp, false);
            if (iszero(out))
                out = fdd_s{ 0.0 };
            return true;
        }

        return false;
    }

    BL_FORCE_INLINE constexpr fdd_s fmod_reduced_or_exact(const fdd_s& x, const fdd_s& y)
    {
        const fdd_s ay = mag(y);
        fdd_s r = mag(x);

        constexpr int exact_reduction_exponent_gap = 49;
        if (frexp_exponent_limb(r.hi) - frexp_exponent_limb(ay.hi) > exact_reduction_exponent_gap)
            return fmod_exact_biguint(x, y);

        for (int iteration = 0; iteration < 128 && r >= ay; ++iteration)
        {
            const int ex = frexp_exponent_limb(r.hi);
            const int ey = frexp_exponent_limb(ay.hi);
            int shift = ex - ey - 52;
            if (shift < 0)
                shift = 0;

            fdd_s scaled = ldexp_terms(ay, shift);
            while (shift > 0 && scaled > r)
            {
                --shift;
                scaled = ldexp_terms(ay, shift);
            }

            if (!(scaled > 0.0) || scaled > r)
                return fmod_exact_biguint(x, y);

            const double q = detail::fp::trunc(r.hi / scaled.hi);
            if (!(q > 0.0) || q >= 0x1p53)
                return fmod_exact_biguint(x, y);

            r = fmod_sub_mul_scalar_expansion(r, scaled, q);
            if (!fmod_normalize_remainder(r, scaled))
                return fmod_exact_biguint(x, y);
        }

        if (!fmod_normalize_remainder(r, ay))
            return fmod_exact_biguint(x, y);

        if (iszero(r))
            return detail::_dd::signed_zero(signbit(x.hi));

        return ispositive(x) ? r : -r;
    }

    BL_FORCE_INLINE constexpr fdd_s fmod_exact_fixed_limb_abs_with_quotient_mod(
        const fdd_s& ax,
        const fdd_s& ay,
        std::uint64_t& quotient_mod)
    {
        constexpr std::uint64_t quotient_mask = 0x7fffffffull;

        const exact_dyadic_fmod dx = exact_from_dd_fmod(ax);
        const exact_dyadic_fmod dy = exact_from_dd_fmod(ay);

        fmod_u128 remainder{};
        int out_exp = 0;

        if (dx.exp2 < dy.exp2)
        {
            const int shift = dy.exp2 - dx.exp2;
            if (fmod_u128_shift_exceeds_capacity(dy.mant, shift))
            {
                quotient_mod = 0;
                remainder = dx.mant;
            }
            else
            {
                const fmod_u128 denominator = fmod_u128_shl_bits(dy.mant, shift);
                remainder = fmod_u128_mod_shift_subtract_with_quotient_mod(dx.mant, denominator, quotient_mod);
            }
            out_exp = dx.exp2;
        }
        else
        {
            remainder = fmod_u128_mod_shift_subtract_with_quotient_mod(dx.mant, dy.mant, quotient_mod);
            const int shift = dx.exp2 - dy.exp2;

            int i = 0;
            for (; i < shift && !fmod_u128_is_zero(remainder); ++i)
            {
                std::uint64_t bit = 0;
                remainder = fmod_u128_double_mod_with_quotient_bit(remainder, dy.mant, bit);
                quotient_mod = ((quotient_mod << 1) | bit) & quotient_mask;
            }

            const int remaining = shift - i;
            if (remaining >= 31)
                quotient_mod = 0;
            else if (remaining > 0)
                quotient_mod = (quotient_mod << remaining) & quotient_mask;

            out_exp = dy.exp2;
        }

        fdd_s out = exact_dyadic_to_dd_fmod(remainder, out_exp, false);
        if (iszero(out))
            return fdd_s{ 0.0 };
        return out;
    }

    // decimal conversion
    BL_FORCE_INLINE constexpr bool try_get_int64(const fdd_s& x, int64_t& out)
    {
        const fdd_s xi = detail::_dd_impl::trunc(x);
        if (xi != x)
            return false;

        constexpr std::int64_t int64_min = std::numeric_limits<std::int64_t>::lowest();
        if (xi == detail::_dd_impl::to_dd(int64_min))
        {
            out = int64_min;
            return true;
        }

        if (absd(xi.hi) >= 0x1p63)
            return false;

        const int64_t hi_part = static_cast<int64_t>(xi.hi);
        const fdd_s rem = sub_finite_inline(xi, detail::_dd_impl::to_dd(hi_part));
        out = hi_part + static_cast<int64_t>(rem.hi + rem.lo);
        return true;
    }

    BL_FORCE_INLINE constexpr bool is_odd_integer(const fdd_s& x) noexcept
    {
        int64_t value{};
        if (try_get_int64(x, value))
            return (value & 1ll) != 0;

        if (x.lo != 0.0 || detail::fp::isinf_or_nan(x.hi))
            return false;

        return double_integer_is_odd(x.hi);
    }

    using dd_significant_decimal_traits = dd_decimal_traits;

    BL_MSVC_NOINLINE constexpr fdd_s round_decimal_exact_to_dd(const detail::exact_decimal::biguint& coeff, int dec_exp, bool neg) noexcept
    {
        return detail::exact_decimal::exact_decimal_to_value<dd_significant_decimal_traits>(coeff, dec_exp, neg);
    }

    // rounding helpers
    BL_FORCE_INLINE constexpr fdd_s round_nearest_away_from_zero(const fdd_s& x) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(x.hi))
            return x;

        if (absd(x.hi) < 0x1p52)
        {
            auto base = static_cast<long long>(x.hi);
            if (static_cast<double>(base) == x.hi)
            {
                if (x.hi < 0.0 && x.lo > 0.0)
                    ++base;
                else if (x.hi > 0.0 && x.lo < 0.0)
                    --base;
            }

            const double base_d = static_cast<double>(base);
            const double frac_hi = x.hi - base_d;
            const double frac_lo = x.lo;
            const double abs_frac_hi = absd(frac_hi);
            const double abs_frac_lo = absd(frac_lo);

            long long rounded = base;
            if (abs_frac_hi > 0.5 + abs_frac_lo)
            {
                rounded += (frac_hi < 0.0 || (frac_hi == 0.0 && signbit(frac_lo))) ? -1 : 1;
            }
            else if (abs_frac_hi >= 0.5 - abs_frac_lo)
            {
                const fdd_s frac = sub_double_finite_inline(x, base_d);
                if (mag(frac) >= fdd_s{ 0.5 })
                    rounded += signbit(frac) ? -1 : 1;
            }

            fdd_s out{ static_cast<double>(rounded), 0.0 };
            if (iszero(out))
                return detail::_dd::signed_zero(signbit(x));
            return out;
        }

        if (signbit(x))
        {
            fdd_s y = -detail::_dd_impl::floor(add_double_finite_inline(-x, 0.5));
            if (iszero(y))
                return fdd_s{ -0.0, 0.0 };
            return y;
        }

        return detail::_dd_impl::floor(add_double_finite_inline(x, 0.5));
    }

    template<typename SignedInt>
    BL_FORCE_INLINE constexpr SignedInt to_signed_integer_or_zero(const fdd_s& x) noexcept
    {
        static_assert(std::is_integral_v<SignedInt> && std::is_signed_v<SignedInt>);
        static_assert(sizeof(SignedInt) <= sizeof(std::int64_t));

        if (detail::fp::isinf_or_nan(x.hi))
            return 0;

        constexpr auto lo_i = static_cast<std::int64_t>(std::numeric_limits<SignedInt>::lowest());
        constexpr auto hi_i = static_cast<std::int64_t>(std::numeric_limits<SignedInt>::max());
        const fdd_s lo = detail::_dd_impl::to_dd(lo_i);
        const fdd_s hi = detail::_dd_impl::to_dd(hi_i);

        if (x < lo || x > hi)
            return 0;

        std::int64_t out = 0;
        if (!try_get_int64(x, out))
            return 0;

        return static_cast<SignedInt>(out);
    }

    template<typename SignedInt>
    BL_FORCE_INLINE constexpr bool try_round_to_signed_integer(const fdd_s& x, bool ties_to_even, SignedInt& out) noexcept
    {
        static_assert(std::is_integral_v<SignedInt> && std::is_signed_v<SignedInt>);
        static_assert(sizeof(SignedInt) <= sizeof(std::int64_t));

        if (detail::fp::isinf_or_nan(x.hi) || absd(x.hi) >= 0x1p52)
            return false;

        constexpr auto lo_i = static_cast<std::int64_t>(std::numeric_limits<SignedInt>::lowest());
        constexpr auto hi_i = static_cast<std::int64_t>(std::numeric_limits<SignedInt>::max());
        if (x < detail::_dd_impl::to_dd(lo_i) || x > detail::_dd_impl::to_dd(hi_i))
            return false;

        std::int64_t base = static_cast<std::int64_t>(x.hi);
        if (static_cast<double>(base) == x.hi)
        {
            if (x.hi < 0.0 && x.lo > 0.0)
                ++base;
            else if (x.hi > 0.0 && x.lo < 0.0)
                --base;
        }

        const fdd_s frac     = sub_double_finite_inline(x, static_cast<double>(base));
        const fdd_s abs_frac = mag(frac);
        std::int64_t rounded = base;

        if (abs_frac > fdd_s{ 0.5 } || (!ties_to_even && abs_frac == fdd_s{ 0.5 }) ||
            (ties_to_even && abs_frac == fdd_s{ 0.5 } && (base & 1ll) != 0))
        {
            rounded += bl::signbit(frac) ? -1 : 1;
        }

        if (rounded < lo_i || rounded > hi_i)
            return false;

        out = static_cast<SignedInt>(rounded);
        return true;
    }

    // sqrt kernels
    [[nodiscard]] BL_FORCE_INLINE constexpr double sqrt_constexpr_head(double x) noexcept
    {
        double y = sqrt_seed(x);
        y = 0.5 * (y + x / y);

        auto square_residual = [x](double value, double& hi, double& lo) constexpr noexcept
        {
            double product{};
            double product_error{};
            detail::fp::two_prod_precise(value, value, product, product_error);

            double diff{};
            double diff_error{};
            detail::fp::two_diff_precise(product, x, diff, diff_error);
            detail::fp::quick_two_sum_precise(diff, product_error + diff_error, hi, lo);
        };

        double best_hi{};
        double best_lo{};
        square_residual(y, best_hi, best_lo);
        if (best_hi == 0.0 && best_lo == 0.0)
            return y;

        const bool negative = best_hi < 0.0 || (best_hi == 0.0 && best_lo < 0.0);
        const double candidate = detail::fp::nextafter(
            y,
            negative ? std::numeric_limits<double>::infinity() : 0.0);

        const double step = detail::_dd::absd(candidate - y);
        const double midpoint = detail::_dd::absd(y * step);
        const double residual_mag = best_hi == 0.0 ? detail::_dd::absd(best_lo) : detail::_dd::absd(best_hi);
        constexpr double guard = 0x1p-48;
        if (midpoint != 0.0)
        {
            if (residual_mag < midpoint * (1.0 - guard))
                return y;
            if (residual_mag > midpoint * (1.0 + guard))
                return candidate;
        }

        double candidate_hi{};
        double candidate_lo{};
        square_residual(candidate, candidate_hi, candidate_lo);

        if (best_hi < 0.0 || (best_hi == 0.0 && best_lo < 0.0))
        {
            best_hi = -best_hi;
            best_lo = -best_lo;
        }
        if (candidate_hi < 0.0 || (candidate_hi == 0.0 && candidate_lo < 0.0))
        {
            candidate_hi = -candidate_hi;
            candidate_lo = -candidate_lo;
        }

        return (candidate_hi < best_hi || (candidate_hi == best_hi && candidate_lo < best_lo))
            ? candidate
            : y;
    }

    // clang-cl /fp:fast can collapse the compensated square back to binary64.
    // Keep reassociation disabled only inside the affected sqrt helpers.
    [[nodiscard]] BL_FORCE_INLINE constexpr double sqrt_tail_square(double c_lo, double correction) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        #if FLTX_HAS_RUNTIME_FMA_PATH
        if (!bl::detail::is_constant_evaluated() && detail::fp::runtime_hardware_fma_enabled())
        {
            return detail::fp::fmadd_fma(c_lo, c_lo, correction);
        }
        #endif

        double product{};
        double product_error{};
        detail::fp::two_prod_precise(c_lo, c_lo, product, product_error);

        double sum{};
        double sum_error{};
        detail::fp::two_sum_precise(product, correction, sum, sum_error);
        return sum + (product_error + sum_error);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sqrt_compensated(const fdd_s& scaled_a, double c) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
#if BL_FP_BARRIER_ACTIVE
        double c_hi = product_split_high(c);             BL_FP_BARRIER(c_hi);
        double c_lo = c - c_hi;                          BL_FP_BARRIER(c_lo);

        double q = c_hi * c_lo;                          BL_FP_BARRIER(q);
        q += q;                                          BL_FP_BARRIER(q);

        double p = c_hi * c_hi;                          BL_FP_BARRIER(p);
        double u = p + q;                                BL_FP_BARRIER(u);

        double correction = p - u;                       BL_FP_BARRIER(correction);
        correction += q;                                 BL_FP_BARRIER(correction);
        double uu = sqrt_tail_square(c_lo, correction);  BL_FP_BARRIER(uu);

        double residual = scaled_a.hi - u;               BL_FP_BARRIER(residual);
        residual -= uu;                                  BL_FP_BARRIER(residual);
        residual += scaled_a.lo;                         BL_FP_BARRIER(residual);

        double denominator = c + c;                      BL_FP_BARRIER(denominator);
        double cc = residual / denominator;              BL_FP_BARRIER(cc);

        double y_hi = c + cc;                            BL_FP_BARRIER(y_hi);
        double y_lo = c - y_hi;                          BL_FP_BARRIER(y_lo);
        y_lo += cc;                                      BL_FP_BARRIER(y_lo);
        return { y_hi, y_lo };
#else
        const double c_hi = product_split_high(c);
        const double c_lo = c - c_hi;

        double q = c_hi * c_lo;
        q += q;

        const double p = c_hi * c_hi;
        const double u = p + q;
        const double uu = sqrt_tail_square(c_lo, (p - u) + q);
        const double cc = (((scaled_a.hi - u) - uu) + scaled_a.lo) / (c + c);

        const double y_hi = c + cc;
        return { y_hi, (c - y_hi) + cc };
#endif
    }

} // namespace detail::_dd

} // namespace bl

#endif
