/**
 * fltx/detail/f128_arithmetic.h - Low-level double-double arithmetic helpers for f128.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F128_DETAIL_ARITHMETIC_INCLUDED
#define F128_DETAIL_ARITHMETIC_INCLUDED
#include "fltx/f128_classification.h"
#include "fltx/f128_conversions.h"
#include "fltx/f128_limits.h"

namespace bl {

namespace detail::_f128 // primitives and kernels
{
    [[nodiscard]] BL_FORCE_INLINE constexpr bool limb_is_zero(double value) noexcept
    {
        return (std::bit_cast<std::uint64_t>(value) & 0x7fffffffffffffffull) == 0;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool value_is_zero(const f128_s& value) noexcept
    {
        return limb_is_zero(value.hi) && limb_is_zero(value.lo);
    }

    // public arithmetic special cases
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s quiet_nan() noexcept
    {
        return { std::bit_cast<double>(0x7ff8000000000000ull), 0.0 };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s signed_infinity(bool negative) noexcept
    {
        return { std::bit_cast<double>(negative ? 0xfff0000000000000ull : 0x7ff0000000000000ull), 0.0 };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s signed_zero(bool negative) noexcept
    {
        return { std::bit_cast<double>(negative ? 0x8000000000000000ull : 0ull), 0.0 };
    }

    [[nodiscard]] BL_NO_INLINE constexpr f128_s add_special(const f128_s& a, const f128_s& b) noexcept
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

    [[nodiscard]] BL_NO_INLINE constexpr f128_s sub_special(const f128_s& a, const f128_s& b) noexcept
    {
        return add_special(a, f128_s{ -b.hi, -b.lo });
    }

    [[nodiscard]] BL_NO_INLINE constexpr f128_s mul_special(const f128_s& a, const f128_s& b) noexcept
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

    [[nodiscard]] BL_NO_INLINE constexpr f128_s div_special(const f128_s& a, const f128_s& b) noexcept
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

    // residual helpers
    BL_FORCE_INLINE constexpr f128_s sub_mul_scalar_exact(const f128_s& r, const f128_s& b, double q) noexcept
    {
        double p{}, e{};
        two_prod_precise(b.hi, q, p, e);
        e += b.lo * q;

        double s{}, t{};
        detail::fp::two_diff_precise(r.hi, p, s, t);
        t += r.lo - e;

        return renorm(s, t);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr double product_split_high(double value) noexcept
    {
        constexpr double split = 134217729.0;
        constexpr int split_shift = 27;

        const double scaled = split * value;
        if (detail::fp::isinf(scaled))
        {
            return detail::fp::ldexp(
                value - (value - detail::fp::ldexp(value, -split_shift)),
                split_shift);
        }

        return scaled - (scaled - value);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s div_compensated_inline(const f128_s& a, const f128_s& b) noexcept
    {
        const double q0 = a.hi / b.hi;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return { q0, 0.0 };
        if (q0 == 0.0 && a.hi == 0.0 && a.lo == 0.0) [[unlikely]]
            return signed_zero(bl::signbit(a) != bl::signbit(b));

        double p0{}, e0{};
        two_prod_precise(q0, b.hi, p0, e0);

        const double q1 = (((a.hi - p0) - e0) + a.lo - (q0 * b.lo)) / b.hi;
        const double hi = q0 + q1;
        return { hi, (q0 - hi) + q1 };
    }

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s div_compensated_inline_checked(const f128_s& a, const f128_s& b) noexcept
    {
        const double q0 = a.hi / b.hi;
        if (detail::fp::isinf_or_nan(q0)) [[unlikely]]
            return { q0, 0.0 };
        if (q0 == 0.0 && a.hi == 0.0 && a.lo == 0.0) [[unlikely]]
            return signed_zero(bl::signbit(a) != bl::signbit(b));

        double p0{}, e0{};
        detail::fp::two_prod_precise_checked(q0, b.hi, p0, e0);

        const double q1 = (((a.hi - p0) - e0) + a.lo - (q0 * b.lo)) / b.hi;
        const double hi = q0 + q1;
        return { hi, (q0 - hi) + q1 };
    }
#endif

    [[nodiscard]] BL_MSVC_NOINLINE constexpr f128_s div_f128_double_runtime(const f128_s& a, double b) noexcept
    {
        return div_compensated_inline(a, f128_s{ b, 0.0 });
    }

    // core arithmetic
    BL_PUSH_PRECISE;
    BL_FORCE_INLINE constexpr void mul_expansion_inline(const f128_s& a, const f128_s& b, double& p, double& e) noexcept
    {
        two_prod_precise(a.hi, b.hi, p, e);

        e += a.hi * b.lo + a.lo * b.hi;
        e += a.lo * b.lo;
    }

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    BL_FORCE_INLINE constexpr void mul_expansion_inline_checked(const f128_s& a, const f128_s& b, double& p, double& e) noexcept
    {
        detail::fp::two_prod_precise_checked(a.hi, b.hi, p, e);

        e += a.hi * b.lo + a.lo * b.hi;
        e += a.lo * b.lo;
    }
#endif
    BL_POP_PRECISE;

    // fused expressions
    BL_PUSH_PRECISE;
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s add_inline(const f128_s& a, const f128_s& b) noexcept
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

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sub_inline(const f128_s& a, const f128_s& b) noexcept
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
    BL_POP_PRECISE;

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_inline(const f128_s& a, const f128_s& b) noexcept
    {
        double p{}, e{};
        two_prod_precise(a.hi, b.hi, p, e);
        e += a.hi * b.lo + a.lo * b.hi;
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return { p, e };
    }

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_inline_checked(const f128_s& a, const f128_s& b) noexcept
    {
        double p{}, e{};
        detail::fp::two_prod_precise_checked(a.hi, b.hi, p, e);
        e += a.hi * b.lo + a.lo * b.hi;
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return { p, e };
    }
#endif

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sqr_inline(const f128_s& a) noexcept
    {
        double p{}, e{};
        two_prod_precise(a.hi, a.hi, p, e);
        e += (a.hi + a.hi) * a.lo;
        e += a.lo * a.lo;
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return { p, e };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s finish_mul_checked_inline(
        const f128_s& a,
        const f128_s& b,
        const f128_s& out) noexcept
    {
        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return mul_special(a, b);
        if (limb_is_zero(out.hi) && (value_is_zero(a) || value_is_zero(b))) [[unlikely]]
            return mul_special(a, b);
        return out;
    }

    // Multiplication layers:
    //
    // - mul_inline is the small arithmetic kernel. Its Dekker path assumes the
    //   leading limbs are within the normal splitter range.
    // - mul_checked_fallback handles the complete range and public special-value
    //   semantics. It is constexpr-capable but stays out of runtime hot paths.
    // - mul_checked_inline is the thin public-operation dispatcher.
    [[nodiscard]] BL_NO_INLINE constexpr f128_s mul_checked_fallback(
        const f128_s& a,
        const f128_s& b) noexcept
    {
        if (detail::fp::isinf_or_nan(a.hi) || detail::fp::isinf_or_nan(b.hi) ||
            value_is_zero(a) || value_is_zero(b)) [[unlikely]]
            return mul_special(a, b);

        #if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        if (detail::fp::dekker_product_needs_scaling(a.hi, b.hi)) [[unlikely]]
            return finish_mul_checked_inline(a, b, mul_inline_checked(a, b));
        #endif

        return finish_mul_checked_inline(a, b, mul_inline(a, b));
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool mul_fast_path_is_safe(
        double a,
        double b) noexcept
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

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_checked_inline(
        const f128_s& a,
        const f128_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            mul_checked_fallback(a, b),
            mul_fast_path_is_safe(a.hi, b.hi)
                ? mul_inline(a, b)
                : mul_checked_fallback(a, b)
        );
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_dekker_checked_inline(const f128_s& a, const f128_s& b) noexcept
    {
        #if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        return finish_mul_checked_inline(a, b, mul_inline_checked(a, b));
        #else
        return mul_checked_inline(a, b);
        #endif
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_accurate_inline(const f128_s& a, const f128_s& b) noexcept
    {
        if (detail::fp::isinf_or_nan(a.hi) || detail::fp::isinf_or_nan(b.hi)) [[unlikely]]
            return mul_checked_inline(a, b);

        double p0{}, q0{};
        two_prod_precise(a.hi, b.hi, p0, q0);
        if (detail::fp::isinf_or_nan(p0)) [[unlikely]]
            return mul_checked_inline(a, b);

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

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sqr_dekker_checked_inline(const f128_s& a) noexcept
    {
        #if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        double p{}, e{};
        detail::fp::two_prod_precise_checked(a.hi, a.hi, p, e);
        e += (a.hi + a.hi) * a.lo;
        e += a.lo * a.lo;
        detail::fp::quick_two_sum_precise(p, e, p, e);
        return finish_mul_checked_inline(a, a, f128_s{ p, e });
        #else
        return sqr_inline(a);
        #endif
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s div_inline(const f128_s& a, const f128_s& b) noexcept
    {
        return div_compensated_inline(a, b);
    }

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s div_inline_checked(const f128_s& a, const f128_s& b) noexcept
    {
        return div_compensated_inline_checked(a, b);
    }
#endif

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s add_double_inline(const f128_s& a, double b) noexcept
    {
        double s{}, e{};
        two_sum_precise(a.hi, b, s, e);
        e += a.lo;
        return renorm(s, e);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s add_double_inline(double a, const f128_s& b) noexcept
    {
        return add_double_inline(b, a);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sub_double_inline(const f128_s& a, double b) noexcept
    {
        return add_double_inline(a, -b);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sub_double_inline(double a, const f128_s& b) noexcept
    {
        return add_double_inline(-b, a);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_double_inline(const f128_s& a, double b) noexcept
    {
        double p{}, e{};
        two_prod_precise(a.hi, b, p, e);

        e += a.lo * b;
        return renorm(p, e);
    }

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_double_inline_checked(const f128_s& a, double b) noexcept
    {
        double p{}, e{};
        detail::fp::two_prod_precise_checked(a.hi, b, p, e);

        e += a.lo * b;
        return renorm(p, e);
    }
#endif

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_double_checked_inline(const f128_s& a, double b) noexcept
    {
        #if defined(FLTX_MATH_USES_CHECKED_DEKKER)
        const f128_s out = detail::fp::dekker_product_needs_scaling(a.hi, b)
            ? mul_double_inline_checked(a, b)
            : mul_double_inline(a, b);
        #else
        const f128_s out = mul_double_inline(a, b);
        #endif

        if (detail::fp::isinf_or_nan(out.hi)) [[unlikely]]
            return mul_special(a, f128_s{ b, 0.0 });
        if (out.hi == 0.0 && ((a.hi == 0.0 && a.lo == 0.0) || b == 0.0)) [[unlikely]]
            return mul_special(a, f128_s{ b, 0.0 });
        return out;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_double_inline(double a, const f128_s& b) noexcept
    {
        return mul_double_inline(b, a);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_pwr2_inline(const f128_s& a, double b) noexcept
    {
        return { a.hi * b, a.lo * b };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s div_double_inline(const f128_s& a, double b) noexcept
    {
        if (bl::detail::is_constant_evaluated())
        {
            if (detail::fp::isnan(a.hi) || detail::fp::isnan(b)) [[unlikely]]
                return std::numeric_limits<f128_s>::quiet_NaN();

            if (isinf(b))
            {
                if (isinf(a.hi))
                    return std::numeric_limits<f128_s>::quiet_NaN();

                const bool neg = signbit(a.hi) ^ signbit(b);
                return signed_zero(neg);
            }

            if (b == 0.0) [[unlikely]]
            {
                if (a.hi == 0.0 && a.lo == 0.0)
                    return std::numeric_limits<f128_s>::quiet_NaN();

                const bool neg = signbit(a.hi) ^ signbit(b);
                return f128_s{ neg ? -std::numeric_limits<double>::infinity()
                             : std::numeric_limits<double>::infinity(), 0.0 };
            }

            if (isinf(a.hi)) [[unlikely]]
            {
                const bool neg = signbit(a.hi) ^ signbit(b);
                return f128_s{ neg ? -std::numeric_limits<double>::infinity()
                             : std::numeric_limits<double>::infinity(), 0.0 };
            }
        }

        return div_compensated_inline(a, f128_s{ b, 0.0 });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s div_double_inline(double a, const f128_s& b) noexcept
    {
        return div_compensated_inline(f128_s{ a, 0.0 }, b);
    }

    BL_PUSH_PRECISE;
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_add_inline(const f128_s& a, const f128_s& b, const f128_s& c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, c.hi, s, t);
        t += e + c.lo;
        return renorm(s, t);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_add_double_lhs_inline(double a, const f128_s& b, const f128_s& c) noexcept
    {
        double p{}, e{};
        two_prod_precise(a, b.hi, p, e);
        e += a * b.lo;

        double s{}, t{};
        two_sum_precise(p, c.hi, s, t);
        t += e + c.lo;
        return renorm(s, t);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_add_double_rhs_inline(const f128_s& a, const f128_s& b, double c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, c, s, t);
        t += e;
        return renorm(s, t);
    }

#if defined(FLTX_MATH_USES_CHECKED_DEKKER)
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_add_inline_checked(const f128_s& a, const f128_s& b, const f128_s& c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline_checked(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, c.hi, s, t);
        t += e + c.lo;
        return renorm(s, t);
    }
#endif

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s mul_sub_inline(const f128_s& a, const f128_s& b, const f128_s& c) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(p, -c.hi, s, t);
        t += e - c.lo;
        return renorm(s, t);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sub_mul_inline(const f128_s& c, const f128_s& a, const f128_s& b) noexcept
    {
        double p{}, e{};
        mul_expansion_inline(a, b, p, e);

        double s{}, t{};
        two_sum_precise(c.hi, -p, s, t);
        t += c.lo - e;
        return renorm(s, t);
    }

    BL_FORCE_INLINE constexpr void mul_add_pair_same_rhs_inline(
        const f128_s& a0,
        const f128_s& a1,
        const f128_s& b,
        const f128_s& c0,
        const f128_s& c1,
        f128_s& out0,
        f128_s& out1) noexcept
    {
        double p0{}, e0{};
        double p1{}, e1{};

        #if FLTX_F128_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (f128_runtime_product_pair_simd_enabled())
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

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s sum_products_inline(const f128_s& a, const f128_s& b, const f128_s& c, const f128_s& d) noexcept
    {
        double p0{}, e0{};
        double p1{}, e1{};

        #if FLTX_F128_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (f128_runtime_product_pair_simd_enabled())
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

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s diff_products_inline(const f128_s& a, const f128_s& b, const f128_s& c, const f128_s& d) noexcept
    {
        double p0{}, e0{};
        double p1{}, e1{};

        #if FLTX_F128_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        if (f128_runtime_product_pair_simd_enabled())
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

} // namespace detail::_f128

// reciprocal helpers
[[nodiscard]] BL_FORCE_INLINE constexpr f128 recip(f128_s b) noexcept
{
    if (iszero(b)) [[unlikely]]
        return detail::_f128::signed_infinity(signbit(b));
    if (isinf(b)) [[unlikely]]
        return detail::_f128::signed_zero(signbit(b));

    constexpr f128_s one = f128_s{ 1.0 };
    f128_s y = f128_s{ 1.0 / b.hi };
    f128_s e = one - b * y;

    y += y * e;
    e = one - b * y;
    y += y * e;

    return y;
}

} // namespace bl

#endif
