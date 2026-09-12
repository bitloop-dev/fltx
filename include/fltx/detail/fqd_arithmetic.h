/**
 * fltx/detail/fqd_arithmetic.h - Low-level quad-double arithmetic helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_DETAIL_ARITHMETIC_INCLUDED
#define FQD_DETAIL_ARITHMETIC_INCLUDED
#include "fltx/detail/fqd_expansion.h"
#include "fltx/fqd_classification.h"
#include "fltx/fqd_limits.h"

namespace bl {

namespace detail::_qd_runtime
{
    // BL_VECTORCALL is intentionally selective: standalone multiplication
    // regresses on MSVC x64, while add/sub/div and fused expressions benefit.

    // Adds finite qd values through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_finite(const fqd_s& a, const fqd_s& b) noexcept;

    // Adds qd values and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_canonical(const fqd_s& a, const fqd_s& b) noexcept;

    // Adds a double-double value to qd through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept;

    // Adds a finite double to qd through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_double_finite(const fqd_s& a, double b) noexcept;

    // Adds a double to qd and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_double_canonical(const fqd_s& a, double b) noexcept;

    // Subtracts finite qd values through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_finite(const fqd_s& a, const fqd_s& b) noexcept;

    // Subtracts qd values and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_canonical(const fqd_s& a, const fqd_s& b) noexcept;

    // Subtracts a double-double value from qd through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept;

    // Subtracts a qd value from double-double through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_dd(detail::_qd::dd_scalar a, const fqd_s& b) noexcept;

    // Subtracts a finite double from qd through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_double_finite(const fqd_s& a, double b) noexcept;

    // Subtracts a double from qd and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_double_canonical(const fqd_s& a, double b) noexcept;

    // Subtracts qd from a double and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_double_canonical(double a, const fqd_s& b) noexcept;

    // Multiplies finite qd values through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s               mul_finite(const fqd_s& a, const fqd_s& b) noexcept;

    // Multiplies qd values and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s               mul_canonical(const fqd_s& a, const fqd_s& b) noexcept;

    // Multiplies qd by double-double through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s               mul_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept;

    // Multiplies qd by a finite double through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s               mul_double_finite(const fqd_s& a, double b) noexcept;

    // Multiplies qd by a double and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s               mul_double_canonical(const fqd_s& a, double b) noexcept;

    // Divides finite qd values through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_finite(const fqd_s& a, const fqd_s& b) noexcept;

    // Divides qd values and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_canonical(const fqd_s& a, const fqd_s& b) noexcept;

    // Divides qd by double-double through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept;

    // Divides double-double by qd through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_dd(detail::_qd::dd_scalar a, const fqd_s& b) noexcept;

    // Divides qd by a finite double through the compiled runtime boundary.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_double_finite(const fqd_s& a, double b) noexcept;

    // Divides qd by a double and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_double_canonical(const fqd_s& a, double b) noexcept;

    // Divides a double by qd and returns the canonical special-value representation.
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_double_canonical(double a, const fqd_s& b) noexcept;

    // fused operations
    [[nodiscard]] BL_NO_INLINE fqd_s               sqr(const fqd_s& a) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s               sqr_canonical(const fqd_s& a) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s               mul_pow2_or_double(const fqd_s& a, double b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL value_sub_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_mul_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_mul_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul_add_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e, const fqd_s& f) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul_add_mul_add_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e, const fqd_s& f, const fqd_s& g, const fqd_s& h) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_add(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_add(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_add_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_add_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_scaled_2_1(const fqd_s& a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_scaled_1_2(const fqd_s& a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_scaled_2_neg1(const fqd_s& a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_scaled_1_neg2(const fqd_s& a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_mul_double(const fqd_s& addend, const fqd_s& value, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_mul_double(const fqd_s& minuend, const fqd_s& value, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_double_sub(const fqd_s& value, double scalar, const fqd_s& subtrahend) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_double_add_mul_double(const fqd_s& a, double a_scalar, const fqd_s& b, double b_scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_double_add_mul_double_add(const fqd_s& a, double a_scalar, const fqd_s& b, double b_scalar, const fqd_s& c) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_add(const fqd_s& numerator, const fqd_s& a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_sub(const fqd_s& numerator, const fqd_s& a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_add_double(const fqd_s& numerator, const fqd_s& base_denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL div_double_sub(const fqd_s& numerator, double scalar, const fqd_s& base_denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL value_sub_mul_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_mul_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_add_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_add_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_sub_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_sub_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_mul_double_div(const fqd_s& addend, const fqd_s& value, double scalar, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_mul_double_div(const fqd_s& minuend, const fqd_s& value, double scalar, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_double_sub_div(const fqd_s& value, double scalar, const fqd_s& subtrahend, const fqd_s& denominator) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL value_sub_mul_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_add_mul_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_sub_mul_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_add_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_add_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_add_sub_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_sub_sub_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL add_mul_double_div_add_double(const fqd_s& addend, const fqd_s& value, double value_scalar, const fqd_s& denominator, double denominator_scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL sub_mul_double_div_add_double(const fqd_s& minuend, const fqd_s& value, double value_scalar, const fqd_s& denominator, double denominator_scalar) noexcept;
    [[nodiscard]] BL_NO_INLINE fqd_s BL_VECTORCALL mul_double_sub_div_add_double(const fqd_s& value, double value_scalar, const fqd_s& subtrahend, const fqd_s& denominator, double denominator_scalar) noexcept;

} // namespace detail::_qd_runtime


namespace detail::_qd // primitives and kernels
{
    [[nodiscard]] BL_FORCE_INLINE constexpr bool is_subnormal_limb(double value) noexcept
    {
        constexpr std::uint64_t magnitude_mask = 0x7fffffffffffffffull;
        constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
        const std::uint64_t magnitude =
            std::bit_cast<std::uint64_t>(value) & magnitude_mask;
        return magnitude != 0 && (magnitude & exponent_mask) == 0;
    }

#if BL_FP_BARRIER_ACTIVE
    [[nodiscard]] BL_FORCE_INLINE constexpr bool has_subnormal_limb(
        const fqd_s& value) noexcept
    {
        return is_subnormal_limb(value.x0) || is_subnormal_limb(value.x1) ||
            is_subnormal_limb(value.x2) || is_subnormal_limb(value.x3);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s scale_terms_guarded(
        const fqd_s& value,
        int exponent) noexcept
    {
        return {
            detail::fp::ldexp(value.x0, exponent),
            detail::fp::ldexp(value.x1, exponent),
            detail::fp::ldexp(value.x2, exponent),
            detail::fp::ldexp(value.x3, exponent)
        };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool needs_product_input_scale(
        const fqd_s& value) noexcept
    {
        const double limbs[] = { value.x0, value.x1, value.x2, value.x3 };
        for (double limb : limbs)
        {
            if (!detail::fp::iszero_or_inf_or_nan(limb) &&
                detail::fp::frexp_exponent(limb) < -900)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] BL_FORCE_INLINE fqd_s mul_fastmath_guarded(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        constexpr int input_scale = 512;
        int a_scale = needs_product_input_scale(a) ? input_scale : 0;
        int b_scale = needs_product_input_scale(b) ? input_scale : 0;
        int result_scale = -(a_scale + b_scale);

        if (a_scale != 0 || b_scale != 0) [[unlikely]]
        {
            const int result_exponent = detail::fp::frexp_exponent(a.x0) +
                detail::fp::frexp_exponent(b.x0) - 1 - result_scale;
            if (result_exponent > 900 && a_scale != 0 && b_scale == 0)
            {
                b_scale = -input_scale;
                result_scale = 0;
            }
            else if (result_exponent > 900 && b_scale != 0 && a_scale == 0)
            {
                a_scale = -input_scale;
                result_scale = 0;
            }

            const fqd_s product = detail::_qd_runtime::mul_finite(
                scale_terms_guarded(a, a_scale),
                scale_terms_guarded(b, b_scale));
            return result_scale == 0
                ? product
                : scale_terms_guarded(product, result_scale);
        }

        return detail::_qd_runtime::mul_finite(a, b);
    }
#endif

    // Constructs the canonical quiet NaN used by public qd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s quiet_nan() noexcept
    {
        return { std::bit_cast<double>(0x7ff8000000000000ull), 0.0, 0.0, 0.0 };
    }

    // Constructs a canonical signed infinity for public qd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s signed_infinity(bool negative) noexcept
    {
        return { std::bit_cast<double>(negative ? 0xfff0000000000000ull : 0x7ff0000000000000ull), 0.0, 0.0, 0.0 };
    }

    // Constructs a canonical signed zero for public qd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s signed_zero(bool negative) noexcept
    {
        return { std::bit_cast<double>(negative ? 0x8000000000000000ull : 0ull), 0.0, 0.0, 0.0 };
    }

    // Resolves non-finite addition results according to the public qd policy.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_special(const fqd_s& a, const fqd_s& b) noexcept
    {
        if (detail::fp::isnan(a.x0) || detail::fp::isnan(b.x0))
            return quiet_nan();

        const bool a_inf = isinf(a.x0);
        const bool b_inf = isinf(b.x0);
        if (a_inf && b_inf && signbit(a.x0) != signbit(b.x0))
            return quiet_nan();
        if (a_inf)
            return signed_infinity(signbit(a.x0));
        return signed_infinity(signbit(b.x0));
    }

    // Resolves non-finite subtraction results according to the public qd policy.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_special(const fqd_s& a, const fqd_s& b) noexcept
    {
        return add_special(a, fqd_s{ -b.x0, -b.x1, -b.x2, -b.x3 });
    }

    // Tests one binary64 limb for zero, infinity, or NaN with one branch guard.
    [[nodiscard]] BL_FORCE_INLINE constexpr bool limb_is_zero_or_nonfinite(double value) noexcept
    {
        const std::uint64_t magnitude_twice = std::bit_cast<std::uint64_t>(value) << 1;
        return (magnitude_twice - 1ULL) >= 0xffdfffffffffffffULL;
    }

    // Resolves the cold zero/non-finite result of canonical qd addition.
    [[nodiscard]] BL_NO_INLINE constexpr fqd_s finish_add_canonical(
        const fqd_s& a,
        const fqd_s& b,
        double result_head) noexcept
    {
        if (result_head == 0.0)
        {
            const bool both_zero = bl::iszero(a) && bl::iszero(b);
            const bool negative = both_zero
                ? bl::signbit(a) && bl::signbit(b)
                : a < -b;
            return signed_zero(negative);
        }

        return add_special(a, b);
    }

    // Resolves the cold zero/non-finite result of canonical qd subtraction.
    [[nodiscard]] BL_NO_INLINE constexpr fqd_s finish_sub_canonical(
        const fqd_s& a,
        const fqd_s& b,
        double result_head) noexcept
    {
        if (result_head == 0.0)
        {
            const bool both_zero = bl::iszero(a) && bl::iszero(b);
            const bool negative = both_zero
                ? bl::signbit(a) && !bl::signbit(b)
                : a < b;
            return signed_zero(negative);
        }

        return sub_special(a, b);
    }

    // Resolves non-finite and signed-zero multiplication results for public qd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_special(const fqd_s& a, const fqd_s& b) noexcept
    {
        if (detail::fp::isnan(a.x0) || detail::fp::isnan(b.x0))
            return quiet_nan();

        const bool a_inf = isinf(a.x0);
        const bool b_inf = isinf(b.x0);
        if ((a_inf && b.x0 == 0.0) || (b_inf && a.x0 == 0.0))
            return quiet_nan();

        const bool negative = bl::signbit(a) != bl::signbit(b);
        if (bl::iszero(a) || bl::iszero(b))
            return signed_zero(negative);

        return signed_infinity(negative);
    }

    // Resolves zero, infinity, and NaN division cases for public qd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_special(const fqd_s& a, const fqd_s& b) noexcept
    {
        if (detail::fp::isnan(a.x0) || detail::fp::isnan(b.x0))
            return quiet_nan();

        const bool a_zero = a.x0 == 0.0;
        const bool b_zero = b.x0 == 0.0;
        const bool negative = bl::signbit(a) != bl::signbit(b);
        if (b_zero)
            return a_zero ? quiet_nan() : signed_infinity(negative);

        const bool a_inf = isinf(a.x0);
        const bool b_inf = isinf(b.x0);
        if (a_inf && b_inf)
            return quiet_nan();
        if (a_inf)
            return signed_infinity(negative);
        return signed_zero(negative);
    }

    // Accumulates one double into a qd expansion with error-free sums.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_scalar_precise(const fqd_s& a, double b) noexcept
    {
        double s0{}, e0{}; two_sum_precise(a.x0, b, s0, e0);
        double s1{}, e1{}; two_sum_precise(a.x1, e0, s1, e1);
        double s2{}, e2{}; two_sum_precise(a.x2, e1, s2, e2);
        double s3{}, e3{}; two_sum_precise(a.x3, e2, s3, e3);

        return renorm5(s0, s1, s2, s3, e3);
    }

    // Compresses an arbitrary expansion into a normalized qd value.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s from_expansion_fast(const double* h, int n) noexcept
    {
        if (n <= 0) return {};

        double comp[40]{};
        const int m = compress_expansion_zeroelim(n, h, comp);

        fqd_s sum{};
        for (int i = 0; i < m; ++i)
            sum = add_scalar_precise(sum, comp[i]);

        return sum;
    }

    // Addition
    // Adds qd expansions without applying canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_finite_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        double s0{}, e0{};
        double s1{}, e1{};
        double s2{}, e2{};
        double s3{}, e3{};

		#if FLTX_FQD_ENABLE_SIMD
        if (qd_runtime_addsub_simd_enabled())
        {
            const simd::f64x2 a01 = simd::f64x2_set(a.x0, a.x1);
            const simd::f64x2 b01 = simd::f64x2_set(b.x0, b.x1);
            const simd::f64x2 a23 = simd::f64x2_set(a.x2, a.x3);
            const simd::f64x2 b23 = simd::f64x2_set(b.x2, b.x3);
            simd::f64x2 s01{}, e01{}, s23{}, e23{};
            simd::f64x2_two_sum(a01, b01, s01, e01);
            simd::f64x2_two_sum(a23, b23, s23, e23);
            simd::f64x2_store(s01, s0, s1);
            simd::f64x2_store(e01, e0, e1);
            simd::f64x2_store(s23, s2, s3);
            simd::f64x2_store(e23, e2, e3);
        }
        else
		#endif
        {
            two_sum_precise(a.x0, b.x0, s0, e0);
            two_sum_precise(a.x1, b.x1, s1, e1);
            two_sum_precise(a.x2, b.x2, s2, e2);
            two_sum_precise(a.x3, b.x3, s3, e3);
        }
        two_sum_precise(s1, e0, s1, e0);
        three_sum(s2, e0, e1);
        three_sum2(s3, e0, e2);

        e0 += e1 + e3;

        if (e0 == 0.0)
            return renorm4(s0, s1, s2, s3);

        return renorm5(s0, s1, s2, s3, e0);
    }

    // Adds a double to qd without canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_double_finite_inline(const fqd_s& a, double b) noexcept
    {
        double c0{}, c1{}, c2{}, c3{}, e{};

        two_sum_precise(a.x0, b, c0, e);
        if (e == 0.0) return renorm4(c0, a.x1, a.x2, a.x3);

        two_sum_precise(a.x1, e, c1, e);
        if (e == 0.0) return renorm4(c0, c1, a.x2, a.x3);

        two_sum_precise(a.x2, e, c2, e);
        if (e == 0.0) return renorm4(c0, c1, c2, a.x3);

        two_sum_precise(a.x3, e, c3, e);
        if (e == 0.0) return renorm4(c0, c1, c2, c3);

        return renorm5(c0, c1, c2, c3, e);
    }

    // Adds a qd expansion to a double through the finite scalar kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_double_finite_inline(double a, const fqd_s& b) noexcept
    {
        return add_double_finite_inline(b, a);
    }

    // Adds qd values and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_canonical_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        const fqd_s out = add_finite_inline(a, b);
        if (limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return finish_add_canonical(a, b, out.x0);
        return out;
    }

    // Adds a double to qd and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_double_canonical_inline(const fqd_s& a, double b) noexcept
    {
        const fqd_s rhs{ b, 0.0, 0.0, 0.0 };
        const fqd_s out = add_double_finite_inline(a, b);
        if (limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return finish_add_canonical(a, rhs, out.x0);
        return out;
    }

    // Adds double-double to qd through constexpr or compiled canonical dispatch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_dd(const fqd_s& a, dd_scalar b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            add_canonical_inline(a, fqd_s{ b.hi, b.lo, 0.0, 0.0 }),
            detail::_qd_runtime::add_dd(a, b)
        );
    }

    // Subtraction
    // Subtracts qd expansions without applying canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_finite_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        double s0{}, e0{};
        double s1{}, e1{};
        double s2{}, e2{};
        double s3{}, e3{};

        #if FLTX_FQD_ENABLE_SIMD
        if (qd_runtime_addsub_simd_enabled())
        {
            const simd::f64x2 a01 = simd::f64x2_set(a.x0, a.x1);
            const simd::f64x2 b01 = simd::f64x2_set(-b.x0, -b.x1);
            const simd::f64x2 a23 = simd::f64x2_set(a.x2, a.x3);
            const simd::f64x2 b23 = simd::f64x2_set(-b.x2, -b.x3);
            simd::f64x2 s01{}, e01{}, s23{}, e23{};
            simd::f64x2_two_sum(a01, b01, s01, e01);
            simd::f64x2_two_sum(a23, b23, s23, e23);
            simd::f64x2_store(s01, s0, s1);
            simd::f64x2_store(e01, e0, e1);
            simd::f64x2_store(s23, s2, s3);
            simd::f64x2_store(e23, e2, e3);
        }
        else
		#endif
        {
            two_sum_precise(a.x0, -b.x0, s0, e0);
            two_sum_precise(a.x1, -b.x1, s1, e1);
            two_sum_precise(a.x2, -b.x2, s2, e2);
            two_sum_precise(a.x3, -b.x3, s3, e3);
        }
        two_sum_precise(s1, e0, s1, e0);
        three_sum(s2, e0, e1);
        three_sum2(s3, e0, e2);

        e0 += e1 + e3;

        if (e0 == 0.0)
            return renorm4(s0, s1, s2, s3);

        return renorm5(s0, s1, s2, s3, e0);
    }

    // Subtracts a double from qd without canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_double_finite_inline(const fqd_s& a, double b) noexcept
    {
        return add_double_finite_inline(a, -b);
    }

    // Subtracts qd from a double through the finite scalar kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_double_finite_inline(double a, const fqd_s& b) noexcept
    {
        return add_double_finite_inline(-b, a);
    }

    // Subtracts qd values and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_canonical_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        const fqd_s out = sub_finite_inline(a, b);
        if (limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return finish_sub_canonical(a, b, out.x0);
        return out;
    }

    // Subtracts a double from qd and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_double_canonical_inline(const fqd_s& a, double b) noexcept
    {
        const fqd_s rhs{ b, 0.0, 0.0, 0.0 };
        const fqd_s out = sub_double_finite_inline(a, b);
        if (limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return finish_sub_canonical(a, rhs, out.x0);
        return out;
    }

    // Subtracts qd from a double and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_double_canonical_inline(double a, const fqd_s& b) noexcept
    {
        const fqd_s lhs{ a, 0.0, 0.0, 0.0 };
        const fqd_s out = sub_double_finite_inline(a, b);
        if (limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return finish_sub_canonical(lhs, b, out.x0);
        return out;
    }

    // Subtracts double-double from qd through constexpr or compiled canonical dispatch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_dd(const fqd_s& a, dd_scalar b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            sub_canonical_inline(a, fqd_s{ b.hi, b.lo, 0.0, 0.0 }),
            detail::_qd_runtime::sub_dd(a, b)
        );
    }

    // Subtracts qd from double-double through constexpr or compiled canonical dispatch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_dd(dd_scalar a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            sub_canonical_inline(fqd_s{ a.hi, a.lo, 0.0, 0.0 }, b),
            detail::_qd_runtime::sub_dd(a, b)
        );
    }

    // Multiplication
    // Stores the ten leading products and their exact product errors.
    struct product_terms
    {
        double p[10]{};
        double q[10]{};
    };

    // Fills qd product terms with hardware FMA, using SIMD when available.
    BL_FORCE_INLINE void fill_mul_products_fma(
        const fqd_s& a,
        const fqd_s& b,
        product_terms& products) noexcept
    {
        #if FLTX_FQD_ENABLE_SIMD && \
            ((FLTX_HAS_SSE2 && (FLTX_HAS_X86_FMA || FLTX_GUARDED_X86_FMA)) || FLTX_HAS_NEON)
        if (qd_runtime_product_simd_enabled())
        {
            simd::f64x2 p01{}, q01{};
            simd::f64x2 p23{}, q23{};
            simd::f64x2 p45{}, q45{};
            simd::f64x2 p67{}, q67{};
            simd::f64x2 p89{}, q89{};

            simd::f64x2_two_prod_fma(simd::f64x2_set(a.x0, a.x0), simd::f64x2_set(b.x0, b.x1), p01, q01);
            simd::f64x2_two_prod_fma(simd::f64x2_set(a.x1, a.x0), simd::f64x2_set(b.x0, b.x2), p23, q23);
            simd::f64x2_two_prod_fma(simd::f64x2_set(a.x1, a.x2), simd::f64x2_set(b.x1, b.x0), p45, q45);
            simd::f64x2_two_prod_fma(simd::f64x2_set(a.x0, a.x1), simd::f64x2_set(b.x3, b.x2), p67, q67);
            simd::f64x2_two_prod_fma(simd::f64x2_set(a.x2, a.x3), simd::f64x2_set(b.x1, b.x0), p89, q89);

            simd::f64x2_store(p01, products.p[0], products.p[1]);
            simd::f64x2_store(q01, products.q[0], products.q[1]);
            simd::f64x2_store(p23, products.p[2], products.p[3]);
            simd::f64x2_store(q23, products.q[2], products.q[3]);
            simd::f64x2_store(p45, products.p[4], products.p[5]);
            simd::f64x2_store(q45, products.q[4], products.q[5]);
            simd::f64x2_store(p67, products.p[6], products.p[7]);
            simd::f64x2_store(q67, products.q[6], products.q[7]);
            simd::f64x2_store(p89, products.p[8], products.p[9]);
            simd::f64x2_store(q89, products.q[8], products.q[9]);
            return;
        }
        #endif

        detail::fp::two_prod_fma(a.x0, b.x0, products.p[0], products.q[0]);
        detail::fp::two_prod_fma(a.x0, b.x1, products.p[1], products.q[1]);
        detail::fp::two_prod_fma(a.x1, b.x0, products.p[2], products.q[2]);
        detail::fp::two_prod_fma(a.x0, b.x2, products.p[3], products.q[3]);
        detail::fp::two_prod_fma(a.x1, b.x1, products.p[4], products.q[4]);
        detail::fp::two_prod_fma(a.x2, b.x0, products.p[5], products.q[5]);
        detail::fp::two_prod_fma(a.x0, b.x3, products.p[6], products.q[6]);
        detail::fp::two_prod_fma(a.x1, b.x2, products.p[7], products.q[7]);
        detail::fp::two_prod_fma(a.x2, b.x1, products.p[8], products.q[8]);
        detail::fp::two_prod_fma(a.x3, b.x0, products.p[9], products.q[9]);
    }

    // Fills qd product terms with ordinary Dekker splitting.
    BL_FORCE_INLINE constexpr void fill_mul_products_dekker(
        const fqd_s& a,
        const fqd_s& b,
        product_terms& products) noexcept
    {
        #if FLTX_FQD_ENABLE_SIMD && (FLTX_HAS_SSE2 || FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (qd_runtime_product_simd_enabled())
        {
            simd::f64x2 p01{}, q01{};
            simd::f64x2 p23{}, q23{};
            simd::f64x2 p45{}, q45{};
            simd::f64x2 p67{}, q67{};
            simd::f64x2 p89{}, q89{};

            simd::f64x2_two_prod_dekker(simd::f64x2_set(a.x0, a.x0), simd::f64x2_set(b.x0, b.x1), p01, q01);
            simd::f64x2_two_prod_dekker(simd::f64x2_set(a.x1, a.x0), simd::f64x2_set(b.x0, b.x2), p23, q23);
            simd::f64x2_two_prod_dekker(simd::f64x2_set(a.x1, a.x2), simd::f64x2_set(b.x1, b.x0), p45, q45);
            simd::f64x2_two_prod_dekker(simd::f64x2_set(a.x0, a.x1), simd::f64x2_set(b.x3, b.x2), p67, q67);
            simd::f64x2_two_prod_dekker(simd::f64x2_set(a.x2, a.x3), simd::f64x2_set(b.x1, b.x0), p89, q89);

            simd::f64x2_store(p01, products.p[0], products.p[1]);
            simd::f64x2_store(q01, products.q[0], products.q[1]);
            simd::f64x2_store(p23, products.p[2], products.p[3]);
            simd::f64x2_store(q23, products.q[2], products.q[3]);
            simd::f64x2_store(p45, products.p[4], products.p[5]);
            simd::f64x2_store(q45, products.q[4], products.q[5]);
            simd::f64x2_store(p67, products.p[6], products.p[7]);
            simd::f64x2_store(q67, products.q[6], products.q[7]);
            simd::f64x2_store(p89, products.p[8], products.p[9]);
            simd::f64x2_store(q89, products.q[8], products.q[9]);
            return;
        }
        #endif

        detail::fp::two_prod_precise_dekker(a.x0, b.x0, products.p[0], products.q[0]);
        detail::fp::two_prod_precise_dekker(a.x0, b.x1, products.p[1], products.q[1]);
        detail::fp::two_prod_precise_dekker(a.x1, b.x0, products.p[2], products.q[2]);
        detail::fp::two_prod_precise_dekker(a.x0, b.x2, products.p[3], products.q[3]);
        detail::fp::two_prod_precise_dekker(a.x1, b.x1, products.p[4], products.q[4]);
        detail::fp::two_prod_precise_dekker(a.x2, b.x0, products.p[5], products.q[5]);
        detail::fp::two_prod_precise_dekker(a.x0, b.x3, products.p[6], products.q[6]);
        detail::fp::two_prod_precise_dekker(a.x1, b.x2, products.p[7], products.q[7]);
        detail::fp::two_prod_precise_dekker(a.x2, b.x1, products.p[8], products.q[8]);
        detail::fp::two_prod_precise_dekker(a.x3, b.x0, products.p[9], products.q[9]);
    }

    // Selects FMA or ordinary Dekker product formation for the current build and runtime.
    BL_FORCE_INLINE constexpr void fill_mul_products_auto(
        const fqd_s& a,
        const fqd_s& b,
        product_terms& products) noexcept
    {
        #if FLTX_HAS_RUNTIME_FMA_PATH
        if (!bl::detail::is_constant_evaluated() &&
            detail::fp::runtime_hardware_fma_enabled())
        {
            fill_mul_products_fma(a, b, products);
            return;
        }
        #endif
        fill_mul_products_dekker(a, b, products);
    }

    #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Fills qd product terms with range-safe Dekker splitting.
    BL_FORCE_INLINE constexpr void fill_mul_products_range_safe_dekker(
        const fqd_s& a,
        const fqd_s& b,
        product_terms& products) noexcept
    {
        detail::fp::two_prod_precise_dekker_range_safe(a.x0, b.x0, products.p[0], products.q[0]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x0, b.x1, products.p[1], products.q[1]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x1, b.x0, products.p[2], products.q[2]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x0, b.x2, products.p[3], products.q[3]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x1, b.x1, products.p[4], products.q[4]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x2, b.x0, products.p[5], products.q[5]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x0, b.x3, products.p[6], products.q[6]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x1, b.x2, products.p[7], products.q[7]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x2, b.x1, products.p[8], products.q[8]);
        detail::fp::two_prod_precise_dekker_range_safe(a.x3, b.x0, products.p[9], products.q[9]);
    }

    // Selects FMA or range-safe Dekker product formation for the current runtime.
    BL_FORCE_INLINE constexpr void fill_mul_products_range_safe_auto(
        const fqd_s& a,
        const fqd_s& b,
        product_terms& products) noexcept
    {
        #if FLTX_HAS_RUNTIME_FMA_PATH
        if (!bl::detail::is_constant_evaluated() &&
            detail::fp::runtime_hardware_fma_enabled())
        {
            fill_mul_products_fma(a, b, products);
            return;
        }
        #endif
        fill_mul_products_range_safe_dekker(a, b, products);
    }
    #endif

    // Accumulates and renormalizes previously formed qd product terms.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s finish_mul_product_inline(
        const fqd_s& a,
        const fqd_s& b,
        product_terms products) noexcept
    {
        double r0{}, r1{};
        double t0{}, t1{};
        double s0{}, s1{}, s2{};

        three_sum(products.p[1], products.p[2], products.q[0]);
        three_sum(products.p[2], products.q[1], products.q[2]);
        three_sum(products.p[3], products.p[4], products.p[5]);

        two_sum_precise(products.p[2], products.p[3], s0, t0);
        two_sum_precise(products.q[1], products.p[4], s1, t1);
        s2 = products.q[2] + products.p[5];
        two_sum_precise(s1, t0, s1, t0);
        s2 += (t0 + t1);

        two_sum_precise(products.q[0], products.q[3], products.q[0], products.q[3]);
        two_sum_precise(products.q[4], products.q[5], products.q[4], products.q[5]);
        two_sum_precise(products.p[6], products.p[7], products.p[6], products.p[7]);
        two_sum_precise(products.p[8], products.p[9], products.p[8], products.p[9]);

        two_sum_precise(products.q[0], products.q[4], t0, t1);
        t1 += products.q[3] + products.q[5];
        two_sum_precise(products.p[6], products.p[8], r0, r1);
        r1 += products.p[7] + products.p[9];
        two_sum_precise(t0, r0, products.q[3], products.q[4]);
        products.q[4] += t1 + r1;

        two_sum_precise(products.q[3], s1, t0, t1);
        t1 += products.q[4];
        t1 += a.x1 * b.x3 + a.x2 * b.x2 + a.x3 * b.x1
            + products.q[6] + products.q[7] + products.q[8] + products.q[9] + s2;

        return renorm5(products.p[0], products.p[1], s0, t0, t1);
    }

    // Forms and renormalizes a qd product through the ordinary product path.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_product_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        product_terms products{};
        fill_mul_products_auto(a, b, products);
        return finish_mul_product_inline(a, b, products);
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Forms and renormalizes a qd product through the range-safe product path.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_product_range_safe_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        product_terms products{};
        fill_mul_products_range_safe_auto(a, b, products);
        return finish_mul_product_inline(a, b, products);
    }
#endif

    // Converts a formed product into the canonical public multiplication result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s finish_mul_canonical_inline(
        const fqd_s& a,
        const fqd_s& b,
        const fqd_s& out) noexcept
    {
        if (detail::fp::isinf_or_nan(out.x0)) [[unlikely]]
            return mul_special(a, b);
        if (out.x0 == 0.0 && (bl::iszero(a) || bl::iszero(b))) [[unlikely]]
            return mul_special(a, b);
        return out;
    }

    // Multiplies qd values with range protection and canonical special handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_canonical_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        if (detail::fp::dekker_product_needs_scaling(a.x0, b.x0)) [[unlikely]]
            return finish_mul_canonical_inline(a, b, mul_product_range_safe_inline(a, b));
        #endif

        return finish_mul_canonical_inline(a, b, mul_product_inline(a, b));
    }

    // Multiplies finite qd values while retaining Dekker range protection.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_finite_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return detail::fp::dekker_product_needs_scaling(a.x0, b.x0)
            ? mul_product_range_safe_inline(a, b)
            : mul_product_inline(a, b);
        #else
        return mul_product_inline(a, b);
        #endif
    }

    // Multiplies qd values with range-safe Dekker formation and canonical finalization.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_dekker_range_safe_canonical_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return finish_mul_canonical_inline(a, b, mul_product_range_safe_inline(a, b));
        #else
        return mul_canonical_inline(a, b);
        #endif
    }

    // Forms a qd-by-double product with the ordinary product path.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_double_product_inline(const fqd_s& a, double b) noexcept
    {
        using namespace detail::_qd;

        double p0{}, p1{}, p2{}, p3{};
        double q0{}, q1{}, q2{};
        double s0{}, s1{}, s2{}, s3{}, s4{};

        #if FLTX_FQD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (qd_runtime_simd_enabled())
        {
            simd::f64x2 p01{}, q01{};
            simd::f64x2 p23{}, q23{};
            const simd::f64x2 bv = simd::f64x2_splat(b);
            simd::f64x2_two_prod_precise(simd::f64x2_set(a.x0, a.x1), bv, p01, q01);
            simd::f64x2_two_prod_precise(simd::f64x2_set(a.x2, a.x3), bv, p23, q23);
            double ignored{};
            simd::f64x2_store(p01, p0, p1);
            simd::f64x2_store(q01, q0, q1);
            simd::f64x2_store(p23, p2, p3);
            simd::f64x2_store(q23, q2, ignored);
        }
        else
            #endif
        {
            two_prod_precise(a.x0, b, p0, q0);
            two_prod_precise(a.x1, b, p1, q1);
            two_prod_precise(a.x2, b, p2, q2);
            p3 = a.x3 * b;
        }

        s0 = p0;
        two_sum_precise(q0, p1, s1, s2);
        three_sum(s2, q1, p2);
        three_sum2(q1, q2, p3);
        s3 = q1;
        s4 = q2 + p2;

        return renorm5(s0, s1, s2, s3, s4);
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Forms a qd-by-double product with range-safe Dekker splitting.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_double_product_range_safe_inline(const fqd_s& a, double b) noexcept
    {
        using namespace detail::_qd;

        double p0{}, p1{}, p2{}, p3{};
        double q0{}, q1{}, q2{};
        double s0{}, s1{}, s2{}, s3{}, s4{};

        detail::fp::two_prod_precise_range_safe(a.x0, b, p0, q0);
        detail::fp::two_prod_precise_range_safe(a.x1, b, p1, q1);
        detail::fp::two_prod_precise_range_safe(a.x2, b, p2, q2);
        p3 = a.x3 * b;

        s0 = p0;
        two_sum_precise(q0, p1, s1, s2);
        three_sum(s2, q1, p2);
        three_sum2(q1, q2, p3);
        s3 = q1;
        s4 = q2 + p2;

        return renorm5(s0, s1, s2, s3, s4);
    }
#endif

    // Multiplies a double by qd through the ordinary scalar product kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_double_product_inline(double a, const fqd_s& b) noexcept
    {
        return mul_double_product_inline(b, a);
    }

    // Multiplies qd by a double with range protection and canonical special handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_double_canonical_inline(const fqd_s& a, double b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        const fqd_s out = detail::fp::dekker_product_needs_scaling(a.x0, b)
            ? mul_double_product_range_safe_inline(a, b)
            : mul_double_product_inline(a, b);
        #else
        const fqd_s out = mul_double_product_inline(a, b);
        #endif

        if (detail::fp::isinf_or_nan(out.x0)) [[unlikely]]
            return mul_special(a, fqd_s{ b, 0.0, 0.0, 0.0 });
        if (out.x0 == 0.0 && (bl::iszero(a) || b == 0.0)) [[unlikely]]
            return mul_special(a, fqd_s{ b, 0.0, 0.0, 0.0 });
        return out;
    }

    // Multiplies finite qd by a double while retaining Dekker range protection.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_double_finite_inline(
        const fqd_s& a,
        double b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return detail::fp::dekker_product_needs_scaling(a.x0, b)
            ? mul_double_product_range_safe_inline(a, b)
            : mul_double_product_inline(a, b);
        #else
        return mul_double_product_inline(a, b);
        #endif
    }

    // Squares a qd expansion through its specialized symmetric product kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sqr_inline(const fqd_s& a) noexcept
    {
        using namespace detail::_qd;

        double p0{}, p1{}, p2{}, p3{}, p4{}, p5{};
        double q0{}, q1{}, q2{}, q3{}, q4{}, q5{};
        double p6{}, p7{}, p8{}, p9{};
        double q6{}, q7{}, q8{}, q9{};
        double r0{}, r1{};
        double t0{}, t1{};
        double s0{}, s1{}, s2{};

        #if FLTX_FQD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (qd_runtime_simd_enabled())
        {
            simd::f64x2 p01{}, q01{};
            simd::f64x2 p34{}, q34{};
            simd::f64x2 p67{}, q67{};

            simd::f64x2_two_prod_precise(simd::f64x2_set(a.x0, a.x0), simd::f64x2_set(a.x0, a.x1), p01, q01);
            simd::f64x2_two_prod_precise(simd::f64x2_set(a.x0, a.x1), simd::f64x2_set(a.x2, a.x1), p34, q34);
            simd::f64x2_two_prod_precise(simd::f64x2_set(a.x0, a.x1), simd::f64x2_set(a.x3, a.x2), p67, q67);

            simd::f64x2_store(p01, p0, p1);
            simd::f64x2_store(q01, q0, q1);
            simd::f64x2_store(p34, p3, p4);
            simd::f64x2_store(q34, q3, q4);
            simd::f64x2_store(p67, p6, p7);
            simd::f64x2_store(q67, q6, q7);
        }
        else
            #endif
        {
            two_prod_precise(a.x0, a.x0, p0, q0);
            two_prod_precise(a.x0, a.x1, p1, q1);
            two_prod_precise(a.x0, a.x2, p3, q3);
            two_prod_precise(a.x1, a.x1, p4, q4);
            two_prod_precise(a.x0, a.x3, p6, q6);
            two_prod_precise(a.x1, a.x2, p7, q7);
        }
        p2 = p1;
        q2 = q1;
        p5 = p3;
        q5 = q3;

        three_sum(p1, p2, q0);
        three_sum(p2, q1, q2);
        three_sum(p3, p4, p5);

        two_sum_precise(p2, p3, s0, t0);
        two_sum_precise(q1, p4, s1, t1);
        s2 = q2 + p5;
        two_sum_precise(s1, t0, s1, t0);
        s2 += (t0 + t1);

        p8 = p7;
        q8 = q7;
        p9 = p6;
        q9 = q6;

        two_sum_precise(q0, q3, q0, q3);
        two_sum_precise(q4, q5, q4, q5);
        two_sum_precise(p6, p7, p6, p7);
        two_sum_precise(p8, p9, p8, p9);

        two_sum_precise(q0, q4, t0, t1);  t1 += (q3 + q5);
        two_sum_precise(p6, p8, r0, r1);  r1 += (p7 + p9);
        two_sum_precise(t0, r0, q3, q4);  q4 += (t1 + r1);

        two_sum_precise(q3, s1, t0, t1);
        t1 += q4;
        t1 += a.x1 * a.x3 + a.x2 * a.x2 + a.x3 * a.x1 + q6 + q7 + q8 + q9 + s2;

        return renorm5(p0, p1, s0, t0, t1);
    }

    // Squares with the symmetric product kernel and public range/special-value policy.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sqr_canonical_inline(const fqd_s& a) noexcept
    {
        if (detail::fp::dekker_product_needs_scaling(a.x0, a.x0)) [[unlikely]]
            return mul_canonical_inline(a, a);
        return finish_mul_canonical_inline(a, a, sqr_inline(a));
    }

    // Multiplies qd by double-double through constexpr or compiled canonical dispatch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_dd(const fqd_s& a, dd_scalar b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            mul_canonical_inline(a, fqd_s{ b.hi, b.lo, 0.0, 0.0 }),
            detail::_qd_runtime::mul_dd(a, b)
        );
    }

    // Division
    // clang-cl /fp:fast can discard the residual roundoff terms used by the
    // quotient refinements. The function-local guards below preserve them
    // without changing fast-math code generation elsewhere.
    // Tests whether one residual product needs range-safe Dekker splitting.
    [[nodiscard]] BL_FORCE_INLINE constexpr bool div_residual_product_needs_range_safe_dekker(
        double a,
        double b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return detail::fp::dekker_product_needs_scaling(a, b);
        #else
        (void)a;
        (void)b;
        return false;
        #endif
    }

    // Forms one exact residual product with range protection when required.
    BL_FORCE_INLINE constexpr void div_residual_product_inline(
        double a,
        double b,
        double& p,
        double& e) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        detail::fp::two_prod_precise_range_safe(a, b, p, e);
        #else
        detail::fp::two_prod_precise(a, b, p, e);
        #endif
    }

    // Subtracts one quotient product with the faster compressed residual path.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_residual_fast_inline(const fqd_s& r, const fqd_s& b, double q) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        double p0{}, e0{};
        double p1{}, e1{};
        double p2{}, e2{};
        double p3{}, e3{};

        [[maybe_unused]] const bool needs_range_safe_dekker =
            div_residual_product_needs_range_safe_dekker(b.x0, q) ||
            div_residual_product_needs_range_safe_dekker(b.x1, q) ||
            div_residual_product_needs_range_safe_dekker(b.x2, q) ||
            div_residual_product_needs_range_safe_dekker(b.x3, q);

        #if FLTX_FQD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (!needs_range_safe_dekker && qd_runtime_simd_enabled())
        {
            simd::f64x2 p01{}, e01{};
            simd::f64x2 p23{}, e23{};
            const simd::f64x2 qv = simd::f64x2_splat(q);
            simd::f64x2_two_prod_precise(simd::f64x2_set(b.x0, b.x1), qv, p01, e01);
            simd::f64x2_two_prod_precise(simd::f64x2_set(b.x2, b.x3), qv, p23, e23);
            simd::f64x2_store(p01, p0, p1);
            simd::f64x2_store(e01, e0, e1);
            simd::f64x2_store(p23, p2, p3);
            simd::f64x2_store(e23, e2, e3);
        }
        else
        #endif
        {
            div_residual_product_inline(b.x0, q, p0, e0);
            div_residual_product_inline(b.x1, q, p1, e1);
            div_residual_product_inline(b.x2, q, p2, e2);
            div_residual_product_inline(b.x3, q, p3, e3);
        }

        double s0 = r.x0-p0; double v0 = s0-r.x0; double u0 = s0-v0; double w0 = r.x0-u0;  u0 = -p0 - v0;
        double s1 = r.x1-p1; double v1 = s1-r.x1; double u1 = s1-v1; double w1 = r.x1-u1;  u1 = -p1 - v1;
        double s2 = r.x2-p2; double v2 = s2-r.x2; double u2 = s2-v2; double w2 = r.x2-u2;  u2 = -p2 - v2;
        double s3 = r.x3-p3; double v3 = s3-r.x3; double u3 = s3-v3; double w3 = r.x3-u3;  u3 = -p3 - v3;

        double t0 = w0 + u0;
        double t1 = w1 + u1;
        double t2 = w2 + u2;
        double t3 = w3 + u3;

        double tail0 = t0 - e0; two_sum_precise(s1, tail0, s1, t0);
        double tail1 = t1 - e1; three_sum(s2, t0, tail1);
        double tail2 = t2 - e2; three_sum2(s3, t0, tail2);

        t0 = t0 + tail1 + t3 - e3;

        return renorm5(s0, s1, s2, s3, t0);
    }

    // Subtracts one quotient product while retaining the full residual expansion.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_residual_exact_inline(const fqd_s& r, const fqd_s& b, double q) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        double p0{}, p1{}, p2{}, p3{};
        double q0{}, q1{}, q2{};
        double s0{}, s1{}, s2{}, s3{}, s4{};

        [[maybe_unused]] const bool needs_range_safe_dekker =
            div_residual_product_needs_range_safe_dekker(b.x0, q) ||
            div_residual_product_needs_range_safe_dekker(b.x1, q) ||
            div_residual_product_needs_range_safe_dekker(b.x2, q) ||
            div_residual_product_needs_range_safe_dekker(b.x3, q);

        #if FLTX_FQD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (!needs_range_safe_dekker && qd_runtime_simd_enabled())
        {
            simd::f64x2 p01{}, q01{};
            simd::f64x2 p23{}, q23{};
            const simd::f64x2 qv = simd::f64x2_splat(q);
            simd::f64x2_two_prod_precise(simd::f64x2_set(b.x0, b.x1), qv, p01, q01);
            simd::f64x2_two_prod_precise(simd::f64x2_set(b.x2, b.x3), qv, p23, q23);
            double ignored{};
            simd::f64x2_store(p01, p0, p1);
            simd::f64x2_store(q01, q0, q1);
            simd::f64x2_store(p23, p2, p3);
            simd::f64x2_store(q23, q2, ignored);
        }
        else
        #endif
        {
            div_residual_product_inline(b.x0, q, p0, q0);
            div_residual_product_inline(b.x1, q, p1, q1);
            div_residual_product_inline(b.x2, q, p2, q2);
            p3 = b.x3 * q;
        }

        s0 = p0;
        two_sum_precise(q0, p1, s1, s2);
        three_sum(s2, q1, p2);
        three_sum2(q1, q2, p3);
        s3 = q1;
        s4 = q2 + p2;

        double c0{}, e0{};
        double c1{}, e1{};
        double c2{}, e2{};
        double c3{}, e3{};

        two_sum_precise(r.x0, -s0, c0, e0);
        two_sum_precise(r.x1, -s1, c1, e1);
        two_sum_precise(r.x2, -s2, c2, e2);
        two_sum_precise(r.x3, -s3, c3, e3);

        two_sum_precise(c1, e0, c1, e0);
        three_sum(c2, e0, e1);
        three_sum2(c3, e0, e2);

        e0 += e1 + e3 - s4;

        return renorm5(c0, c1, c2, c3, e0);
    }

    // Refines an initial qd quotient with exact then fast residual corrections.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_correction_inline(
        const fqd_s& a,
        const fqd_s& b,
        double inv_b0,
        double q0) noexcept
    {
        using namespace detail::_qd;

        fqd_s r = div_residual_exact_inline(a, b, q0);

        const double q1 = r.x0 * inv_b0;
        r = div_residual_exact_inline(r, b, q1);

        const double q2 = r.x0 * inv_b0;
        r = div_residual_fast_inline(r, b, q2);

        const double q3 = r.x0 * inv_b0;
        r = div_residual_fast_inline(r, b, q3);

        const double q4 = r.x0 * inv_b0;

        return renorm5(q0, q1, q2, q3, q4);
    }

    // Divides qd after the caller has handled exceptional denominator cases.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_prechecked_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        const double inv_b0 = 1.0 / b.x0;
        const double q0 = a.x0 * inv_b0;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return fqd_s{ q0, 0.0, 0.0, 0.0 };
        if (q0 == 0.0 && bl::iszero(a)) [[unlikely]]
            return signed_zero(bl::signbit(a) != bl::signbit(b));

        return div_correction_inline(a, b, inv_b0, q0);
    }

    // Divides qd values with exceptional-denominator handling and tiny-divisor scaling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_canonical_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(b.x0)) [[unlikely]]
            return div_special(a, b);

        if (detail::fp::absd(b.x0) < 0x1p-500
            && detail::fp::absd(a.x0) < 0x1p500) [[unlikely]]
        {
            constexpr int scale = 512;
            const fqd_s scaled_a{
                detail::fp::ldexp_limb(a.x0, scale),
                detail::fp::ldexp_limb(a.x1, scale),
                detail::fp::ldexp_limb(a.x2, scale),
                detail::fp::ldexp_limb(a.x3, scale)
            };
            const fqd_s scaled_b{
                detail::fp::ldexp_limb(b.x0, scale),
                detail::fp::ldexp_limb(b.x1, scale),
                detail::fp::ldexp_limb(b.x2, scale),
                detail::fp::ldexp_limb(b.x3, scale)
            };
            return div_prechecked_inline(scaled_a, scaled_b);
        }

        return div_prechecked_inline(a, b);
    }

    // Divides finite qd values with tiny-divisor scaling but no canonical special handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_finite_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        if (detail::fp::absd(b.x0) < 0x1p-500
            && detail::fp::absd(a.x0) < 0x1p500) [[unlikely]]
        {
            constexpr int scale = 512;
            const fqd_s scaled_a{
                detail::fp::ldexp_limb(a.x0, scale),
                detail::fp::ldexp_limb(a.x1, scale),
                detail::fp::ldexp_limb(a.x2, scale),
                detail::fp::ldexp_limb(a.x3, scale)
            };
            const fqd_s scaled_b{
                detail::fp::ldexp_limb(b.x0, scale),
                detail::fp::ldexp_limb(b.x1, scale),
                detail::fp::ldexp_limb(b.x2, scale),
                detail::fp::ldexp_limb(b.x3, scale)
            };
            const double inv_b0 = 1.0 / scaled_b.x0;
            return div_correction_inline(scaled_a, scaled_b, inv_b0, scaled_a.x0 * inv_b0);
        }

        const double inv_b0 = 1.0 / b.x0;
        return div_correction_inline(a, b, inv_b0, a.x0 * inv_b0);
    }

    // Divides qd by a double after exceptional denominator cases are handled.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_double_prechecked_inline(const fqd_s& a, double b) noexcept
    {
        using namespace detail::_qd;

        if (bl::detail::is_constant_evaluated())
        {
            if (detail::fp::isnan(a.x0) || detail::fp::isnan(b))
                return std::numeric_limits<fqd_s>::quiet_NaN();

            if (isinf(b))
            {
                if (isinf(a))
                    return std::numeric_limits<fqd_s>::quiet_NaN();

                const bool neg = signbit(a.x0) ^ signbit(b);
                return signed_zero(neg);
            }

            if (b == 0.0)
            {
                if (iszero(a))
                    return std::numeric_limits<fqd_s>::quiet_NaN();

                const bool neg = signbit(a.x0) ^ signbit(b);
                return fqd_s{ neg ? -std::numeric_limits<double>::infinity()
                                   : std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
            }

            if (isinf(a))
            {
                const bool neg = signbit(a.x0) ^ signbit(b);
                return fqd_s{ neg ? -std::numeric_limits<double>::infinity()
                                   : std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
            }
        }

        const double inv_b = 1.0 / b;
        const fqd_s divisor{ b, 0.0, 0.0, 0.0 };

        const double q0 = a.x0 * inv_b;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return fqd_s{ q0, 0.0, 0.0, 0.0 };
        if (q0 == 0.0 && bl::iszero(a)) [[unlikely]]
            return signed_zero(bl::signbit(a) != signbit(b));

        fqd_s r = div_residual_exact_inline(a, divisor, q0);

        const double q1 = r.x0 * inv_b; r = div_residual_fast_inline(r, divisor, q1);
        const double q2 = r.x0 * inv_b; r = div_residual_fast_inline(r, divisor, q2);
        const double q3 = r.x0 * inv_b; r = div_residual_fast_inline(r, divisor, q3);
        const double q4 = r.x0 * inv_b;

        return renorm5(q0, q1, q2, q3, q4);
    }

    // Divides a double by qd after exceptional denominator cases are handled.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_double_prechecked_inline(double a, const fqd_s& b) noexcept
    {
        using namespace detail::_qd;

        if (bl::detail::is_constant_evaluated())
        {
            if (detail::fp::isnan(a) || detail::fp::isnan(b.x0))
                return std::numeric_limits<fqd_s>::quiet_NaN();

            if (isinf(b))
            {
                if (isinf(a))
                    return std::numeric_limits<fqd_s>::quiet_NaN();

                const bool neg = signbit(a) ^ signbit(b.x0);
                return signed_zero(neg);
            }

            if (iszero(b))
            {
                if (a == 0.0)
                    return std::numeric_limits<fqd_s>::quiet_NaN();

                const bool neg = signbit(a) ^ signbit(b.x0);
                return fqd_s{ neg ? -std::numeric_limits<double>::infinity()
                                   : std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
            }

            if (isinf(a))
            {
                const bool neg = signbit(a) ^ signbit(b.x0);
                return fqd_s{ neg ? -std::numeric_limits<double>::infinity()
                                   : std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
            }
        }

        if (b.x1 == 0.0 && b.x2 == 0.0 && b.x3 == 0.0) [[unlikely]]
            return div_double_prechecked_inline(fqd_s{ a, 0.0, 0.0, 0.0 }, b.x0);

        const double inv_b0 = 1.0 / b.x0;
        const double q0     = a * inv_b0;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return fqd_s{ q0, 0.0, 0.0, 0.0 };
        if (q0 == 0.0 && a == 0.0) [[unlikely]]
            return signed_zero(signbit(a) != bl::signbit(b));

        double p0{}, p1{}, p2{}, p3{};
        double e0{}, e1{}, e2{};
        double s0{}, s1{}, s2{}, s3{}, s4{};

        div_residual_product_inline(b.x0, q0, p0, e0);
        div_residual_product_inline(b.x1, q0, p1, e1);
        div_residual_product_inline(b.x2, q0, p2, e2);
        p3 = b.x3 * q0;

        s0 = p0;
        two_sum_precise(e0, p1, s1, s2);
        three_sum(s2, e1, p2);
        three_sum2(e1, e2, p3);
        s3 = e1;
        s4 = e2 + p2;

        double c0{}, t0{};
        two_sum_precise(a, -s0, c0, t0);

        double c1 = -s1;
        double c2 = -s2;
        double c3 = -s3;
        double t1 = 0.0;
        double t2 = 0.0;

        two_sum_precise(c1, t0, c1, t0);
        three_sum(c2, t0, t1);
        three_sum2(c3, t0, t2);
        t0 += t1 - s4;

        fqd_s r = renorm5(c0, c1, c2, c3, t0);

        const double q1 = r.x0 * inv_b0; r = div_residual_fast_inline(r, b, q1);
        const double q2 = r.x0 * inv_b0; r = div_residual_fast_inline(r, b, q2);
        const double q3 = r.x0 * inv_b0; r = div_residual_fast_inline(r, b, q3);
        const double q4 = r.x0 * inv_b0;

        return renorm5(q0, q1, q2, q3, q4);
    }

    // Divides qd by a double with canonical exceptional-denominator handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_double_canonical_inline(const fqd_s& a, double b) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(b)) [[unlikely]]
            return div_special(a, fqd_s{ b, 0.0, 0.0, 0.0 });

        return div_double_prechecked_inline(a, b);
    }

    // Divides a double by qd with canonical exceptional-denominator handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_double_canonical_inline(double a, const fqd_s& b) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(b.x0)) [[unlikely]]
            return div_special(fqd_s{ a, 0.0, 0.0, 0.0 }, b);

        return div_double_prechecked_inline(a, b);
    }

    // Divides finite qd by a finite double without canonical special handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_double_finite_inline(
        const fqd_s& a,
        double b) noexcept
    {
        using namespace detail::_qd;

        const double inv_b = 1.0 / b;
        const fqd_s divisor{ b, 0.0, 0.0, 0.0 };
        const double q0 = a.x0 * inv_b;
        fqd_s r = div_residual_exact_inline(a, divisor, q0);

        const double q1 = r.x0 * inv_b; r = div_residual_fast_inline(r, divisor, q1);
        const double q2 = r.x0 * inv_b; r = div_residual_fast_inline(r, divisor, q2);
        const double q3 = r.x0 * inv_b; r = div_residual_fast_inline(r, divisor, q3);
        const double q4 = r.x0 * inv_b;

        return renorm5(q0, q1, q2, q3, q4);
    }

    // Divides qd by double-double through constexpr or compiled canonical dispatch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_dd(const fqd_s& a, dd_scalar b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            div_canonical_inline(a, fqd_s{ b.hi, b.lo, 0.0, 0.0 }),
            detail::_qd_runtime::div_dd(a, b)
        );
    }

    // Divides double-double by qd through constexpr or compiled canonical dispatch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_dd(dd_scalar a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            div_canonical_inline(fqd_s{ a.hi, a.lo, 0.0, 0.0 }, b),
            detail::_qd_runtime::div_dd(a, b)
        );
    }

} // namespace detail::_qd

// reciprocal helpers
[[nodiscard]] BL_FORCE_INLINE constexpr fqd recip(fqd_s b) noexcept
{
    using namespace detail::_qd;

    if (iszero(b)) [[unlikely]]
        return signed_infinity(signbit(b));
    if (isinf(b)) [[unlikely]]
        return signed_zero(signbit(b));

    constexpr fqd_s one = fqd_s{ 1.0 };

#if BL_FP_BARRIER_ACTIVE
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        div_prechecked_inline(one, b),
        detail::_qd_runtime::div_finite(one, b)
    );
#else
    const double inv_b0 = 1.0 / b.x0;
    const double q0     = inv_b0;
    fqd_s r = div_residual_exact_inline(one, b, q0);

    const double q1 = r.x0 * inv_b0; r = div_residual_fast_inline(r, b, q1);
    const double q2 = r.x0 * inv_b0; r = div_residual_fast_inline(r, b, q2);
    const double q3 = r.x0 * inv_b0; r = div_residual_fast_inline(r, b, q3);
    const double q4 = r.x0 * inv_b0;

    return renorm5(q0, q1, q2, q3, q4);
#endif
}

} // namespace bl

#endif
