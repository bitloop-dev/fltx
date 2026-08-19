/**
 * fltx/detail/f128_math_transcendental.h - f128 transcendental math implementation details.
 *
 * f128 cbrt, exp/log, pow, trig, hyperbolic, erf, and gamma implementations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F128_DETAIL_MATH_TRANSCENDENTAL_INCLUDED
#define F128_DETAIL_MATH_TRANSCENDENTAL_INCLUDED
#include "fltx/detail/f128_math_basic.h"
#include "fltx/f128_numbers.h"

namespace bl {

[[nodiscard]] BL_FORCE_INLINE constexpr double detail::_f128_impl::log_as_double(f128_s a);

namespace detail::_f128 // primitives and kernels
{
    // polynomial evaluation helpers
    BL_FORCE_INLINE constexpr f128_s horner_forward_inline(const f128_s* coeffs, std::size_t count, const f128_s& x) noexcept
    {
        if (count == 0)
            return {};

        f128_s p = coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
            p = mul_add_inline(p, x, coeffs[i]);
        return p;
    }

    BL_FORCE_INLINE constexpr f128_s horner_forward(const f128_s* coeffs, std::size_t count, const f128_s& x) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            return horner_forward_inline(coeffs, count, x);
        }

        return detail::_f128_runtime::horner_forward(coeffs, count, x);
    }

    BL_FORCE_INLINE constexpr f128_s horner_reverse_inline(const f128_s* coeffs, std::size_t count, const f128_s& x) noexcept
    {
        if (count == 0)
            return {};

        f128_s p = coeffs[count - 1];
        for (std::size_t i = count - 1; i > 0; --i)
            p = mul_add_inline(p, x, coeffs[i - 1]);
        return p;
    }

    BL_FORCE_INLINE constexpr f128_s horner_reverse(const f128_s* coeffs, std::size_t count, const f128_s& x) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            return horner_reverse_inline(coeffs, count, x);
        }

        return detail::_f128_runtime::horner_reverse(coeffs, count, x);
    }

    BL_FORCE_INLINE constexpr void horner_pair_forward_inline(
        const f128_s* left_coeffs,
        const f128_s* right_coeffs,
        std::size_t count,
        const f128_s& x,
        f128_s& left_out,
        f128_s& right_out) noexcept
    {
        if (count == 0)
        {
            left_out = f128_s{};
            right_out = f128_s{};
            return;
        }

        f128_s left  = left_coeffs[0];
        f128_s right = right_coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
            mul_add_pair_same_rhs_inline(left, right, x, left_coeffs[i], right_coeffs[i], left, right);

        left_out = left;
        right_out = right;
    }

    BL_FORCE_INLINE constexpr void horner_pair_forward(
        const f128_s* left_coeffs,
        const f128_s* right_coeffs,
        std::size_t count,
        const f128_s& x,
        f128_s& left_out,
        f128_s& right_out) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            horner_pair_forward_inline(left_coeffs, right_coeffs, count, x, left_out, right_out);
            return;
        }

        detail::_f128_runtime::horner_pair_forward(left_coeffs, right_coeffs, count, x, left_out, right_out);
    }

    // expm1/log1p functions
    BL_MSVC_NOINLINE constexpr f128_s log1p_series_reduced(const f128_s& x)
    {
        const f128_s z = div_prechecked_inline(x, add_double_finite_inline(x, 2.0));
        const f128_s z2 = mul_product_inline(z, z);

        f128_s term = z;
        f128_s sum  = z;

        for (int k = 3; k <= 81; k += 2)
        {
            term = mul_product_inline(term, z2);
            const f128_s add = div_double_prechecked_inline(term, static_cast<double>(k));
            sum = add_finite_inline(sum, add);

            const f128_s asum  = mag(sum);
            const f128_s scale = (asum > f128_s{ 1.0 }) ? asum : f128_s{ 1.0 };
            if (mag(add) <= mul_product_inline(convergence_epsilon, scale))
                break;
        }

        return add_finite_inline(sum, sum);
    }

    BL_MSVC_NOINLINE constexpr f128_s log_normalized_series(const f128_s& m)
    {
        constexpr f128_s inv_odd[] = {
            { 0x1.5555555555555p-2, 0x1.5555555555555p-56 }, // 1/3
            { 0x1.999999999999ap-3, -0x1.999999999999ap-57 }, // 1/5
            { 0x1.2492492492492p-3, 0x1.2492492492492p-57 }, // 1/7
            { 0x1.c71c71c71c71cp-4, 0x1.c71c71c71c71cp-58 }, // 1/9
            { 0x1.745d1745d1746p-4, -0x1.745d1745d1746p-59 }, // 1/11
            { 0x1.3b13b13b13b14p-4, -0x1.3b13b13b13b14p-58 }, // 1/13
            { 0x1.1111111111111p-4, 0x1.1111111111111p-60 }, // 1/15
            { 0x1.e1e1e1e1e1e1ep-5, 0x1.e1e1e1e1e1e1ep-61 }, // 1/17
            { 0x1.af286bca1af28p-5, 0x1.af286bca1af28p-59 }, // 1/19
            { 0x1.8618618618618p-5, 0x1.8618618618618p-59 }, // 1/21
            { 0x1.642c8590b2164p-5, 0x1.642c8590b2164p-60 }, // 1/23
            { 0x1.47ae147ae147bp-5, -0x1.eb851eb851eb8p-61 }, // 1/25
            { 0x1.2f684bda12f68p-5, 0x1.2f684bda12f68p-59 }, // 1/27
            { 0x1.1a7b9611a7b96p-5, 0x1.1a7b9611a7b96p-61 }, // 1/29
            { 0x1.0842108421084p-5, 0x1.0842108421084p-60 }, // 1/31
            { 0x1.f07c1f07c1f08p-6, -0x1.f07c1f07c1f08p-61 }, // 1/33
            { 0x1.d41d41d41d41dp-6, 0x1.0750750750750p-60 }, // 1/35
            { 0x1.bacf914c1bad0p-6, -0x1.bacf914c1bad0p-60 }, // 1/37
            { 0x1.a41a41a41a41ap-6, 0x1.0690690690690p-60 }, // 1/39
            { 0x1.8f9c18f9c18fap-6, -0x1.f3831f3831f38p-61 }, // 1/41
            { 0x1.7d05f417d05f4p-6, 0x1.7d05f417d05f4p-62 }, // 1/43
        };

        const f128_s z = div_prechecked_inline(sub_double_finite_inline(m, 1.0), add_double_finite_inline(m, 1.0));
        const f128_s z2 = mul_product_inline(z, z);

        f128_s p = inv_odd[sizeof(inv_odd) / sizeof(inv_odd[0]) - 1];
        for (int i = static_cast<int>(sizeof(inv_odd) / sizeof(inv_odd[0])) - 2; i >= 0; --i)
            p = mul_add_inline(p, z2, inv_odd[i]);

        p = mul_add_double_rhs_inline(p, z2, 1.0);
        return mul_double_product_inline(mul_product_inline(z, p), 2.0);
    }

    // exponential functions
    BL_MSVC_NOINLINE constexpr f128_s expm1_tiny(const f128_s& r)
    {
        constexpr int coeff_count = static_cast<int>(sizeof(exp_inv_fact) / sizeof(exp_inv_fact[0]));

        f128_s p = mul_product_inline(r, r);
        f128_s sum = add_finite_inline(r, mul_pwr2_inline(p, 0.5));
        const double threshold = absd(r.hi) * convergence_epsilon.hi;

        p = mul_product_inline(p, r);
        for (int i = 0; i < coeff_count; ++i)
        {
            const f128_s term = mul_product_inline(p, exp_inv_fact[i]);
            sum = add_finite_inline(sum, term);
            if (absd(term.hi) <= threshold)
                break;
            p = mul_product_inline(p, r);
        }

        return sum;
    }

    BL_FORCE_INLINE constexpr f128_s exp_integer_factor(
        int n,
        int& binary_scale) noexcept
    {
        binary_scale = 0;
        if (n == 0)
            return f128_s{ 1.0 };

        const bool negative = n < 0;
        std::uint32_t exponent = static_cast<std::uint32_t>(negative ? -n : n);
        const f128_s* table = negative ? exp_integer_inv_table : exp_integer_table;
        f128_s factor{ 1.0 };
#if BL_FP_BARRIER_ACTIVE
        // Keep the factor expansion normal, then restore its binary scale exactly.
        constexpr int binary_scaling_threshold = -600;
        if (negative && n < binary_scaling_threshold)
        {
            int upward_scale = static_cast<int>(detail::fp::ceil(
                static_cast<double>(-n) * std::numbers::log2e_v<double>));
            if (upward_scale > 1023)
                upward_scale = 1023;
            factor = _ldexp(factor, upward_scale);
            binary_scale = -upward_scale;
        }
#endif
#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        const bool checked_product = detail::fp::exp_scale_needs_checked_product(n);
#endif

        for (std::size_t i = 0; exponent != 0 && i < (sizeof(exp_integer_table) / sizeof(exp_integer_table[0])); ++i)
        {
            if ((exponent & 1u) != 0)
            {
#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
                factor = checked_product ? mul_product_range_safe_inline(factor, table[i]) : mul_product_inline(factor, table[i]);
#else
                factor = mul_product_inline(factor, table[i]);
#endif
            }
            exponent >>= 1u;
        }

        return factor;
    }

    BL_FORCE_INLINE constexpr double exp_nearest_integer(double x) noexcept
    {
        return static_cast<double>(
            x >= 0.0
                ? static_cast<int>(x + 0.5)
                : static_cast<int>(x - 0.5));
    }

    BL_FORCE_INLINE constexpr bool exp_overflows(const f128_s& x) noexcept
    {
        if (x.hi != exp_overflow_cutoff.hi)
            return x.hi > exp_overflow_cutoff.hi;
        return x > exp_overflow_cutoff;
    }

    BL_FORCE_INLINE constexpr bool exp_underflows_to_zero(const f128_s& x) noexcept
    {
        if (x.hi != exp_zero_cutoff.hi)
            return x.hi < exp_zero_cutoff.hi;
        return x < exp_zero_cutoff;
    }

    BL_FORCE_INLINE constexpr int exp_reduction_integer(const f128_s& x) noexcept
    {
        int n = static_cast<int>(exp_nearest_integer(x.hi));
        return n > 709 ? 709 : n;
    }

    BL_MSVC_NOINLINE constexpr f128_s exp_general_scaled_with_n(const f128_s& x, bool sub_one, int n) noexcept
    {
        const double nd = static_cast<double>(n);
        const f128_s reduced = sub_double_finite_inline(x, nd);
        const f128_s r = mul_pwr2_inline(reduced, 0.0078125);

        f128_s e = expm1_tiny(r);
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));

        if (n == 0)
            return sub_one ? e : add_double_finite_inline(e, 1.0);

        int factor_scale = 0;
        const f128_s factor = exp_integer_factor(n, factor_scale);
#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        const f128_s scaled = detail::fp::exp_scale_needs_checked_product(n)
            ? mul_add_range_safe_inline(factor, e, factor)
            : mul_add_inline(factor, e, factor);
#else
        const f128_s scaled = mul_add_inline(factor, e, factor);
#endif
        const f128_s result = factor_scale == 0
            ? scaled
            : _ldexp(scaled, factor_scale);
        return sub_one ? sub_double_finite_inline(result, 1.0) : result;
    }

    BL_MSVC_NOINLINE constexpr f128_s exp_general_scaled(const f128_s& x, bool sub_one) noexcept
    {
        return exp_general_scaled_with_n(x, sub_one, exp_reduction_integer(x));
    }

    BL_MSVC_NOINLINE constexpr f128_s _exp(const f128_s& x)
    {
        if (isnan(x))
            return x;
        if (isinf(x))
            return (x.hi < 0.0) ? f128_s{ 0.0 } : std::numeric_limits<f128_s>::infinity();

        if (exp_overflows(x))
            return std::numeric_limits<f128_s>::infinity();

        if (exp_underflows_to_zero(x))
            return f128_s{ 0.0 };

        if (iszero(x))
            return f128_s{ 1.0 };

        return exp_general_scaled(x, false);
    }

    BL_MSVC_NOINLINE constexpr f128_s _exp2(const f128_s& x)
    {
        if (isnan(x))
            return x;
        if (isinf(x))
            return (x.hi < 0.0) ? f128_s{ 0.0 } : std::numeric_limits<f128_s>::infinity();

        if (x.hi > 1023.0 || x.hi < -1074.0)
            return _exp(mul_product_inline(x, std::numbers::ln2_v<f128_s>));

        if (iszero(x))
            return f128_s{ 1.0 };

        const double kd = exp_nearest_integer(x.hi);
        const int k = static_cast<int>(kd);
        const f128_s reduced = sub_double_finite_inline(x, kd);
        const f128_s r = mul_pwr2_inline(mul_product_inline(reduced, std::numbers::ln2_v<f128_s>), 0.0078125);

        f128_s e = expm1_tiny(r);
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_pwr2_inline(e, 2.0));

        return _ldexp(add_double_finite_inline(e, 1.0), k);
    }

    // logarithm functions
    BL_MSVC_NOINLINE constexpr f128_s _log(const f128_s& a)
    {
        if (isnan(a))
            return a;
        if (iszero(a))
            return f128_s{ -std::numeric_limits<double>::infinity(), 0.0 };
        if (a.hi < 0.0 || (a.hi == 0.0 && a.lo < 0.0))
            return std::numeric_limits<f128_s>::quiet_NaN();
        if (isinf(a))
            return a;

#if BL_FP_BARRIER_ACTIVE
        f128_s input = a;
        BL_FP_BARRIER(input.hi);
        BL_FP_BARRIER(input.lo);
#else
        const f128_s& input = a;
#endif

        int exp2 = 0;
        if (bl::detail::is_constant_evaluated()) {
            exp2 = detail::fp::frexp_exponent(input.hi);
        }
        else {
            (void)std::frexp(input.hi, &exp2);
        }

        f128_s m = _ldexp(input, -exp2);
#if BL_FP_BARRIER_ACTIVE
        BL_FP_BARRIER(m.hi);
        BL_FP_BARRIER(m.lo);
#endif
        if (m < sqrt_half)
        {
            m = mul_double_product_inline(m, 2.0);
            --exp2;
        }

        const f128_s exp2_ln2 = mul_double_product_inline(std::numbers::ln2_v<f128_s>, static_cast<double>(exp2));
        return add_finite_inline(exp2_ln2, log_normalized_series(m));
    }

    BL_FORCE_INLINE constexpr bool f128_try_exact_binary_log2(const f128_s& x, int& out) noexcept
    {
        if (!(x.hi > 0.0) || x.lo != 0.0)
            return false;

        const std::uint64_t bits = std::bit_cast<std::uint64_t>(x.hi);
        const std::uint32_t exp_bits = static_cast<std::uint32_t>((bits >> 52) & 0x7ffu);
        const std::uint64_t frac_bits = bits & ((std::uint64_t{ 1 } << 52) - 1);

        if (exp_bits == 0 || exp_bits == 0x7ffu || frac_bits != 0)
            return false;

        out = static_cast<int>(exp_bits) - 1023;
        return true;
    }

    // power functions
    [[nodiscard]] BL_FORCE_INLINE constexpr bool f128_try_pow10_ldexp_chunks(
        int pow5_count,
        int binary_exponent_per_input_exponent,
        int exponent,
        f128_s& out) noexcept
    {
        if (pow5_count <= 0)
            return false;
        if (exponent == 0)
        {
            out = f128_s{ 1.0 };
            return true;
        }

        const int chunk_limit = exponent > 0
            ? detail::_f128::pow10_f128_max_exponent / pow5_count
            : (-detail::_f128::pow10_f128_min_exponent) / pow5_count;
        if (chunk_limit <= 0)
            return false;

        f128_s value{ 1.0 };
        int remaining = exponent;
        while (remaining != 0)
        {
            const int chunk = remaining > 0
                ? ((remaining > chunk_limit) ? chunk_limit : remaining)
            : ((remaining < -chunk_limit) ? -chunk_limit : remaining);

            int decimal_exponent = 0;
            if (!detail::fp::checked_exponent_product(pow5_count, chunk, decimal_exponent))
                return false;
            if (decimal_exponent < detail::_f128::pow10_f128_min_exponent ||
                decimal_exponent > detail::_f128::pow10_f128_max_exponent)
            {
                return false;
            }

            int binary_exponent = 0;
            if (!detail::fp::checked_exponent_product(binary_exponent_per_input_exponent, chunk, binary_exponent))
                return false;

            const f128_s term = detail::_f128_impl::ldexp(
                detail::_f128_impl::pow10_128(decimal_exponent),
                binary_exponent);
            value = mul_product_inline(value, term);
            if (detail::fp::isinf_or_nan(value.hi))
                return false;

            remaining -= chunk;
        }

        out = value;
        return true;
    }

    template<detail::fp::non_bool_integral Exp>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_special_pow_components(
        bool pow10_base,
        int pow2_log2,
        bool negative_base,
        Exp y,
        f128_s& out)
    {
        int exponent = 0;
        if (!detail::fp::try_int_exponent(y, exponent))
            return false;

        f128_s value{};
        if (pow10_base)
        {
            value = detail::_f128_impl::pow10_128(exponent);
        }
        else
        {
            int binary_exponent = 0;
            if (!detail::fp::checked_exponent_product(pow2_log2, exponent, binary_exponent))
                return false;

            if (binary_exponent >= std::numeric_limits<double>::max_exponent)
                value = std::numeric_limits<f128_s>::infinity();
            else if (binary_exponent < std::numeric_limits<double>::min_exponent - std::numeric_limits<double>::digits)
                value = f128_s{ 0.0 };
            else
                value = detail::_f128_impl::ldexp(f128_s{ 1.0 }, binary_exponent);
        }

        out = (negative_base && detail::fp::is_odd_integral(y)) ? -value : value;
        return true;
    }

    template<detail::fp::non_bool_integral Base, detail::fp::non_bool_integral Exp>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_integral_pow_finite_raw(Base x, Exp y, f128_s& out) noexcept
    {
        using UBase = std::make_unsigned_t<std::remove_cvref_t<Base>>;

        const UBase magnitude_base = detail::fp::unsigned_abs(x);
        const bool negative_base = x < Base{ 0 };

        if (magnitude_base == UBase{ 10 })
            return try_special_pow_components(true, 0, negative_base, y, out);

        int pow2_log2 = 0;
        if (detail::fp::try_unsigned_power_of_two_log2(magnitude_base, pow2_log2))
            return try_special_pow_components(false, pow2_log2, negative_base, y, out);

        int pow5_count = 0;
        if (!detail::fp::factor_power_of_two_five(magnitude_base, pow2_log2, pow5_count))
            return false;

        int exponent = 0;
        if (!detail::fp::try_int_exponent(y, exponent))
            return false;

        int decimal_exponent = 0;
        if (!detail::fp::checked_exponent_product(pow5_count, exponent, decimal_exponent))
            return false;

        const int binary_exponent_per_input_exponent = pow2_log2 - pow5_count;
        f128_s value{};
        if (decimal_exponent >= detail::_f128::pow10_f128_min_exponent &&
            decimal_exponent <= detail::_f128::pow10_f128_max_exponent)
        {
            int binary_exponent = 0;
            if (!detail::fp::checked_exponent_product(binary_exponent_per_input_exponent, exponent, binary_exponent))
                return false;

            value = detail::_f128_impl::ldexp(
                detail::_f128_impl::pow10_128(decimal_exponent),
                binary_exponent);
        }
        else if (!f128_try_pow10_ldexp_chunks(
            pow5_count,
            binary_exponent_per_input_exponent,
            exponent,
            value))
        {
            return false;
        }

        out = (negative_base && detail::fp::is_odd_integral(y)) ? -value : value;
        return true;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s pow_mul_adaptive(const f128_s& a, const f128_s& b) noexcept
    {
        return detail::fp::dekker_product_needs_scaling(a.hi, b.hi)
            ? mul_dekker_range_safe_canonical_inline(a, b)
            : mul_canonical_inline(a, b);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s pow_sqr_adaptive(const f128_s& a) noexcept
    {
        return detail::fp::dekker_product_needs_scaling(a.hi, a.hi)
            ? sqr_dekker_range_safe_canonical_inline(a)
            : sqr_inline(a);
    }

    template<bool Checked, class ExpUnsigned>
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s powi_nonnegative_impl(f128_s base, ExpUnsigned exp) noexcept
    {
        if (exp == ExpUnsigned{ 0 })
            return f128_s{ 1.0 };
        if (exp == ExpUnsigned{ 1 })
            return base;
        if (exp == ExpUnsigned{ 2 })
            return Checked ? pow_sqr_adaptive(base) : sqr_inline(base);
        if (exp == ExpUnsigned{ 3 })
        {
            const f128_s squared = Checked ? pow_sqr_adaptive(base) : sqr_inline(base);
            return Checked ? pow_mul_adaptive(squared, base) : mul_canonical_inline(squared, base);
        }
        if (exp == ExpUnsigned{ 4 })
        {
            const f128_s squared = Checked ? pow_sqr_adaptive(base) : sqr_inline(base);
            return Checked ? pow_sqr_adaptive(squared) : sqr_inline(squared);
        }

        f128_s result{ 1.0 };
        while (exp != ExpUnsigned{ 0 })
        {
            if ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 })
                result = Checked ? pow_mul_adaptive(result, base) : mul_canonical_inline(result, base);

            exp >>= 1;
            if (exp != ExpUnsigned{ 0 })
                base = Checked ? pow_sqr_adaptive(base) : sqr_inline(base);
        }

        return result;
    }

    template<class ExpUnsigned>
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s powi_nonnegative_unchecked(f128_s base, ExpUnsigned exp) noexcept
    {
        if (exp == ExpUnsigned{ 0 })
            return f128_s{ 1.0 };
        if (exp == ExpUnsigned{ 1 })
            return base;
        if (exp == ExpUnsigned{ 2 })
            return sqr_inline(base);
        if (exp == ExpUnsigned{ 3 })
            return mul_product_inline(sqr_inline(base), base);
        if (exp == ExpUnsigned{ 4 })
            return sqr_inline(sqr_inline(base));

        f128_s result{ 1.0 };
        while (exp != ExpUnsigned{ 0 })
        {
            if ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 })
                result = mul_product_inline(result, base);

            exp >>= 1;
            if (exp != ExpUnsigned{ 0 })
                base = sqr_inline(base);
        }

        return result;
    }

    template<class ExpUnsigned>
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s powi_nonnegative_fast(f128_s base, ExpUnsigned exp) noexcept
    {
        if (detail::fp::ipow_loop_needs_range_safe_dekker(base.hi, exp)) [[unlikely]]
            return powi_nonnegative_impl<true>(base, exp);
        return powi_nonnegative_impl<false>(base, exp);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s reciprocal_pow_result(const f128_s& value) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(value.hi)) [[unlikely]]
            return div_special(f128_s{ 1.0 }, value);

        return div_double_prechecked_inline(1.0, value);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_exact_integer_pow_base_value(const f128_s& value, std::int64_t& out) noexcept
    {
        if (value.lo != 0.0 || !detail::fp::isfinite(value.hi))
            return false;

        const bool negative = value.hi < 0.0;
        const double magnitude = negative ? -value.hi : value.hi;
        if (magnitude > detail::fp::exact_double_integer_limit_double)
            return false;

        const double integral = detail::fp::trunc(magnitude);
        if (integral != magnitude)
            return false;

        out = static_cast<std::int64_t>(integral);
        if (negative)
            out = -out;
        return true;
    }

    template<detail::fp::non_bool_integral Exp>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_exact_integer_pow_base(const f128_s& x, Exp y, f128_s& out) noexcept
    {
        std::int64_t integer_base = 0;
        if (!try_exact_integer_pow_base_value(x, integer_base))
            return false;

        if (detail::_f128::try_integral_pow_finite_raw(integer_base, y, out))
            return true;

        using U = std::make_unsigned_t<std::remove_cvref_t<Exp>>;
        const U magnitude = detail::fp::unsigned_abs(y);
        const bool split_safe = detail::fp::integral_pow_split_safe(integer_base, magnitude) ||
            !detail::fp::ipow_loop_needs_range_safe_dekker(x.hi, magnitude);

        if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
        {
            if (y < 0)
            {
                if (detail::fp::integral_pow_reciprocal_underflows_binary64(integer_base, magnitude))
                {
                    const bool negative_zero = detail::fp::negative_integral_pow_result_is_negative(integer_base, magnitude);
                    out = detail::_f128::signed_zero(negative_zero);
                    return true;
                }

                if (split_safe)
                {
                    const f128_s powered = detail::_f128::powi_nonnegative_unchecked(x, magnitude);
                    out = detail::_f128::reciprocal_pow_result(powered);
                    return true;
                }

                const f128_s reciprocal = detail::_f128::reciprocal_pow_result(x);
                out = (integer_base != 0)
                    ? detail::_f128::powi_nonnegative_unchecked(reciprocal, magnitude)
                    : detail::_f128::powi_nonnegative_impl<false>(reciprocal, magnitude);
                return true;
            }
        }

        out = split_safe
            ? detail::_f128::powi_nonnegative_unchecked(x, magnitude)
            : detail::_f128::powi_nonnegative_fast<U>(x, magnitude);
        return true;
    }

    template<detail::fp::non_bool_integral Exp>
    [[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s ipow_integer(const f128_s& x, Exp y)
    {
        using U = std::make_unsigned_t<std::remove_cvref_t<Exp>>;
        const U magnitude = detail::fp::unsigned_abs(y);

        if (magnitude == U{0})
            return f128_s{1.0};
        if (isnan(x))
            return x;
        if (isinf(x))
        {
            const bool negative = signbit(x) && (magnitude & U{1}) != U{0};
            if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
            {
                if (y < 0)
                    return detail::_f128::signed_zero(negative);
            }
            return f128_s{
                negative
                    ? -std::numeric_limits<double>::infinity()
                    : std::numeric_limits<double>::infinity()};
        }

        f128_s special{};
        if (detail::_f128::try_exact_integer_pow_base(x, y, special))
            return special;

        if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
        {
            if (y < 0)
            {
                const f128_s reciprocal = detail::_f128::reciprocal_pow_result(x);
                return detail::_f128::powi_nonnegative_impl<false>(reciprocal, magnitude);
            }
        }

        return detail::_f128::powi_nonnegative_fast<U>(x, magnitude);
    }

    BL_FORCE_INLINE constexpr f128_s polish_eighth_root(const f128_s& x, const f128_s& y)
    {
        if (iszero(y))
            return y;

        const f128_s y2 = mul_product_inline(y, y);
        const f128_s y4 = mul_product_inline(y2, y2);
        const f128_s y7 = mul_product_inline(mul_product_inline(y4, y2), y);
        const f128_s y8 = mul_product_inline(y4, y4);
        const f128_s correction = div_double_prechecked_inline(div_prechecked_inline(sub_finite_inline(x, y8), y7), 8.0);

        return add_finite_inline(y, correction);
    }

    BL_FORCE_INLINE constexpr f128_s pow_positive_eighth_fraction(const f128_s& x, int numerator)
    {
        const f128_s r2 = detail::_f128_impl::sqrt(x);
        if (numerator == 4)
            return r2;

        const f128_s r4 = detail::_f128_impl::sqrt(r2);
        if (numerator == 2)
            return r4;

        f128_s out{ 1.0 };
        if ((numerator & 4) != 0)
            out = mul_product_inline(out, r2);
        if ((numerator & 2) != 0)
            out = mul_product_inline(out, r4);
        if ((numerator & 1) != 0)
        {
            const f128_s r8 = polish_eighth_root(x, detail::_f128_impl::sqrt(r4));
            if (numerator == 1)
                return r8;
            out = mul_product_inline(out, r8);
        }
        return out;
    }

    BL_FORCE_INLINE constexpr bool pow_dyadic_eighth_exponent_in_range(int64_t n) noexcept
    {
        if (n == std::numeric_limits<int64_t>::min())
            return false;

        const bool neg = n < 0;
        const std::uint64_t magnitude = neg ? static_cast<std::uint64_t>(-n) : static_cast<std::uint64_t>(n);
        return magnitude <= 1024;
    }

    BL_FORCE_INLINE constexpr bool try_get_pow_dyadic_eighth_exponent(const f128_s& x, const f128_s& y, int64_t& n)
    {
        if (x.hi < 0.0 || (x.hi == 0.0 && signbit(x.hi)))
            return false;

        if (!try_get_int64(mul_double_product_inline(y, 8.0), n))
            return false;

        return pow_dyadic_eighth_exponent_in_range(n);
    }

    BL_FORCE_INLINE constexpr bool try_get_pow_dyadic_eighth_exponent(const f128_s& x, double y, int64_t& n) noexcept
    {
        if (x.hi < 0.0 || (x.hi == 0.0 && signbit(x.hi)))
            return false;

        const double scaled = y * 8.0;
        if (detail::fp::isinf_or_nan(scaled) || absd(scaled) >= 0x1p63)
            return false;

        const double rounded = detail::fp::trunc(scaled);
        if (rounded != scaled)
            return false;

        n = static_cast<int64_t>(rounded);
        return pow_dyadic_eighth_exponent_in_range(n);
    }

    BL_NO_INLINE constexpr f128_s pow_dyadic_eighth_unchecked(const f128_s& x, int64_t n)
    {
        if (n == 0)
            return f128_s{ 1.0 };

        const bool neg = n < 0;
        const std::uint64_t magnitude = neg ? static_cast<std::uint64_t>(-n) : static_cast<std::uint64_t>(n);
        const std::uint64_t whole = magnitude / 8u;
        const int rem = static_cast<int>(magnitude & 7u);

        f128_s result = (whole == 0u) ? f128_s{ 1.0 } : detail::fp::powi_by_squaring(x, static_cast<int64_t>(whole));
        if (rem != 0)
            result = mul_product_inline(result, pow_positive_eighth_fraction(x, rem));
        if (neg)
            result = recip(result);

        return result;
    }

    BL_FORCE_INLINE constexpr f128_s pow_from_log_product(const f128_s& product)
    {
        if (detail::_f128::mag(product) <= f128_s{ 0.125 })
            return add_double_finite_inline(expm1_tiny(product), 1.0);

        return _exp(product);
    }

    BL_FORCE_INLINE constexpr f128_s log_for_pow_positive(const f128_s& x)
    {
        const f128_s xm1 = sub_double_finite_inline(x, 1.0);
        if (detail::_f128::mag(xm1) <= f128_s{ 0.5 })
            return log1p_series_reduced(xm1);

        return _log(x);
    }

    BL_FORCE_INLINE constexpr f128_s pow_log_product_accurate(const f128_s& y, const f128_s& log_x)
    {
        return mul_accurate_inline(y, log_x);
    }

    // sine/cosine functions
    BL_FORCE_INLINE constexpr bool remainder_pio2(const f128_s& x, long long& n_out, f128_s& r_out)
    {
        const double ax = fabs(x.hi);
        if (detail::fp::isinf_or_nan(ax))
            return false;

        if (ax > 7.0e15)
            return false;

        const f128_s t = mul_product_inline(x, invpi2);

        double qd = round_nearest_even_value(t.hi);
        if (detail::fp::isinf_or_nan(qd) ||
            qd < static_cast<double>(std::numeric_limits<long long>::min()) ||
            qd > static_cast<double>(std::numeric_limits<long long>::max()))
        {
            return false;
        }

        const f128_s delta = sub_double_finite_inline(t, qd);
        if (delta.hi > 0.5 || (delta.hi == 0.5 && delta.lo > 0.0))
            qd += 1.0;
        else if (delta.hi < -0.5 || (delta.hi == -0.5 && delta.lo < 0.0))
            qd -= 1.0;

        if (qd < static_cast<double>(std::numeric_limits<long long>::min()) ||
            qd > static_cast<double>(std::numeric_limits<long long>::max()))
        {
            return false;
        }

        constexpr f128_s pi_2_hi{ pi_2_hi_d };
        constexpr f128_s pi_2_mid{ pi_2_mid_d };
        constexpr f128_s pi_2_lo{ pi_2_lo_d };
        constexpr f128_s pi_4{ pi_4_hi };

        long long n = static_cast<long long>(qd);
        const double q = static_cast<double>(n);

        f128_s r = x;
        r = sub_finite_inline(r, mul_double_product_inline(pi_2_hi, q));
        r = sub_finite_inline(r, mul_double_product_inline(pi_2_mid, q));
        r = sub_finite_inline(r, mul_double_product_inline(pi_2_lo, q));

        if (r > pi_4)
        {
            ++n;
            r = sub_finite_inline(r, pi_2_hi);
            r = sub_finite_inline(r, pi_2_mid);
            r = sub_finite_inline(r, pi_2_lo);
        }
        else if (r < -pi_4)
        {
            --n;
            r = add_finite_inline(r, pi_2_hi);
            r = add_finite_inline(r, pi_2_mid);
            r = add_finite_inline(r, pi_2_lo);
        }

        // The split-constant reduction is fast, but its relative error grows
        // near a large multiple of pi/2. Let the exact reducer handle only
        // those rare cancellation cases.
        if (detail::fp::absd(r.hi) <= ax * 0x1p-72)
            return false;

        n_out = n;
        r_out = r;
        return true;
    }

    BL_MSVC_NOINLINE constexpr bool remainder_pio2_payne_hanek(const f128_s& x, long long& n_out, f128_s& r_out)
    {
        detail::exact_decimal::biguint magnitude;
        int binary_exp = 0;
        bool unused_neg = false;
        if (!detail::exact_decimal::exact_binary_components<f128_decimal_traits>(mag(x), magnitude, binary_exp, unused_neg))
        {
            n_out = 0;
            r_out = signbit(x) ? f128_s{ -0.0, 0.0 } : f128_s{ 0.0, 0.0 };
            return true;
        }

        const detail::exact_decimal::biguint two_over_pi = detail::exact_decimal::from_words(
            detail::trig_reduce::two_over_pi_fixed_words,
            static_cast<int>(sizeof(detail::trig_reduce::two_over_pi_fixed_words) / sizeof(detail::trig_reduce::two_over_pi_fixed_words[0])));
        static_assert(sizeof(detail::trig_reduce::two_over_pi_fixed_words) / sizeof(detail::trig_reduce::two_over_pi_fixed_words[0]) <= detail::exact_decimal::biguint::max_words);
        static_assert(detail::exact_decimal::biguint::max_words * 32 >= detail::trig_reduce::two_over_pi_fixed_bits + 105);

        const detail::exact_decimal::biguint product = detail::exact_decimal::mul_big(magnitude, two_over_pi);
        const int scale_bits = detail::trig_reduce::two_over_pi_fixed_bits - binary_exp;
        if (scale_bits <= 0)
            return false;

        const detail::exact_decimal::biguint rem = detail::exact_decimal::low_bits_copy(product, scale_bits);
        const bool half_bit = rem.get_bit(scale_bits - 1);
        const bool sticky = detail::exact_decimal::any_low_bits_set(rem, scale_bits - 1);
        const bool integer_odd = product.get_bit(scale_bits);
        const bool round_up = half_bit && (sticky || integer_odd);

        unsigned n_mod4 =
            (product.get_bit(scale_bits) ? 1u : 0u) |
            (product.get_bit(scale_bits + 1) ? 2u : 0u);
        if (round_up)
            n_mod4 = (n_mod4 + 1u) & 3u;

        detail::exact_decimal::biguint y_coeff = rem;
        bool y_neg = false;
        if (round_up)
        {
            y_coeff.clear();
            y_coeff.set_bit(scale_bits);
            y_coeff.sub_inplace(rem);
            y_neg = !y_coeff.is_zero();
        }

        f128_s r = exact_dyadic_to_f128_fmod_big(y_coeff, -scale_bits, y_neg);
        r = mul_product_inline(r, pi_2);

        if (r > pi_4)
        {
            r = sub_finite_inline(r, pi_2);
            n_mod4 = (n_mod4 + 1u) & 3u;
        }
        else if (r < -pi_4)
        {
            r = add_finite_inline(r, pi_2);
            n_mod4 = (n_mod4 + 3u) & 3u;
        }

        if (signbit(x))
        {
            r = -r;
            n_mod4 = (4u - n_mod4) & 3u;
        }

        n_out = static_cast<long long>(n_mod4);
        r_out = r;
        return true;
    }

    BL_FORCE_INLINE constexpr f128_s sin_kernel_small(const f128_s& x)
    {
        using namespace detail::_f128;

        if (detail::fp::absd(x.hi) < 0x1p-56)
            return x;

        const f128_s t = mul_product_inline(x, x);

        const f128_s ps = horner_forward(
            f128_sin_coeffs_pi4 + f128_trig_small_coeff_offset,
            f128_trig_small_coeff_count,
            t);

        return mul_add_inline(mul_product_inline(x, t), ps, x);
    }

    BL_FORCE_INLINE constexpr f128_s cos_kernel_small(const f128_s& x)
    {
        using namespace detail::_f128;

        const f128_s t = mul_product_inline(x, x);

        const f128_s pc = horner_forward(
            f128_cos_coeffs_pi4 + f128_trig_small_coeff_offset,
            f128_trig_small_coeff_count,
            t);

        return mul_add_double_rhs_inline(t, pc, 1.0);
    }

    BL_FORCE_INLINE constexpr void sincos_kernel_small(const f128_s& x, f128_s& s_out, f128_s& c_out)
    {
        using namespace detail::_f128;

        if (detail::fp::absd(x.hi) < 0x1p-56)
        {
            s_out = x;
            c_out = f128_s{ 1.0 };
            return;
        }

        const f128_s t = mul_product_inline(x, x);

        f128_s ps{};
        f128_s pc{};
        horner_pair_forward(
            f128_sin_coeffs_pi4 + f128_trig_small_coeff_offset,
            f128_cos_coeffs_pi4 + f128_trig_small_coeff_offset,
            f128_trig_small_coeff_count,
            t,
            ps,
            pc);

        const f128_s xt = mul_product_inline(x, t);
        s_out = mul_add_inline(xt, ps, x);
        c_out = mul_add_double_rhs_inline(t, pc, 1.0);
    }

    BL_FORCE_INLINE constexpr void sincos_kernel_pi64_reduced(const f128_s& x, f128_s& s_out, f128_s& c_out)
    {
        int k = static_cast<int>(round_nearest_even_value(x.hi * 20.371832715762604));
        if (k < -16)
            k = -16;
        else if (k > 16)
            k = 16;

        if (k == 0)
        {
            sincos_kernel_small(x, s_out, c_out);
            return;
        }

        const f128_s a = mul_double_product_inline(std::numbers::pi_v<f128_s>, static_cast<double>(k) * 0.015625);
        const f128_s u = sub_finite_inline(x, a);

        f128_s su{}, cu{};
        sincos_kernel_small(u, su, cu);

        const int table_index = k < 0 ? -k : k;
        const f128_s sa = k < 0 ? -f128_sin_table_pi64[table_index] : f128_sin_table_pi64[table_index];
        const f128_s ca = f128_cos_table_pi64[table_index];

        s_out = sum_products_inline(ca, su, sa, cu);
        c_out = diff_products_inline(ca, cu, sa, su);
    }

    BL_FORCE_INLINE constexpr f128_s sin_kernel_pi4(const f128_s& x)
    {
        if (detail::fp::absd(x.hi) < 0x1p-56)
            return x;

        const f128_s t = mul_product_inline(x, x);

        const f128_s ps = horner_forward(f128_sin_coeffs_pi4, f128_trig_coeff_count_pi4, t);

        return mul_add_inline(mul_product_inline(x, t), ps, x);
    }

    BL_FORCE_INLINE constexpr f128_s cos_kernel_pi4(const f128_s& x)
    {
        const f128_s t = mul_product_inline(x, x);

        const f128_s pc = horner_forward(f128_cos_coeffs_pi4, f128_trig_coeff_count_pi4, t);

        return mul_add_double_rhs_inline(t, pc, 1.0);
    }

    // inverse trig functions
    BL_FORCE_INLINE constexpr f128_s atan_series_reduced(const f128_s& z)
    {
        constexpr int count = static_cast<int>(sizeof(f128_atan_reduced_coeffs) / sizeof(f128_atan_reduced_coeffs[0]));

        const f128_s z2 = mul_product_inline(z, z);
        const f128_s p = horner_reverse(f128_atan_reduced_coeffs, static_cast<std::size_t>(count), z2);

        return mul_product_inline(z, p);
    }

    BL_FORCE_INLINE constexpr f128_s atan_core_unit(const f128_s& z)
    {
        int k = static_cast<int>(round_nearest_even_value(z.hi * 16.0));
        if (k <= 0)
            return atan_series_reduced(z);
        if (k > 16)
            k = 16;

        const double a = static_cast<double>(k) * 0.0625;
        const f128_s u = div_prechecked_inline(
            sub_double_finite_inline(z, a),
            add_double_finite_inline(mul_double_product_inline(z, a), 1.0));

        return add_finite_inline(f128_atan_reduced_table_16[k], atan_series_reduced(u));
    }

    BL_FORCE_INLINE constexpr f128_s _atan(const f128_s& x)
    {
        if (isnan(x))  return x;
        if (iszero(x)) return x;
        if (isinf(x))  return signbit(x.hi) ? -pi_2 : pi_2;

        const bool neg = x.hi < 0.0;
        const f128_s ax = neg ? -x : x;

        if (ax > f128_s{ 1.0 })
        {
            const f128_s core = atan_core_unit(div_double_prechecked_inline(1.0, ax));
            const f128_s out  = sub_finite_inline(pi_2, core);
            return neg ? -out : out;
        }

        const f128_s out = atan_core_unit(ax);
        return neg ? -out : out;
    }

    BL_FORCE_INLINE constexpr f128_s _asin(const f128_s& x)
    {
        if (isnan(x))
            return x;

        const f128_s ax = detail::_f128::mag(x);
        if (ax > f128_s{ 1.0 })
            return std::numeric_limits<f128_s>::quiet_NaN();
        if (ax == f128_s{ 1.0 })
            return (x.hi < 0.0) ? -pi_2 : pi_2;

        if (ax <= f128_s{ 0.5 })
            return _atan(div_prechecked_inline(x, detail::_f128_impl::sqrt(sub_double_finite_inline(1.0, mul_product_inline(x, x)))));

        const f128_s t = detail::_f128_impl::sqrt(div_prechecked_inline(sub_double_finite_inline(1.0, ax), add_double_finite_inline(ax, 1.0)));
        const f128_s a = sub_finite_inline(pi_2, mul_double_product_inline(_atan(t), 2.0));
        return (x.hi < 0.0) ? -a : a;
    }

    BL_FORCE_INLINE constexpr f128_s _acos(const f128_s& x)
    {
        if (isnan(x))
            return x;

        const f128_s ax = detail::_f128::mag(x);
        if (ax > f128_s{ 1.0 })
            return std::numeric_limits<f128_s>::quiet_NaN();
        if (x == f128_s{ 1.0 })
            return f128_s{ 0.0 };
        if (x == f128_s{ -1.0 })
            return std::numbers::pi_v<f128_s>;

        return sub_finite_inline(pi_2, detail::_f128::_asin(x));
    }

    // inverse hyperbolic functions
    BL_MSVC_NOINLINE constexpr f128_s atanh_small_series(const f128_s& x)
    {
        const f128_s x2 = mul_product_inline(x, x);
        f128_s sum   = x;
        f128_s power = x;

        for (int k = 1; k <= 32; ++k)
        {
            power = mul_product_inline(power, x2);
            const f128_s term = div_double_prechecked_inline(power, static_cast<double>(2 * k + 1));
            sum = add_finite_inline(sum, term);

            if (mag(term) <= convergence_epsilon)
                break;
        }

        return sum;
    }

    // erf/erfc functions
    [[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s erf_cheb_eval(
        const f128_s& x,
        const f128_s* coeffs,
        std::size_t count,
        double shift)
    {
        const f128_s t = sub_double_finite_inline(mul_double_product_inline(x, 2.0), shift);
        f128_s b1{ 0.0 };
        f128_s b2{ 0.0 };

        for (int i = static_cast<int>(count) - 1; i >= 1; --i)
        {
            const f128_s b0 = add_finite_inline(
                sub_finite_inline(mul_double_product_inline(mul_product_inline(t, b1), 2.0), b2),
                coeffs[i]);
            b2 = b1;
            b1 = b0;
        }

        return add_finite_inline(mul_sub_inline(t, b1, b2), coeffs[0]);
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s erf_positive_cheb_1_2(const f128_s& x)
    {
        return erf_cheb_eval(x, f128_erf_cheb_1_2, f128_erf_cheb_1_2_coeff_count, 3.0);
    }

    BL_FORCE_INLINE constexpr f128_s erf_positive_series(const f128_s& x)
    {
        const f128_s xx = mul_product_inline(x, x);
        f128_s power = x;
        f128_s sum   = x;

        for (int n = 1; n < 256; ++n)
        {
            power = mul_product_inline(power, div_double_prechecked_inline(-xx, static_cast<double>(n)));
            const f128_s term = div_double_prechecked_inline(power, static_cast<double>(2 * n + 1));
            sum = add_finite_inline(sum, term);
            if (mag(term) < convergence_epsilon)
                break;
        }

        return mul_pwr2_inline(mul_product_inline(std::numbers::inv_sqrtpi_v<f128_s>, sum), 2.0);
    }

    BL_FORCE_INLINE constexpr f128_s erfc_positive_cf(const f128_s& x)
    {
        const f128_s z = mul_product_inline(x, x);
        constexpr f128_s tiny = f128_s{ 1.0e-300 };
        constexpr f128_s convergence = f128_s{ 0x1p-101 }; // 32 * convergence_epsilon

        f128_s b = add_double_finite_inline(z, 0.5);
        f128_s c = div_double_prechecked_inline(1.0, tiny);
        f128_s d = div_double_prechecked_inline(1.0, b);
        f128_s h = d;

        double an = -0.5;
        double an_step = -2.5;
        for (int i = 1; i <= 96; ++i)
        {
            b = add_double_finite_inline(b, 2.0);

            d = mul_add_double_lhs_inline(an, d, b);
            if (mag(d) < tiny)
                d = tiny;

            c = add_finite_inline(b, div_double_prechecked_inline(an, c));
            if (mag(c) < tiny)
                c = tiny;

            d = div_double_prechecked_inline(1.0, d);
            const f128_s delta = mul_product_inline(d, c);
            h = mul_product_inline(h, delta);

            if (mag(sub_double_finite_inline(delta, 1.0)) <= convergence)
                break;

            an += an_step;
            an_step -= 2.0;
        }

        const f128_s out = mul_product_inline(mul_product_inline(mul_product_inline(detail::_f128_impl::exp(-z), x), std::numbers::inv_sqrtpi_v<f128_s>), h);
        return out;
    }

    // gamma functions
    BL_MSVC_NOINLINE constexpr f128_s lgamma1p_series(const f128_s& y) noexcept
    {
        constexpr int count = static_cast<int>(sizeof(lgamma1p_coeff) / sizeof(lgamma1p_coeff[0]));

        const f128_s p = horner_reverse(lgamma1p_coeff, static_cast<std::size_t>(count), y);

        return mul_product_inline(y, mul_add_inline(y, p, -std::numbers::egamma_v<f128_s>));
    }

    BL_MSVC_NOINLINE constexpr f128_s lgamma1p5_series(const f128_s& y) noexcept
    {
        constexpr int count = static_cast<int>(sizeof(lgamma1p5_coeff) / sizeof(lgamma1p5_coeff[0]));

        const f128_s p = horner_reverse(lgamma1p5_coeff, static_cast<std::size_t>(count), y);

        const f128_s constant = sub_finite_inline(half_log_two_pi, mul_double_product_inline(std::numbers::ln2_v<f128_s>, 1.5));
        const f128_s linear   = sub_finite_inline(sub_double_finite_inline(2.0, std::numbers::egamma_v<f128_s>), mul_double_product_inline(std::numbers::ln2_v<f128_s>, 2.0));
        return mul_add_inline(y, mul_add_inline(y, p, linear), constant);
    }

    BL_MSVC_NOINLINE constexpr bool try_lgamma_near_one_or_two(const f128_s& x, f128_s& out) noexcept
    {
        const f128_s y1 = sub_double_finite_inline(x, 1.0);
        if (mag(y1) <= f128_s{ 0.25 })
        {
            out = lgamma1p_series(y1);
            return true;
        }

        const f128_s y15 = sub_double_finite_inline(x, 1.5);
        if (mag(y15) <= f128_s{ 0.25 })
        {
            out = lgamma1p5_series(y15);
            return true;
        }

        const f128_s y2 = sub_double_finite_inline(x, 2.0);
        if (mag(y2) <= f128_s{ 0.25 })
        {
            out = add_finite_inline(log1p_series_reduced(y2), lgamma1p_series(y2));
            return true;
        }

        return false;
    }

    BL_MSVC_NOINLINE constexpr f128_s lgamma_stirling_asymptotic(const f128_s& z) noexcept
    {
        const f128_s inv    = div_double_prechecked_inline(1.0, z);
        const f128_s inv2   = mul_product_inline(inv, inv);
        const f128_s series = mul_product_inline(inv, horner_reverse(
            lgamma_stirling_coeffs,
            sizeof(lgamma_stirling_coeffs) / sizeof(lgamma_stirling_coeffs[0]),
            inv2));

        return add_finite_inline(add_finite_inline(sub_finite_inline(mul_product_inline(sub_double_finite_inline(z, 0.5), detail::_f128_impl::log(z)), z), half_log_two_pi), series);
    }

    BL_MSVC_NOINLINE constexpr void positive_recurrence_product(const f128_s& x, const f128_s& asymptotic_min, f128_s& z, f128_s& product, int& product_scale2) noexcept
    {
        z = x;
        product = f128_s{ 1.0 };
        product_scale2 = 0;

        while (z < asymptotic_min)
        {
            product = mul_product_inline(product, z);

            const double hi = product.hi;
            if (hi != 0.0)
            {
                const int exponent = detail::fp::frexp_exponent_limb(hi);
                if (exponent > 512 || exponent < -512)
                {
                    product = detail::_f128_impl::ldexp(product, -exponent);
                    product_scale2 += exponent;
                }
            }

            z = add_double_finite_inline(z, 1.0);
        }
    }

    BL_MSVC_NOINLINE constexpr f128_s lgamma_positive_low_range(const f128_s& x) noexcept
    {
        f128_s y = x;
        f128_s product{ 1.0 };
        bool shifted_up = false;

        if (y < f128_s{ 0.75 })
        {
            shifted_up = true;
            do
            {
                product = mul_product_inline(product, y);
                y = add_double_finite_inline(y, 1.0);
            }
            while (y < f128_s{ 0.75 });
        }
        else
        {
            while (y > f128_s{ 2.25 })
            {
                y = sub_double_finite_inline(y, 1.0);
                product = mul_product_inline(product, y);
            }
        }

        f128_s local{};
        try_lgamma_near_one_or_two(y, local);

        if (product == f128_s{ 1.0 })
            return local;

        const f128_s correction = detail::_f128_impl::log(product);
        return shifted_up ? sub_finite_inline(local, correction) : add_finite_inline(local, correction);
    }

    BL_MSVC_NOINLINE constexpr f128_s gamma_positive_low_range(const f128_s& x) noexcept
    {
        f128_s y = x;
        f128_s product{ 1.0 };
        bool shifted_up = false;

        if (y < f128_s{ 0.75 })
        {
            shifted_up = true;
            do
            {
                product = mul_product_inline(product, y);
                y = add_double_finite_inline(y, 1.0);
            }
            while (y < f128_s{ 0.75 });
        }
        else
        {
            while (y > f128_s{ 2.25 })
            {
                y = sub_double_finite_inline(y, 1.0);
                product = mul_product_inline(product, y);
            }
        }

        f128_s local_lgamma{};
        try_lgamma_near_one_or_two(y, local_lgamma);
        const f128_s local_gamma = detail::_f128_impl::exp(local_lgamma);
#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        return shifted_up ? div_prechecked_range_safe_inline(local_gamma, product) : mul_product_inline(local_gamma, product);
#else
        return shifted_up ? div_prechecked_inline(local_gamma, product) : mul_product_inline(local_gamma, product);
#endif
    }

    BL_MSVC_NOINLINE constexpr f128_s lgamma_positive_recurrence(const f128_s& x) noexcept
    {
        f128_s near_value{};
        if (try_lgamma_near_one_or_two(x, near_value))
            return near_value;

        if (x <= f128_s{ 16.0 })
            return lgamma_positive_low_range(x);

        constexpr f128_s asymptotic_min = f128_s{ 40.0 };

        f128_s z{};
        f128_s product{};
        int product_scale2 = 0;
        positive_recurrence_product(x, asymptotic_min, z, product, product_scale2);

        return sub_finite_inline(
            sub_finite_inline(lgamma_stirling_asymptotic(z), detail::_f128_impl::log(product)),
            mul_double_product_inline(std::numbers::ln2_v<f128_s>, static_cast<double>(product_scale2)));
    }

    BL_MSVC_NOINLINE constexpr f128_s gamma_positive_recurrence(const f128_s& x) noexcept
    {
        f128_s near_lgamma{};
        if (try_lgamma_near_one_or_two(x, near_lgamma))
            return detail::_f128_impl::exp(near_lgamma);

        if (x <= f128_s{ 16.0 })
            return gamma_positive_low_range(x);

        constexpr f128_s asymptotic_min = f128_s{ 40.0 };

        f128_s z{};
        f128_s product{};
        int product_scale2 = 0;
        positive_recurrence_product(x, asymptotic_min, z, product, product_scale2);

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        f128_s out = (product == f128_s{ 1.0 })
            ? detail::_f128_impl::exp(lgamma_stirling_asymptotic(z))
            : div_prechecked_inline(detail::_f128_impl::exp(lgamma_stirling_asymptotic(z)), product);
#else
        f128_s out = div_prechecked_inline(detail::_f128_impl::exp(lgamma_stirling_asymptotic(z)), product);
#endif
        if (product_scale2 != 0)
            out = detail::_f128_impl::ldexp(out, -product_scale2);

        return out;
    }

    BL_MSVC_NOINLINE constexpr f128_s sinpi_reduced(const f128_s& x) noexcept
    {
        const f128_s n = detail::_f128_impl::round_nearest_even(x);
        const f128_s r = sub_finite_inline(x, n);
        f128_s out = detail::_f128_impl::sin(mul_product_inline(std::numbers::pi_v<f128_s>, r));
        if (is_odd_integer(n))
            out = -out;
        return out;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s cbrt_constexpr_seed(const f128_s& ax)
    {
        return f128_s{ detail::fp::cbrt_seed(ax.hi), 0.0 };
    }
}

namespace detail::_f128_runtime
{
    [[nodiscard]] BL_FORCE_INLINE f128_s cbrt_seed(const f128_s& ax)
    {
        int exp2 = 0;
        double mantissa = std::frexp(ax.hi, &exp2);
        int rem = exp2 % 3;
        if (rem < 0)
            rem += 3;
        if (rem != 0)
        {
            mantissa = std::ldexp(mantissa, rem);
            exp2 -= rem;
        }

        f128_s y{ std::cbrt(mantissa), 0.0 };
        if (exp2 != 0)
            y = detail::_f128::_ldexp(y, exp2 / 3);
        return y;
    }
}

namespace detail::_f128
{
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s cbrt_seed(const f128_s& ax)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            cbrt_constexpr_seed(ax),
            detail::_f128_runtime::cbrt_seed(ax)
        );
    }

    BL_PUSH_PRECISE;
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s cbrt_compensated(const f128_s& ax, double c) noexcept
    {
        double c2_hi{}, c2_lo{};
        two_prod_precise(c, c, c2_hi, c2_lo);

        double c3_hi{}, c3_lo{};
        two_prod_precise(c2_hi, c, c3_hi, c3_lo);
#if BL_FP_BARRIER_ACTIVE
        BL_FP_BARRIER(c3_lo);
        double c2_tail = c2_lo * c;  BL_FP_BARRIER(c2_tail);
        c3_lo += c2_tail;             BL_FP_BARRIER(c3_lo);

        double residual_hi{}, residual_lo{};
        two_sum_precise(ax.hi, -c3_hi, residual_hi, residual_lo);
        BL_FP_BARRIER(residual_lo);
        residual_lo += ax.lo;  BL_FP_BARRIER(residual_lo);
        residual_lo -= c3_lo;  BL_FP_BARRIER(residual_lo);

        const f128_s residual = renorm(residual_hi, residual_lo);
        double derivative = 3.0 * c2_hi;             BL_FP_BARRIER(derivative);
        double inv_derivative = 1.0 / derivative;    BL_FP_BARRIER(inv_derivative);
        double cc = residual.hi * inv_derivative;    BL_FP_BARRIER(cc);
        double correction = residual.lo * inv_derivative;  BL_FP_BARRIER(correction);
        cc += correction;                            BL_FP_BARRIER(cc);

        double y_hi = c + cc;    BL_FP_BARRIER(y_hi);
        double y_lo = c - y_hi;  BL_FP_BARRIER(y_lo);
        y_lo += cc;               BL_FP_BARRIER(y_lo);
        return { y_hi, y_lo };
#else
        c3_lo += c2_lo * c;

        double residual_hi{}, residual_lo{};
        two_sum_precise(ax.hi, -c3_hi, residual_hi, residual_lo);
        residual_lo += ax.lo - c3_lo;

        const f128_s residual = renorm(residual_hi, residual_lo);
        const double inv_derivative = 1.0 / (3.0 * c2_hi);
        const double cc = residual.hi * inv_derivative + residual.lo * inv_derivative;

        const double y_hi = c + cc;
        return { y_hi, (c - y_hi) + cc };
#endif
    }
    BL_POP_PRECISE;

} // namespace detail::_f128

// exponential functions
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::exp(const f128_s& x)
{
    return _exp(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::exp2(const f128_s& x)
{
    return _exp2(x);
}

// logarithm functions
[[nodiscard]] BL_FORCE_INLINE constexpr double detail::_f128_impl::log_as_double(f128_s a)
{
    const double hi = a.hi;
    if (hi <= 0.0)
        return detail::fp::log(static_cast<double>(a));

    return detail::fp::log(hi) + detail::fp::log1p(a.lo / hi);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::log(const f128_s& a)
{
    return _log(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::log2(const f128_s& a)
{
    if (isnan(a))
        return a;
    if (iszero(a))
        return f128_s{ -std::numeric_limits<double>::infinity(), 0.0 };
    if (signbit(a))
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (isinf(a))
        return a;

    int exact_exp2{};
    if (f128_try_exact_binary_log2(a, exact_exp2))
        return f128_s{ static_cast<double>(exact_exp2), 0.0 };

    return mul_product_inline(_log(a), std::numbers::log2e_v<f128_s>);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::log10(const f128_s& x)
{
    if (isnan(x))
        return x;
    if (iszero(x))
        return f128_s{ -std::numeric_limits<double>::infinity(), 0.0 };
    if (signbit(x))
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (isinf(x))
        return x;

    if (detail::fp::isfinite(x.hi) && x.hi > 0.0 && x.lo == 0.0)
    {
        const int exp2 =
            detail::fp::frexp_exponent(x.hi);
        const int k0 =
            static_cast<int>(detail::fp::floor((exp2 - 1) * 0.30102999566398114));

        for (int k = k0 - 2; k <= k0 + 2; ++k)
        {
            const f128_s pow10 = detail::_f128_impl::pow10_128(k);
            if (pow10.hi >= std::numeric_limits<double>::min() && x == pow10)
                return f128_s{ static_cast<double>(k), 0.0 };
        }
    }

    return mul_product_inline(_log(x), std::numbers::log10e_v<f128_s>);
}

// expm1/log1p functions
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::expm1(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (x == f128_s{ 0.0 })
        return x;
    if (isinf(x))
        return signbit(x)
            ? f128_s{ -1.0, 0.0 }
            : std::numeric_limits<f128_s>::infinity();

    if (exp_overflows(x))
        return std::numeric_limits<f128_s>::infinity();

    if (exp_underflows_to_zero(x))
        return f128_s{ -1.0, 0.0 };

    return exp_general_scaled(x, true);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::log1p(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (x == f128_s{ -1.0 })
        return f128_s{ -std::numeric_limits<double>::infinity(), 0.0 };
    if (x < f128_s{ -1.0 })
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (isinf(x))
        return x;
    if (iszero(x))
        return x;

    const f128_s ax = detail::_f128::mag(x);
    if (ax <= f128_s{ 0.25 })
        return log1p_series_reduced(x);

    const f128_s u = add_double_finite_inline(x, 1.0);
    return _log(u);
}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::cbrt(const f128_s& x)
{
    using namespace detail::_f128;

    if (detail::fp::iszero_or_inf_or_nan(x.hi))
        return x;

    const bool neg = signbit(x);
    const f128_s ax = neg ? -x : x;

    f128_s y = cbrt_compensated(ax, cbrt_seed(ax).hi);
    if (bl::detail::is_constant_evaluated())
        y = cbrt_compensated(ax, y.hi);

    if (neg)
        y = -y;

    return y;
}

// power functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::pow(const f128_s& x, const f128_s& y)
{
    if (iszero(y))
        return f128_s{ 1.0 };

    if (x == f128_s{ 1.0 } || (x == f128_s{ -1.0 } && detail::fp::isinf(y.hi)))
        return f128_s{ 1.0 };

    if (detail::fp::isnan(x.hi) || detail::fp::isnan(y.hi))
        return std::numeric_limits<f128_s>::quiet_NaN();

    if (detail::fp::isinf(y.hi))
    {
        const f128_s ax = detail::_f128::mag(x);
        if (ax == f128_s{ 1.0 })
            return f128_s{ 1.0 };
        if (ax < f128_s{ 1.0 })
            return signbit(y) ? std::numeric_limits<f128_s>::infinity() : f128_s{ 0.0 };
        return signbit(y) ? f128_s{ 0.0 } : std::numeric_limits<f128_s>::infinity();
    }

    const f128_s yi = detail::_f128_impl::trunc(y);
    const bool y_is_int = (yi == y);

    int64_t yi64{};
    if (y_is_int && try_get_int64(yi, yi64))
        return detail::fp::powi_by_squaring(x, yi64);

    int64_t dyadic_exponent{};
    if (try_get_pow_dyadic_eighth_exponent(x, y, dyadic_exponent))
        return pow_dyadic_eighth_unchecked(x, dyadic_exponent);

    if (x.hi < 0.0 || (x.hi == 0.0 && signbit(x.hi)))
    {
        if (!y_is_int)
            return std::numeric_limits<f128_s>::quiet_NaN();

        const f128_s magnitude = pow_from_log_product(pow_log_product_accurate(y, log_for_pow_positive(-x)));
        return is_odd_integer(yi) ? -magnitude : magnitude;
    }

    return pow_from_log_product(pow_log_product_accurate(y, log_for_pow_positive(x)));
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::pow(const f128_s& x, double y)
{
    if (y == 0.0)
        return f128_s{ 1.0 };

    if (x == f128_s{ 1.0 } || (x == f128_s{ -1.0 } && detail::fp::isinf(y)))
        return f128_s{ 1.0 };

    if (detail::fp::isnan(x.hi) || detail::fp::isnan(y))
        return std::numeric_limits<f128_s>::quiet_NaN();

    if (detail::fp::isinf(y))
    {
        const f128_s ax = detail::_f128::mag(x);
        if (ax == f128_s{ 1.0 })
            return f128_s{ 1.0 };
        if (ax < f128_s{ 1.0 })
            return detail::fp::signbit(y) ? std::numeric_limits<f128_s>::infinity() : f128_s{ 0.0 };
        return detail::fp::signbit(y) ? f128_s{ 0.0 } : std::numeric_limits<f128_s>::infinity();
    }

    if (y == 1.0)  return x;
    if (y == 2.0)  return x * x;
    if (y == -1.0) return div_double_prechecked_inline(1.0, x);
    if (y == 0.5)  return detail::_f128_impl::sqrt(x);

    double yi{};
    if (bl::detail::is_constant_evaluated())
    {
        yi = (y < 0.0)
            ? detail::fp::ceil(y)
            : detail::fp::floor(y);
    }
    else
    {
        yi = std::trunc(y);
    }

    const bool y_is_int = (yi == y);

    if (y_is_int && absd(yi) < 0x1p63)
        return detail::fp::powi_by_squaring(x, static_cast<int64_t>(yi));

    int64_t dyadic_exponent{};
    if (try_get_pow_dyadic_eighth_exponent(x, y, dyadic_exponent))
        return pow_dyadic_eighth_unchecked(x, dyadic_exponent);

    if (x.hi < 0.0 || (x.hi == 0.0 && signbit(x.hi)))
    {
        if (!y_is_int)
            return std::numeric_limits<f128_s>::quiet_NaN();

        const f128_s magnitude = pow_from_log_product(log_for_pow_positive(-x) * y);
        const bool y_is_odd =
            (absd(yi) < 0x1p53) &&
            ((static_cast<int64_t>(yi) & 1ll) != 0);

        return y_is_odd ? -magnitude : magnitude;
    }

    return pow_from_log_product(log_for_pow_positive(x) * y);
}

// inverse trig functions
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::atan(const f128_s& x)
{
    return detail::_f128::_atan(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::asin(const f128_s& x)
{
    return detail::_f128::_asin(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::acos(const f128_s& x)
{
    return detail::_f128::_acos(x);
}

// sine/cosine functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr bool detail::_f128_impl::sincos(const f128_s& x, f128_s& s_out, f128_s& c_out)
{
    if (iszero(x))
    {
        s_out = x;
        c_out = f128_s{ 1.0 };
        return true;
    }

    const double ax = detail::_f128::fabs(x.hi);
    if (detail::fp::isinf_or_nan(ax))
    {
        s_out = f128_s{ std::numeric_limits<double>::quiet_NaN() };
        c_out = s_out;
        return false;
    }

    if (ax <= pi_4_hi)
    {
        sincos_kernel_pi64_reduced(x, s_out, c_out);
        s_out = s_out;
        c_out = c_out;
        return true;
    }

    long long n = 0;
    f128_s r{};
    if (!remainder_pio2(x, n, r))
    {
        if (!remainder_pio2_payne_hanek(x, n, r))
        {
            s_out = f128_s{ std::numeric_limits<double>::quiet_NaN() };
            c_out = s_out;
            return false;
        }
    }

    f128_s sr{}, cr{};
    sincos_kernel_pi64_reduced(r, sr, cr);

    switch ((int)(n & 3))
    {
    case 0: s_out = sr;  c_out = cr;  break;
    case 1: s_out = cr;  c_out = -sr; break;
    case 2: s_out = -sr; c_out = -cr; break;
    default: s_out = -cr; c_out = sr;  break;
    }

    s_out = s_out;
    c_out = c_out;
    return true;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::sin(const f128_s& x)
{
    if (iszero(x))
        return x;

    const double ax = detail::_f128::fabs(x.hi);
    if (detail::fp::isinf_or_nan(ax))
        return f128_s{ std::numeric_limits<double>::quiet_NaN() };

    if (ax <= pi_4_hi)
        return sin_kernel_pi4(x);

    long long n = 0;
    f128_s r{};
    if (!remainder_pio2(x, n, r))
    {
        if (!remainder_pio2_payne_hanek(x, n, r))
            return f128_s{ std::numeric_limits<double>::quiet_NaN() };
    }

    switch ((int)(n & 3))
    {
    case 0: return sin_kernel_pi4(r);
    case 1: return cos_kernel_pi4(r);
    case 2: return -sin_kernel_pi4(r);
    default: return -cos_kernel_pi4(r);
    }
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::cos(const f128_s& x)
{
    const double ax = detail::_f128::fabs(x.hi);
    if (detail::fp::isinf_or_nan(ax))
        return f128_s{ std::numeric_limits<double>::quiet_NaN() };

    if (ax <= pi_4_hi)
        return cos_kernel_pi4(x);

    long long n = 0;
    f128_s r{};
    if (!remainder_pio2(x, n, r))
    {
        if (!remainder_pio2_payne_hanek(x, n, r))
            return f128_s{ std::numeric_limits<double>::quiet_NaN() };
    }

    switch ((int)(n & 3))
    {
    case 0: return cos_kernel_pi4(r);
    case 1: return -sin_kernel_pi4(r);
    case 2: return -cos_kernel_pi4(r);
    default: return sin_kernel_pi4(r);
    }
}

// tangent and atan2
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::tan(const f128_s& x)
{
    if (iszero(x))
        return x;

    f128_s s{}, c{};
    if (detail::_f128_impl::sincos(x, s, c))
        return div_prechecked_inline(s, c);
    const double xd = (double)x;
    if (bl::detail::is_constant_evaluated()) {
        return f128_s{ detail::fp::tan(xd) };
    } else {
        return f128_s{ std::tan(xd) };
    }
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::atan2(const f128_s& y, const f128_s& x)
{
    if (detail::fp::isnan(x.hi) || detail::fp::isnan(y.hi))
        return std::numeric_limits<f128_s>::quiet_NaN();

    if (isinf(y))
    {
        if (isinf(x))
        {
            if (x.hi < 0.0)
                return signbit(y) ? -detail::_f128::pi_3_4 : detail::_f128::pi_3_4;
            return signbit(y) ? -detail::_f128::pi_4 : detail::_f128::pi_4;
        }

        return signbit(y) ? -detail::_f128::pi_2 : detail::_f128::pi_2;
    }
    if (isinf(x))
    {
        if (x.hi < 0.0)
            return signbit(y) ? -std::numbers::pi_v<f128_s> : std::numbers::pi_v<f128_s>;
        return detail::_f128::signed_zero(signbit(y));
    }

    if (iszero(x))
    {
        if (iszero(y))
        {
            if (signbit(x))
                return signbit(y) ? -std::numbers::pi_v<f128_s> : std::numbers::pi_v<f128_s>;
            return y;
        }
        return ispositive(y) ? detail::_f128::pi_2 : -detail::_f128::pi_2;
    }

    if (iszero(y))
    {
        if (x.hi < 0.0)
            return signbit(y.hi) ? -std::numbers::pi_v<f128_s> : std::numbers::pi_v<f128_s>;
        return y;
    }

    const f128_s ax = detail::_f128::mag(x);
    const f128_s ay = detail::_f128::mag(y);

    if (ax == ay)
    {
        if (x.hi < 0.0)
        {
            return
                (y.hi < 0.0) ? -detail::_f128::pi_3_4 : detail::_f128::pi_3_4;
        }

        return
            (y.hi < 0.0) ? -detail::_f128::pi_4 : detail::_f128::pi_4;
    }

    if (ax >= ay)
    {
        #if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        const f128_s ratio = detail::fp::dekker_product_needs_scaling(y.hi, x.hi)
            ? detail::_f128::div_prechecked_range_safe_inline(y, x)
            : detail::_f128::div_prechecked_inline(y, x);
        #else
        const f128_s ratio = detail::_f128::div_prechecked_inline(y, x);
        #endif
        f128_s a = detail::_f128::_atan(ratio);

        if (x.hi < 0.0)
            a = detail::_f128::add_finite_inline(a, (y.hi < 0.0) ? -std::numbers::pi_v<f128_s> : std::numbers::pi_v<f128_s>);
        return a;
    }

    #if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    const f128_s ratio = detail::fp::dekker_product_needs_scaling(x.hi, y.hi)
        ? detail::_f128::div_prechecked_range_safe_inline(x, y)
        : detail::_f128::div_prechecked_inline(x, y);
    #else
    const f128_s ratio = detail::_f128::div_prechecked_inline(x, y);
    #endif
    const f128_s a = detail::_f128::_atan(ratio);
    return
        (y.hi < 0.0) ? detail::_f128::sub_finite_inline(-detail::_f128::pi_2, a) : detail::_f128::sub_finite_inline(detail::_f128::pi_2, a);
}

// sinh/cosh/tanh
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::sinh(const f128_s& x)
{
    using namespace detail::_f128;

    if (detail::fp::iszero_or_inf_or_nan(x.hi))
        return x;

    const f128_s ax = detail::_f128::mag(x);
    if (ax <= f128_s{ 0.5 })
    {
        const f128_s e = detail::_f128_impl::expm1(x);
        return
            mul_double_product_inline(div_prechecked_inline(mul_product_inline(e, add_double_finite_inline(e, 2.0)), add_double_finite_inline(e, 1.0)), 0.5);
    }

    const f128_s ex = detail::_f128_impl::exp(ax);
#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    f128_s out = detail::fp::exp_inverse_is_negligible(ax.hi)
        ? mul_pwr2_inline(ex, 0.5)
        : mul_double_product_inline(sub_finite_inline(ex, div_double_prechecked_inline(1.0, ex)), 0.5);
#else
    f128_s out = mul_double_product_inline(sub_finite_inline(ex, div_double_prechecked_inline(1.0, ex)), 0.5);
#endif
    if (signbit(x))
        out = -out;
    return out;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::cosh(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (isinf(x))
        return std::numeric_limits<f128_s>::infinity();

    const f128_s ax = detail::_f128::mag(x);
    const f128_s ex = detail::_f128_impl::exp(ax);
#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    if (detail::fp::exp_inverse_is_negligible(ax.hi))
        return mul_pwr2_inline(ex, 0.5);
#endif

    return mul_double_product_inline(add_finite_inline(ex, div_double_prechecked_inline(1.0, ex)), 0.5);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::tanh(const f128_s& x)
{
    using namespace detail::_f128;

    if (detail::fp::iszero_or_nan(x.hi))
        return x;
    if (isinf(x))
        return signbit(x) ? f128_s{ -1.0 } : f128_s{ 1.0 };

    const f128_s ax = detail::_f128::mag(x);
    if (ax > f128_s{ 40.0 })
        return signbit(x) ? f128_s{ -1.0 } : f128_s{ 1.0 };

    if (ax >= f128_s{ 0.5 })
    {
        const f128_s e = detail::_f128_impl::exp(mul_double_product_inline(ax, -2.0));
        f128_s out = sub_double_finite_inline(1.0, div_prechecked_inline(mul_double_product_inline(e, 2.0), add_double_finite_inline(e, 1.0)));
        if (signbit(x))
            out = -out;
        return out;
    }

    const f128_s e = detail::_f128_impl::expm1(x);
    const f128_s ep1 = add_double_finite_inline(e, 1.0);
    return
        div_prechecked_inline(
            mul_product_inline(e, add_double_finite_inline(e, 2.0)),
            add_double_finite_inline(mul_product_inline(ep1, ep1), 1.0));
}

// inverse hyperbolic functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::asinh(const f128_s& x)
{
    using namespace detail::_f128;

    if (detail::fp::iszero_or_inf_or_nan(x.hi))
        return x;

    const f128_s ax = detail::_f128::mag(x);
    f128_s out{};
    if (ax > f128_s{ 0x1p500 })
        out = add_finite_inline(detail::_f128_impl::log(ax), std::numbers::ln2_v<f128_s>);
    else if (ax <= f128_s{ 0.5 })
    {
        const f128_s ax2 = mul_product_inline(ax, ax);
        out = detail::_f128_impl::log1p(add_finite_inline(
            ax,
            div_prechecked_inline(ax2, add_double_finite_inline(detail::_f128_impl::sqrt(add_double_finite_inline(ax2, 1.0)), 1.0))));
    }
    else
        out = detail::_f128_impl::log(add_finite_inline(ax, detail::_f128_impl::sqrt(add_double_finite_inline(mul_product_inline(ax, ax), 1.0))));

    if (signbit(x))
        out = -out;
    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::acosh(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (x < f128_s{ 1.0 })
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (x == f128_s{ 1.0 })
        return f128_s{ 0.0 };
    if (isinf(x))
        return x;

    f128_s out{};
    if (x > f128_s{ 0x1p500 })
        out = add_finite_inline(detail::_f128_impl::log(x), std::numbers::ln2_v<f128_s>);
    else if (x < f128_s{ 1.25 })
    {
        const f128_s xm1 = sub_double_finite_inline(x, 1.0);
        out = detail::_f128_impl::log1p(add_finite_inline(
            xm1,
            detail::_f128_impl::sqrt(mul_product_inline(xm1, add_double_finite_inline(x, 1.0)))));
    }
    else
        out = detail::_f128_impl::log(add_finite_inline(
            x,
            detail::_f128_impl::sqrt(mul_product_inline(sub_double_finite_inline(x, 1.0), add_double_finite_inline(x, 1.0)))));

    return out;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::atanh(const f128_s& x)
{
    using namespace detail::_f128;

    if (detail::fp::iszero_or_nan(x.hi))
        return x;

    const f128_s ax = detail::_f128::mag(x);
    if (ax > f128_s{ 1.0 })
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (ax == f128_s{ 1.0 })
        return signbit(x)
            ? f128_s{ -std::numeric_limits<double>::infinity(), 0.0 }
            : f128_s{  std::numeric_limits<double>::infinity(), 0.0 };

    if (ax <= f128_s{ 0.125 })
    {
        return atanh_small_series(x);
    }

    if (ax < f128_s{ 0.25 })
    {
        const f128_s r = div_prechecked_inline(mul_double_product_inline(x, 2.0), sub_double_finite_inline(1.0, x));
        return mul_double_product_inline(detail::_f128_impl::log1p(r), 0.5);
    }

    const f128_s out = mul_double_product_inline(
        detail::_f128_impl::log(div_prechecked_inline(add_double_finite_inline(x, 1.0), sub_double_finite_inline(1.0, x))),
        0.5);
    return out;
}

// erf/erfc functions
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::erf(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (isinf(x))
        return signbit(x) ? f128_s{ -1.0 } : f128_s{ 1.0 };
    if (iszero(x))
        return x;

    const bool neg = signbit(x);
    const f128_s ax = neg ? -x : x;

    f128_s out{};
    if (ax < f128_s{ 1.0 })
        out = erf_positive_series(ax);
    else if (ax < f128_s{ 2.0 })
        out = erf_positive_cheb_1_2(ax);
    else
        out = ax > f128_s{ 27.0 } ? f128_s{ 1.0 } : sub_double_finite_inline(1.0, erfc_positive_cf(ax));

    if (neg)
        out = -out;

    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s detail::_f128_impl::erfc(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (x == f128_s{ 0.0 })
        return f128_s{ 1.0 };
    if (isinf(x))
        return signbit(x) ? f128_s{ 2.0 } : f128_s{ 0.0 };

    if (signbit(x))
    {
        const f128_s ax = -x;
        if (ax < f128_s{ 1.0 })
            return add_double_finite_inline(erf_positive_series(ax), 1.0);
        if (ax < f128_s{ 2.0 })
            return add_double_finite_inline(erf_positive_cheb_1_2(ax), 1.0);
        if (ax > f128_s{ 27.0 })
            return f128_s{ 2.0 };
        return sub_double_finite_inline(2.0, erfc_positive_cf(ax));
    }

    // Keep the short series near zero and switch to the fixed midrange approximation before it gets expensive.
    if (x < f128_s{ 1.0 })
        return sub_double_finite_inline(1.0, erf_positive_series(x));
    if (x < f128_s{ 2.0 })
        return sub_double_finite_inline(1.0, erf_positive_cheb_1_2(x));

    if (x > f128_s{ 27.0 })
        return f128_s{ 0.0 };

    return erfc_positive_cf(x);
}

// gamma functions
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::lgamma(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (isinf(x))
        return std::numeric_limits<f128_s>::infinity();

    if (x > f128_s{ 0.0 })
        return lgamma_positive_recurrence(x);

    const f128_s xi = detail::_f128_impl::trunc(x);
    if (xi == x)
        return std::numeric_limits<f128_s>::infinity();

    const f128_s sinpix = sinpi_reduced(x);
    if (iszero(sinpix))
        return std::numeric_limits<f128_s>::infinity();

    const f128_s out =
        sub_finite_inline(
            sub_finite_inline(detail::_f128_impl::log(std::numbers::pi_v<f128_s>), detail::_f128_impl::log(detail::_f128::mag(sinpix))),
            lgamma_positive_recurrence(sub_double_finite_inline(1.0, x)));

    return out;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::tgamma(const f128_s& x)
{
    using namespace detail::_f128;

    if (isnan(x))
        return x;
    if (isinf(x))
        return signbit(x)
            ? std::numeric_limits<f128_s>::quiet_NaN()
            : std::numeric_limits<f128_s>::infinity();
    if (iszero(x))
        return f128_s{ signbit(x) ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity(), 0.0 };

    if (x > f128_s{ 0.0 })
        return gamma_positive_recurrence(x);

    const f128_s xi = detail::_f128_impl::trunc(x);
    if (xi == x)
        return std::numeric_limits<f128_s>::quiet_NaN();

    const f128_s sinpix = sinpi_reduced(x);
    if (iszero(sinpix))
        return std::numeric_limits<f128_s>::quiet_NaN();

    const f128_s out = div_prechecked_inline(std::numbers::pi_v<f128_s>, mul_product_inline(sinpix, gamma_positive_recurrence(sub_double_finite_inline(1.0, x))));
    return out;
}

} // namespace bl

#endif // F128_DETAIL_MATH_TRANSCENDENTAL_INCLUDED
