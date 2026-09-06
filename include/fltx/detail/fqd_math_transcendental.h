/**
 * fltx/detail/fqd_math_transcendental.h - qd transcendental math implementation details.
 *
 * qd cbrt, exp/log, pow, trig, hyperbolic, erf, and gamma implementations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_DETAIL_MATH_TRANSCENDENTAL_INCLUDED
#define FQD_DETAIL_MATH_TRANSCENDENTAL_INCLUDED
#include "fltx/detail/fqd_math_basic.h"
#include "fltx/fqd_numbers.h"

namespace bl {

[[nodiscard]] BL_MSVC_NOINLINE constexpr double detail::_qd_impl::log_as_double(fqd_s a) noexcept;

namespace detail::_qd // primitives and kernels
{
    // polynomial evaluation helpers
    BL_FORCE_INLINE constexpr fqd_s mul_add_horner_step(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            return mul_add_inline(a, b, c);
        }

        return detail::_qd_runtime::mul_add_horner_step(a, b, c);
    }

    BL_FORCE_INLINE constexpr fqd_s horner_forward_inline(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept
    {
        if (count == 0)
            return {};

        fqd_s p = coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
            p = mul_add_inline(p, x, coeffs[i]);
        return p;
    }

    BL_FORCE_INLINE constexpr fqd_s horner_forward(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            return horner_forward_inline(coeffs, count, x);
        }

        return detail::_qd_runtime::horner_forward(coeffs, count, x);
    }

    BL_FORCE_INLINE constexpr fqd_s horner_reverse_inline(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept
    {
        if (count == 0)
            return {};

        fqd_s p = coeffs[count - 1];
        for (std::size_t i = count - 1; i > 0; --i)
            p = mul_add_inline(p, x, coeffs[i - 1]);
        return p;
    }

    BL_FORCE_INLINE constexpr fqd_s horner_reverse(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            return horner_reverse_inline(coeffs, count, x);
        }

        return detail::_qd_runtime::horner_reverse(coeffs, count, x);
    }

    BL_FORCE_INLINE constexpr void horner_pair_forward_inline(
        const fqd_s* left_coeffs,
        const fqd_s* right_coeffs,
        std::size_t count,
        const fqd_s& x,
        fqd_s& left_out,
        fqd_s& right_out) noexcept
    {
        if (count == 0)
        {
            left_out = fqd_s{};
            right_out = fqd_s{};
            return;
        }

        fqd_s left  = left_coeffs[0];
        fqd_s right = right_coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
        {
            left = mul_add_inline(left, x, left_coeffs[i]);
            right = mul_add_inline(right, x, right_coeffs[i]);
        }

        left_out = left;
        right_out = right;
    }

    BL_FORCE_INLINE constexpr void horner_pair_forward(
        const fqd_s* left_coeffs,
        const fqd_s* right_coeffs,
        std::size_t count,
        const fqd_s& x,
        fqd_s& left_out,
        fqd_s& right_out) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            horner_pair_forward_inline(left_coeffs, right_coeffs, count, x, left_out, right_out);
            return;
        }

        detail::_qd_runtime::horner_pair_forward(left_coeffs, right_coeffs, count, x, left_out, right_out);
    }

    // expm1/log1p functions
    BL_MSVC_NOINLINE constexpr fqd_s expm1_tiny(const fqd_s& r)
    {
        constexpr std::size_t coeff_count = sizeof(exp_inv_fact) / sizeof(exp_inv_fact[0]);
        fqd_s p = horner_reverse(exp_inv_fact, coeff_count, r);
        p = bl::detail::is_constant_evaluated()
            ? mul_add_double_rhs_inline(p, r, 0.5)
            : mul_add_horner_step(p, r, fqd_s{ 0.5 });
        return mul_add_horner_step(sqr_inline(r), p, r);
    }

    BL_MSVC_NOINLINE constexpr fqd_s log1p_series_reduced(const fqd_s& x)
    {
        if (!bl::detail::is_constant_evaluated())
        {
            return detail::_qd_runtime::log1p_series_reduced(x);
        }

        const fqd_s z = div_add_double_inline(x, x, 2.0);
        const fqd_s z2 = sqr_inline(z);

        fqd_s term = z;
        fqd_s sum  = z;

        for (int k = 3; k <= 257; k += 2)
        {
            term = mul_product_inline(term, z2);
            const fqd_s add = div_double_prechecked_inline(term, static_cast<double>(k));
            sum = add_finite_inline(sum, add);

            const fqd_s asum  = mag(sum);
            const fqd_s scale = (asum > fqd_s{ 1.0 }) ? asum : fqd_s{ 1.0 };
            if (mag(add) <= mul_product_inline(convergence_epsilon, scale))
                break;
        }

        return add_finite_inline(sum, sum);
    }

    // exponential functions
    BL_FORCE_INLINE constexpr fqd_s expm1_tiny_fast(const fqd_s& r) noexcept
    {
        constexpr std::size_t coeff_count = sizeof(exp_inv_fact) / sizeof(exp_inv_fact[0]);
        fqd_s p = exp_inv_fact[coeff_count - 1];
        for (std::size_t i = coeff_count - 1; i > 0; --i)
            p = mul_add_inline(p, r, exp_inv_fact[i - 1]);

        p = mul_add_double_rhs_inline(p, r, 0.5);
        return mul_add_inline(sqr_inline(r), p, r);
    }

    BL_FORCE_INLINE constexpr fqd_s exp_integer_factor(int n) noexcept
    {
        if (n == 0)
            return fqd_s{ 1.0 };

        const bool negative = n < 0;
        std::uint32_t exponent = static_cast<std::uint32_t>(negative ? -n : n);
        fqd_s factor{ 1.0 };
        const fqd_s* table = negative ? exp_integer_inv_table : exp_integer_table;
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        const bool checked_product = detail::fp::exp_scale_needs_checked_product(n);
#endif
        for (std::size_t i = 0; exponent != 0 && i < (sizeof(exp_integer_table) / sizeof(exp_integer_table[0])); ++i)
        {
            if ((exponent & 1u) != 0)
            {
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
                factor = checked_product ? mul_product_range_safe_inline(factor, table[i]) : mul_product_inline(factor, table[i]);
#else
                factor = mul_product_inline(factor, table[i]);
#endif
            }
            exponent >>= 1u;
        }

        return factor;
    }

    BL_MSVC_NOINLINE constexpr fqd_s div_refined_once(const fqd_s& a, const fqd_s& b) noexcept
    {
        const fqd_s q = div_prechecked_inline(a, b);
        return add_finite_inline(q, div_prechecked_inline(sub_finite_inline(a, mul_canonical_inline(b, q)), b));
    }

    BL_MSVC_NOINLINE constexpr fqd_s div_refined_once(double a, const fqd_s& b) noexcept
    {
        const fqd_s q = div_double_prechecked_inline(a, b);
        return add_finite_inline(q, div_prechecked_inline(sub_double_finite_inline(a, mul_canonical_inline(b, q)), b));
    }

    BL_FORCE_INLINE constexpr bool exp_overflows(const fqd_s& x) noexcept
    {
        if (x.x0 != exp_overflow_cutoff.x0)
            return x.x0 > exp_overflow_cutoff.x0;
        return x > exp_overflow_cutoff;
    }

    BL_FORCE_INLINE constexpr bool exp_underflows_to_zero(const fqd_s& x) noexcept
    {
        if (x.x0 != exp_zero_cutoff.x0)
            return x.x0 < exp_zero_cutoff.x0;
        return x < exp_zero_cutoff;
    }

    BL_FORCE_INLINE constexpr int exp_reduction_integer(const fqd_s& x) noexcept
    {
        int n = static_cast<int>(round_nearest_even_value(x.x0));
        return n > 709 ? 709 : n;
    }

    BL_MSVC_NOINLINE constexpr fqd_s double_expm1_argument_five_times(fqd_s value) noexcept
    {
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        return value;
    }

    BL_MSVC_NOINLINE constexpr fqd_s double_expm1_argument_twice(fqd_s value) noexcept
    {
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        value = mul_add_inline(value, value, mul_double_product_inline(value, 2.0));
        return value;
    }

    BL_MSVC_NOINLINE constexpr fqd_s exp_general_scaled_with_n(const fqd_s& x, bool sub_one, int n) noexcept
    {
        fqd_s reduced = sub_double_finite_inline(x, static_cast<double>(n));

        const fqd_s r = mul_double_product_inline(reduced, 0.03125);
        fqd_s e = double_expm1_argument_five_times(expm1_tiny_fast(r));

        if (n == 0)
            return sub_one ? e : add_scalar_precise(e, 1.0);

        const fqd_s factor = exp_integer_factor(n);
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        const fqd_s scaled = detail::fp::exp_scale_needs_checked_product(n)
            ? add_finite_inline(factor, mul_product_range_safe_inline(factor, e))
            : mul_add_inline(factor, e, factor);
#else
        const fqd_s scaled = mul_add_inline(factor, e, factor);
#endif
        return sub_one ? add_scalar_precise(scaled, -1.0) : scaled;
    }

    BL_MSVC_NOINLINE constexpr fqd_s exp_general_scaled(const fqd_s& x, bool sub_one) noexcept
    {
        return exp_general_scaled_with_n(x, sub_one, exp_reduction_integer(x));
    }

    BL_MSVC_NOINLINE constexpr fqd_s exp_general_scaled_precise_with_n(const fqd_s& x, bool sub_one, int n) noexcept
    {
        fqd_s reduced = sub_double_finite_inline(x, static_cast<double>(n));

        const fqd_s r = mul_double_product_inline(reduced, 0.0078125);
        fqd_s e = double_expm1_argument_five_times(expm1_tiny_fast(r));
        e = double_expm1_argument_twice(e);

        if (n == 0)
            return sub_one ? e : add_scalar_precise(e, 1.0);

        const fqd_s factor = exp_integer_factor(n);
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        const fqd_s scaled = detail::fp::exp_scale_needs_checked_product(n)
            ? add_finite_inline(factor, mul_product_range_safe_inline(factor, e))
            : mul_add_inline(factor, e, factor);
#else
        const fqd_s scaled = mul_add_inline(factor, e, factor);
#endif
        return sub_one ? add_scalar_precise(scaled, -1.0) : scaled;
    }

    BL_MSVC_NOINLINE constexpr fqd_s exp_general_scaled_precise(const fqd_s& x, bool sub_one) noexcept
    {
        return exp_general_scaled_precise_with_n(x, sub_one, exp_reduction_integer(x));
    }

    BL_MSVC_NOINLINE constexpr fqd_s log1p_newton_small(const fqd_s& frac) noexcept
    {
        const bool constexpr_path = bl::detail::is_constant_evaluated();
        fqd_s x = constexpr_path
            ? fqd_s{ detail::fp::log1p(frac.x0) }
            : fqd_s{ std::log1p(frac.x0) };

        if (constexpr_path)
        {
            const fqd_s em1 = exp_general_scaled(x, true);
            x = add_finite_inline(x, div_prechecked_inline(sub_finite_inline(frac, em1), add_scalar_precise(em1, 1.0)));
            const fqd_s em2 = exp_general_scaled(x, true);
            x = add_finite_inline(x, div_prechecked_inline(sub_finite_inline(frac, em2), add_scalar_precise(em2, 1.0)));
            const fqd_s em3 = exp_general_scaled(x, true);
            x = add_finite_inline(x, div_prechecked_inline(sub_finite_inline(frac, em3), add_scalar_precise(em3, 1.0)));
        }
        else
        {
            for (int i = 0; i < 2; ++i)
            {
                const fqd_s em1 = exp_general_scaled(x, true);
                x = add_finite_inline(x, div_prechecked_inline(sub_finite_inline(frac, em1), add_scalar_precise(em1, 1.0)));
            }
        }

        return x;
    }

    BL_MSVC_NOINLINE constexpr fqd_s exp_from_reduced_64(const fqd_s& x, bool base2) noexcept
    {
        const fqd_s t = base2 ? x : mul_product_inline(x, std::numbers::log2e_v<fqd_s>);
        const int m = static_cast<int>(round_nearest_even_value(t.x0 * 64.0));
        int n = m / 64;
        int j = m - n * 64;
        if (j < 0)
        {
            j += 64;
            --n;
        }

        const fqd_s reduced = base2
            ? sub_double_finite_inline(x, static_cast<double>(n) + static_cast<double>(j) / 64.0)
            : sub_finite_inline(x, mul_double_product_inline(std::numbers::ln2_v<fqd_s>, static_cast<double>(n) + static_cast<double>(j) / 64.0));

        const fqd_s r = mul_double_product_inline(base2 ? mul_product_inline(reduced, std::numbers::ln2_v<fqd_s>) : reduced, 0.25);
        fqd_s e = expm1_tiny_fast(r);
        e = mul_add_inline(e, e, mul_double_product_inline(e, 2.0));
        e = mul_add_inline(e, e, mul_double_product_inline(e, 2.0));

        return _ldexp(mul_product_inline(exp2_table_64[j], add_scalar_precise(e, 1.0)), n);
    }

    // logarithm functions
    BL_MSVC_NOINLINE constexpr fqd_s log_with_fast_exp_correction(const fqd_s& a) noexcept
    {
        if (isnan(a))
            return a;
        if (iszero(a))
            return fqd_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
        if (a.x0 < 0.0 || (a.x0 == 0.0 && (a.x1 < 0.0 || (a.x1 == 0.0 && (a.x2 < 0.0 || (a.x2 == 0.0 && a.x3 < 0.0))))))
            return std::numeric_limits<fqd_s>::quiet_NaN();
        if (isinf(a))
            return a;

        const fqd_s frac = sub_double_finite_inline(a, 1.0);
        if (mag(frac) < fqd_s{ 0.5 })
            return log1p_newton_small(frac);

        fqd_s scaled_a = a;
        int exp2_adjust = 0;
        if (detail::fp::absd(a.x0) < std::numeric_limits<double>::min())
        {
            scaled_a = ldexp_terms(a, 64);
            exp2_adjust = -64;
        }

        const int mantissa_exp2 = detail::fp::frexp_exponent_limb(scaled_a.x0);
        int exp2 = mantissa_exp2 + exp2_adjust;

        fqd_s m = _ldexp(scaled_a, -mantissa_exp2);
        if (m < sqrt_half)
        {
            m *= 2.0;
            --exp2;
        }
        if (m < fqd_s{ 1.0 })
        {
            m *= 2.0;
            --exp2;
        }

        double log2_m{};
        if (bl::detail::is_constant_evaluated())
        {
            log2_m = detail::fp::log(m.x0) * 1.4426950408889634074;
        }
        else
        {
            log2_m = std::log2(m.x0);
        }

        int j = static_cast<int>(round_nearest_even_value(log2_m * 64.0));
        if (j < 0)
            j = 0;
        else if (j > 64)
            j = 64;

        const fqd_s c = (j == 64) ? fqd_s{ 2.0 } : exp2_table_64[j];
        const fqd_s u = div_prechecked_inline(sub_finite_inline(m, c), add_finite_inline(m, c));
        const fqd_s u2 = sqr_inline(u);

        fqd_s p = log_atanh_coeffs[11];
        for (std::size_t i = 11; i > 0; --i)
            p = mul_add_inline(p, u2, log_atanh_coeffs[i - 1]);

        const fqd_s log_m = add_finite_inline(mul_product_inline(u, p), ln2_table_64[j]);
        fqd_s y = add_mul_double_inline(
            log_m,
            std::numbers::ln2_v<fqd_s>,
            static_cast<double>(exp2));
        if (exp2 >= -96 && exp2 <= 96)
        {
            for (int i = 0; i < 3; ++i)
            {
                const fqd_s correction = mul_add_double_rhs_inline(a, exp_general_scaled(-y, false), -1.0);
                y = add_finite_inline(y, correction);
                if (mag(correction) <= mul_product_inline(convergence_epsilon, mag(y)))
                    break;
            }
        }
        else
        {
            const fqd_s correction = mul_add_double_rhs_inline(m, exp_general_scaled(-log_m, false), -1.0);
            y = add_finite_inline(y, correction);
        }
        return y;
    }

    BL_NO_INLINE constexpr fqd_s exp_for_pow(const fqd_s& x) noexcept
    {
        if (isnan(x))
            return x;
        if (isinf(x))
            return (x.x0 < 0.0) ? fqd_s{ 0.0 } : std::numeric_limits<fqd_s>::infinity();
        if (exp_overflows(x))
            return std::numeric_limits<fqd_s>::infinity();
        if (exp_underflows_to_zero(x))
            return fqd_s{ 0.0 };
        if (iszero(x))
            return fqd_s{ 1.0 };

        return exp_general_scaled_precise(x, false);
    }

    BL_MSVC_NOINLINE constexpr fqd_s _exp(const fqd_s& x)
    {
        if (isnan(x))
            return x;
        if (isinf(x))
            return (x.x0 < 0.0) ? fqd_s{ 0.0 } : std::numeric_limits<fqd_s>::infinity();

        if (exp_overflows(x))
            return std::numeric_limits<fqd_s>::infinity();

        if (exp_underflows_to_zero(x))
            return fqd_s{ 0.0 };

        if (iszero(x))
            return fqd_s{ 1.0 };

        return exp_general_scaled(x, false);
    }

    BL_MSVC_NOINLINE constexpr fqd_s _exp2(const fqd_s& x)
    {
        if (isnan(x))
            return x;
        if (isinf(x))
            return (x.x0 < 0.0) ? fqd_s{ 0.0 } : std::numeric_limits<fqd_s>::infinity();

        if (x.x0 > 1023.0 || x.x0 < -1074.0)
            return _exp(mul_product_inline(x, std::numbers::ln2_v<fqd_s>));

        if (iszero(x))
            return fqd_s{ 1.0 };

        return exp_from_reduced_64(x, true);
    }

    BL_MSVC_NOINLINE constexpr fqd_s _log(const fqd_s& a)
    {
        if (isnan(a))
            return a;
        if (iszero(a))
            return fqd_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
        if (signbit(a))
            return std::numeric_limits<fqd_s>::quiet_NaN();
        if (isinf(a))
            return a;

        return log_with_fast_exp_correction(a);
    }

    BL_MSVC_NOINLINE constexpr fqd_s _expm1(const fqd_s& x)
    {
        if (isnan(x))
            return x;
        if (x == fqd_s{ 0.0 })
            return x;
        if (isinf(x))
            return signbit(x.x0)
                ? fqd_s{ -1.0, 0.0, 0.0, 0.0 }
                : std::numeric_limits<fqd_s>::infinity();

        if (exp_overflows(x))
            return std::numeric_limits<fqd_s>::infinity();

        if (exp_underflows_to_zero(x))
            return fqd_s{ -1.0, 0.0, 0.0, 0.0 };

        return exp_general_scaled(x, true);
    }

    BL_FORCE_INLINE constexpr bool qd_try_exact_binary_log2(const fqd_s& x, int& out) noexcept
    {
        if (!(x.x0 > 0.0) || x.x1 != 0.0 || x.x2 != 0.0 || x.x3 != 0.0)
            return false;

        const std::uint64_t bits = std::bit_cast<std::uint64_t>(x.x0);
        const std::uint32_t exp_bits = static_cast<std::uint32_t>((bits >> 52) & 0x7ffu);
        const std::uint64_t frac_bits = bits & ((std::uint64_t{ 1 } << 52) - 1);

        if (exp_bits == 0 || exp_bits == 0x7ffu || frac_bits != 0)
            return false;

        out = static_cast<int>(exp_bits) - 1023;
        return true;
    }

    // power functions
    [[nodiscard]] BL_FORCE_INLINE constexpr bool qd_try_pow10_ldexp_chunks(
        int pow5_count,
        int binary_exponent_per_input_exponent,
        int exponent,
        fqd_s& out) noexcept
    {
        if (pow5_count <= 0)
            return false;
        if (exponent == 0)
        {
            out = fqd_s{ 1.0 };
            return true;
        }

        const int chunk_limit = exponent > 0
            ? detail::_qd::pow10_qd_max_exponent / pow5_count
            : (-detail::_qd::pow10_qd_min_exponent) / pow5_count;
        if (chunk_limit <= 0)
            return false;

        fqd_s value{ 1.0 };
        int remaining = exponent;
        while (remaining != 0)
        {
            const int chunk = remaining > 0
                ? ((remaining > chunk_limit) ? chunk_limit : remaining)
            : ((remaining < -chunk_limit) ? -chunk_limit : remaining);

            int decimal_exponent = 0;
            if (!detail::fp::checked_exponent_product(pow5_count, chunk, decimal_exponent))
                return false;
            if (decimal_exponent < detail::_qd::pow10_qd_min_exponent ||
                decimal_exponent > detail::_qd::pow10_qd_max_exponent)
            {
                return false;
            }

            int binary_exponent = 0;
            if (!detail::fp::checked_exponent_product(binary_exponent_per_input_exponent, chunk, binary_exponent))
                return false;

            const fqd_s term = detail::_qd_impl::ldexp(
                detail::_qd_impl::pow10_fqd(decimal_exponent),
                binary_exponent);
            value = mul_product_inline(value, term);
            if (detail::fp::isinf_or_nan(value.x0))
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
        fqd_s& out)
    {
        int exponent = 0;
        if (!detail::fp::try_int_exponent(y, exponent))
            return false;

        fqd_s value{};
        if (pow10_base)
        {
            value = detail::_qd_impl::pow10_fqd(exponent);
        }
        else
        {
            int binary_exponent = 0;
            if (!detail::fp::checked_exponent_product(pow2_log2, exponent, binary_exponent))
                return false;

            if (binary_exponent >= std::numeric_limits<double>::max_exponent)
                value = std::numeric_limits<fqd_s>::infinity();
            else if (binary_exponent < std::numeric_limits<double>::min_exponent - std::numeric_limits<double>::digits)
                value = fqd_s{ 0.0 };
            else
                value = detail::_qd_impl::ldexp(fqd_s{ 1.0 }, binary_exponent);
        }

        out = (negative_base && detail::fp::is_odd_integral(y)) ? -value : value;
        return true;
    }

    template<detail::fp::non_bool_integral Base, detail::fp::non_bool_integral Exp>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_integral_pow_finite_raw(Base x, Exp y, fqd_s& out) noexcept
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
        fqd_s value{};
        if (decimal_exponent >= detail::_qd::pow10_qd_min_exponent &&
            decimal_exponent <= detail::_qd::pow10_qd_max_exponent)
        {
            int binary_exponent = 0;
            if (!detail::fp::checked_exponent_product(binary_exponent_per_input_exponent, exponent, binary_exponent))
                return false;

            value = detail::_qd_impl::ldexp(
                detail::_qd_impl::pow10_fqd(decimal_exponent),
                binary_exponent);
        }
        else if (!qd_try_pow10_ldexp_chunks(
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

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s pow_mul_adaptive(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::fp::dekker_product_needs_scaling(a.x0, b.x0)
            ? mul_dekker_range_safe_canonical_inline(a, b)
            : mul_canonical_inline(a, b);
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s pow_sqr_adaptive(const fqd_s& a) noexcept
    {
        return detail::fp::dekker_product_needs_scaling(a.x0, a.x0)
            ? mul_dekker_range_safe_canonical_inline(a, a)
            : sqr_inline(a);
    }

    template<class ExpUnsigned>
    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s powi_nonnegative_checked(fqd_s base, ExpUnsigned exp) noexcept
    {
        if (exp == ExpUnsigned{ 0 })
            return fqd_s{ 1.0 };
        if (exp == ExpUnsigned{ 1 })
            return base;
        if (exp == ExpUnsigned{ 2 })
            return pow_sqr_adaptive(base);
        if (exp == ExpUnsigned{ 3 })
        {
            const fqd_s squared = pow_sqr_adaptive(base);
            return pow_mul_adaptive(squared, base);
        }
        if (exp == ExpUnsigned{ 4 })
        {
            return pow_sqr_adaptive(pow_sqr_adaptive(base));
        }

        fqd_s result{ 1.0 };
        while (exp != ExpUnsigned{ 0 })
        {
            if ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 })
                result = pow_mul_adaptive(result, base);

            exp >>= 1;
            if (exp != ExpUnsigned{ 0 })
                base = pow_sqr_adaptive(base);
        }

        return result;
    }

    template<class ExpUnsigned>
    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s powi_nonnegative(fqd_s base, ExpUnsigned exp) noexcept
    {
        if (exp == ExpUnsigned{ 0 })
            return fqd_s{ 1.0 };
        if (exp == ExpUnsigned{ 1 })
            return base;
        if (exp == ExpUnsigned{ 2 })
            return sqr_inline(base);
        if (exp == ExpUnsigned{ 3 })
            return mul_canonical_inline(sqr_inline(base), base);
        if (exp == ExpUnsigned{ 4 })
            return sqr_inline(sqr_inline(base));

        fqd_s result{ 1.0 };
        while (exp != ExpUnsigned{ 0 })
        {
            if ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 })
                result = mul_canonical_inline(result, base);

            exp >>= 1;
            if (exp != ExpUnsigned{ 0 })
                base = sqr_inline(base);
        }

        return result;
    }

    template<class ExpUnsigned>
    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s powi_nonnegative_unchecked(fqd_s base, ExpUnsigned exp) noexcept
    {
        if (exp == ExpUnsigned{ 0 })
            return fqd_s{ 1.0 };
        if (exp == ExpUnsigned{ 1 })
            return base;
        if (exp == ExpUnsigned{ 2 })
            return sqr_inline(base);
        if (exp == ExpUnsigned{ 3 })
            return mul_product_inline(sqr_inline(base), base);
        if (exp == ExpUnsigned{ 4 })
            return sqr_inline(sqr_inline(base));

        fqd_s result{ 1.0 };
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
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s powi_nonnegative_fast(fqd_s base, ExpUnsigned exp) noexcept
    {
        if (detail::fp::ipow_loop_needs_range_safe_dekker(base.x0, exp)) [[unlikely]]
            return powi_nonnegative_checked(base, exp);
        return powi_nonnegative(base, exp);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s reciprocal_pow_result(const fqd_s& value) noexcept
    {
        return fqd_s{ 1.0 } / value;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_exact_integer_pow_base_value(const fqd_s& value, std::int64_t& out) noexcept
    {
        if (value.x1 != 0.0 || value.x2 != 0.0 || value.x3 != 0.0 || !detail::fp::isfinite(value.x0))
            return false;

        const bool negative = value.x0 < 0.0;
        const double magnitude = negative ? -value.x0 : value.x0;
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
    [[nodiscard]] BL_MSVC_NOINLINE constexpr bool try_exact_integer_pow_base(const fqd_s& x, Exp y, fqd_s& out) noexcept
    {
        std::int64_t integer_base = 0;
        if (!try_exact_integer_pow_base_value(x, integer_base))
            return false;

        if (detail::_qd::try_integral_pow_finite_raw(integer_base, y, out))
            return true;

        using U = std::make_unsigned_t<std::remove_cvref_t<Exp>>;
        const U magnitude = detail::fp::unsigned_abs(y);
        const bool split_safe = detail::fp::integral_pow_split_safe(integer_base, magnitude) ||
            !detail::fp::ipow_loop_needs_range_safe_dekker(x.x0, magnitude);

        if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
        {
            if (y < 0)
            {
                if (detail::fp::integral_pow_reciprocal_underflows_binary64(integer_base, magnitude))
                {
                    const bool negative_zero = detail::fp::negative_integral_pow_result_is_negative(integer_base, magnitude);
                    out = detail::_qd::signed_zero(negative_zero);
                    return true;
                }

                if (split_safe)
                {
                    const fqd_s powered = detail::_qd::powi_nonnegative_unchecked(x, magnitude);
                    out = detail::_qd::reciprocal_pow_result(powered);
                    return true;
                }

                const fqd_s reciprocal = detail::_qd::reciprocal_pow_result(x);
                out = (integer_base != 0)
                    ? detail::_qd::powi_nonnegative_unchecked(reciprocal, magnitude)
                    : detail::_qd::powi_nonnegative(reciprocal, magnitude);
                return true;
            }
        }

        out = split_safe
            ? detail::_qd::powi_nonnegative_unchecked(x, magnitude)
            : detail::_qd::powi_nonnegative_fast<U>(x, magnitude);
        return true;
    }

    template<detail::fp::non_bool_integral Exp>
    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s ipow_integer(const fqd_s& x, Exp y)
    {
        using U = std::make_unsigned_t<std::remove_cvref_t<Exp>>;
        const U magnitude = detail::fp::unsigned_abs(y);

        if (magnitude == U{0})
            return fqd_s{1.0};
        if (isnan(x))
            return x;
        if (isinf(x))
        {
            const bool negative = signbit(x) && (magnitude & U{1}) != U{0};
            if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
            {
                if (y < 0)
                    return detail::_qd::signed_zero(negative);
            }
            return fqd_s{
                negative
                    ? -std::numeric_limits<double>::infinity()
                    : std::numeric_limits<double>::infinity()};
        }

        fqd_s special{};
        if (detail::_qd::try_exact_integer_pow_base(x, y, special))
            return special;

        if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
        {
            if (y < 0)
            {
                const fqd_s reciprocal = detail::_qd::reciprocal_pow_result(x);
                return detail::_qd::powi_nonnegative(reciprocal, magnitude);
            }
        }

        return detail::_qd::powi_nonnegative_fast<U>(x, magnitude);
    }

    BL_MSVC_NOINLINE constexpr fqd_s polish_eighth_root(const fqd_s& x, const fqd_s& y)
    {
        if (iszero(y))
            return y;

        const fqd_s y2 = sqr_inline(y);
        const fqd_s y4 = sqr_inline(y2);
        const fqd_s y7 = mul_product_inline(mul_product_inline(y4, y2), y);
        const fqd_s y8 = sqr_inline(y4);
        const fqd_s correction = div_double_prechecked_inline(div_prechecked_inline(sub_finite_inline(x, y8), y7), 8.0);

        return add_finite_inline(y, correction);
    }

    BL_MSVC_NOINLINE constexpr fqd_s pow_positive_eighth_fraction(const fqd_s& x, int numerator)
    {
        const fqd_s r2 = detail::_qd_impl::sqrt(x);
        if (numerator == 4)
            return r2;

        const fqd_s r4 = detail::_qd_impl::sqrt(r2);
        if (numerator == 2)
            return r4;

        fqd_s out{ 1.0 };
        if ((numerator & 4) != 0)
            out = mul_product_inline(out, r2);
        if ((numerator & 2) != 0)
            out = mul_product_inline(out, r4);
        if ((numerator & 1) != 0)
        {
            const fqd_s r8 = polish_eighth_root(x, detail::_qd_impl::sqrt(r4));
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
        const uint64_t magnitude = neg ? static_cast<uint64_t>(-n) : static_cast<uint64_t>(n);
        return magnitude <= 1024;
    }

    BL_FORCE_INLINE constexpr bool try_get_pow_dyadic_eighth_exponent(const fqd_s& x, const fqd_s& y, int64_t& n)
    {
        if (x.x0 < 0.0 || (x.x0 == 0.0 && signbit(x.x0)))
            return false;

        if (!try_get_int64(mul_double_product_inline(y, 8.0), n))
            return false;

        return pow_dyadic_eighth_exponent_in_range(n);
    }

    BL_FORCE_INLINE constexpr bool try_get_pow_dyadic_eighth_exponent(const fqd_s& x, double y, int64_t& n) noexcept
    {
        if (x.x0 < 0.0 || (x.x0 == 0.0 && signbit(x.x0)))
            return false;

        const double scaled = y * 8.0;
        if (detail::fp::isinf_or_nan(scaled) || absd(scaled) >= 0x1p63)
            return false;

        const double rounded = trunc(scaled);
        if (rounded != scaled)
            return false;

        n = static_cast<int64_t>(rounded);
        return pow_dyadic_eighth_exponent_in_range(n);
    }

    BL_NO_INLINE constexpr fqd_s pow_dyadic_eighth_unchecked(const fqd_s& x, int64_t n)
    {
        if (n == 0)
            return fqd_s{ 1.0 };

        const bool neg = n < 0;
        const uint64_t magnitude = neg ? static_cast<uint64_t>(-n) : static_cast<uint64_t>(n);
        const uint64_t whole     = magnitude / 8u;
        const int rem = static_cast<int>(magnitude & 7u);

        fqd_s result = (whole == 0u) ? fqd_s{ 1.0 } : detail::fp::powi_by_squaring(x, static_cast<int64_t>(whole));
        if (rem != 0)
            result = mul_product_inline(result, pow_positive_eighth_fraction(x, rem));
        if (neg)
            result = recip(result);

        return result;
    }

    BL_FORCE_INLINE constexpr biguint biguint_from_fmod_u320(const fmod_u320& value)
    {
        biguint out{};
        static_assert(biguint::max_words >= 10);

        for (int i = 0; i < 5; ++i)
        {
            out.words[2 * i] = static_cast<std::uint32_t>(value.word[i]);
            out.words[2 * i + 1] = static_cast<std::uint32_t>(value.word[i] >> 32);
        }
        out.size = 10;
        out.trim();
        return out;
    }

    BL_NO_INLINE constexpr bool remainder_pi2_payne_hanek(const fqd_s& x, long long& n_out, fqd_s& r_out)
    {
        const bool neg = signbit(x.x0);
        const exact_dyadic_fmod_fixed dx = exact_from_qd_fmod_fixed(mag(x));
        if (fmod_u320_is_zero(dx.mant))
        {
            n_out = 0;
            r_out = neg ? fqd_s{ -0.0, 0.0, 0.0, 0.0 } : fqd_s{ 0.0, 0.0, 0.0, 0.0 };
            return true;
        }

        // Retaining 1888 bits of 2/pi leaves more than 650 guard bits after
        // reducing the largest finite qd input. Dropping the lowest five
        // words also lets the full 320-bit exact expansion accumulator fit in
        // the compact shared big integer; truncating its high product words
        // would corrupt the quotient quadrant near multiples of pi/2.
        constexpr int omitted_low_words = 5;
        constexpr int fixed_bits =
            detail::trig_reduce::two_over_pi_fixed_bits - omitted_low_words * 32;
        constexpr int word_count =
            static_cast<int>(sizeof(detail::trig_reduce::two_over_pi_fixed_words)
                / sizeof(detail::trig_reduce::two_over_pi_fixed_words[0]))
            - omitted_low_words;
        const biguint two_over_pi = from_words(
            detail::trig_reduce::two_over_pi_fixed_words + omitted_low_words,
            word_count);
        static_assert(word_count <= biguint::max_words);
        static_assert(
            biguint::max_words * 32 >=
            fixed_bits + static_cast<int>(sizeof(fmod_u320) * 8));

        const biguint product = mul_big(biguint_from_fmod_u320(dx.mant), two_over_pi);
        const int scale_bits = fixed_bits - dx.exp2;
        if (scale_bits <= 0)
            return false;

        const biguint rem = low_bits_copy(product, scale_bits);
        const bool half_bit = rem.get_bit(scale_bits - 1);
        const bool sticky = any_low_bits_set(rem, scale_bits - 1);
        const bool integer_odd = product.get_bit(scale_bits);
        const bool round_up = half_bit && (sticky || integer_odd);

        unsigned n_mod4 =
            (product.get_bit(scale_bits) ? 1u : 0u) |
            (product.get_bit(scale_bits + 1) ? 2u : 0u);
        if (round_up)
            n_mod4 = (n_mod4 + 1u) & 3u;

        biguint y_coeff = rem;
        bool y_neg = false;
        if (round_up)
        {
            y_coeff.clear();
            y_coeff.set_bit(scale_bits);
            y_coeff.sub_inplace(rem);
            y_neg = !y_coeff.is_zero();
        }

        fqd_s r = exact_dyadic_to_qd_fmod(y_coeff, -scale_bits, y_neg);
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

        if (neg)
        {
            r = -r;
            n_mod4 = (4u - n_mod4) & 3u;
        }

        n_out = static_cast<long long>(n_mod4);
        r_out = r;
        return true;
    }

    // sine/cosine functions

    BL_MSVC_NOINLINE constexpr bool remainder_pi2(const fqd_s& x, long long& n_out, fqd_s& r_out)
    {
        if (detail::fp::isinf_or_nan(x.x0))
            return false;

        if (mag(x) <= pi_4)
        {
            n_out = 0;
            r_out = x;
            return true;
        }

        const fqd_s q = detail::_qd_impl::round_nearest_even(mul_product_inline(x, invpi2));
        const double qd = q.x0;

        if (detail::fp::isinf_or_nan(qd) || detail::fp::absd(qd) >= 0x1p52 ||
            q.x1 != 0.0 || q.x2 != 0.0 || q.x3 != 0.0)
        {
            return remainder_pi2_payne_hanek(x, n_out, r_out);
        }

        long long n = (long long)qd;
        fqd_s r = x;
        r = sub_mul_double_inline(r, q, pi_2.x0);
        r = sub_mul_double_inline(r, q, pi_2.x1);
        r = sub_mul_double_inline(r, q, pi_2.x2);
        r = sub_mul_double_inline(r, q, pi_2.x3);
        r = sub_mul_double_inline(r, q, pi_2_tail_4);

        if (r > pi_4)
        {
            r = sub_finite_inline(r, pi_2);
            ++n;
        }
        else if (r < -pi_4)
        {
            r = add_finite_inline(r, pi_2);
            --n;
        }

        // The split-constant path is deliberately cheap. Very close to a
        // multiple of pi/2, however, cancellation consumes its guard digits;
        // use the exact reducer only for that rare case.
        if (detail::fp::absd(r.x0) <= detail::fp::absd(x.x0) * 0x1p-80)
            return remainder_pi2_payne_hanek(x, n_out, r_out);

        n_out = n;
        r_out = r;
        return true;
    }

    #if FLTX_FQD_ENABLE_SIMD
    BL_FORCE_INLINE constexpr fqd_s mul_from_two_prod_terms(
        double p0, double p1, double p2, double p3, double p4, double p5,
        double p6, double p7, double p8, double p9,
        double q0, double q1, double q2, double q3, double q4, double q5,
        double q6, double q7, double q8, double q9,
        double tail_mul0, double tail_mul1, double tail_mul2) noexcept
    {
        double r0{}, r1{};
        double t0{}, t1{};
        double s0{}, s1{}, s2{};

        three_sum(p1, p2, q0);
        three_sum(p2, q1, q2);
        three_sum(p3, p4, p5);

        two_sum_precise(p2, p3, s0, t0);
        two_sum_precise(q1, p4, s1, t1);
        s2 = q2 + p5;
        two_sum_precise(s1, t0, s1, t0);
        s2 += (t0 + t1);

        two_sum_precise(q0, q3, q0, q3);
        two_sum_precise(q4, q5, q4, q5);
        two_sum_precise(p6, p7, p6, p7);
        two_sum_precise(p8, p9, p8, p9);

        two_sum_precise(q0, q4, t0, t1);
        t1 += (q3 + q5);

        two_sum_precise(p6, p8, r0, r1);
        r1 += (p7 + p9);

        two_sum_precise(t0, r0, q3, q4);
        q4 += (t1 + r1);

        two_sum_precise(q3, s1, t0, t1);
        t1 += q4;

        t1 += tail_mul0 + tail_mul1 + tail_mul2
            + q6 + q7 + q8 + q9 + s2;

        return renorm5(p0, p1, s0, t0, t1);
    }

    BL_FORCE_INLINE void mul_pair_simd(
        const fqd_s& a0, const fqd_s& b0,
        const fqd_s& a1, const fqd_s& b1,
        fqd_s& out0, fqd_s& out1) noexcept
    {
        double p00{}, p10{}, p20{}, p30{}, p40{}, p50{};
        double q00{}, q10{}, q20{}, q30{}, q40{}, q50{};

        double p01{}, p11{}, p21{}, p31{}, p41{}, p51{};
        double q01{}, q11{}, q21{}, q31{}, q41{}, q51{};

        two_prod_precise(a0.x0, b0.x0, p00, q00);
        two_prod_precise(a0.x0, b0.x1, p10, q10);
        two_prod_precise(a0.x1, b0.x0, p20, q20);
        two_prod_precise(a0.x0, b0.x2, p30, q30);
        two_prod_precise(a0.x1, b0.x1, p40, q40);
        two_prod_precise(a0.x2, b0.x0, p50, q50);

        two_prod_precise(a1.x0, b1.x0, p01, q01);
        two_prod_precise(a1.x0, b1.x1, p11, q11);
        two_prod_precise(a1.x1, b1.x0, p21, q21);
        two_prod_precise(a1.x0, b1.x2, p31, q31);
        two_prod_precise(a1.x1, b1.x1, p41, q41);
        two_prod_precise(a1.x2, b1.x0, p51, q51);

        const simd::f64x2 ax0 = simd::f64x2_set(a0.x0, a1.x0);
        const simd::f64x2 ax1 = simd::f64x2_set(a0.x1, a1.x1);
        const simd::f64x2 ax2 = simd::f64x2_set(a0.x2, a1.x2);
        const simd::f64x2 ax3 = simd::f64x2_set(a0.x3, a1.x3);

        const simd::f64x2 bx0 = simd::f64x2_set(b0.x0, b1.x0);
        const simd::f64x2 bx1 = simd::f64x2_set(b0.x1, b1.x1);
        const simd::f64x2 bx2 = simd::f64x2_set(b0.x2, b1.x2);
        const simd::f64x2 bx3 = simd::f64x2_set(b0.x3, b1.x3);

        simd::f64x2 p6{}, p7{}, p8{}, p9{};
        simd::f64x2 q6{}, q7{}, q8{}, q9{};

        simd::f64x2_two_prod_precise(ax0, bx3, p6, q6);
        simd::f64x2_two_prod_precise(ax1, bx2, p7, q7);
        simd::f64x2_two_prod_precise(ax2, bx1, p8, q8);
        simd::f64x2_two_prod_precise(ax3, bx0, p9, q9);

        alignas(16) double p6v[2], p7v[2], p8v[2], p9v[2];
        alignas(16) double q6v[2], q7v[2], q8v[2], q9v[2];

        simd::f64x2_store_array(p6, p6v);
        simd::f64x2_store_array(p7, p7v);
        simd::f64x2_store_array(p8, p8v);
        simd::f64x2_store_array(p9, p9v);
        simd::f64x2_store_array(q6, q6v);
        simd::f64x2_store_array(q7, q7v);
        simd::f64x2_store_array(q8, q8v);
        simd::f64x2_store_array(q9, q9v);

        out0 = mul_from_two_prod_terms(
            p00, p10, p20, p30, p40, p50,
            p6v[0], p7v[0], p8v[0], p9v[0],
            q00, q10, q20, q30, q40, q50,
            q6v[0], q7v[0], q8v[0], q9v[0],
            a0.x1 * b0.x3, a0.x2 * b0.x2, a0.x3 * b0.x1
        );

        out1 = mul_from_two_prod_terms(
            p01, p11, p21, p31, p41, p51,
            p6v[1], p7v[1], p8v[1], p9v[1],
            q01, q11, q21, q31, q41, q51,
            q6v[1], q7v[1], q8v[1], q9v[1],
            a1.x1 * b1.x3, a1.x2 * b1.x2, a1.x3 * b1.x1
        );
    }
    #endif

    BL_MSVC_NOINLINE constexpr fqd_s sin_kernel_pi4_inline(const fqd_s& r)
    {
        if (detail::fp::absd(r.x0) < 0x1p-107)
            return r;

        const fqd_s t = sqr_inline(r);

        const fqd_s ps = horner_forward_inline(qd_sin_coeffs_pi4, qd_trig_coeff_count_pi4, t);

        return mul_add_inline(mul_product_inline(r, t), ps, r);
    }

    BL_MSVC_NOINLINE constexpr fqd_s cos_kernel_pi4_inline(const fqd_s& r)
    {
        const fqd_s t = sqr_inline(r);

        const fqd_s pc = horner_forward_inline(qd_cos_coeffs_pi4, qd_trig_coeff_count_pi4, t);

        return mul_add_double_rhs_inline(t, pc, 1.0);
    }

    BL_MSVC_NOINLINE constexpr void sincos_kernel_pi4_inline(const fqd_s& r, fqd_s& s_out, fqd_s& c_out)
    {
        if (detail::fp::absd(r.x0) < 0x1p-107)
        {
            s_out = r;
            c_out = fqd_s{ 1.0 };
            return;
        }

        const fqd_s t = sqr_inline(r);

        fqd_s ps{};
        fqd_s pc{};
        horner_pair_forward_inline(qd_sin_coeffs_pi4, qd_cos_coeffs_pi4, qd_trig_coeff_count_pi4, t, ps, pc);

        s_out = mul_add_inline(mul_product_inline(r, t), ps, r);
        c_out = mul_add_double_rhs_inline(t, pc, 1.0);
    }

    BL_MSVC_NOINLINE constexpr void sincos_kernel_small(const fqd_s& r, fqd_s& s_out, fqd_s& c_out)
    {
        if (detail::fp::absd(r.x0) < 0x1p-107)
        {
            s_out = r;
            c_out = fqd_s{ 1.0 };
            return;
        }

        const fqd_s t = sqr_inline(r);

        fqd_s ps{};
        fqd_s pc{};
        horner_pair_forward(
            qd_sin_coeffs_pi4 + qd_trig_small_coeff_offset,
            qd_cos_coeffs_pi4 + qd_trig_small_coeff_offset,
            qd_trig_small_coeff_count,
            t,
            ps,
            pc);

        const fqd_s rt = mul_product_inline(r, t);
        s_out = mul_add_horner_step(rt, ps, r);
        c_out = bl::detail::is_constant_evaluated()
            ? mul_add_double_rhs_inline(t, pc, 1.0)
            : mul_add_horner_step(t, pc, fqd_s{ 1.0 });
    }

    BL_MSVC_NOINLINE constexpr void sincos_kernel_pi64_reduced(const fqd_s& r, fqd_s& s_out, fqd_s& c_out)
    {
        int k = static_cast<int>(round_nearest_even_value(r.x0 * 20.371832715762604));
        if (k < -16)
            k = -16;
        else if (k > 16)
            k = 16;

        if (k == 0)
        {
            sincos_kernel_small(r, s_out, c_out);
            return;
        }

        const fqd_s a = mul_double_product_inline(std::numbers::pi_v<fqd_s>, static_cast<double>(k) * 0.015625);
        const fqd_s u = sub_finite_inline(r, a);

        fqd_s su{};
        fqd_s cu{};
        sincos_kernel_small(u, su, cu);

        const int table_index = k < 0 ? -k : k;
        const fqd_s sa = k < 0 ? -qd_sin_table_pi64[table_index] : qd_sin_table_pi64[table_index];
        const fqd_s ca = qd_cos_table_pi64[table_index];

        s_out = mul_add_mul_inline(ca, su, sa, cu);
        c_out = mul_sub_mul_inline(ca, cu, sa, su);
    }

    BL_MSVC_NOINLINE constexpr fqd_s sin_kernel_pi4(const fqd_s& r)
    {
        if (detail::fp::absd(r.x0) < 0x1p-107)
            return r;

        if (bl::detail::is_constant_evaluated())
        {
            return sin_kernel_pi4_inline(r);
        }

        const fqd_s t = sqr_inline(r);
        const fqd_s ps = horner_forward(qd_sin_coeffs_pi4, qd_trig_coeff_count_pi4, t);

        const fqd_s rt = mul_product_inline(r, t);
        return mul_add_horner_step(rt, ps, r);
    }

    BL_MSVC_NOINLINE constexpr fqd_s cos_kernel_pi4(const fqd_s& r)
    {
        if (bl::detail::is_constant_evaluated())
        {
            return cos_kernel_pi4_inline(r);
        }

        const fqd_s t = sqr_inline(r);
        const fqd_s pc = horner_forward(qd_cos_coeffs_pi4, qd_trig_coeff_count_pi4, t);

        return bl::detail::is_constant_evaluated()
            ? mul_add_double_rhs_inline(t, pc, 1.0)
            : mul_add_horner_step(t, pc, fqd_s{ 1.0 });
    }

    BL_MSVC_NOINLINE constexpr bool _sincos(const fqd_s& x, fqd_s& s_out, fqd_s& c_out)
    {
        const double ax = fabs(x.x0);
        if (detail::fp::isinf_or_nan(ax))
        {
            s_out = fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };
            c_out = s_out;
            return false;
        }

        if (ax <= static_cast<double>(pi_4))
        {
            sincos_kernel_pi64_reduced(x, s_out, c_out);
            return true;
        }

        long long n = 0;
        fqd_s r{};
        if (!remainder_pi2(x, n, r))
        {
            s_out = fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };
            c_out = s_out;
            return false;
        }

        fqd_s sr{}, cr{};
        sincos_kernel_pi64_reduced(r, sr, cr);

        switch ((int)(n & 3LL))
        {
        case 0: s_out = sr;  c_out = cr;  break;
        case 1: s_out = cr;  c_out = -sr; break;
        case 2: s_out = -sr; c_out = -cr; break;
        default: s_out = -cr; c_out = sr;  break;
        }

        return true;
    }

    // inverse trig functions
    BL_MSVC_NOINLINE constexpr fqd_s atan_series_reduced(const fqd_s& z)
    {
        const fqd_s z2 = sqr_inline(z);
        const fqd_s p = horner_reverse(qd_atan_reduced_coeffs, qd_atan_reduced_coeff_count, z2);
        return mul_product_inline(z, p);
    }

    BL_MSVC_NOINLINE constexpr fqd_s atan_core_unit(const fqd_s& z)
    {
        using namespace detail::_qd;

        int k = static_cast<int>(round_nearest_even_value(z.x0 * 16.0));
        if (k <= 0)
            return atan_series_reduced(z);
        if (k > 16)
            k = 16;

        const double a = static_cast<double>(k) * 0.0625;
        const fqd_s u = div_refined_once(
            sub_double_finite_inline(z, a),
            add_scalar_precise(mul_double_product_inline(z, a), 1.0));

        return add_finite_inline(qd_atan_reduced_table_16[k], atan_series_reduced(u));
    }

    BL_MSVC_NOINLINE constexpr fqd_s _atan(const fqd_s& x)
    {
        using namespace detail::_qd;

        if (isnan(x))  return x;
        if (iszero(x)) return x;
        if (isinf(x))  return signbit(x.x0) ? -pi_2 : pi_2;

        const bool neg = x.x0 < 0.0;
        const fqd_s ax = neg ? -x : x;

        if (ax > fqd_s{ 1.0 })
        {
            const fqd_s core = atan_core_unit(div_refined_once(1.0, ax));
            const fqd_s out  = sub_finite_inline(pi_2, core);
            return neg ? -out : out;
        }

        const fqd_s out = atan_core_unit(ax);
        return neg ? -out : out;
    }

    BL_FORCE_INLINE constexpr fqd_s _asin(const fqd_s& x)
    {
        using namespace detail::_qd;

        if (isnan(x))
            return x;

        const fqd_s ax = detail::_qd::mag(x);
        if (ax > fqd_s{ 1.0 })
            return std::numeric_limits<fqd_s>::quiet_NaN();
        if (ax == fqd_s{ 1.0 })
            return (x.x0 < 0.0) ? -pi_2 : pi_2;

        const fqd_s c = detail::_qd_impl::sqrt_accurate(mul_product_inline(sub_double_finite_inline(1.0, x), add_double_finite_inline(x, 1.0)));
        return detail::_qd_impl::atan2(x, c);
    }

    // inverse hyperbolic functions
    BL_MSVC_NOINLINE constexpr fqd_s atanh_small_series(const fqd_s& x)
    {
        const fqd_s x2 = sqr_inline(x);
        fqd_s sum   = x;
        fqd_s power = x;

        for (int k = 1; k <= 128; ++k)
        {
            power = mul_product_inline(power, x2);
            const fqd_s term = div_double_prechecked_inline(power, static_cast<double>(2 * k + 1));
            sum = add_finite_inline(sum, term);

            const fqd_s threshold = mul_double_product_inline(mul_product_inline(convergence_epsilon, mag(sum)), 0.0625);
            if (mag(term) <= threshold)
                break;
        }

        return sum;
    }

    // erf/erfc functions
    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s erf_cheb_eval(
        const fqd_s& x,
        const fqd_s* coeffs,
        std::size_t count,
        double shift)
    {
        if (!bl::detail::is_constant_evaluated())
        {
            return detail::_qd_runtime::cheb_eval(x, coeffs, count, shift);
        }

        const fqd_s t = sub_double_finite_inline(mul_double_product_inline(x, 2.0), shift);
        fqd_s b1{ 0.0 };
        fqd_s b2{ 0.0 };

        for (int i = static_cast<int>(count) - 1; i >= 1; --i)
        {
            const fqd_s b0 = add_finite_inline(
                mul_double_sub_pow2_inline(mul_product_inline(t, b1), 2.0, b2),
                coeffs[i]);
            b2 = b1;
            b1 = b0;
        }

        return add_finite_inline(mul_sub_inline(t, b1, b2), coeffs[0]);
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s erf_positive_cheb(const fqd_s& x)
    {
        if (x < fqd_s{ 1.0 })
            return erf_cheb_eval(x, qd_erf_cheb_0_1, qd_erf_cheb_coeff_count, 1.0);
        if (x < fqd_s{ 2.0 })
            return erf_cheb_eval(x, qd_erf_cheb_1_2, qd_erf_cheb_coeff_count, 3.0);
        return erf_cheb_eval(x, qd_erf_cheb_2_3, qd_erf_cheb_coeff_count, 5.0);
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s erf_positive_series(const fqd_s& x)
    {
        const fqd_s xx = mul_product_inline(x, x);
        fqd_s power = x;
        fqd_s sum   = x;

        for (int n = 1; n < 512; ++n)
        {
            power = mul_product_inline(
                power,
                div_double_prechecked_inline(-xx, static_cast<double>(n)));

            const fqd_s term = div_double_prechecked_inline(power, static_cast<double>(2 * n + 1));

            sum = add_finite_inline(sum, term);
            if (mag(term) < convergence_epsilon)
                break;
        }

        return mul_double_product_inline(
            mul_product_inline(std::numbers::inv_sqrtpi_v<fqd_s>, sum),
            2.0);
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s erfc_positive_cheb_3_4(const fqd_s& x)
    {
        return erf_cheb_eval(x, qd_erfc_cheb_3_4, qd_erfc_cheb_coeff_count, 7.0);
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr bool erfc_continued_fraction_step(
        fqd_s& b,
        fqd_s& c,
        fqd_s& d,
        fqd_s& h,
        double an,
        const fqd_s& tiny,
        const fqd_s& convergence) noexcept
    {
        b = add_double_finite_inline(b, 2.0);

        d = add_mul_double_inline(b, d, an);
        if (mag(d) < tiny)
            d = tiny;

        c = add_finite_inline(b, div_double_prechecked_inline(an, c));
        if (mag(c) < tiny)
            c = tiny;

        d = div_double_prechecked_inline(1.0, d);
        const fqd_s delta = mul_product_inline(d, c);
        h = mul_product_inline(h, delta);
        return mag(sub_double_finite_inline(delta, 1.0)) <= convergence;
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s erfc_positive_cf(const fqd_s& x)
    {
        const fqd_s z = sqr_inline(x);
        constexpr fqd_s tiny = fqd_s{ 1.0e-300, 0.0, 0.0, 0.0 };

        fqd_s b = add_double_finite_inline(z, 0.5);
        const fqd_s convergence = mul_double_product_inline(convergence_epsilon, 64.0);
        fqd_s c = div_double_prechecked_inline(1.0, tiny);
        fqd_s d = div_double_prechecked_inline(1.0, b);
        fqd_s h = d;

        double an = -0.5;
        double an_step = -2.5;
        for (int i = 1; i <= 160; ++i)
        {
            if (erfc_continued_fraction_step(b, c, d, h, an, tiny, convergence))
                break;

            an += an_step;
            an_step -= 2.0;
        }

        return mul_product_inline(
            mul_product_inline(_exp(-z), x),
            mul_product_inline(std::numbers::inv_sqrtpi_v<fqd_s>, h));
    }

    // erfc(13) is below the qd accuracy target, so the tails round to 0/1/2.
    inline constexpr fqd_s erf_saturation_cutoff{ 13.0 };

    // gamma functions

    BL_NO_INLINE constexpr fqd_s lgamma1p_series(const fqd_s& y) noexcept
    {
        constexpr int count = static_cast<int>(sizeof(lgamma1p_coeff) / sizeof(lgamma1p_coeff[0]));

        fqd_s p = horner_reverse(lgamma1p_coeff, static_cast<std::size_t>(count), y);

        return mul_product_inline(y, mul_add_inline(y, p, -std::numbers::egamma_v<fqd_s>));
    }

    BL_NO_INLINE constexpr bool try_lgamma_near_one_or_two(const fqd_s& x, fqd_s& out) noexcept
    {
        const fqd_s y1 = sub_double_finite_inline(x, 1.0);
        if (mag(y1) <= fqd_s{ 0.25 })
        {
            out = lgamma1p_series(y1);
            return true;
        }

        const fqd_s y2 = sub_double_finite_inline(x, 2.0);
        if (mag(y2) <= fqd_s{ 0.25 })
        {
            out = add_finite_inline(log1p_series_reduced(y2), lgamma1p_series(y2));
            return true;
        }

        return false;
    }

    BL_NO_INLINE constexpr bool try_lgamma_short_recurrence(const fqd_s& x, fqd_s& out) noexcept
    {
        if (!(x > fqd_s{ 0.0 }) || !(x < fqd_s{ 32.0 }))
            return false;

        fqd_s z = x;
        fqd_s product{ 1.0 };
        bool shifted_up = false;

        while (z < fqd_s{ 1.0 })
        {
            product = mul_product_inline(product, z);
            z = add_double_finite_inline(z, 1.0);
            shifted_up = true;
        }
        while (z > fqd_s{ 2.25 })
        {
            z = sub_double_finite_inline(z, 1.0);
            product = mul_product_inline(product, z);
        }

        fqd_s near_value{};
        if (!try_lgamma_near_one_or_two(z, near_value))
            return false;

        const fqd_s log_product = _log(product);
        out = shifted_up ? sub_finite_inline(near_value, log_product) : add_finite_inline(near_value, log_product);
        return true;
    }

    BL_NO_INLINE constexpr void positive_recurrence_product(const fqd_s& x, const fqd_s& asymptotic_min, fqd_s& z, fqd_s& product, int& product_exp2) noexcept
    {
        z = x;
        product = fqd_s{ 1.0 };
        product_exp2 = 0;

        while (z < asymptotic_min)
        {
            product = mul_product_inline(product, z);

            const double hi = product.x0;
            if (hi != 0.0)
            {
                const int e = detail::fp::frexp_exponent_limb(hi);
                if (e > 512 || e < -512)
                {
                    product = detail::_qd_impl::ldexp(product, -e);
                    product_exp2 += e;
                }
            }

            z = add_double_finite_inline(z, 1.0);
        }
    }

    BL_NO_INLINE constexpr fqd_s lgamma_stirling_asymptotic(const fqd_s& z) noexcept
    {
        const fqd_s inv    = div_double_prechecked_inline(1.0, z);
        const fqd_s inv2   = sqr_eval(inv);
        const fqd_s series = mul_eval(inv, horner_reverse(
            lgamma_stirling_coeffs,
            sizeof(lgamma_stirling_coeffs) / sizeof(lgamma_stirling_coeffs[0]),
            inv2));

        return add_eval(
            add_eval(mul_sub_eval(sub_double_eval(z, 0.5), detail::_qd_impl::log(z), z), half_log_two_pi),
            series);
    }

    BL_NO_INLINE constexpr fqd_s lgamma_positive_recurrence(const fqd_s& x) noexcept
    {
        fqd_s near_value{};
        if (try_lgamma_near_one_or_two(x, near_value))
            return near_value;
        if (try_lgamma_short_recurrence(x, near_value))
            return near_value;

        constexpr fqd_s asymptotic_min = fqd_s{ 128.0 };

        fqd_s z{};
        fqd_s product{};
        int product_exp2 = 0;
        positive_recurrence_product(x, asymptotic_min, z, product, product_exp2);

        return sub_mul_double_eval(
            sub_eval(lgamma_stirling_asymptotic(z), detail::_qd_impl::log(product)),
            std::numbers::ln2_v<fqd_s>,
            static_cast<double>(product_exp2));
    }

    BL_MSVC_NOINLINE constexpr fqd_s sinpi_reduced(const fqd_s& x) noexcept
    {
        const fqd_s n = detail::_qd_impl::round_nearest_even(x);
        const fqd_s r = sub_finite_inline(x, n);
        fqd_s out = detail::_qd_impl::sin(mul_product_inline(std::numbers::pi_v<fqd_s>, r));
        if (is_odd_integer(n))
            out = -out;
        return out;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s cbrt_constexpr_seed(const fqd_s& ax)
    {
        return fqd_s{ detail::fp::cbrt_seed(ax.x0), 0.0, 0.0, 0.0 };
    }
}

namespace detail::_qd_runtime
{
    [[nodiscard]] BL_FORCE_INLINE fqd_s cbrt_seed(const fqd_s& ax)
    {
        int exp2 = 0;
        double mantissa = std::frexp(ax.x0, &exp2);
        int rem = exp2 % 3;
        if (rem < 0)
            rem += 3;
        if (rem != 0)
        {
            mantissa = std::ldexp(mantissa, rem);
            exp2 -= rem;
        }

        fqd_s y{ std::cbrt(mantissa), 0.0, 0.0, 0.0 };
        if (exp2 != 0)
            y = detail::_qd_impl::ldexp(y, exp2 / 3);
        return y;
    }
}

namespace detail::_qd
{
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s cbrt_seed(const fqd_s& ax)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            cbrt_constexpr_seed(ax),
            detail::_qd_runtime::cbrt_seed(ax)
        );
    }

    [[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s cbrt_tail_step(const fqd_s& ax, const fqd_s& current) noexcept
    {
        const double inv_derivative = 1.0 / (3.0 * current.x0 * current.x0);
        const fqd_s current_squared = sqr_inline(current);
        const fqd_s residual = value_sub_mul_inline(ax, current_squared, current);
        return add_mul_double_inline(current, residual, inv_derivative);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr int scale_cbrt_input(fqd_s& value) noexcept
    {
        if (value.x0 >= 0x1p-300 && value.x0 <= 0x1p300)
            return 0;

        const int exponent = detail::fp::frexp_exponent_limb(value.x0);
        int remainder = exponent % 3;
        if (remainder < 0)
            remainder += 3;

        value = ldexp_terms(value, remainder - exponent);
        return (exponent - remainder) / 3;
    }

} // namespace detail::_qd

// exponential functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::exp(const fqd_s& x)
{
    return _exp(x);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::exp2(const fqd_s& x)
{
    return _exp2(x);
}

// logarithm functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr double detail::_qd_impl::log_as_double(fqd_s a) noexcept
{
    const double hi = a.x0;
    if (hi <= 0.0)
        return detail::fp::log(static_cast<double>(a));

    const double lo = (a.x1 + a.x2) + a.x3;
    if (!bl::detail::is_constant_evaluated())
    {
        return std::log(hi) + std::log1p(lo / hi);
    }

    return detail::fp::log(hi) + detail::fp::log1p(lo / hi);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::log(const fqd_s& a)
{
    return _log(a);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::log2(const fqd_s& a)
{
    if (isnan(a))
        return a;
    if (iszero(a))
        return fqd_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
    if (signbit(a))
        return std::numeric_limits<fqd_s>::quiet_NaN();
    if (isinf(a))
        return a;

    int exact_exp2{};
    if (qd_try_exact_binary_log2(a, exact_exp2))
        return fqd_s{ static_cast<double>(exact_exp2), 0.0, 0.0, 0.0 };

    return mul_product_inline(_log(a), std::numbers::log2e_v<fqd_s>);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::log10(const fqd_s& a)
{
    if (isnan(a))
        return a;
    if (iszero(a))
        return fqd_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
    if (signbit(a))
        return std::numeric_limits<fqd_s>::quiet_NaN();
    if (isinf(a))
        return a;

    if (detail::fp::isfinite(a.x0) && a.x0 > 0.0)
    {
        const int exp2 = detail::fp::frexp_exponent(a.x0);
        const int k0 = static_cast<int>(detail::fp::floor((exp2 - 1) * 0.30102999566398114));

        for (int k = k0 - 2; k <= k0 + 2; ++k)
        {
            const fqd_s pow10 = detail::_qd_impl::pow10_fqd(k);
            if (pow10.x0 >= std::numeric_limits<double>::min() && a == pow10)
                return fqd_s{ static_cast<double>(k), 0.0, 0.0, 0.0 };
        }
    }

    return mul_product_inline(_log(a), std::numbers::log10e_v<fqd_s>);
}

// expm1/log1p functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::expm1(const fqd_s& x)
{
    return _expm1(x);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::log1p(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (x == fqd_s{ -1.0 })
        return fqd_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
    if (x < fqd_s{ -1.0 })
        return std::numeric_limits<fqd_s>::quiet_NaN();
    if (isinf(x))
        return x;
    if (iszero(x))
        return x;

    const fqd_s ax = detail::_qd::mag(x);
    if (ax <= fqd_s{ 0.5 })
        return log1p_newton_small(x);

    const fqd_s u = add_double_finite_inline(x, 1.0);
    if (sub_double_finite_inline(u, 1.0) == x)
        return detail::_qd_impl::log(u);

    if (x > fqd_s{ 0.0 } && x <= fqd_s{ 1.0 })
    {
        const fqd_s t = div_prechecked_inline(x, add_double_finite_inline(detail::_qd_impl::sqrt(add_double_finite_inline(x, 1.0)), 1.0));
        return mul_double_product_inline(log1p_newton_small(t), 2.0);
    }

    if (x > fqd_s{ 0.0 })
        return detail::_qd_impl::log(u);

    const fqd_s y = sub_double_finite_inline(u, 1.0);
    if (iszero(y))
        return x;

    return mul_product_inline(detail::_qd_impl::log(u), div_prechecked_inline(x, y));
}


// roots
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::cbrt(const fqd_s& x)
{
    if (detail::fp::iszero_or_inf_or_nan(x.x0))
        return x;

    const bool neg = signbit(x);
    fqd_s ax = neg ? -x : x;
    const int result_scale = scale_cbrt_input(ax);

    fqd_s y = cbrt_seed(ax);
    y = cbrt_tail_step(ax, y);
    y = cbrt_tail_step(ax, y);
    y = cbrt_tail_step(ax, y);
    y = cbrt_tail_step(ax, y);

    if (result_scale != 0)
        y = ldexp_terms(y, result_scale);

    if (neg)
        y = -y;

    return y;
}

// power functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::pow(const fqd_s& x, const fqd_s& y)
{
    if (iszero(y))
        return fqd_s{ 1.0 };

    if (x == fqd_s{ 1.0 } || (x == fqd_s{ -1.0 } && detail::fp::isinf(y.x0)))
        return fqd_s{ 1.0 };

    if (detail::fp::isnan(x.x0) || detail::fp::isnan(y.x0))
        return std::numeric_limits<fqd_s>::quiet_NaN();

    if (detail::fp::isinf(y.x0))
    {
        const fqd_s ax = mag(x);
        if (ax == fqd_s{ 1.0 })
            return fqd_s{ 1.0 };
        if (ax < fqd_s{ 1.0 })
            return signbit(y) ? std::numeric_limits<fqd_s>::infinity() : fqd_s{ 0.0 };
        return signbit(y) ? fqd_s{ 0.0 } : std::numeric_limits<fqd_s>::infinity();
    }

    const fqd_s yi = detail::_qd_impl::trunc(y);
    const bool y_is_int = (yi == y);

    int64_t yi64{};
    if (y_is_int && try_get_int64(yi, yi64))
        return detail::fp::powi_by_squaring(x, yi64);

    int64_t dyadic_exponent{};
    if (try_get_pow_dyadic_eighth_exponent(x, y, dyadic_exponent))
        return pow_dyadic_eighth_unchecked(x, dyadic_exponent);

    if (x.x0 < 0.0 || (x.x0 == 0.0 && signbit(x.x0)))
    {
        if (!y_is_int)
            return std::numeric_limits<fqd_s>::quiet_NaN();

        const fqd_s magnitude = exp_for_pow(mul_product_inline(y, _log(-x)));
        return is_odd_integer(yi) ? -magnitude : magnitude;
    }

    return exp_for_pow(mul_product_inline(y, _log(x)));
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::pow(const fqd_s& x, double y)
{
    if (y == 0.0)
        return fqd_s{ 1.0 };

    if (x == fqd_s{ 1.0 } || (x == fqd_s{ -1.0 } && detail::fp::isinf(y)))
        return fqd_s{ 1.0 };

    if (detail::fp::isnan(x.x0) || detail::fp::isnan(y))
        return std::numeric_limits<fqd_s>::quiet_NaN();

    if (detail::fp::isinf(y))
    {
        const fqd_s ax = mag(x);
        if (ax == fqd_s{ 1.0 })
            return fqd_s{ 1.0 };
        if (ax < fqd_s{ 1.0 })
            return detail::fp::signbit(y) ? std::numeric_limits<fqd_s>::infinity() : fqd_s{ 0.0 };
        return detail::fp::signbit(y) ? fqd_s{ 0.0 } : std::numeric_limits<fqd_s>::infinity();
    }

    if (y == 1.0) return x;
    if (y == 2.0) return sqr_inline(x);
    if (y == -1.0) return div_refined_once(1.0, x);
    if (y == 0.5) return detail::_qd_impl::sqrt(x);

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

    if (x.x0 < 0.0 || (x.x0 == 0.0 && signbit(x.x0)))
    {
        if (!y_is_int)
            return std::numeric_limits<fqd_s>::quiet_NaN();

        const fqd_s magnitude = exp_for_pow(mul_double_product_inline(_log(-x), y));
        const bool y_is_odd =
            (absd(yi) < 0x1p53) &&
            ((static_cast<int64_t>(yi) & 1ll) != 0);

        return y_is_odd ? -magnitude : magnitude;
    }

    return exp_for_pow(mul_double_product_inline(_log(x), y));
}

// inverse trig functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::atan(const fqd_s& x)
{
    return _atan(x);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::asin(const fqd_s& x)
{
    return _asin(x);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::acos(const fqd_s& x)
{
    if (isnan(x))
        return x;

    const fqd_s ax = detail::_qd::mag(x);
    if (ax > fqd_s{ 1.0 })
        return std::numeric_limits<fqd_s>::quiet_NaN();
    if (x == fqd_s{ 1.0 })
        return fqd_s{ 0.0 };
    if (x == fqd_s{ -1.0 })
        return std::numbers::pi_v<fqd_s>;

    return sub_finite_inline(pi_2, _asin(x));
}

// sine/cosine functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr bool detail::_qd_impl::sincos(const fqd_s& x, fqd_s& s_out, fqd_s& c_out)
{
    if (iszero(x))
    {
        s_out = x;
        c_out = fqd_s{ 1.0 };
        return true;
    }

    bool ret = _sincos(x, s_out, c_out);
    s_out = s_out;
    c_out = c_out;
    return ret;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::sin(const fqd_s& x)
{
    if (iszero(x))
        return x;

    const double ax = detail::_qd::fabs(x.x0);
    if (detail::fp::isinf_or_nan(ax))
        return fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };

    if (ax <= static_cast<double>(pi_4))
        return sin_kernel_pi4(x);

    long long n = 0;
    fqd_s r{};
    if (!remainder_pi2(x, n, r))
        return fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };
    switch ((int)(n & 3LL))
    {
    case 0: return sin_kernel_pi4(r);
    case 1: return cos_kernel_pi4(r);
    case 2: return -sin_kernel_pi4(r);
    default: return -cos_kernel_pi4(r);
    }
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::cos(const fqd_s& x)
{
    const double ax = detail::_qd::fabs(x.x0);
    if (detail::fp::isinf_or_nan(ax))
        return fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };

    if (ax <= static_cast<double>(pi_4))
        return cos_kernel_pi4(x);

    long long n = 0;
    fqd_s r{};
    if (!remainder_pi2(x, n, r))
        return fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };

    switch ((int)(n & 3LL))
    {
    case 0: return cos_kernel_pi4(r);
    case 1: return -sin_kernel_pi4(r);
    case 2: return -cos_kernel_pi4(r);
    default: return sin_kernel_pi4(r);
    }
}

// tangent and atan2
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::tan(const fqd_s& x)
{
    if (iszero(x))
        return x;

    fqd_s s{}, c{};
    if (_sincos(x, s, c))
        return s / c;

    return fqd_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::atan2(const fqd_s& y, const fqd_s& x)
{
    if (detail::fp::isnan(x.x0) || detail::fp::isnan(y.x0))
        return std::numeric_limits<fqd_s>::quiet_NaN();

    if (isinf(y))
    {
        if (isinf(x))
        {
            if (x.x0 < 0.0)
                return signbit(y) ? -pi_3_4 : pi_3_4;
            return signbit(y) ? -pi_4 : pi_4;
        }

        return signbit(y) ? -pi_2 : pi_2;
    }
    if (isinf(x))
    {
        if (x.x0 < 0.0)
            return signbit(y) ? -std::numbers::pi_v<fqd_s> : std::numbers::pi_v<fqd_s>;
        return detail::_qd::signed_zero(signbit(y));
    }

    if (iszero(x))
    {
        if (iszero(y))
        {
            if (signbit(x))
                return signbit(y) ? -std::numbers::pi_v<fqd_s> : std::numbers::pi_v<fqd_s>;
            return y;
        }
        return ispositive(y) ? pi_2 : -pi_2;
    }

    if (iszero(y))
    {
        if (x.x0 < 0.0)
            return signbit(y.x0) ? -std::numbers::pi_v<fqd_s> : std::numbers::pi_v<fqd_s>;
        return y;
    }

    const fqd_s ax = detail::_qd::mag(x);
    const fqd_s ay = detail::_qd::mag(y);

    if (ax == ay)
    {
        if (x.x0 < 0.0)
        {
            return
                (y.x0 < 0.0) ? -pi_3_4 : pi_3_4;
        }

        return
            (y.x0 < 0.0) ? -pi_4 : pi_4;
    }

    if (ax >= ay)
    {
        fqd_s ratio = div_refined_once(y, x);
        if (iszero(ratio)) [[unlikely]]
            ratio = detail::_qd::signed_zero(signbit(y) != signbit(x));

        fqd_s a = _atan(ratio);

        if (x.x0 < 0.0)
            a += (y.x0 < 0.0) ? -std::numbers::pi_v<fqd_s> : std::numbers::pi_v<fqd_s>;
        return a;
    }

    fqd_s ratio = div_refined_once(x, y);
    if (iszero(ratio)) [[unlikely]]
        ratio = detail::_qd::signed_zero(signbit(x) != signbit(y));

    fqd_s a = _atan(ratio);
    return (y.x0 < 0.0) ? (-pi_2 - a) : (pi_2 - a);
}

// sinh/cosh/tanh
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::sinh(const fqd_s& x)
{
    if (detail::fp::iszero_or_inf_or_nan(x.x0))
        return x;

    const fqd_s ax = detail::_qd::mag(x);
    if (ax <= fqd_s{ 0.375 })
    {
        const fqd_s em1 = _expm1(x);
        return div_prechecked_inline(
            mul_add_inline(em1, em1, mul_double_product_inline(em1, 2.0)),
            mul_double_product_inline(add_scalar_precise(em1, 1.0), 2.0));
    }

    const fqd_s ex     = _exp(ax);
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    fqd_s out = detail::fp::exp_inverse_is_negligible(ax.x0)
        ? _ldexp(ex, -1)
        : mul_double_product_inline(sub_finite_inline(ex, div_refined_once(1.0, ex)), 0.5);
#else
    const fqd_s inv_ex = div_refined_once(1.0, ex);
    fqd_s out = mul_double_product_inline(sub_finite_inline(ex, inv_ex), 0.5);
#endif
    if (signbit(x))
        out = -out;
    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::cosh(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (isinf(x))
        return std::numeric_limits<fqd_s>::infinity();

    const fqd_s ax     = detail::_qd::mag(x);
    const fqd_s ex     = _exp(ax);
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    if (detail::fp::exp_inverse_is_negligible(ax.x0))
        return _ldexp(ex, -1);
#endif

    const fqd_s inv_ex = div_refined_once(1.0, ex);
    return
        mul_double_product_inline(add_finite_inline(ex, inv_ex), 0.5);
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::tanh(const fqd_s& x)
{
    using namespace detail::_qd;
    if (detail::fp::iszero_or_nan(x.x0))
        return x;
    if (isinf(x))
        return signbit(x) ? fqd_s{ -1.0 } : fqd_s{ 1.0 };

    const fqd_s ax = detail::_qd::mag(x);
    if (ax > fqd_s{ 80.0 })
        return signbit(x) ? fqd_s{ -1.0 } : fqd_s{ 1.0 };

    fqd_s out{};
    if (ax >= fqd_s{ 0.5 })
    {
        const fqd_s e = _exp(mul_double_product_inline(ax, -2.0));
        out = sub_double_finite_inline(1.0, div_refined_once(mul_double_product_inline(e, 2.0), add_scalar_precise(e, 1.0)));
    }
    else
    {
        const fqd_s em1 = _expm1(ax);
        const fqd_s one_plus = add_scalar_precise(em1, 1.0);
        out = div_refined_once(
            mul_add_inline(em1, em1, mul_double_product_inline(em1, 2.0)),
            add_scalar_precise(sqr_inline(one_plus), 1.0));
    }

    if (signbit(x))
        out = -out;
    return out;
}

// inverse hyperbolic functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::asinh(const fqd_s& x)
{
    if (detail::fp::iszero_or_inf_or_nan(x.x0))
        return x;

    const fqd_s ax = detail::_qd::mag(x);
    fqd_s out{};
    if (ax > fqd_s{ 0x1p500 })
        out = add_finite_inline(detail::_qd_impl::log(ax), std::numbers::ln2_v<fqd_s>);
    else if (ax <= fqd_s{ 0.5 })
    {
        const fqd_s ax2 = sqr_inline(ax);
        const fqd_s r = add_raw5_double_inline(sqr_raw5_inline(ax), 1.0);
        out = detail::_qd_impl::log1p(add_finite_inline(
            ax,
            div_prechecked_inline(ax2, add_double_finite_inline(detail::_qd_impl::sqrt(r), 1.0))));
    }
    else
        out = detail::_qd_impl::log(add_finite_inline(ax, detail::_qd_impl::sqrt(add_raw5_double_inline(sqr_raw5_inline(ax), 1.0))));

    if (signbit(x))
        out = -out;
    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::acosh(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (x < fqd_s{ 1.0 })
        return std::numeric_limits<fqd_s>::quiet_NaN();
    if (x == fqd_s{ 1.0 })
        return fqd_s{ 0.0 };
    if (isinf(x))
        return x;

    fqd_s out{};
    if (x > fqd_s{ 0x1p500 })
        out = add_finite_inline(detail::_qd_impl::log(x), std::numbers::ln2_v<fqd_s>);
    else if (x < fqd_s{ 1.25 })
    {
        const fqd_s xm1 = sub_double_finite_inline(x, 1.0);
        out = detail::_qd_impl::log1p(add_finite_inline(
            xm1,
            detail::_qd_impl::sqrt_accurate(mul_product_inline(xm1, add_double_finite_inline(x, 1.0)))));
    }
    else
        out = detail::_qd_impl::log(add_finite_inline(
            x,
            detail::_qd_impl::sqrt_accurate(mul_product_inline(sub_double_finite_inline(x, 1.0), add_double_finite_inline(x, 1.0)))));

    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::atanh(const fqd_s& x)
{
    if (detail::fp::iszero_or_nan(x.x0))
        return x;

    const fqd_s ax = detail::_qd::mag(x);
    if (ax > fqd_s{ 1.0 })
        return std::numeric_limits<fqd_s>::quiet_NaN();

    if (ax == fqd_s{ 1.0 })
        return signbit(x)
        ? fqd_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 }
        : fqd_s{ std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };

    if (ax <= fqd_s{ 0.125 })
    {
        return atanh_small_series(x);
    }

    if (ax < fqd_s{ 0.25 })
    {
        const fqd_s r = div_prechecked_inline(mul_double_product_inline(x, 2.0), sub_double_finite_inline(1.0, x));
        return mul_double_product_inline(detail::_qd_impl::log1p(r), 0.5);
    }

    const fqd_s out = mul_double_product_inline(
        detail::_qd_impl::log(div_refined_once(add_double_finite_inline(x, 1.0), sub_double_finite_inline(1.0, x))),
        0.5);
    return out;
}

// erf/erfc functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::erf(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (isinf(x))
        return signbit(x) ? fqd_s{ -1.0 } : fqd_s{ 1.0 };
    if (iszero(x))
        return x;

    const bool neg = signbit(x);
    const fqd_s ax = neg ? -x : x;

    if (ax >= erf_saturation_cutoff)
        return neg ? fqd_s{ -1.0 } : fqd_s{ 1.0 };

    fqd_s out{};

    if (ax < fqd_s{ 1.0 })
    {
        out = erf_positive_series(ax);
    }
    else if (ax < fqd_s{ 3.0 })
    {
        out = erf_positive_cheb(ax);
    }
    else if (ax < fqd_s{ 4.0 })
    {
        out = sub_double_finite_inline(1.0, erfc_positive_cheb_3_4(ax));
    }
    else
    {
        out = sub_double_finite_inline(1.0, erfc_positive_cf(ax));
    }

    if (neg)
        out = -out;

    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::erfc(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (x == fqd_s{ 0.0 })
        return fqd_s{ 1.0 };
    if (isinf(x))
        return signbit(x) ? fqd_s{ 2.0 } : fqd_s{ 0.0 };

    if (signbit(x))
    {
        const fqd_s ax = -x;
        if (ax >= erf_saturation_cutoff)
            return fqd_s{ 2.0 };
        return add_double_finite_inline(detail::_qd_impl::erf(ax), 1.0);
    }

    if (x < fqd_s{ 1.0 })
        return sub_double_finite_inline(1.0, erf_positive_series(x));

    if (x < fqd_s{ 3.0 })
        return sub_double_finite_inline(1.0, erf_positive_cheb(x));

    if (x < fqd_s{ 4.0 })
        return erfc_positive_cheb_3_4(x);

    if (x >= erf_saturation_cutoff)
        return fqd_s{ 0.0 };

    return erfc_positive_cf(x);
}

// gamma functions
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::lgamma(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (isinf(x))
        return std::numeric_limits<fqd_s>::infinity();

    if (x > fqd_s{ 0.0 })
        return lgamma_positive_recurrence(x);

    const fqd_s xi = detail::_qd_impl::trunc(x);
    if (xi == x)
        return std::numeric_limits<fqd_s>::infinity();

    const fqd_s sinpix = detail::_qd::sinpi_reduced(x);
    if (iszero(sinpix))
        return std::numeric_limits<fqd_s>::infinity();

    const fqd_s out = sub_eval(
        sub_eval(mul_double_eval(half_log_pi, 2.0), detail::_qd_impl::log(detail::_qd::mag(sinpix))),
        lgamma_positive_recurrence(sub_double_finite_inline(1.0, x)));

    return out;
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd_s detail::_qd_impl::tgamma(const fqd_s& x)
{
    if (isnan(x))
        return x;
    if (isinf(x))
        return signbit(x)
        ? std::numeric_limits<fqd_s>::quiet_NaN()
        : std::numeric_limits<fqd_s>::infinity();
    if (iszero(x))
        return fqd_s{ signbit(x) ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };

    if (x > fqd_s{ 0.0 })
        return _exp(lgamma_positive_recurrence(x));

    const fqd_s xi = detail::_qd_impl::trunc(x);
    if (xi == x)
        return std::numeric_limits<fqd_s>::quiet_NaN();

    const fqd_s sinpix = detail::_qd::sinpi_reduced(x);
    if (iszero(sinpix))
        return std::numeric_limits<fqd_s>::quiet_NaN();

    const fqd_s log_abs = sub_eval(
        sub_eval(mul_double_eval(half_log_pi, 2.0), detail::_qd_impl::log(detail::_qd::mag(sinpix))),
        lgamma_positive_recurrence(sub_double_finite_inline(1.0, x)));
    fqd_s out = _exp(log_abs);
    if (signbit(sinpix))
        out = -out;
    return out;
}

} // namespace bl

#endif // QD_DETAIL_MATH_TRANSCENDENTAL_INCLUDED
