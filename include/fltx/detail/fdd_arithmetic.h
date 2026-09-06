/**
 * fltx/detail/fdd_arithmetic.h - Low-level double-double arithmetic helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_DETAIL_ARITHMETIC_INCLUDED
#define FDD_DETAIL_ARITHMETIC_INCLUDED
#include "fltx/fdd_classification.h"
#include "fltx/fdd_conversions.h"
#include "fltx/fdd_limits.h"

namespace bl {

namespace detail::_dd // primitives and kernels
{
    // Tests one binary64 limb for either sign of zero without floating-point comparisons.
    [[nodiscard]] BL_FORCE_INLINE constexpr bool limb_is_zero(double value) noexcept
    {
        return (std::bit_cast<std::uint64_t>(value) & 0x7fffffffffffffffull) == 0;
    }

    // Tests whether both limbs of a dd value are zero.
    [[nodiscard]] BL_FORCE_INLINE constexpr bool value_is_zero(const fdd_s& value) noexcept
    {
        return limb_is_zero(value.hi) && limb_is_zero(value.lo);
    }

    // Constructs the canonical quiet NaN used by public dd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s quiet_nan() noexcept
    {
        return { std::bit_cast<double>(0x7ff8000000000000ull), 0.0 };
    }

    // Constructs a canonical signed infinity for public dd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s signed_infinity(bool negative) noexcept
    {
        return { std::bit_cast<double>(negative ? 0xfff0000000000000ull : 0x7ff0000000000000ull), 0.0 };
    }

    // Constructs a canonical signed zero for public dd arithmetic.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s signed_zero(bool negative) noexcept
    {
        return { std::bit_cast<double>(negative ? 0x8000000000000000ull : 0ull), 0.0 };
    }

    // Resolves non-finite addition results according to the public dd policy.
    [[nodiscard]] BL_NO_INLINE constexpr fdd_s add_special(const fdd_s& a, const fdd_s& b) noexcept
    {
        if (detail::fp::isnan(a.hi) || detail::fp::isnan(b.hi))
            return quiet_nan();

        const bool a_inf = isinf(a.hi);
        const bool b_inf = isinf(b.hi);
        if (a_inf && b_inf && signbit(a.hi) != signbit(b.hi))
            return quiet_nan();
        if (a_inf)
            return signed_infinity(signbit(a.hi));
        if (b_inf)
            return signed_infinity(signbit(b.hi));

        return signed_infinity(signbit(a));
    }

    // Resolves non-finite subtraction results according to the public dd policy.
    [[nodiscard]] BL_NO_INLINE constexpr fdd_s sub_special(const fdd_s& a, const fdd_s& b) noexcept
    {
        return add_special(a, fdd_s{ -b.hi, -b.lo });
    }

    // Restores the sign that renormalization can lose when two negative values
    // add to zero. For nonzero results the sign bit is already set, so the OR
    // is value-preserving and avoids a hot-path branch.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s restore_add_result_sign(
        const fdd_s& a,
        const fdd_s& b,
        fdd_s out) noexcept
    {
        constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
        const std::uint64_t negative_sign =
            std::bit_cast<std::uint64_t>(a.hi) &
            std::bit_cast<std::uint64_t>(b.hi) & sign_mask;
        out.hi = std::bit_cast<double>(
            std::bit_cast<std::uint64_t>(out.hi) | negative_sign);
        return out;
    }

    // Restores the sign that renormalization can lose when a negative value
    // minus a positive value rounds to zero.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s restore_sub_result_sign(
        const fdd_s& a,
        const fdd_s& b,
        fdd_s out) noexcept
    {
        constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
        const std::uint64_t negative_sign =
            std::bit_cast<std::uint64_t>(a.hi) &
            ~std::bit_cast<std::uint64_t>(b.hi) & sign_mask;
        out.hi = std::bit_cast<double>(
            std::bit_cast<std::uint64_t>(out.hi) | negative_sign);
        return out;
    }

    // Resolves non-finite and signed-zero multiplication results for public dd arithmetic.
    [[nodiscard]] BL_NO_INLINE constexpr fdd_s mul_special(const fdd_s& a, const fdd_s& b) noexcept
    {
        if (detail::fp::isnan(a.hi) || detail::fp::isnan(b.hi))
            return quiet_nan();

        const bool a_inf = isinf(a.hi);
        const bool b_inf = isinf(b.hi);
        const bool a_zero = value_is_zero(a);
        const bool b_zero = value_is_zero(b);
        if ((a_inf && b_zero) || (b_inf && a_zero))
            return quiet_nan();

        const bool negative = bl::signbit(a) != bl::signbit(b);
        if (a_zero || b_zero)
            return signed_zero(negative);

        return signed_infinity(negative);
    }

    // Resolves zero, infinity, and NaN division cases for public dd arithmetic.
    [[nodiscard]] BL_NO_INLINE constexpr fdd_s div_special(const fdd_s& a, const fdd_s& b) noexcept
    {
        if (detail::fp::isnan(a.hi) || detail::fp::isnan(b.hi))
            return quiet_nan();

        const bool a_zero = a.hi == 0.0;
        const bool b_zero = b.hi == 0.0;
        const bool negative = bl::signbit(a) != bl::signbit(b);
        if (b_zero)
            return a_zero ? quiet_nan() : signed_infinity(negative);

        const bool a_inf = isinf(a.hi);
        const bool b_inf = isinf(b.hi);
        if (a_inf && b_inf)
            return quiet_nan();
        if (a_inf)
            return signed_infinity(negative);
        return signed_zero(negative);
    }

    // ----- Addition -----

    BL_PUSH_PRECISE;
    // Adds dd expansions without applying canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s add_finite_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        double s1{}, s2{};
        two_sum_precise(a.hi, b.hi, s1, s2);

        double t1{}, t2{};
        two_sum_precise(a.lo, b.lo, t1, t2);

        s2 += t1;
        detail::fp::quick_two_sum_precise(s1, s2, s1, s2);
        s2 += t2;
        detail::fp::quick_two_sum_precise(s1, s2, s1, s2);
        return { s1, s2 };
    }

    // Adds dd values and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s add_canonical_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        const fdd_s out = restore_add_result_sign(a, b, add_finite_inline(a, b));
        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return add_special(a, b);
        return out;
    }

    BL_POP_PRECISE;

    // Adds a double to a dd expansion without canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s add_double_finite_inline(const fdd_s& a, double b) noexcept
    {
        double s{}, e{};
        two_sum_precise(a.hi, b, s, e);
#if BL_FP_BARRIER_ACTIVE
        BL_FP_BARRIER(e);
        e += a.lo;
        BL_FP_BARRIER(e);
#else
        e += a.lo;
#endif
        return renorm(s, e);
    }

    // Adds a dd expansion to a double through the finite scalar kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s add_double_finite_inline(double a, const fdd_s& b) noexcept
    {
        return add_double_finite_inline(b, a);
    }

    // Adds a double to dd and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s add_double_canonical_inline(const fdd_s& a, double b) noexcept
    {
        const fdd_s rhs{ b, 0.0 };
        const fdd_s out = restore_add_result_sign(a, rhs, add_double_finite_inline(a, b));
        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return add_special(a, rhs);
        return out;
    }

    // ----- Subtraction -----

    BL_PUSH_PRECISE;
    // Subtracts dd expansions without applying canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sub_finite_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        double s1{}, s2{};
        detail::fp::two_diff_precise(a.hi, b.hi, s1, s2);

        double t1{}, t2{};
        detail::fp::two_diff_precise(a.lo, b.lo, t1, t2);

        s2 += t1;
        detail::fp::quick_two_sum_precise(s1, s2, s1, s2);
        s2 += t2;
        detail::fp::quick_two_sum_precise(s1, s2, s1, s2);
        return { s1, s2 };
    }

    // Subtracts dd values and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sub_canonical_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        const fdd_s out = restore_sub_result_sign(a, b, sub_finite_inline(a, b));
        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return sub_special(a, b);
        return out;
    }
    BL_POP_PRECISE;

    // Subtracts a double from dd without canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sub_double_finite_inline(const fdd_s& a, double b) noexcept
    {
        return add_double_finite_inline(a, -b);
    }

    // Subtracts dd from a double through the finite scalar kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sub_double_finite_inline(double a, const fdd_s& b) noexcept
    {
        return add_double_finite_inline(-b, a);
    }

    // Subtracts a double from dd and canonicalizes the result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sub_double_canonical_inline(const fdd_s& a, double b) noexcept
    {
        const fdd_s rhs{ b, 0.0 };
        const fdd_s out = restore_sub_result_sign(a, rhs, sub_double_finite_inline(a, b));
        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return sub_special(a, rhs);
        return out;
    }

    // ----- Multiplication -----

    BL_PUSH_PRECISE;
    // Forms an unrenormalized dd product with the ordinary Dekker split.
    BL_FORCE_INLINE constexpr void mul_expansion_inline(const fdd_s& a, const fdd_s& b, double& p, double& e) noexcept
    {
        two_prod_precise(a.hi, b.hi, p, e);

        e += a.hi * b.lo + a.lo * b.hi;
        e += a.lo * b.lo;
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Forms an unrenormalized dd product with range-safe Dekker splitting.
    BL_FORCE_INLINE constexpr void mul_expansion_range_safe_inline(const fdd_s& a, const fdd_s& b, double& p, double& e) noexcept
    {
        detail::fp::two_prod_precise_range_safe(a.hi, b.hi, p, e);

        e += a.hi * b.lo + a.lo * b.hi;
        e += a.lo * b.lo;
    }
#endif
    BL_POP_PRECISE;

    // Forms and renormalizes a dd product with the ordinary Dekker split.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_product_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        double p{}, e{};
        two_prod_precise(a.hi, b.hi, p, e);
#if BL_FP_BARRIER_ACTIVE
        BL_FP_BARRIER(e);
        double cross = a.hi * b.lo;  BL_FP_BARRIER(cross);
        double term = a.lo * b.hi;   BL_FP_BARRIER(term);
        cross += term;               BL_FP_BARRIER(cross);
        e += cross;                  BL_FP_BARRIER(e);
#else
        e += a.hi * b.lo + a.lo * b.hi;
#endif
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return { p, e };
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Forms and renormalizes a dd product with range-safe Dekker splitting.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_product_range_safe_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        double p{}, e{};
        detail::fp::two_prod_precise_range_safe(a.hi, b.hi, p, e);
        e += a.hi * b.lo + a.lo * b.hi;
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return { p, e };
    }
#endif

    // Squares a dd expansion with the ordinary Dekker split.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sqr_inline(const fdd_s& a) noexcept
    {
        double p{}, e{};
        two_prod_precise(a.hi, a.hi, p, e);
#if BL_FP_BARRIER_ACTIVE
        BL_FP_BARRIER(e);
        double doubled_hi = a.hi + a.hi;  BL_FP_BARRIER(doubled_hi);
        double cross = doubled_hi * a.lo;  BL_FP_BARRIER(cross);
        e += cross;                         BL_FP_BARRIER(e);
        double tail = a.lo * a.lo;          BL_FP_BARRIER(tail);
        e += tail;                          BL_FP_BARRIER(e);
#else
        e += (a.hi + a.hi) * a.lo;
        e += a.lo * a.lo;
#endif
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return { p, e };
    }

    // Converts a formed product into the canonical public multiplication result.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s finish_mul_canonical_inline(const fdd_s& a, const fdd_s& b, const fdd_s& out) noexcept
    {
        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return mul_special(a, b);
        if (limb_is_zero(out.hi) && (value_is_zero(a) || value_is_zero(b))) [[unlikely]]
            return mul_special(a, b);
        return out;
    }

    // Handles the complete multiplication range and canonical public special values.
    [[nodiscard]] BL_NO_INLINE constexpr fdd_s mul_canonical_fallback(const fdd_s& a, const fdd_s& b) noexcept
    {
        if (detail::fp::isinf_or_nan(a.hi) || detail::fp::isinf_or_nan(b.hi) ||
            value_is_zero(a) || value_is_zero(b)) [[unlikely]]
            return mul_special(a, b);

        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        if (detail::fp::dekker_product_needs_scaling(a.hi, b.hi)) [[unlikely]]
            return finish_mul_canonical_inline(a, b, mul_product_range_safe_inline(a, b));
        #endif

        return finish_mul_canonical_inline(a, b, mul_product_inline(a, b));
    }

    // Tests whether ordinary Dekker multiplication is safe for two leading limbs.
    [[nodiscard]] BL_FORCE_INLINE constexpr bool mul_product_fast_path_is_range_safe(double a, double b) noexcept
    {
        constexpr std::uint64_t sign_mask = std::uint64_t{ 1 } << 63;
        constexpr std::uint64_t minimum = UINT64_C(0x0370000000000000); // 2^-968
        constexpr std::uint64_t maximum = UINT64_C(0x7e30000000000000); // 2^996

        const std::uint64_t a_magnitude = std::bit_cast<std::uint64_t>(a) & ~sign_mask;
        const std::uint64_t b_magnitude = std::bit_cast<std::uint64_t>(b) & ~sign_mask;
        const std::uint64_t product_exponent_sum = (a_magnitude >> 52) + (b_magnitude >> 52);

        // The magnitude bounds keep Dekker's split away from overflow and
        // underflow. An exponent-field sum of at most 3068 guarantees that
        // multiplying the two leading limbs stays finite. Keep the tests
        // branchless so callers need only one cold-path branch.
        return
            ((a_magnitude - minimum) <= (maximum - minimum)) &
            ((b_magnitude - minimum) <= (maximum - minimum)) &
            (product_exponent_sum <= 3068u);
    }

    // Multiplies dd values through the hot product path or the canonical fallback.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_canonical_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            mul_canonical_fallback(a, b),
            mul_product_fast_path_is_range_safe(a.hi, b.hi)
                ? mul_product_inline(a, b)
                : mul_canonical_fallback(a, b)
        );
    }

    // Multiplies finite dd values while retaining Dekker range protection.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_finite_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return detail::fp::dekker_product_needs_scaling(a.hi, b.hi)
            ? mul_product_range_safe_inline(a, b)
            : mul_product_inline(a, b);
        #else
        return mul_product_inline(a, b);
        #endif
    }

    // Multiplies dd values with range-safe Dekker formation and canonical finalization.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_dekker_range_safe_canonical_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return finish_mul_canonical_inline(a, b, mul_product_range_safe_inline(a, b));
        #else
        return mul_canonical_inline(a, b);
        #endif
    }

    // Multiplies dd values while retaining additional cross-product error terms.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_accurate_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        if (detail::fp::isinf_or_nan(a.hi) || detail::fp::isinf_or_nan(b.hi)) [[unlikely]]
            return mul_canonical_inline(a, b);

        double p0{}, q0{};
        two_prod_precise(a.hi, b.hi, p0, q0);
        if (detail::fp::isinf_or_nan(p0)) [[unlikely]]
            return mul_canonical_inline(a, b);

        double p1{}, q1{};
        double p2{}, q2{};
        two_prod_precise(a.hi, b.lo, p1, q1);
        two_prod_precise(a.lo, b.hi, p2, q2);

        double p12{}, e12{};
        double p012{}, e012{};
        two_sum_precise(p1, p2, p12, e12);
        two_sum_precise(q0, p12, p012, e012);

        const double tail = e12 + e012 + q1 + q2 + (a.lo * b.lo);
        return renorm(p0, p012 + tail);
    }

    // Squares dd with range-safe Dekker formation and canonical finalization.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sqr_dekker_range_safe_canonical_inline(const fdd_s& a) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        double p{}, e{};
        detail::fp::two_prod_precise_range_safe(a.hi, a.hi, p, e);
        e += (a.hi + a.hi) * a.lo;
        e += a.lo * a.lo;
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return finish_mul_canonical_inline(a, a, fdd_s{ p, e });
        #else
        return sqr_inline(a);
        #endif
    }

    // Forms a dd-by-double product with the ordinary Dekker split.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_double_product_inline(const fdd_s& a, double b) noexcept
    {
        double p{}, e{};
        two_prod_precise(a.hi, b, p, e);

        e += a.lo * b;
        return renorm(p, e);
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Forms a dd-by-double product with range-safe Dekker splitting.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_double_product_range_safe_inline(const fdd_s& a, double b) noexcept
    {
        double p{}, e{};
        detail::fp::two_prod_precise_range_safe(a.hi, b, p, e);

        e += a.lo * b;
        return renorm(p, e);
    }
#endif

    // Multiplies dd by a double with range protection and canonical special handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_double_canonical_inline(const fdd_s& a, double b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        const fdd_s out = detail::fp::dekker_product_needs_scaling(a.hi, b)
            ? mul_double_product_range_safe_inline(a, b)
            : mul_double_product_inline(a, b);
        #else
        const fdd_s out = mul_double_product_inline(a, b);
        #endif

        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return mul_special(a, fdd_s{ b, 0.0 });
        if (out.hi == 0.0 && ((a.hi == 0.0 && a.lo == 0.0) || b == 0.0)) [[unlikely]]
            return mul_special(a, fdd_s{ b, 0.0 });
        return out;
    }

    // Multiplies finite dd by a double while retaining Dekker range protection.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_double_finite_inline(
        const fdd_s& a,
        double b) noexcept
    {
        #if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
        return detail::fp::dekker_product_needs_scaling(a.hi, b)
            ? mul_double_product_range_safe_inline(a, b)
            : mul_double_product_inline(a, b);
        #else
        return mul_double_product_inline(a, b);
        #endif
    }

    // Multiplies a double by dd through the ordinary scalar product kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_double_product_inline(double a, const fdd_s& b) noexcept
    {
        return mul_double_product_inline(b, a);
    }

    // Scales both dd limbs by an exactly representable power of two.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_pwr2_inline(const fdd_s& a, double b) noexcept
    {
        return { a.hi * b, a.lo * b };
    }

    // ----- Division -----

    // clang-cl /fp:fast can reassociate the quotient compensation back to
    // binary64 precision. Keep the exception function-local so other kernels
    // and every other compiler retain their normal fast-math code generation.

    // Computes an exact dd residual after subtracting one quotient product.
    BL_FORCE_INLINE constexpr fdd_s div_residual_exact_inline(const fdd_s& r, const fdd_s& b, double q) noexcept
    {
        double p{}, e{};
        two_prod_precise(b.hi, q, p, e);
        e += b.lo * q;

        double s{}, t{};
        detail::fp::two_diff_precise(r.hi, p, s, t);
        t += r.lo - e;

        return renorm(s, t);
    }

#if BL_FP_BARRIER_ACTIVE
    // Applies the compensated quotient correction with explicit evaluation barriers.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_correction_inline(
        const fdd_s& a,
        const fdd_s& b,
        double q0,
        double p0,
        double e0) noexcept
    {
        double residual = a.hi - p0;  BL_FP_BARRIER(residual);
        residual -= e0;               BL_FP_BARRIER(residual);
        residual += a.lo;             BL_FP_BARRIER(residual);
        residual -= q0 * b.lo;        BL_FP_BARRIER(residual);

        double q1 = residual / b.hi;  BL_FP_BARRIER(q1);
        double hi = q0 + q1;          BL_FP_BARRIER(hi);
        double lo = q0 - hi;          BL_FP_BARRIER(lo);
        lo += q1;                     BL_FP_BARRIER(lo);

        return { hi, lo };
    }
#endif

    // Divides dd after the caller has handled exceptional denominator cases.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_prechecked_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        const double q0 = a.hi / b.hi;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return { q0, 0.0 };
        if (q0 == 0.0 && a.hi == 0.0 && a.lo == 0.0) [[unlikely]]
            return signed_zero(bl::signbit(a) != bl::signbit(b));

        double p0{}, e0{};
        two_prod_precise(q0, b.hi, p0, e0);

#if BL_FP_BARRIER_ACTIVE
        return div_correction_inline(a, b, q0, p0, e0);
#else
        const double q1 = (((a.hi - p0) - e0) + a.lo - (q0 * b.lo)) / b.hi;
        const double hi = q0 + q1;
        return { hi, (q0 - hi) + q1 };
#endif
    }

    // Divides finite dd values without canonical special-value handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_finite_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        const double q0 = a.hi / b.hi;

        double p0{}, e0{};
        two_prod_precise(q0, b.hi, p0, e0);

#if BL_FP_BARRIER_ACTIVE
        return div_correction_inline(a, b, q0, p0, e0);
#else
        const double q1 = (((a.hi - p0) - e0) + a.lo - (q0 * b.lo)) / b.hi;
        const double hi = q0 + q1;
        return { hi, (q0 - hi) + q1 };
#endif
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Divides prechecked dd values with a range-safe Dekker residual product.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_prechecked_range_safe_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        const double q0 = a.hi / b.hi;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return { q0, 0.0 };
        if (q0 == 0.0 && a.hi == 0.0 && a.lo == 0.0) [[unlikely]]
            return signed_zero(bl::signbit(a) != bl::signbit(b));

        double p0{}, e0{};
        detail::fp::two_prod_precise_range_safe(q0, b.hi, p0, e0);

#if BL_FP_BARRIER_ACTIVE
        return div_correction_inline(a, b, q0, p0, e0);
#else
        const double q1 = (((a.hi - p0) - e0) + a.lo - (q0 * b.lo)) / b.hi;
        const double hi = q0 + q1;
        return { hi, (q0 - hi) + q1 };
#endif
    }
#endif

    // Divides dd values after applying the public exceptional-denominator policy.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_canonical_inline(const fdd_s& a, const fdd_s& b) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(b.hi)) [[unlikely]]
            return div_special(a, b);

        return div_prechecked_inline(a, b);
    }

    // Divides dd by a double after exceptional denominator cases are handled.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_double_prechecked_inline(const fdd_s& a, double b) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            if (detail::fp::isnan(a.hi) || detail::fp::isnan(b)) [[unlikely]]
                return std::numeric_limits<fdd_s>::quiet_NaN();

            if (isinf(b))
            {
                if (isinf(a.hi))
                    return std::numeric_limits<fdd_s>::quiet_NaN();

                const bool neg = signbit(a.hi) ^ signbit(b);
                return signed_zero(neg);
            }

            if (b == 0.0) [[unlikely]]
            {
                if (a.hi == 0.0 && a.lo == 0.0)
                    return std::numeric_limits<fdd_s>::quiet_NaN();

                const bool neg = signbit(a.hi) ^ signbit(b);
                return fdd_s{ neg ? -std::numeric_limits<double>::infinity()
                             : std::numeric_limits<double>::infinity(), 0.0 };
            }

            if (isinf(a.hi)) [[unlikely]]
            {
                const bool neg = signbit(a.hi) ^ signbit(b);
                return fdd_s{ neg ? -std::numeric_limits<double>::infinity()
                             : std::numeric_limits<double>::infinity(), 0.0 };
            }
        }

        return div_prechecked_inline(a, fdd_s{ b, 0.0 });
    }

    // Divides a double by dd through the prechecked full-value kernel.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_double_prechecked_inline(double a, const fdd_s& b) noexcept
    {
        return div_prechecked_inline(fdd_s{ a, 0.0 }, b);
    }

    // Divides dd by a double with canonical exceptional-denominator handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_double_canonical_inline(
        const fdd_s& a,
        double b) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(b)) [[unlikely]]
            return div_special(a, fdd_s{ b, 0.0 });

        return div_double_prechecked_inline(a, b);
    }

    // Divides finite dd by a finite double without canonical special handling.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s div_double_finite_inline(
        const fdd_s& a,
        double b) noexcept
    {
        return div_finite_inline(a, fdd_s{ b, 0.0 });
    }

    // Supporting product split, extracts the high splitter component while avoiding intermediate overflow.
    [[nodiscard]] BL_FORCE_INLINE constexpr double product_split_high(double value) noexcept
    {
#if defined(_MSC_VER) && defined(__clang__) && defined(FLTX_FAST_MATH)
#pragma clang fp reassociate(off)
#endif
        constexpr double split = 134217729.0;
        constexpr int split_shift = 27;

#if BL_FP_BARRIER_ACTIVE
        double scaled = split * value;  BL_FP_BARRIER(scaled);
        if (detail::fp::isinf(scaled))
        {
            double shifted = detail::fp::ldexp(value, -split_shift);  BL_FP_BARRIER(shifted);
            double delta = value - shifted;                           BL_FP_BARRIER(delta);
            double high = value - delta;                              BL_FP_BARRIER(high);
            high = detail::fp::ldexp(high, split_shift);              BL_FP_BARRIER(high);
            return high;
        }

        double delta = scaled - value;  BL_FP_BARRIER(delta);
        double high = scaled - delta;   BL_FP_BARRIER(high);
        return high;
#else
        const double scaled = split * value;
        if (detail::fp::isinf(scaled))
        {
            return detail::fp::ldexp(
                value - (value - detail::fp::ldexp(value, -split_shift)),
                split_shift);
        }

        return scaled - (scaled - value);
#endif
    }

    // ----- Fused expressions -----

    BL_PUSH_PRECISE;
    // Computes (a * b + c) while retaining the full product expansion.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_add_inline(const fdd_s& a, const fdd_s& b, const fdd_s& c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, c.hi, s, t);
        t += e + c.lo;
        return renorm(s, t);
    }

    // Computes (a * b + c) when a is a double.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_add_double_lhs_inline(double a, const fdd_s& b, const fdd_s& c) noexcept
    {
        double p{}, e{};
        two_prod_precise(a, b.hi, p, e);
        e += a * b.lo;

        double s{}, t{};
        two_sum_precise(p, c.hi, s, t);
        t += e + c.lo;
        return renorm(s, t);
    }

    // Computes (a * b + c) when c is a double.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_add_double_rhs_inline(const fdd_s& a, const fdd_s& b, double c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, c, s, t);
        t += e;
        return renorm(s, t);
    }

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
    // Computes (a * b + c) with range-safe Dekker product formation.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_add_range_safe_inline(const fdd_s& a, const fdd_s& b, const fdd_s& c) noexcept
    {
        double p{}, e{};
        mul_expansion_range_safe_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, c.hi, s, t);
        t += e + c.lo;
        return renorm(s, t);
    }
#endif

    // Computes (a * b - c) while retaining the full product expansion.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mul_sub_inline(const fdd_s& a, const fdd_s& b, const fdd_s& c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, -c.hi, s, t);
        t += e - c.lo;
        return renorm(s, t);
    }

    // Computes (c - a * b) while retaining the full product expansion.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sub_mul_inline(const fdd_s& c, const fdd_s& a, const fdd_s& b) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(c.hi, -p, s, t);
        t += c.lo - e;
        return renorm(s, t);
    }

    // Computes (a0 * b + c0) and (a1 * b + c1) together so the shared-b products can use SIMD.
    BL_FORCE_INLINE constexpr void mul_add_pair_same_rhs_inline(
        const fdd_s& a0,
        const fdd_s& a1,
        const fdd_s& b,
        const fdd_s& c0,
        const fdd_s& c1,
        fdd_s& out0,
        fdd_s& out1) noexcept
    {
        double p0{}, e0{};
        double p1{}, e1{};

        #if FLTX_FDD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (dd_runtime_product_pair_simd_enabled())
        {
            simd::f64x2 p{}, e{};
            const simd::f64x2 ah = simd::f64x2_set(a0.hi, a1.hi);
            const simd::f64x2 bh = simd::f64x2_splat(b.hi);
            const simd::f64x2 al = simd::f64x2_set(a0.lo, a1.lo);
            const simd::f64x2 bl = simd::f64x2_splat(b.lo);
            simd::f64x2_two_prod_precise(ah, bh, p, e);
            e = simd::f64x2_add(e, simd::f64x2_mul(ah, bl));
            e = simd::f64x2_add(e, simd::f64x2_mul(al, bh));
            e = simd::f64x2_add(e, simd::f64x2_mul(al, bl));
            simd::f64x2_store(p, p0, p1);
            simd::f64x2_store(e, e0, e1);
        }
        else
        #endif
        {
            mul_expansion_inline(a0, b, p0, e0);
            mul_expansion_inline(a1, b, p1, e1);
        }

        double s0{}, t0{};
        two_sum_precise(p0, c0.hi, s0, t0);
        t0 += e0 + c0.lo;
        out0 = renorm(s0, t0);

        double s1{}, t1{};
        two_sum_precise(p1, c1.hi, s1, t1);
        t1 += e1 + c1.lo;
        out1 = renorm(s1, t1);
    }

    // Computes (a * b + c * d) together so the independent products can use SIMD.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sum_products_inline(const fdd_s& a, const fdd_s& b, const fdd_s& c, const fdd_s& d) noexcept
    {
        double p0{}, e0{};
        double p1{}, e1{};

        #if FLTX_FDD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (dd_runtime_product_pair_simd_enabled())
        {
            simd::f64x2 p{}, e{};
            const simd::f64x2 ah = simd::f64x2_set(a.hi, c.hi);
            const simd::f64x2 bh = simd::f64x2_set(b.hi, d.hi);
            const simd::f64x2 al = simd::f64x2_set(a.lo, c.lo);
            const simd::f64x2 bl = simd::f64x2_set(b.lo, d.lo);
            simd::f64x2_two_prod_precise(ah, bh, p, e);
            e = simd::f64x2_add(e, simd::f64x2_mul(ah, bl));
            e = simd::f64x2_add(e, simd::f64x2_mul(al, bh));
            e = simd::f64x2_add(e, simd::f64x2_mul(al, bl));
            simd::f64x2_store(p, p0, p1);
            simd::f64x2_store(e, e0, e1);
        }
        else
        #endif
        {
            mul_expansion_inline(a, b, p0, e0);
            mul_expansion_inline(c, d, p1, e1);
        }

        double s{}, t{};
        two_sum_precise(p0, p1, s, t);
        t += e0 + e1;
        return renorm(s, t);
    }

    // Computes (a * b - c * d) together so the independent products can use SIMD.
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s diff_products_inline(const fdd_s& a, const fdd_s& b, const fdd_s& c, const fdd_s& d) noexcept
    {
        double p0{}, e0{};
        double p1{}, e1{};

        #if FLTX_FDD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (dd_runtime_product_pair_simd_enabled())
        {
            simd::f64x2 p{}, e{};
            const simd::f64x2 ah = simd::f64x2_set(a.hi, c.hi);
            const simd::f64x2 bh = simd::f64x2_set(b.hi, d.hi);
            const simd::f64x2 al = simd::f64x2_set(a.lo, c.lo);
            const simd::f64x2 bl = simd::f64x2_set(b.lo, d.lo);
            simd::f64x2_two_prod_precise(ah, bh, p, e);
            e = simd::f64x2_add(e, simd::f64x2_mul(ah, bl));
            e = simd::f64x2_add(e, simd::f64x2_mul(al, bh));
            e = simd::f64x2_add(e, simd::f64x2_mul(al, bl));
            simd::f64x2_store(p, p0, p1);
            simd::f64x2_store(e, e0, e1);
        }
        else
        #endif
        {
            mul_expansion_inline(a, b, p0, e0);
            mul_expansion_inline(c, d, p1, e1);
        }

        double s{}, t{};
        two_sum_precise(p0, -p1, s, t);
        t += e0 - e1;
        return renorm(s, t);
    }
    BL_POP_PRECISE;

} // namespace detail::_dd

// reciprocal helpers
[[nodiscard]] BL_FORCE_INLINE constexpr fdd recip(fdd_s b) noexcept
{
    if (iszero(b)) [[unlikely]]
        return detail::_dd::signed_infinity(signbit(b));
    if (isinf(b)) [[unlikely]]
        return detail::_dd::signed_zero(signbit(b));

    constexpr fdd_s one = fdd_s{ 1.0 };
    fdd_s y = fdd_s{ 1.0 / b.hi };
    fdd_s e = one - b * y;

    y += y * e;
    e = one - b * y;
    y += y * e;

    return y;
}

} // namespace bl

#endif
