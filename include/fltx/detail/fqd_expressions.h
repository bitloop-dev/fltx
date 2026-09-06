/**
 * fltx/detail/fqd_expressions.h - fqd fused operations and expression templates.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_DETAIL_EXPRESSIONS_INCLUDED
#define FQD_DETAIL_EXPRESSIONS_INCLUDED
#include <type_traits>
#include <utility>

#ifndef FQD_INCLUDED
#include "fltx/fqd.h"
#endif

namespace bl {

namespace detail::_qd // primitives and kernels
{
    // raw expansion helpers
    struct qd_raw5 { double x0, x1, x2, x3, x4; };
    using detail::fp::exact_pow2_scale_info;
    using detail::fp::pow2_scale_info;

    BL_FORCE_INLINE constexpr bool finite_scaled_limb_is_safe(double value, double scaled) noexcept
    {
        if (bl::detail::fp::iszero_or_inf_or_nan(value))
            return true;

        if (bl::detail::fp::iszero_or_inf_or_nan(scaled))
            return false;

        constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
        const std::uint64_t scaled_abs = std::bit_cast<std::uint64_t>(scaled) & 0x7fffffffffffffffull;
        return (scaled_abs & exponent_mask) != 0;
    }

    BL_FORCE_INLINE constexpr qd_raw5 sqr_raw5_inline(const fqd_s& a) noexcept
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
            detail::fp::two_prod_precise(a.x0, a.x0, p0, q0);
            detail::fp::two_prod_precise(a.x0, a.x1, p1, q1);
            detail::fp::two_prod_precise(a.x0, a.x2, p3, q3);
            detail::fp::two_prod_precise(a.x1, a.x1, p4, q4);
            detail::fp::two_prod_precise(a.x0, a.x3, p6, q6);
            detail::fp::two_prod_precise(a.x1, a.x2, p7, q7);
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

        return { p0, p1, s0, t0, t1 };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr qd_raw5 mul_raw5_from_products(
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

        return { products.p[0], products.p[1], s0, t0, t1 };
    }

    [[nodiscard]] BL_FORCE_INLINE qd_raw5 mul_raw5_hardware_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        product_terms products{};
        fill_mul_products_fma(a, b, products);
        return mul_raw5_from_products(a, b, products);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr qd_raw5 mul_raw5_dekker_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        product_terms products{};
        fill_mul_products_dekker(a, b, products);
        return mul_raw5_from_products(a, b, products);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr qd_raw5 mul_raw5_inline(
        const fqd_s& a,
        const fqd_s& b) noexcept
    {
        if (&a == &b)
            return sqr_raw5_inline(a);

        product_terms products{};
        fill_mul_products_auto(a, b, products);
        return mul_raw5_from_products(a, b, products);
    }

    BL_FORCE_INLINE constexpr qd_raw5 neg_raw5(qd_raw5 v) noexcept
    {
        return { -v.x0, -v.x1, -v.x2, -v.x3, -v.x4 };
    }

    // scaling helpers
    BL_FORCE_INLINE constexpr fqd_s mul_double_product_inline(const fqd_s& a, double b) noexcept;
    BL_FORCE_INLINE constexpr fqd_s scale_unchecked_inline(const fqd_s& a, double scalar) noexcept
    {
        return { a.x0 * scalar, a.x1 * scalar, a.x2 * scalar, a.x3 * scalar };
    }

    BL_FORCE_INLINE constexpr fqd_s scale_pow2_inline(const fqd_s& a, int exponent, bool negative) noexcept
    {
        #if defined(FLTX_FAST_MATH)
        if (!bl::detail::is_constant_evaluated())
        {
            const double scale = bl::detail::fp::scalbn(negative ? -1.0 : 1.0, exponent);
            return scale_unchecked_inline(a, scale);
        }
        #endif

        const double x0 = bl::detail::fp::scalbn(a.x0, exponent);
        const double x1 = bl::detail::fp::scalbn(a.x1, exponent);
        const double x2 = bl::detail::fp::scalbn(a.x2, exponent);
        const double x3 = bl::detail::fp::scalbn(a.x3, exponent);

        #if !defined(FLTX_FAST_MATH)
        if (!finite_scaled_limb_is_safe(a.x0, x0) ||
            !finite_scaled_limb_is_safe(a.x1, x1) ||
            !finite_scaled_limb_is_safe(a.x2, x2) ||
            !finite_scaled_limb_is_safe(a.x3, x3))
        {
            const double scalar = negative ? -bl::detail::fp::scalbn(1.0, exponent) : bl::detail::fp::scalbn(1.0, exponent);
            return mul_double_product_inline(a, scalar);
        }
        #endif

        if (negative)
            return { -x0, -x1, -x2, -x3 };

        return { x0, x1, x2, x3 };
    }

    BL_FORCE_INLINE constexpr fqd_s scale_pow2_or_checked_inline(const fqd_s& a,double scalar,pow2_scale_info scale) noexcept
    {
        #if defined(FLTX_FAST_MATH)
        if (!bl::detail::is_constant_evaluated())
            return scale_unchecked_inline(a, scalar);
        #endif

        return scale_pow2_inline(a, scale.exponent, scale.negative);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_pow2_or_double_inline(const fqd_s& a, double b) noexcept
    {
        const pow2_scale_info scale = exact_pow2_scale_info(b);
        if (scale.valid)
            return scale_pow2_or_checked_inline(a, b, scale);

        return mul_double_product_inline(a, b);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_pow2_or_double_canonical_inline(const fqd_s& a, double b) noexcept
    {
        if (bl::detail::fp::isinf_or_nan(a.x0) || bl::detail::fp::isinf_or_nan(b)) [[unlikely]]
            return mul_special(a, fqd_s{ b, 0.0, 0.0, 0.0 });

        const fqd_s out = mul_pow2_or_double_inline(a, b);
        if (bl::detail::fp::isinf_or_nan(out.x0)) [[unlikely]]
            return signed_infinity(bl::signbit(a) != signbit(b));
        return out;
    }

    // raw5 accumulation
    BL_FORCE_INLINE constexpr fqd_s add_raw5_value_inline(qd_raw5 p, const fqd_s& v) noexcept
    {
        double s0{}, e0{};
        double s1{}, e1{};
        double s2{}, e2{};
        double s3{}, e3{};

        two_sum_precise(p.x0, v.x0, s0, e0);
        two_sum_precise(p.x1, v.x1, s1, e1);
        two_sum_precise(p.x2, v.x2, s2, e2);
        two_sum_precise(p.x3, v.x3, s3, e3);

        two_sum_precise(s1, e0, s1, e0);
        three_sum(s2, e0, e1);
        three_sum2(s3, e0, e2);

        e0 += e1 + e3 + p.x4;

        if (e0 == 0.0)
            return renorm4(s0, s1, s2, s3);

        return renorm5(s0, s1, s2, s3, e0);
    }

    BL_FORCE_INLINE constexpr fqd_s add_raw5_raw5_inline(qd_raw5 a, qd_raw5 b) noexcept
    {
        double s0{}, e0{};
        double s1{}, e1{};
        double s2{}, e2{};
        double s3{}, e3{};

        two_sum_precise(a.x0, b.x0, s0, e0);
        two_sum_precise(a.x1, b.x1, s1, e1);
        two_sum_precise(a.x2, b.x2, s2, e2);
        two_sum_precise(a.x3, b.x3, s3, e3);

        two_sum_precise(s1, e0, s1, e0);
        three_sum(s2, e0, e1);
        three_sum2(s3, e0, e2);

        e0 += e1 + e3 + a.x4 + b.x4;

        if (e0 == 0.0)
            return renorm4(s0, s1, s2, s3);

        return renorm5(s0, s1, s2, s3, e0);
    }

    BL_FORCE_INLINE constexpr fqd_s add_raw5_raw5_value_inline(qd_raw5 a, qd_raw5 b, const fqd_s& v) noexcept
    {
        return add_finite_inline(add_raw5_raw5_inline(a, b), v);
    }

    BL_FORCE_INLINE constexpr fqd_s add_raw5_double_inline(qd_raw5 p, double v) noexcept
    {
        return add_raw5_value_inline(p, fqd_s{ v });
    }

    // fused expression kernels
    BL_FORCE_INLINE constexpr fqd_s mul_add_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return add_raw5_value_inline(mul_raw5_inline(a, b), c);
    }

    FLTX_X86_FMA_KERNEL_INLINE fqd_s mul_add_hardware_inline(
        const fqd_s& a,
        const fqd_s& b,
        const fqd_s& c) noexcept
    {
        return add_raw5_value_inline(mul_raw5_hardware_inline(a, b), c);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_dekker_inline(
        const fqd_s& a,
        const fqd_s& b,
        const fqd_s& c) noexcept
    {
        return add_raw5_value_inline(mul_raw5_dekker_inline(a, b), c);
    }

    [[nodiscard]] BL_NO_INLINE constexpr fqd_s mul_add_exact_inline(
        const fqd_s& a,
        const fqd_s& b,
        const fqd_s& c) noexcept
    {
        const double a_limb[4]{ a.x0, a.x1, a.x2, a.x3 };
        const double b_limb[4]{ b.x0, b.x1, b.x2, b.x3 };
        const double c_limb[4]{ c.x0, c.x1, c.x2, c.x3 };
        double expansion[40]{};
        int count = 0;

        for (double ai : a_limb)
        {
            for (double bi : b_limb)
            {
                double product{}, error{};
                detail::fp::two_prod_precise(ai, bi, product, error);
                count = detail::fp::grow_expansion_zeroelim(count, expansion, error);
                count = detail::fp::grow_expansion_zeroelim(count, expansion, product);
            }
        }

        for (double ci : c_limb)
            count = detail::fp::grow_expansion_zeroelim(count, expansion, ci);

        return from_expansion_fast(expansion, count);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_double_rhs_inline(const fqd_s& a, const fqd_s& b, double c) noexcept
    {
        return add_raw5_double_inline(mul_raw5_inline(a, b), c);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return add_raw5_value_inline(mul_raw5_inline(a, b), -c);
    }

    BL_FORCE_INLINE constexpr fqd_s value_sub_mul_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return add_raw5_value_inline(neg_raw5(mul_raw5_inline(b, c)), a);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return add_raw5_raw5_inline(mul_raw5_inline(a, b), mul_raw5_inline(c, d));
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return add_raw5_raw5_inline(mul_raw5_inline(a, b), neg_raw5(mul_raw5_inline(c, d)));
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_add_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return add_raw5_raw5_value_inline(mul_raw5_inline(a, b), mul_raw5_inline(c, d), e);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_sub_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return add_raw5_raw5_value_inline(mul_raw5_inline(a, b), mul_raw5_inline(c, d), -e);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_add_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return add_raw5_raw5_value_inline(mul_raw5_inline(a, b), neg_raw5(mul_raw5_inline(c, d)), e);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_sub_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return add_raw5_raw5_value_inline(mul_raw5_inline(a, b), neg_raw5(mul_raw5_inline(c, d)), -e);
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_add_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return add_finite_inline(add_finite_inline(a, b), c);
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_add_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return add_finite_inline(sub_finite_inline(a, b), c);
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_sub_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return sub_finite_inline(add_finite_inline(a, b), c);
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_sub_inline(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return sub_finite_inline(sub_finite_inline(a, b), c);
    }

    template<int Scale> BL_FORCE_INLINE constexpr double scale_limb_inline(double value) noexcept
    {
        if constexpr (Scale == 1)
            return value;
        else if constexpr (Scale == -1)
            return -value;
        else
            return value * static_cast<double>(Scale);
    }

    template<int AScale, int BScale> BL_FORCE_INLINE constexpr fqd_s add_scaled_inline(const fqd_s& a, const fqd_s& b) noexcept
    {
        double s0{}, e0{};
        double s1{}, e1{};
        double s2{}, e2{};
        double s3{}, e3{};

        two_sum_precise(scale_limb_inline<AScale>(a.x0), scale_limb_inline<BScale>(b.x0), s0, e0);
        two_sum_precise(scale_limb_inline<AScale>(a.x1), scale_limb_inline<BScale>(b.x1), s1, e1);
        two_sum_precise(scale_limb_inline<AScale>(a.x2), scale_limb_inline<BScale>(b.x2), s2, e2);
        two_sum_precise(scale_limb_inline<AScale>(a.x3), scale_limb_inline<BScale>(b.x3), s3, e3);

        two_sum_precise(s1, e0, s1, e0);
        three_sum(s2, e0, e1);
        three_sum2(s3, e0, e2);

        e0 += e1 + e3;

        if (e0 == 0.0)
            return renorm4(s0, s1, s2, s3);

        return renorm5(s0, s1, s2, s3, e0);
    }

    BL_FORCE_INLINE constexpr qd_raw5 mul_double_raw5_inline(const fqd_s& a, double b) noexcept
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

        return { s0, s1, s2, s3, s4 };
    }

    BL_FORCE_INLINE constexpr fqd_s add_mul_double_inline(const fqd_s& add, const fqd_s& value, double scalar) noexcept
    {
        const pow2_scale_info scale = exact_pow2_scale_info(scalar);
        if (scale.valid)
            return add_finite_inline(add, scale_pow2_or_checked_inline(value, scalar, scale));

        return add_raw5_value_inline(mul_double_raw5_inline(value, scalar), add);
    }

    BL_FORCE_INLINE constexpr fqd_s sub_mul_double_inline(const fqd_s& min, const fqd_s& value, double scalar) noexcept
    {
        const pow2_scale_info scale = exact_pow2_scale_info(scalar);
        if (scale.valid)
            return sub_finite_inline(min, scale_pow2_or_checked_inline(value, scalar, scale));

        return add_raw5_value_inline(neg_raw5(mul_double_raw5_inline(value, scalar)), min);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_sub_inline(const fqd_s& value, double scalar, const fqd_s& sub) noexcept
    {
        const pow2_scale_info scale = exact_pow2_scale_info(scalar);
        if (scale.valid)
            return sub_finite_inline(scale_pow2_or_checked_inline(value, scalar, scale), sub);

        return add_raw5_value_inline(mul_double_raw5_inline(value, scalar), -sub);
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_sub_pow2_inline(const fqd_s& value, double scalar, const fqd_s& sub) noexcept
    {
        const pow2_scale_info scale = exact_pow2_scale_info(scalar);
        if (scale.valid)
            return sub_finite_inline(scale_pow2_or_checked_inline(value, scalar, scale), sub);

        return mul_double_sub_inline(value, scalar, sub);
    }

    BL_FORCE_INLINE constexpr fqd_s div_add_double_inline(const fqd_s& numerator, const fqd_s& base_den, double scalar) noexcept
    {
        return div_prechecked_inline(numerator, add_double_finite_inline(base_den, scalar));
    }

    BL_FORCE_INLINE constexpr fqd_s div_double_sub_inline(const fqd_s& numerator, double scalar, const fqd_s& base_den) noexcept
    {
        return div_prechecked_inline(numerator, sub_double_finite_inline(scalar, base_den));
    }

} // namespace detail::_qd

#if defined(FLTX_ENABLE_FQD_EXPRESSIONS) && FLTX_ENABLE_FQD_EXPRESSIONS

namespace detail::_qd_expr
{
    // expression traits
    template<class T> using clean_t = std::remove_cv_t<std::remove_reference_t<T>>;
    template<class T> using left_t  = clean_t<decltype(std::declval<T>().left)>;
    template<class T> using right_t = clean_t<decltype(std::declval<T>().right)>;
    template<class T> using prod_t  = clean_t<decltype(std::declval<T>().prod)>;
    template<class T> using value_t = clean_t<decltype(std::declval<T>().value)>;

    template<class T>
    inline constexpr bool is_direct_conversion_target_v =
        std::is_same_v<T, clean_t<T>> &&
        (std::is_same_v<T, fdd_s> ||
         std::is_same_v<T, fdd> ||
         detail::fp::is_native_arithmetic_scalar_v<T>);

    template<class T, class Expr>
    [[nodiscard]] BL_FORCE_INLINE constexpr T convert_expression(const Expr& expr) noexcept;

    // expression nodes
    template<class Storage>
    struct value_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = Storage;

        Storage value;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ value }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    // The storage template argument associates every expression tree with bl for ADL.
    using leaf_expr = value_expr<fqd_s>;

    template<class L>
    struct mul_double_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        double right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L>
    struct add_double_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        double right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class R>
    struct double_sub_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        double left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L, class R>
    struct add_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L, class R>
    struct sub_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L, class R>
    struct mul_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L, class R>
    struct div_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L>
    struct div_double_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        L left;
        double right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class R>
    struct double_div_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        double left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L, class R, int RSign>
    struct prod_pair_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        static constexpr int r_sign = RSign;

        L left;
        R right;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class P, class V, int VSign>
    struct prod_value_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        static constexpr int v_sign = VSign;

        P prod;
        V value;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class L, class R, class V, int RSign, int VSign>
    struct prod_pair_value_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        static constexpr int r_sign = RSign;
        static constexpr int v_sign = VSign;

        L left;
        R right;
        V value;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<class A, class B, class C>
    struct prod_triple_add_expr
    {
        using fltx_expression_value_type = fqd;
        using fltx_expression_storage_type = fqd_s;

        A first;
        B second;
        C third;

        BL_FORCE_INLINE constexpr operator fqd() const noexcept { return fqd{ eval_to_qd_s(*this) }; }

        template<class T, std::enable_if_t<is_direct_conversion_target_v<T>, int> = 0>
        [[nodiscard]] explicit constexpr operator T() const noexcept { return convert_expression<T>(*this); }
    };

    template<>                                           struct is_expr< leaf_expr >                             : std::true_type {};
    template<class L>                                    struct is_expr< mul_double_expr<L> >                    : std::true_type {};
    template<class L>                                    struct is_expr< add_double_expr<L> >                    : std::true_type {};
    template<class R>                                    struct is_expr< double_sub_expr<R> >                    : std::true_type {};
    template<class L, class R>                           struct is_expr< add_expr<L, R> >                        : std::true_type {};
    template<class L, class R>                           struct is_expr< sub_expr<L, R> >                        : std::true_type {};
    template<class L, class R>                           struct is_expr< mul_expr<L, R> >                        : std::true_type {};
    template<class L, class R>                           struct is_expr< div_expr<L, R> >                        : std::true_type {};
    template<class L>                                    struct is_expr< div_double_expr<L> >                    : std::true_type {};
    template<class R>                                    struct is_expr< double_div_expr<R> >                    : std::true_type {};
    template<class L, class R, int S>                    struct is_expr< prod_pair_expr<L, R, S> >               : std::true_type {};
    template<class P, class V, int S>                    struct is_expr< prod_value_expr<P, V, S> >              : std::true_type {};
    template<class L, class R, class V, int RS, int VS>  struct is_expr< prod_pair_value_expr<L, R, V, RS, VS> > : std::true_type {};
    template<class A, class B, class C>                  struct is_expr< prod_triple_add_expr<A, B, C> >         : std::true_type {};

    template<class T> inline constexpr bool is_qd_value_v         = std::is_same_v<clean_t<T>, fqd>;
    template<class T> inline constexpr bool is_leaf_v               = std::is_same_v<clean_t<T>, leaf_expr>;
    template<class T> inline constexpr bool is_expr_v               = is_expr<clean_t<T>>::value;
    template<class T> inline constexpr bool is_operand_v            = is_expr_v<T> || is_qd_value_v<T>;
    template<class T> inline constexpr bool is_integer_scalar_v     = detail::_qd::is_integer_scalar_v<clean_t<T>>;

    template<bool Condition> using enable_when = std::enable_if_t<Condition, int>;
    template<class L, class R> using enable_expr_pair    = enable_when<is_expr_v<L> && is_expr_v<R>>;
    template<class T> using enable_expr = enable_when<is_expr_v<T>>;
    template<class L, class T> using enable_expr_integer = enable_when<is_expr_v<L> && is_integer_scalar_v<T>>;
    template<class T, class R> using enable_integer_expr = enable_when<is_integer_scalar_v<T> && is_expr_v<R>>;
    template<class L, class R> using pub_operand         = enable_when<is_operand_v<L> && is_operand_v<R> && (is_qd_value_v<L> || is_qd_value_v<R>)>;
    template<class T> using pub_qd = enable_when<is_qd_value_v<T>>;
    template<class L, class T> using pub_qd_int        = enable_when<is_qd_value_v<L> && is_integer_scalar_v<T>>;
    template<class T, class R> using pub_int_qd        = enable_when<is_integer_scalar_v<T> && is_qd_value_v<R>>;

    template<class T> inline constexpr bool is_add_v        = false;
    template<class T> inline constexpr bool is_sub_v        = false;
    template<class T> inline constexpr bool is_mul_v        = false;
    template<class T> inline constexpr bool is_div_v        = false;
    template<class T> inline constexpr bool is_mul_double_v = false;
    template<class T> inline constexpr bool is_div_double_v = false;
    template<class T> inline constexpr bool is_add_double_v = false;
    template<class T> inline constexpr bool is_double_div_v = false;
    template<class T> inline constexpr bool is_double_sub_v = false;

    template<class L, class R> inline constexpr bool is_add_v<add_expr<L, R>> = true;
    template<class L, class R> inline constexpr bool is_sub_v<sub_expr<L, R>> = true;
    template<class L, class R> inline constexpr bool is_mul_v<mul_expr<L, R>> = true;
    template<class L, class R> inline constexpr bool is_div_v<div_expr<L, R>> = true;

    template<class L> inline constexpr bool is_mul_double_v<mul_double_expr<L>> = true;
    template<class L> inline constexpr bool is_add_double_v<add_double_expr<L>> = true;
    template<class L> inline constexpr bool is_div_double_v<div_double_expr<L>> = true;
    template<class R> inline constexpr bool is_double_div_v<double_div_expr<R>> = true;
    template<class R> inline constexpr bool is_double_sub_v<double_sub_expr<R>> = true;

    template<class T> inline constexpr bool is_leaf_prod_v = false;
    template<class T> inline constexpr bool is_leaf_add_v  = false;
    template<class T> inline constexpr bool is_leaf_sub_v  = false;

    template<class L, class R> inline constexpr bool is_leaf_prod_v<mul_expr<L, R>> = is_leaf_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_add_v<add_expr<L, R>>  = is_leaf_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_sub_v<sub_expr<L, R>>  = is_leaf_v<L> && is_leaf_v<R>;

    template<class T> inline constexpr bool is_leaf_add_add_leaf_v = false;
    template<class T> inline constexpr bool is_leaf_add_sub_leaf_v = false;
    template<class T> inline constexpr bool is_leaf_sub_add_leaf_v = false;
    template<class T> inline constexpr bool is_leaf_sub_sub_leaf_v = false;

    template<class L, class R> inline constexpr bool is_leaf_add_add_leaf_v<add_expr<L, R>> = is_leaf_add_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_add_sub_leaf_v<sub_expr<L, R>> = is_leaf_add_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_sub_add_leaf_v<add_expr<L, R>> = is_leaf_sub_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_sub_sub_leaf_v<sub_expr<L, R>> = is_leaf_sub_v<L> && is_leaf_v<R>;

    template<class T> inline constexpr bool is_add_prod_prod_v = false;
    template<class T> inline constexpr bool is_sub_prod_prod_v = false;
    template<class T> inline constexpr bool is_prod_add_leaf_v = false;
    template<class T> inline constexpr bool is_prod_sub_leaf_v = false;
    template<class T> inline constexpr bool is_leaf_sub_prod_v = false;

    template<class L, class R> inline constexpr bool is_add_prod_prod_v<add_expr<L, R>> = is_leaf_prod_v<L> && is_leaf_prod_v<R>;
    template<class L, class R> inline constexpr bool is_sub_prod_prod_v<sub_expr<L, R>> = is_leaf_prod_v<L> && is_leaf_prod_v<R>;
    template<class L, class R> inline constexpr bool is_prod_add_leaf_v<add_expr<L, R>> = is_leaf_prod_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_prod_sub_leaf_v<sub_expr<L, R>> = is_leaf_prod_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_sub_prod_v<sub_expr<L, R>> = is_leaf_v<L> && is_leaf_prod_v<R>;

    template<class T> inline constexpr bool is_add_prod_prod_add_prod_v          = false;
    template<class T> inline constexpr bool is_add_prod_prod_add_add_prod_prod_v = false;

    template<class L, class R> inline constexpr bool is_add_prod_prod_add_prod_v<add_expr<L, R>>          = is_add_prod_prod_v<L> && is_leaf_prod_v<R>;
    template<class L, class R> inline constexpr bool is_add_prod_prod_add_add_prod_prod_v<add_expr<L, R>> = is_add_prod_prod_v<L> && is_add_prod_prod_v<R>;

    template<class T> inline constexpr bool is_mul_double_add_mul_double_v          = false;
    template<class T> inline constexpr bool is_mul_double_add_mul_double_add_leaf_v = false;
    template<class T> inline constexpr bool is_mul_double_add_leaf_v                = false;
    template<class T> inline constexpr bool is_leaf_add_mul_double_v                = false;
    template<class T> inline constexpr bool is_mul_double_sub_leaf_v                = false;
    template<class T> inline constexpr bool is_leaf_sub_mul_double_v                = false;

    template<class L, class R> inline constexpr bool is_mul_double_add_mul_double_v<add_expr<L, R>>          = is_mul_double_v<L> && is_mul_double_v<R>;
    template<class L, class R> inline constexpr bool is_mul_double_add_mul_double_add_leaf_v<add_expr<L, R>> = is_mul_double_add_mul_double_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_mul_double_add_leaf_v<add_expr<L, R>>                = is_mul_double_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_add_mul_double_v<add_expr<L, R>>                = is_leaf_v<L> && is_mul_double_v<R>;
    template<class L, class R> inline constexpr bool is_mul_double_sub_leaf_v<sub_expr<L, R>>                = is_mul_double_v<L> && is_leaf_v<R>;
    template<class L, class R> inline constexpr bool is_leaf_sub_mul_double_v<sub_expr<L, R>>                = is_leaf_v<L> && is_mul_double_v<R>;

    template<class T> inline constexpr bool is_prod_pair_v       = false;
    template<class T> inline constexpr bool is_prod_value_v      = false;
    template<class T> inline constexpr bool is_prod_pair_value_v = false;
    template<class T> inline constexpr bool is_prod_triple_add_v = false;

    template<class L, class R, int S> inline constexpr bool is_prod_pair_v<prod_pair_expr<L, R, S>>                                       = true;
    template<class P, class V, int S> inline constexpr bool is_prod_value_v<prod_value_expr<P, V, S>>                                     = true;
    template<class A, class B, class C> inline constexpr bool is_prod_triple_add_v<prod_triple_add_expr<A, B, C>>                         = true;
    template<class L, class R, class V, int RS, int VS> inline constexpr bool is_prod_pair_value_v<prod_pair_value_expr<L, R, V, RS, VS>> = true;

    template<class T> BL_FORCE_INLINE constexpr auto as_expr(T&& value) noexcept
    {
        if constexpr (is_expr_v<T>)
            return std::forward<T>(value);
        else
            return leaf_expr{ static_cast<const fqd_s&>(value) };
    }

    BL_FORCE_INLINE constexpr const fqd_s& leaf_value(const leaf_expr& expr) noexcept
    {
        return expr.value;
    }

    template<class T> using expr_t = clean_t<decltype(as_expr(std::declval<T>()))>;

    // evaluation dispatch
    // Evaluates full qd addition through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s add_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_canonical_inline(a, b),
            detail::_qd_runtime::add_canonical(a, b)
        );
    }

    // Evaluates full qd subtraction through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s sub_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_FP_IF_CONSTEVAL_WARNING_PUSH
        BL_FP_IF_CONSTEVAL
        {
            if (detail::fp::isinf_or_nan(a.x0) || detail::fp::isinf_or_nan(b.x0)) [[unlikely]]
                return detail::_qd::sub_special(a, b);
        }
        BL_FP_IF_CONSTEVAL_WARNING_POP

        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_canonical_inline(a, b),
            detail::_qd_runtime::sub_canonical(a, b)
        );
    }

    // Evaluates full qd multiplication through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s mul_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_FP_IF_CONSTEVAL_WARNING_PUSH
        BL_FP_IF_CONSTEVAL
        {
            if (detail::fp::isinf_or_nan(a.x0) || detail::fp::isinf_or_nan(b.x0) || bl::iszero(a) || bl::iszero(b)) [[unlikely]]
                return detail::_qd::mul_special(a, b);
        }
        BL_FP_IF_CONSTEVAL_WARNING_POP

#if BL_FP_BARRIER_ACTIVE
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_canonical_inline(a, b),
            detail::_qd::mul_fastmath_guarded(a, b)
        );
#else
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_canonical_inline(a, b),
            detail::_qd_runtime::mul_canonical(a, b)
        );
#endif
    }

    // Evaluates full qd division through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s div_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_canonical_inline(a, b),
            detail::_qd_runtime::div_canonical(a, b)
        );
    }

    // Evaluates qd-plus-double addition through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s add_double_eval(const fqd_s& a, double b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_double_canonical_inline(a, b),
            detail::_qd_runtime::add_double_canonical(a, b)
        );
    }

    // Evaluates qd-minus-double subtraction through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s sub_double_eval(const fqd_s& a, double b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_double_canonical_inline(a, b),
            detail::_qd_runtime::sub_double_canonical(a, b)
        );
    }

    // Evaluates double-minus-qd subtraction through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s sub_double_eval(double a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_double_canonical_inline(a, b),
            detail::_qd_runtime::sub_double_canonical(a, b)
        );
    }

    // Evaluates qd-by-double division through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s div_double_eval(const fqd_s& a, double b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_double_canonical_inline(a, b),
            detail::_qd_runtime::div_double_canonical(a, b)
        );
    }

    // Evaluates double-by-qd division through constexpr or compiled canonical dispatch.
    BL_FORCE_INLINE constexpr fqd_s div_double_eval(double a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_double_canonical_inline(a, b),
            detail::_qd_runtime::div_double_canonical(a, b)
        );
    }

    // Evaluates qd-by-double multiplication with the power-of-two fast path.
    BL_FORCE_INLINE constexpr fqd_s mul_pow2_or_double_eval(const fqd_s& a, double b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_pow2_or_double_canonical_inline(a, b),
            detail::_qd_runtime::mul_pow2_or_double(a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_add_inline(a, b, c),
            detail::_qd_runtime::mul_add(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_sub_inline(a, b, c),
            detail::_qd_runtime::mul_sub(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s value_sub_mul_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::value_sub_mul_inline(a, b, c),
            detail::_qd_runtime::value_sub_mul(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_add_mul_inline(a, b, c, d),
            detail::_qd_runtime::mul_add_mul(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_sub_mul_inline(a, b, c, d),
            detail::_qd_runtime::mul_sub_mul(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_add_mul_add_inline(a, b, c, d, e),
            detail::_qd_runtime::mul_add_mul_add(a, b, c, d, e)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_add_mul_sub_inline(a, b, c, d, e),
            detail::_qd_runtime::mul_add_mul_sub(a, b, c, d, e)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_sub_mul_add_inline(a, b, c, d, e),
            detail::_qd_runtime::mul_sub_mul_add(a, b, c, d, e)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_sub_mul_sub_inline(a, b, c, d, e),
            detail::_qd_runtime::mul_sub_mul_sub(a, b, c, d, e)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_add_add_inline(a, b, c),
            detail::_qd_runtime::add_add_add(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_sub_add_inline(a, b, c),
            detail::_qd_runtime::add_sub_add(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_add_sub_inline(a, b, c),
            detail::_qd_runtime::add_add_sub(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_sub_sub_inline(a, b, c),
            detail::_qd_runtime::add_sub_sub(a, b, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_scaled_2_1_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            (detail::_qd::add_scaled_inline<2, 1>(a, b)),
            detail::_qd_runtime::add_scaled_2_1(a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_scaled_1_2_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            (detail::_qd::add_scaled_inline<1, 2>(a, b)),
            detail::_qd_runtime::add_scaled_1_2(a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_scaled_2_neg1_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            (detail::_qd::add_scaled_inline<2, -1>(a, b)),
            detail::_qd_runtime::add_scaled_2_neg1(a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_scaled_1_neg2_eval(const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            (detail::_qd::add_scaled_inline<1, -2>(a, b)),
            detail::_qd_runtime::add_scaled_1_neg2(a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_mul_double_eval(const fqd_s& add, const fqd_s& value, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_mul_double_inline(add, value, scalar),
            detail::_qd_runtime::add_mul_double(add, value, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s sub_mul_double_eval(const fqd_s& min, const fqd_s& value, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_mul_double_inline(min, value, scalar),
            detail::_qd_runtime::sub_mul_double(min, value, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_sub_eval(const fqd_s& value, double scalar, const fqd_s& sub) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::mul_double_sub_inline(value, scalar, sub),
            detail::_qd_runtime::mul_double_sub(value, scalar, sub)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s div_add_double_eval(const fqd_s& numerator, const fqd_s& base_den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_add_double_inline(numerator, base_den, scalar),
            detail::_qd_runtime::div_add_double(numerator, base_den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s div_double_sub_eval(const fqd_s& numerator, double scalar, const fqd_s& base_den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_double_sub_inline(numerator, scalar, base_den),
            detail::_qd_runtime::div_double_sub(numerator, scalar, base_den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_finite_inline(detail::_qd::mul_add_inline(a, b, c), d),
            detail::_qd_runtime::mul_add_add(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_finite_inline(detail::_qd::mul_add_inline(a, b, c), d),
            detail::_qd_runtime::mul_add_sub(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_finite_inline(detail::_qd::mul_sub_inline(a, b, c), d),
            detail::_qd_runtime::mul_sub_add(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_finite_inline(detail::_qd::mul_sub_inline(a, b, c), d),
            detail::_qd_runtime::mul_sub_sub(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_add_mul_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e, const fqd_s& f) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_finite_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), detail::_qd::mul_product_inline(e, f)),
            detail::_qd_runtime::mul_add_mul_add_mul(a, b, c, d, e, f)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_add_mul_add_mul_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e, const fqd_s& f, const fqd_s& g, const fqd_s& h) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_finite_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), detail::_qd::mul_add_mul_inline(e, f, g, h)),
            detail::_qd_runtime::mul_add_mul_add_mul_add_mul(a, b, c, d, e, f, g, h)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_add_add_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_finite_inline(detail::_qd::add_add_add_inline(a, b, c), d),
            detail::_qd_runtime::add_add_add_add(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_add_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_finite_inline(detail::_qd::add_add_add_inline(a, b, c), d),
            detail::_qd_runtime::add_add_add_sub(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_sub_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_finite_inline(detail::_qd::add_add_sub_inline(a, b, c), d),
            detail::_qd_runtime::add_add_sub_sub(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_sub_sub_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::sub_finite_inline(detail::_qd::add_sub_sub_inline(a, b, c), d),
            detail::_qd_runtime::add_sub_sub_sub(a, b, c, d)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_add_mul_double_eval(const fqd_s& a, double a_scalar, const fqd_s& b, double b_scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_raw5_raw5_inline(detail::_qd::mul_double_raw5_inline(a, a_scalar), detail::_qd::mul_double_raw5_inline(b, b_scalar)),
            detail::_qd_runtime::mul_double_add_mul_double(a, a_scalar, b, b_scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_add_mul_double_add_eval(const fqd_s& a, double a_scalar, const fqd_s& b, double b_scalar, const fqd_s& c) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::add_raw5_raw5_value_inline(detail::_qd::mul_double_raw5_inline(a, a_scalar), detail::_qd::mul_double_raw5_inline(b, b_scalar), c),
            detail::_qd_runtime::mul_double_add_mul_double_add(a, a_scalar, b, b_scalar, c)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s div_add_eval(const fqd_s& numerator, const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(numerator, detail::_qd::add_finite_inline(a, b)),
            detail::_qd_runtime::div_add(numerator, a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s div_sub_eval(const fqd_s& numerator, const fqd_s& a, const fqd_s& b) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(numerator, detail::_qd::sub_finite_inline(a, b)),
            detail::_qd_runtime::div_sub(numerator, a, b)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_add_inline(a, b, c), den),
            detail::_qd_runtime::mul_add_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_inline(a, b, c), den),
            detail::_qd_runtime::mul_sub_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s value_sub_mul_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::value_sub_mul_inline(a, b, c), den),
            detail::_qd_runtime::value_sub_mul_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), den),
            detail::_qd_runtime::mul_add_mul_div(a, b, c, d, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_mul_inline(a, b, c, d), den),
            detail::_qd_runtime::mul_sub_mul_div(a, b, c, d, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_add_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_add_add_inline(a, b, c), den),
            detail::_qd_runtime::add_add_add_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_add_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_sub_add_inline(a, b, c), den),
            detail::_qd_runtime::add_sub_add_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_sub_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_add_sub_inline(a, b, c), den),
            detail::_qd_runtime::add_add_sub_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_sub_div_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_sub_sub_inline(a, b, c), den),
            detail::_qd_runtime::add_sub_sub_div(a, b, c, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_mul_double_div_eval(const fqd_s& add, const fqd_s& value, double scalar, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_mul_double_inline(add, value, scalar), den),
            detail::_qd_runtime::add_mul_double_div(add, value, scalar, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s sub_mul_double_div_eval(const fqd_s& min, const fqd_s& value, double scalar, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::sub_mul_double_inline(min, value, scalar), den),
            detail::_qd_runtime::sub_mul_double_div(min, value, scalar, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_sub_div_eval(const fqd_s& value, double scalar, const fqd_s& sub, const fqd_s& den) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_double_sub_inline(value, scalar, sub), den),
            detail::_qd_runtime::mul_double_sub_div(value, scalar, sub, den)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_add_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::mul_add_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::mul_sub_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s value_sub_mul_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::value_sub_mul_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::value_sub_mul_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_add_mul_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::mul_add_mul_div_add_double(a, b, c, d, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_sub_mul_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_mul_inline(a, b, c, d), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::mul_sub_mul_div_add_double(a, b, c, d, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_add_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_add_add_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::add_add_add_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_add_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_sub_add_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::add_sub_add_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_add_sub_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_add_sub_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::add_add_sub_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_sub_sub_div_add_double_eval(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& den, double scalar) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_sub_sub_inline(a, b, c), detail::_qd::add_double_finite_inline(den, scalar)),
            detail::_qd_runtime::add_sub_sub_div_add_double(a, b, c, den, scalar)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s add_mul_double_div_add_double_eval(const fqd_s& add, const fqd_s& value, double val_s, const fqd_s& den, double den_s) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::add_mul_double_inline(add, value, val_s), detail::_qd::add_double_finite_inline(den, den_s)),
            detail::_qd_runtime::add_mul_double_div_add_double(add, value, val_s, den, den_s)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s sub_mul_double_div_add_double_eval(const fqd_s& min, const fqd_s& value, double val_s, const fqd_s& den, double den_s) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::sub_mul_double_inline(min, value, val_s), detail::_qd::add_double_finite_inline(den, den_s)),
            detail::_qd_runtime::sub_mul_double_div_add_double(min, value, val_s, den, den_s)
        );
    }

    BL_FORCE_INLINE constexpr fqd_s mul_double_sub_div_add_double_eval(const fqd_s& value, double val_s, const fqd_s& sub, const fqd_s& den, double den_s) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_qd::div_prechecked_inline(detail::_qd::mul_double_sub_inline(value, val_s, sub), detail::_qd::add_double_finite_inline(den, den_s)),
            detail::_qd_runtime::mul_double_sub_div_add_double(value, val_s, sub, den, den_s)
        );
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod(const Prod& prod) noexcept
    {
        return mul_eval(leaf_value(prod.left), leaf_value(prod.right));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_add_value(const Prod& prod, const leaf_expr& value) noexcept
    {
        return mul_add_eval(leaf_value(prod.left), leaf_value(prod.right), leaf_value(value));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_sub_value(const Prod& prod, const leaf_expr& value) noexcept
    {
        return mul_sub_eval(leaf_value(prod.left), leaf_value(prod.right), leaf_value(value));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_value_sub_leaf_prod(const leaf_expr& value, const Prod& prod) noexcept
    {
        return value_sub_mul_eval(leaf_value(value), leaf_value(prod.left), leaf_value(prod.right));
    }

    template<class A, class B> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_add_leaf_prod(const A& a, const B& b) noexcept
    {
        return mul_add_mul_eval(
            leaf_value(a.left), leaf_value(a.right),
            leaf_value(b.left), leaf_value(b.right));
    }

    template<class A, class B> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_sub_leaf_prod(const A& a, const B& b) noexcept
    {
        return mul_sub_mul_eval(
            leaf_value(a.left), leaf_value(a.right),
            leaf_value(b.left), leaf_value(b.right));
    }

    template<class A, class B> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_add_leaf_prod_add_value(const A& a, const B& b, const leaf_expr& v) noexcept
    {
        return mul_add_mul_add_eval(
            leaf_value(a.left), leaf_value(a.right),
            leaf_value(b.left), leaf_value(b.right),
            leaf_value(v));
    }

    template<class A, class B> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_add_leaf_prod_sub_value(const A& a, const B& b, const leaf_expr& v) noexcept
    {
        return mul_add_mul_sub_eval(
            leaf_value(a.left), leaf_value(a.right),
            leaf_value(b.left), leaf_value(b.right),
            leaf_value(v));
    }

    template<class A, class B> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_sub_leaf_prod_add_value(const A& a, const B& b, const leaf_expr& v) noexcept
    {
        return mul_sub_mul_add_eval(
            leaf_value(a.left), leaf_value(a.right),
            leaf_value(b.left), leaf_value(b.right),
            leaf_value(v));
    }

    template<class A, class B> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_sub_leaf_prod_sub_value(const A& a, const B& b, const leaf_expr& v) noexcept
    {
        return mul_sub_mul_sub_eval(
            leaf_value(a.left), leaf_value(a.right),
            leaf_value(b.left), leaf_value(b.right),
            leaf_value(v));
    }

    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_prod_pair_expr(const Expr& expr) noexcept
    {
        if constexpr (clean_t<Expr>::r_sign > 0)
            return eval_leaf_prod_add_leaf_prod(expr.left, expr.right);
        else
            return eval_leaf_prod_sub_leaf_prod(expr.left, expr.right);
    }

    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_prod_value_expr(const Expr& expr) noexcept
    {
        if constexpr (clean_t<Expr>::v_sign > 0)
            return eval_leaf_prod_add_value(expr.prod, expr.value);
        else
            return eval_leaf_prod_sub_value(expr.prod, expr.value);
    }

    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_prod_pair_value_expr(const Expr& expr) noexcept
    {
        if constexpr (clean_t<Expr>::r_sign > 0 && clean_t<Expr>::v_sign > 0)
            return eval_leaf_prod_add_leaf_prod_add_value(expr.left, expr.right, expr.value);
        else if constexpr (clean_t<Expr>::r_sign > 0)
            return eval_leaf_prod_add_leaf_prod_sub_value(expr.left, expr.right, expr.value);
        else if constexpr (clean_t<Expr>::v_sign > 0)
            return eval_leaf_prod_sub_leaf_prod_add_value(expr.left, expr.right, expr.value);
        else
            return eval_leaf_prod_sub_leaf_prod_sub_value(expr.left, expr.right, expr.value);
    }

    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_prod_triple_add_expr(const Expr& expr) noexcept
    {
        return mul_add_mul_add_mul_eval(
            leaf_value(expr.first.left), leaf_value(expr.first.right),
            leaf_value(expr.second.left), leaf_value(expr.second.right),
            leaf_value(expr.third.left), leaf_value(expr.third.right));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_add_value_add_value(const Prod& prod, const leaf_expr& a, const leaf_expr& b) noexcept
    {
        return mul_add_add_eval(
            leaf_value(prod.left), leaf_value(prod.right), leaf_value(a), leaf_value(b));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_add_value_sub_value(const Prod& prod, const leaf_expr& add, const leaf_expr& sub) noexcept
    {
        return mul_add_sub_eval(
            leaf_value(prod.left), leaf_value(prod.right), leaf_value(add), leaf_value(sub));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_sub_value_add_value(const Prod& prod, const leaf_expr& sub, const leaf_expr& add) noexcept
    {
        return mul_sub_add_eval(
            leaf_value(prod.left), leaf_value(prod.right), leaf_value(sub), leaf_value(add));
    }

    template<class Prod> BL_FORCE_INLINE constexpr fqd_s eval_leaf_prod_sub_value_sub_value(const Prod& prod, const leaf_expr& a, const leaf_expr& b) noexcept
    {
        return mul_sub_sub_eval(
            leaf_value(prod.left), leaf_value(prod.right), leaf_value(a), leaf_value(b));
    }

    template<class Numerator> BL_FORCE_INLINE constexpr fqd_s eval_div_with_den(const Numerator& numerator, const fqd_s& den) noexcept
    {
        using NType = clean_t<Numerator>;

        if constexpr (is_prod_value_v<NType>)
        {
            if constexpr (NType::v_sign > 0)
            {
                return mul_add_div_eval(leaf_value(numerator.prod.left), leaf_value(numerator.prod.right), leaf_value(numerator.value), den);
            }
            else
            {
                return mul_sub_div_eval(leaf_value(numerator.prod.left), leaf_value(numerator.prod.right), leaf_value(numerator.value), den);
            }
        }
        else if constexpr (is_prod_pair_v<NType>)
        {
            if constexpr (NType::r_sign > 0)
            {
                return mul_add_mul_div_eval(
                    leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                    leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                    den);
            }
            else
            {
                return mul_sub_mul_div_eval(
                    leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                    leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                    den);
            }
        }
        else if constexpr (is_prod_add_leaf_v<NType>)
        {
            return mul_add_div_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den);
        }
        else if constexpr (is_prod_sub_leaf_v<NType>)
        {
            return mul_sub_div_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den);
        }
        else if constexpr (is_leaf_prod_v<NType>)
        {
            return div_eval(eval_leaf_prod(numerator), den);
        }
        else if constexpr (is_add_prod_prod_v<NType>)
        {
            return mul_add_mul_div_eval(
                leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                den);
        }
        else if constexpr (is_sub_prod_prod_v<NType>)
        {
            return mul_sub_mul_div_eval(
                leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                den);
        }
        else if constexpr (is_leaf_add_add_leaf_v<NType>)
        {
            return add_add_add_div_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den);
        }
        else if constexpr (is_leaf_sub_add_leaf_v<NType>)
        {
            return add_sub_add_div_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den);
        }
        else if constexpr (is_leaf_add_sub_leaf_v<NType>)
        {
            return add_add_sub_div_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den);
        }
        else if constexpr (is_leaf_sub_sub_leaf_v<NType>)
        {
            return add_sub_sub_div_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den);
        }
        else if constexpr (is_add_v<NType>)
        {
            using LType = clean_t<decltype(numerator.left)>;
            using RType = clean_t<decltype(numerator.right)>;

            if constexpr (is_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return add_mul_double_div_eval(leaf_value(numerator.right), eval_to_qd_s(numerator.left.left), numerator.left.right, den);
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return add_mul_double_div_eval(leaf_value(numerator.left), eval_to_qd_s(numerator.right.left), numerator.right.right, den);
            }
            else
            {
                return div_eval(eval_to_qd_s(numerator), den);
            }
        }
        else if constexpr (is_sub_v<NType>)
        {
            using LType = clean_t<decltype(numerator.left)>;
            using RType = clean_t<decltype(numerator.right)>;

            if constexpr (is_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return value_sub_mul_div_eval(leaf_value(numerator.left), leaf_value(numerator.right.left), leaf_value(numerator.right.right), den);
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return mul_double_sub_div_eval(eval_to_qd_s(numerator.left.left), numerator.left.right, leaf_value(numerator.right), den);
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return sub_mul_double_div_eval(leaf_value(numerator.left), eval_to_qd_s(numerator.right.left), numerator.right.right, den);
            }
            else
            {
                return div_eval(eval_to_qd_s(numerator), den);
            }
        }
        else
        {
            return div_eval(eval_to_qd_s(numerator), den);
        }
    }

    template<class Numerator> BL_FORCE_INLINE constexpr fqd_s eval_div_with_add_double_den(const Numerator& numerator, const fqd_s& den, double scalar) noexcept
    {
        using NType = clean_t<Numerator>;

        if constexpr (is_prod_value_v<NType>)
        {
            if constexpr (NType::v_sign > 0)
            {
                return mul_add_div_add_double_eval(leaf_value(numerator.prod.left), leaf_value(numerator.prod.right), leaf_value(numerator.value), den, scalar);
            }
            else
            {
                return mul_sub_div_add_double_eval(leaf_value(numerator.prod.left), leaf_value(numerator.prod.right), leaf_value(numerator.value), den, scalar);
            }
        }
        else if constexpr (is_prod_pair_v<NType>)
        {
            if constexpr (NType::r_sign > 0)
            {
                return mul_add_mul_div_add_double_eval(
                    leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                    leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                    den, scalar);
            }
            else
            {
                return mul_sub_mul_div_add_double_eval(
                    leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                    leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                    den, scalar);
            }
        }
        else if constexpr (is_prod_add_leaf_v<NType>)
        {
            return mul_add_div_add_double_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den, scalar);
        }
        else if constexpr (is_prod_sub_leaf_v<NType>)
        {
            return mul_sub_div_add_double_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den, scalar);
        }
        else if constexpr (is_add_prod_prod_v<NType>)
        {
            return mul_add_mul_div_add_double_eval(
                leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                den, scalar);
        }
        else if constexpr (is_sub_prod_prod_v<NType>)
        {
            return mul_sub_mul_div_add_double_eval(
                leaf_value(numerator.left.left), leaf_value(numerator.left.right),
                leaf_value(numerator.right.left), leaf_value(numerator.right.right),
                den, scalar);
        }
        else if constexpr (is_leaf_add_add_leaf_v<NType>)
        {
            return add_add_add_div_add_double_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den, scalar);
        }
        else if constexpr (is_leaf_sub_add_leaf_v<NType>)
        {
            return add_sub_add_div_add_double_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den, scalar);
        }
        else if constexpr (is_leaf_add_sub_leaf_v<NType>)
        {
            return add_add_sub_div_add_double_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den, scalar);
        }
        else if constexpr (is_leaf_sub_sub_leaf_v<NType>)
        {
            return add_sub_sub_div_add_double_eval(leaf_value(numerator.left.left), leaf_value(numerator.left.right), leaf_value(numerator.right), den, scalar);
        }
        else if constexpr (is_add_v<NType>)
        {
            using LType = clean_t<decltype(numerator.left)>;
            using RType = clean_t<decltype(numerator.right)>;

            if constexpr (is_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return add_mul_double_div_add_double_eval(leaf_value(numerator.right), eval_to_qd_s(numerator.left.left), numerator.left.right, den, scalar);
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return add_mul_double_div_add_double_eval(leaf_value(numerator.left), eval_to_qd_s(numerator.right.left), numerator.right.right, den, scalar);
            }
            else
            {
                return div_add_double_eval(eval_to_qd_s(numerator), den, scalar);
            }
        }
        else if constexpr (is_sub_v<NType>)
        {
            using LType = clean_t<decltype(numerator.left)>;
            using RType = clean_t<decltype(numerator.right)>;

            if constexpr (is_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return value_sub_mul_div_add_double_eval(leaf_value(numerator.left), leaf_value(numerator.right.left), leaf_value(numerator.right.right), den, scalar);
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return mul_double_sub_div_add_double_eval(eval_to_qd_s(numerator.left.left), numerator.left.right, leaf_value(numerator.right), den, scalar);
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return sub_mul_double_div_add_double_eval(leaf_value(numerator.left), eval_to_qd_s(numerator.right.left), numerator.right.right, den, scalar);
            }
            else
            {
                return div_add_double_eval(eval_to_qd_s(numerator), den, scalar);
            }
        }
        else
        {
            return div_add_double_eval(eval_to_qd_s(numerator), den, scalar);
        }
    }

    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_eager(const Expr& expr) noexcept;
    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_eager(const Expr& expr) noexcept
    {
        using ExprType = clean_t<Expr>;

        if constexpr (is_leaf_v<ExprType>)
        {
            return leaf_value(expr);
        }
        else if constexpr (is_prod_pair_v<ExprType>)
        {
            return eval_prod_pair_expr(expr);
        }
        else if constexpr (is_prod_value_v<ExprType>)
        {
            return eval_prod_value_expr(expr);
        }
        else if constexpr (is_prod_pair_value_v<ExprType>)
        {
            return eval_prod_pair_value_expr(expr);
        }
        else if constexpr (is_prod_triple_add_v<ExprType>)
        {
            return eval_prod_triple_add_expr(expr);
        }
        else if constexpr (is_mul_v<ExprType>)
        {
            const fqd_s left  = eval_eager(expr.left);
            const fqd_s right = eval_eager(expr.right);

            return mul_eval(left, right);
        }
        else if constexpr (is_mul_double_v<ExprType>)
        {
            const fqd_s left = eval_eager(expr.left);

            return mul_pow2_or_double_eval(left, expr.right);
        }
        else if constexpr (is_add_double_v<ExprType>)
        {
            const fqd_s left = eval_eager(expr.left);

            return add_double_eval(left, expr.right);
        }
        else if constexpr (is_double_sub_v<ExprType>)
        {
            const fqd_s right = eval_eager(expr.right);

            return sub_double_eval(expr.left, right);
        }
        else if constexpr (is_div_double_v<ExprType>)
        {
            const fqd_s left = eval_eager(expr.left);

            return div_double_eval(left, expr.right);
        }
        else if constexpr (is_double_div_v<ExprType>)
        {
            const fqd_s right = eval_eager(expr.right);

            return div_double_eval(expr.left, right);
        }
        else if constexpr (is_add_v<ExprType>)
        {
            const fqd_s left  = eval_eager(expr.left);
            const fqd_s right = eval_eager(expr.right);

            return add_eval(left, right);
        }
        else if constexpr (is_div_v<ExprType>)
        {
            const fqd_s left  = eval_eager(expr.left);
            const fqd_s right = eval_eager(expr.right);

            return div_eval(left, right);
        }
        else
        {
            const fqd_s left  = eval_eager(expr.left);
            const fqd_s right = eval_eager(expr.right);

            return sub_eval(left, right);
        }
    }

    template<class Expr> BL_FORCE_INLINE constexpr fqd_s eval_to_qd_s(const Expr& expr) noexcept
    {
        using ExprType = clean_t<Expr>;

        if constexpr (is_leaf_v<ExprType>)
        {
            return leaf_value(expr);
        }
        else if constexpr (is_prod_pair_v<ExprType>)
        {
            return eval_prod_pair_expr(expr);
        }
        else if constexpr (is_prod_value_v<ExprType>)
        {
            return eval_prod_value_expr(expr);
        }
        else if constexpr (is_prod_pair_value_v<ExprType>)
        {
            return eval_prod_pair_value_expr(expr);
        }
        else if constexpr (is_prod_triple_add_v<ExprType>)
        {
            return eval_prod_triple_add_expr(expr);
        }
        else if constexpr (is_mul_v<ExprType>)
        {
            if constexpr (is_leaf_prod_v<ExprType>)
            {
                return eval_leaf_prod(expr);
            }
            else
            {
                return eval_eager(expr);
            }
        }
        else if constexpr (is_mul_double_v<ExprType>)
        {
            const fqd_s left = eval_to_qd_s(expr.left);

            return mul_pow2_or_double_eval(left, expr.right);
        }
        else if constexpr (is_add_double_v<ExprType>)
        {
            const fqd_s left = eval_to_qd_s(expr.left);

            return add_double_eval(left, expr.right);
        }
        else if constexpr (is_double_sub_v<ExprType>)
        {
            const fqd_s right = eval_to_qd_s(expr.right);

            return sub_double_eval(expr.left, right);
        }
        else if constexpr (is_div_double_v<ExprType>)
        {
            const fqd_s left = eval_to_qd_s(expr.left);

            return div_double_eval(left, expr.right);
        }
        else if constexpr (is_double_div_v<ExprType>)
        {
            const fqd_s right = eval_to_qd_s(expr.right);

            return div_double_eval(expr.left, right);
        }
        else if constexpr (is_div_v<ExprType>)
        {
            using RType = clean_t<decltype(expr.right)>;

            if constexpr (is_add_double_v<RType>)
            {
                const fqd_s base_den = eval_to_qd_s(expr.right.left);

                return eval_div_with_add_double_den(expr.left, base_den, expr.right.right);
            }
            else if constexpr (is_double_sub_v<RType>)
            {
                const fqd_s left_value = eval_to_qd_s(expr.left);
                const fqd_s base_den   = eval_to_qd_s(expr.right.right);

                return div_double_sub_eval(left_value, expr.right.left, base_den);
            }
            else if constexpr (is_leaf_add_v<RType>)
            {
                const fqd_s left_value = eval_to_qd_s(expr.left);

                return div_add_eval(left_value, leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_leaf_sub_v<RType>)
            {
                const fqd_s left_value = eval_to_qd_s(expr.left);

                return div_sub_eval(left_value, leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else
            {
                const fqd_s right_value = eval_to_qd_s(expr.right);

                return eval_div_with_den(expr.left, right_value);
            }
        }
        else if constexpr (is_add_v<ExprType>)
        {
            using LType = clean_t<decltype(expr.left)>;
            using RType = clean_t<decltype(expr.right)>;

            if constexpr (is_add_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_add_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_sub_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_leaf_v<LType> && is_add_prod_prod_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_add_value(expr.right.left, expr.right.right, expr.left);
            }
            else if constexpr (is_leaf_v<LType> && is_sub_prod_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.right.left, expr.right.right, expr.left);
            }
            else if constexpr (is_prod_add_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_add_value(expr.left.left, expr.right, expr.left.right);
            }
            else if constexpr (is_prod_sub_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_sub_value(expr.left.left, expr.right, expr.left.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_prod_add_leaf_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_add_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_prod_sub_leaf_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_sub_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_sub_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.left, expr.right.right, expr.right.left);
            }
            else if constexpr (is_leaf_sub_prod_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.right, expr.left.right, expr.left.left);
            }

            if constexpr (is_leaf_add_add_leaf_v<LType> && is_leaf_v<RType>)
            {
                return add_add_add_add_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_add_v<LType> && is_leaf_add_v<RType>)
            {
                return add_add_add_add_eval(
                    leaf_value(expr.left.left), leaf_value(expr.left.right),
                    leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_leaf_add_v<LType> && is_leaf_v<RType>)
            {
                return add_add_add_eval(leaf_value(expr.left.left), leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_sub_v<LType> && is_leaf_v<RType>)
            {
                return add_sub_add_eval(leaf_value(expr.left.left), leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_v<LType> && is_leaf_add_v<RType>)
            {
                return add_add_add_eval(leaf_value(expr.left), leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_leaf_v<LType> && is_leaf_sub_v<RType>)
            {
                return add_sub_add_eval(leaf_value(expr.left), leaf_value(expr.right.right), leaf_value(expr.right.left));
            }
            else if constexpr (is_prod_add_leaf_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_add_value_add_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_prod_sub_leaf_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_value_add_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_add_v<RType>)
            {
                return eval_leaf_prod_add_value_add_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_sub_v<RType>)
            {
                return eval_leaf_prod_add_value_sub_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_v<LType> && is_prod_add_leaf_v<RType>)
            {
                return eval_leaf_prod_add_value_add_value(expr.right.left, expr.right.right, expr.left);
            }
            else if constexpr (is_leaf_v<LType> && is_prod_sub_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_value_add_value(expr.right.left, expr.right.right, expr.left);
            }
            else if constexpr (is_mul_double_v<LType> && is_mul_double_add_leaf_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.left.left), expr.right.left.right,
                    leaf_value(expr.right.right));
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_add_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.right.left), expr.right.right.right,
                    leaf_value(expr.right.left));
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_sub_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.right.left), -expr.right.right.right,
                    leaf_value(expr.right.left));
            }
            else if constexpr (is_mul_double_add_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left.left), expr.left.left.right,
                    eval_to_qd_s(expr.right.left), expr.right.right,
                    leaf_value(expr.left.right));
            }
            else if constexpr (is_leaf_add_mul_double_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.right.left), expr.left.right.right,
                    eval_to_qd_s(expr.right.left), expr.right.right,
                    leaf_value(expr.left.left));
            }
            else if constexpr (is_leaf_sub_mul_double_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.right.left), -expr.left.right.right,
                    eval_to_qd_s(expr.right.left), expr.right.right,
                    leaf_value(expr.left.left));
            }
            else if constexpr (is_mul_double_add_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left.left), expr.left.left.right,
                    eval_to_qd_s(expr.left.right.left), expr.left.right.right,
                    leaf_value(expr.right));
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_add_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.right.left.left), expr.right.left.right,
                    eval_to_qd_s(expr.right.right.left), expr.right.right.right,
                    leaf_value(expr.left));
            }
            else if constexpr (is_mul_double_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.left), expr.right.right);
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return add_mul_double_eval(leaf_value(expr.right), eval_to_qd_s(expr.left.left), expr.left.right);
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return add_mul_double_eval(leaf_value(expr.left), eval_to_qd_s(expr.right.left), expr.right.right);
            }
            else if constexpr (is_add_prod_prod_add_add_prod_prod_v<ExprType>)
            {
                return mul_add_mul_add_mul_add_mul_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right.left), leaf_value(expr.left.right.right),
                    leaf_value(expr.right.left.left), leaf_value(expr.right.left.right),
                    leaf_value(expr.right.right.left), leaf_value(expr.right.right.right));
            }
            else if constexpr (is_add_prod_prod_add_prod_v<ExprType>)
            {
                return mul_add_mul_add_mul_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right.left), leaf_value(expr.left.right.right),
                    leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_leaf_prod_v<LType> && is_add_prod_prod_v<RType>)
            {
                return mul_add_mul_add_mul_eval(
                    leaf_value(expr.right.left.left), leaf_value(expr.right.left.right),
                    leaf_value(expr.right.right.left), leaf_value(expr.right.right.right),
                    leaf_value(expr.left.left), leaf_value(expr.left.right));
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod(expr.left, expr.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_add_value(expr.left, expr.right);
            }
            else if constexpr (is_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_add_value(expr.right, expr.left);
            }
            else if constexpr (is_add_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return mul_add_mul_add_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right.left), leaf_value(expr.left.right.right),
                    leaf_value(expr.right));
            }
            else if constexpr (is_sub_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return mul_sub_mul_add_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right.left), leaf_value(expr.left.right.right),
                    leaf_value(expr.right));
            }
            else
            {
                return eval_eager(expr);
            }
        }
        else
        {
            using LType = clean_t<decltype(expr.left)>;
            using RType = clean_t<decltype(expr.right)>;

            if constexpr (is_add_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_sub_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_sub_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_sub_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_leaf_v<LType> && is_sub_prod_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.right.right, expr.right.left, expr.left);
            }
            else if constexpr (is_prod_add_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.left.left, expr.right, expr.left.right);
            }
            else if constexpr (is_prod_sub_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_sub_value(expr.left.left, expr.right, expr.left.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_prod_add_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_sub_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_prod_sub_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod_add_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_sub_prod_v<RType>)
            {
                return eval_leaf_prod_add_leaf_prod_sub_value(expr.left, expr.right.right, expr.right.left);
            }

            if constexpr (is_leaf_add_add_leaf_v<LType> && is_leaf_v<RType>)
            {
                return add_add_add_sub_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_add_sub_leaf_v<LType> && is_leaf_v<RType>)
            {
                return add_add_sub_sub_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_sub_sub_leaf_v<LType> && is_leaf_v<RType>)
            {
                return add_sub_sub_sub_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_add_v<LType> && is_leaf_add_v<RType>)
            {
                return add_add_sub_sub_eval(
                    leaf_value(expr.left.left), leaf_value(expr.left.right),
                    leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_leaf_add_v<LType> && is_leaf_v<RType>)
            {
                return add_add_sub_eval(leaf_value(expr.left.left), leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_sub_v<LType> && is_leaf_v<RType>)
            {
                return add_sub_sub_eval(leaf_value(expr.left.left), leaf_value(expr.left.right), leaf_value(expr.right));
            }
            else if constexpr (is_leaf_v<LType> && is_leaf_add_v<RType>)
            {
                return add_sub_sub_eval(leaf_value(expr.left), leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_leaf_v<LType> && is_leaf_sub_v<RType>)
            {
                return add_sub_add_eval(leaf_value(expr.left), leaf_value(expr.right.left), leaf_value(expr.right.right));
            }
            else if constexpr (is_prod_add_leaf_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_add_value_sub_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_prod_sub_leaf_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_value_sub_value(expr.left.left, expr.left.right, expr.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_add_v<RType>)
            {
                return eval_leaf_prod_sub_value_sub_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_sub_v<RType>)
            {
                return eval_leaf_prod_sub_value_add_value(expr.left, expr.right.left, expr.right.right);
            }
            else if constexpr (is_mul_double_v<LType> && is_mul_double_add_leaf_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.left.left), -expr.right.left.right,
                    -leaf_value(expr.right.right));
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_add_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.right.left), -expr.right.right.right,
                    -leaf_value(expr.right.left));
            }
            else if constexpr (is_mul_double_v<LType> && is_mul_double_sub_leaf_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.left.left), -expr.right.left.right,
                    leaf_value(expr.right.right));
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_sub_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left), expr.left.right,
                    eval_to_qd_s(expr.right.right.left), expr.right.right.right,
                    -leaf_value(expr.right.left));
            }
            else if constexpr (is_mul_double_add_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left.left), expr.left.left.right,
                    eval_to_qd_s(expr.right.left), -expr.right.right,
                    leaf_value(expr.left.right));
            }
            else if constexpr (is_leaf_add_mul_double_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.right.left), expr.left.right.right,
                    eval_to_qd_s(expr.right.left), -expr.right.right,
                    leaf_value(expr.left.left));
            }
            else if constexpr (is_mul_double_sub_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.left.left), expr.left.left.right,
                    eval_to_qd_s(expr.right.left), -expr.right.right,
                    -leaf_value(expr.left.right));
            }
            else if constexpr (is_leaf_sub_mul_double_v<LType> && is_mul_double_v<RType>)
            {
                return mul_double_add_mul_double_add_eval(
                    eval_to_qd_s(expr.left.right.left), -expr.left.right.right,
                    eval_to_qd_s(expr.right.left), -expr.right.right,
                    leaf_value(expr.left.left));
            }
            else if constexpr (is_mul_double_v<LType> && is_leaf_v<RType>)
            {
                return mul_double_sub_eval(eval_to_qd_s(expr.left.left), expr.left.right, leaf_value(expr.right));
            }
            else if constexpr (is_leaf_v<LType> && is_mul_double_v<RType>)
            {
                return sub_mul_double_eval(leaf_value(expr.left), eval_to_qd_s(expr.right.left), expr.right.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_leaf_prod_sub_leaf_prod(expr.left, expr.right);
            }
            else if constexpr (is_leaf_prod_v<LType> && is_leaf_v<RType>)
            {
                return eval_leaf_prod_sub_value(expr.left, expr.right);
            }
            else if constexpr (is_leaf_v<LType> && is_leaf_prod_v<RType>)
            {
                return eval_value_sub_leaf_prod(expr.left, expr.right);
            }
            else if constexpr (is_add_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return mul_add_mul_sub_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right.left), leaf_value(expr.left.right.right),
                    leaf_value(expr.right));
            }
            else if constexpr (is_sub_prod_prod_v<LType> && is_leaf_v<RType>)
            {
                return mul_sub_mul_sub_eval(
                    leaf_value(expr.left.left.left), leaf_value(expr.left.left.right),
                    leaf_value(expr.left.right.left), leaf_value(expr.left.right.right),
                    leaf_value(expr.right));
            }
            else
            {
                return eval_eager(expr);
            }
        }
    }

    template<class T, class Expr>
    [[nodiscard]] BL_FORCE_INLINE constexpr T convert_expression(const Expr& expr) noexcept
    {
        static_assert(is_direct_conversion_target_v<T>);
        const fqd eager{ eval_to_qd_s(expr) };
        return static_cast<T>(eager);
    }

    // expression builders
    template<class L, class R> BL_FORCE_INLINE constexpr auto make_add_expr(L&& a, R&& b) noexcept
    {
        using LExpr = expr_t<L>;
        using RExpr = expr_t<R>;

        LExpr l_expr = as_expr(std::forward<L>(a));
        RExpr r_expr = as_expr(std::forward<R>(b));

        if constexpr (is_leaf_prod_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            return prod_pair_expr<LExpr, RExpr, 1>{ l_expr, r_expr };
        }
        else if constexpr (is_prod_pair_v<LExpr> && is_leaf_v<RExpr>)
        {
            using LProd = left_t<LExpr>;
            using RProd = right_t<LExpr>;
            return prod_pair_value_expr<LProd, RProd, RExpr, LExpr::r_sign, 1>{
                l_expr.left, l_expr.right, r_expr
            };
        }
        else if constexpr (is_leaf_v<LExpr> && is_prod_pair_v<RExpr>)
        {
            using LProd = left_t<RExpr>;
            using RProd = right_t<RExpr>;
            return prod_pair_value_expr<LProd, RProd, LExpr, RExpr::r_sign, 1>{
                r_expr.left, r_expr.right, l_expr
            };
        }
        else if constexpr (is_prod_value_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            using Prod = prod_t<LExpr>;
            using Value = value_t<LExpr>;
            return prod_pair_value_expr<Prod, RExpr, Value, 1, LExpr::v_sign>{
                l_expr.prod, r_expr, l_expr.value
            };
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_prod_value_v<RExpr>)
        {
            using Prod = prod_t<RExpr>;
            using Value = value_t<RExpr>;
            return prod_pair_value_expr<LExpr, Prod, Value, 1, RExpr::v_sign>{
                l_expr, r_expr.prod, r_expr.value
            };
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_leaf_sub_prod_v<RExpr>)
        {
            using RProd = right_t<RExpr>;
            using Value = left_t<RExpr>;
            return prod_pair_value_expr<LExpr, RProd, Value, -1, 1>{
                l_expr, r_expr.right, r_expr.left
            };
        }
        else if constexpr (is_leaf_sub_prod_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            using LProd = right_t<LExpr>;
            using Value = left_t<LExpr>;
            return prod_pair_value_expr<RExpr, LProd, Value, -1, 1>{
                r_expr, l_expr.right, l_expr.left
            };
        }
        else if constexpr (is_prod_pair_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            if constexpr (LExpr::r_sign > 0)
            {
                using LProd = left_t<LExpr>;
                using RProd = right_t<LExpr>;
                return prod_triple_add_expr<LProd, RProd, RExpr>{
                    l_expr.left, l_expr.right, r_expr
                };
            }
            else
            {
                return add_expr<LExpr, RExpr>{ l_expr, r_expr };
            }
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_prod_pair_v<RExpr>)
        {
            if constexpr (RExpr::r_sign > 0)
            {
                using LProd = left_t<RExpr>;
                using RProd = right_t<RExpr>;
                return prod_triple_add_expr<LProd, RProd, LExpr>{
                    r_expr.left, r_expr.right, l_expr
                };
            }
            else
            {
                return add_expr<LExpr, RExpr>{ l_expr, r_expr };
            }
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_leaf_v<RExpr>)
        {
            return prod_value_expr<LExpr, RExpr, 1>{ l_expr, r_expr };
        }
        else if constexpr (is_leaf_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            return prod_value_expr<RExpr, LExpr, 1>{ r_expr, l_expr };
        }
        else
        {
            return add_expr<LExpr, RExpr>{ l_expr, r_expr };
        }
    }

    template<class L, class R> BL_FORCE_INLINE constexpr auto make_sub_expr(L&& a, R&& b) noexcept
    {
        using LExpr = expr_t<L>;
        using RExpr = expr_t<R>;

        LExpr l_expr = as_expr(std::forward<L>(a));
        RExpr r_expr = as_expr(std::forward<R>(b));

        if constexpr (is_leaf_prod_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            return prod_pair_expr<LExpr, RExpr, -1>{ l_expr, r_expr };
        }
        else if constexpr (is_prod_pair_v<LExpr> && is_leaf_v<RExpr>)
        {
            using LProd = left_t<LExpr>;
            using RProd = right_t<LExpr>;
            return prod_pair_value_expr<LProd, RProd, RExpr, LExpr::r_sign, -1>{
                l_expr.left, l_expr.right, r_expr
            };
        }
        else if constexpr (is_leaf_v<LExpr> && is_prod_pair_v<RExpr>)
        {
            if constexpr (RExpr::r_sign < 0)
            {
                using LProd = right_t<RExpr>;
                using RProd = left_t<RExpr>;
                return prod_pair_value_expr<LProd, RProd, LExpr, -1, 1>{
                    r_expr.right, r_expr.left, l_expr
                };
            }
            else
            {
                return sub_expr<LExpr, RExpr>{ l_expr, r_expr };
            }
        }
        else if constexpr (is_prod_value_v<LExpr> && is_leaf_prod_v<RExpr>)
        {
            using Prod = prod_t<LExpr>;
            using Value = value_t<LExpr>;
            return prod_pair_value_expr<Prod, RExpr, Value, -1, LExpr::v_sign>{
                l_expr.prod, r_expr, l_expr.value
            };
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_prod_value_v<RExpr>)
        {
            using Prod = prod_t<RExpr>;
            using Value = value_t<RExpr>;
            return prod_pair_value_expr<LExpr, Prod, Value, -1, -RExpr::v_sign>{
                l_expr, r_expr.prod, r_expr.value
            };
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_leaf_sub_prod_v<RExpr>)
        {
            using RProd = right_t<RExpr>;
            using Value = left_t<RExpr>;
            return prod_pair_value_expr<LExpr, RProd, Value, 1, -1>{
                l_expr, r_expr.right, r_expr.left
            };
        }
        else if constexpr (is_leaf_prod_v<LExpr> && is_leaf_v<RExpr>)
        {
            return prod_value_expr<LExpr, RExpr, -1>{ l_expr, r_expr };
        }
        else
        {
            return sub_expr<LExpr, RExpr>{ l_expr, r_expr };
        }
    }

    template<class L, class R> BL_FORCE_INLINE constexpr auto make_mul_expr(L&& a, R&& b) noexcept
    {
        using LExpr = expr_t<L>;
        using RExpr = expr_t<R>;

        return mul_expr<LExpr, RExpr>{
            as_expr(std::forward<L>(a)),
            as_expr(std::forward<R>(b))
        };
    }

    template<class L, class R> BL_FORCE_INLINE constexpr auto make_div_expr(L&& a, R&& b) noexcept
    {
        using LExpr = expr_t<L>;
        using RExpr = expr_t<R>;

        return div_expr<LExpr, RExpr>{
            as_expr(std::forward<L>(a)),
            as_expr(std::forward<R>(b))
        };
    }

    template<class L> BL_FORCE_INLINE constexpr auto make_mul_double_expr(L&& a, double b) noexcept
    {
        return mul_double_expr<expr_t<L>>{ as_expr(std::forward<L>(a)), b };
    }

    template<class L> BL_FORCE_INLINE constexpr auto make_add_double_expr(L&& a, double b) noexcept
    {
        return add_double_expr<expr_t<L>>{ as_expr(std::forward<L>(a)), b };
    }

    template<class R> BL_FORCE_INLINE constexpr auto make_double_sub_expr(double a, R&& b) noexcept
    {
        return double_sub_expr<expr_t<R>>{ a, as_expr(std::forward<R>(b)) };
    }

    template<class L> BL_FORCE_INLINE constexpr auto make_div_double_expr(L&& a, double b) noexcept
    {
        return div_double_expr<expr_t<L>>{ as_expr(std::forward<L>(a)), b };
    }

    template<class R> BL_FORCE_INLINE constexpr auto make_double_div_expr(double a, R&& b) noexcept
    {
        return double_div_expr<expr_t<R>>{ a, as_expr(std::forward<R>(b)) };
    }

    template<class L, class T> BL_FORCE_INLINE constexpr auto add_integer_expr(L&& a, T b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_add_double_expr(std::forward<L>(a), static_cast<double>(b));
        }
        else
        {
            return make_add_expr(std::forward<L>(a), detail::_qd::integer_to_qd(b));
        }
    }

    template<class T, class R> BL_FORCE_INLINE constexpr auto integer_add_expr(T a, R&& b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_add_double_expr(std::forward<R>(b), static_cast<double>(a));
        }
        else
        {
            return make_add_expr(detail::_qd::integer_to_qd(a), std::forward<R>(b));
        }
    }

    template<class L, class T> BL_FORCE_INLINE constexpr auto sub_integer_expr(L&& a, T b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_add_double_expr(std::forward<L>(a), -static_cast<double>(b));
        }
        else
        {
            return make_sub_expr(std::forward<L>(a), detail::_qd::integer_to_qd(b));
        }
    }

    template<class T, class R> BL_FORCE_INLINE constexpr auto integer_sub_expr(T a, R&& b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_double_sub_expr(static_cast<double>(a), std::forward<R>(b));
        }
        else
        {
            return make_sub_expr(detail::_qd::integer_to_qd(a), std::forward<R>(b));
        }
    }

    template<class L, class T> BL_FORCE_INLINE constexpr auto mul_integer_expr(L&& a, T b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_mul_double_expr(std::forward<L>(a), static_cast<double>(b));
        }
        else
        {
            return make_mul_expr(std::forward<L>(a), detail::_qd::integer_to_qd(b));
        }
    }

    template<class T, class R> BL_FORCE_INLINE constexpr auto integer_mul_expr(T a, R&& b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_mul_double_expr(std::forward<R>(b), static_cast<double>(a));
        }
        else
        {
            return make_mul_expr(detail::_qd::integer_to_qd(a), std::forward<R>(b));
        }
    }

    template<class L, class T> BL_FORCE_INLINE constexpr auto div_integer_expr(L&& a, T b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_div_double_expr(std::forward<L>(a), static_cast<double>(b));
        }
        else
        {
            return make_div_expr(std::forward<L>(a), detail::_qd::integer_to_qd(b));
        }
    }

    template<class T, class R> BL_FORCE_INLINE constexpr auto integer_div_expr(T a, R&& b) noexcept
    {
        if constexpr (detail::_qd::integer_type_fits_exact_double_v<T>)
        {
            return make_double_div_expr(static_cast<double>(a), std::forward<R>(b));
        }
        else
        {
            return make_div_expr(detail::_qd::integer_to_qd(a), std::forward<R>(b));
        }
    }

    // internal operators
    template<class L, class R, enable_expr_pair<L, R> = 0> BL_FORCE_INLINE constexpr auto operator+(L&& a, R&& b) noexcept
    {
        return make_add_expr(std::forward<L>(a), std::forward<R>(b));
    }

    template<class L, class R, enable_expr_pair<L, R> = 0> BL_FORCE_INLINE constexpr auto operator-(L&& a, R&& b) noexcept
    {
        return make_sub_expr(std::forward<L>(a), std::forward<R>(b));
    }

    template<class L, class R, enable_expr_pair<L, R> = 0> BL_FORCE_INLINE constexpr auto operator*(L&& a, R&& b) noexcept
    {
        return make_mul_expr(std::forward<L>(a), std::forward<R>(b));
    }

    template<class L, enable_expr<L> = 0> BL_FORCE_INLINE constexpr auto operator*(L&& a, double b) noexcept
    {
        return make_mul_double_expr(std::forward<L>(a), b);
    }

    template<class R, enable_expr<R> = 0> BL_FORCE_INLINE constexpr auto operator*(double a, R&& b) noexcept
    {
        return make_mul_double_expr(std::forward<R>(b), a);
    }

    template<class L, enable_expr<L> = 0> BL_FORCE_INLINE constexpr auto operator*(L&& a, float b) noexcept
    {
        return std::forward<L>(a) * static_cast<double>(b);
    }

    template<class R, enable_expr<R> = 0> BL_FORCE_INLINE constexpr auto operator*(float a, R&& b) noexcept
    {
        return static_cast<double>(a) * std::forward<R>(b);
    }

    template<class L, enable_expr<L> = 0> BL_FORCE_INLINE constexpr auto operator+(L&& a, double b) noexcept
    {
        return make_add_double_expr(std::forward<L>(a), b);
    }

    template<class R, enable_expr<R> = 0> BL_FORCE_INLINE constexpr auto operator+(double a, R&& b) noexcept
    {
        return make_add_double_expr(std::forward<R>(b), a);
    }

    template<class L, enable_expr<L> = 0> BL_FORCE_INLINE constexpr auto operator-(L&& a, double b) noexcept
    {
        return make_add_double_expr(std::forward<L>(a), -b);
    }

    template<class R, enable_expr<R> = 0> BL_FORCE_INLINE constexpr auto operator-(double a, R&& b) noexcept
    {
        return make_double_sub_expr(a, std::forward<R>(b));
    }

    template<class L, class T, enable_expr_integer<L, T> = 0> BL_FORCE_INLINE constexpr auto operator+(L&& a, T b)   noexcept { return add_integer_expr(std::forward<L>(a), b); }
    template<class T, class R, enable_integer_expr<T, R> = 0> BL_FORCE_INLINE constexpr auto operator+(T a, R&& b)   noexcept { return integer_add_expr(a, std::forward<R>(b)); }
    template<class L, class T, enable_expr_integer<L, T> = 0> BL_FORCE_INLINE constexpr auto operator-(L&& a, T b)   noexcept { return sub_integer_expr(std::forward<L>(a), b); }
    template<class T, class R, enable_integer_expr<T, R> = 0> BL_FORCE_INLINE constexpr auto operator-(T a, R&& b)   noexcept { return integer_sub_expr(a, std::forward<R>(b)); }
    template<class L, class T, enable_expr_integer<L, T> = 0> BL_FORCE_INLINE constexpr auto operator*(L&& a, T b)   noexcept { return mul_integer_expr(std::forward<L>(a), b); }
    template<class T, class R, enable_integer_expr<T, R> = 0> BL_FORCE_INLINE constexpr auto operator*(T a, R&& b)   noexcept { return integer_mul_expr(a, std::forward<R>(b)); }

    template<class L, class R, enable_expr_pair<L, R> = 0> BL_FORCE_INLINE constexpr auto operator/(L&& a, R&& b) noexcept { return make_div_expr(std::forward<L>(a), std::forward<R>(b)); }
    template<class L, class T, enable_expr_integer<L, T> = 0> BL_FORCE_INLINE constexpr auto operator/(L&& a, T b)   noexcept { return div_integer_expr(std::forward<L>(a), b); }
    template<class T, class R, enable_integer_expr<T, R> = 0> BL_FORCE_INLINE constexpr auto operator/(T a, R&& b)   noexcept { return integer_div_expr(a, std::forward<R>(b)); }

    template<class L, enable_expr<L> = 0> BL_FORCE_INLINE constexpr auto operator/(L&& a, double b) noexcept
    {
        return make_div_double_expr(std::forward<L>(a), b);
    }

    template<class R, enable_expr<R> = 0> BL_FORCE_INLINE constexpr auto operator/(double a, R&& b) noexcept
    {
        return make_double_div_expr(a, std::forward<R>(b));
    }

    template<class L, enable_expr<L> = 0> BL_FORCE_INLINE constexpr auto operator/(L&& a, float b) noexcept
    {
        return std::forward<L>(a) / static_cast<double>(b);
    }

    template<class R, enable_expr<R> = 0> BL_FORCE_INLINE constexpr auto operator/(float a, R&& b) noexcept
    {
        return static_cast<double>(a) / std::forward<R>(b);
    }

} // namespace detail::_qd_expr

// public operators
template<class L, class R, detail::_qd_expr::pub_operand<L, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(L&& a, R&& b) noexcept
{
    return detail::_qd_expr::make_add_expr(std::forward<L>(a), std::forward<R>(b));
}

template<class L, class R, detail::_qd_expr::pub_operand<L, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(L&& a, R&& b) noexcept
{
    return detail::_qd_expr::make_sub_expr(std::forward<L>(a), std::forward<R>(b));
}

template<class L, class R, detail::_qd_expr::pub_operand<L, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(L&& a, R&& b) noexcept
{
    return detail::_qd_expr::make_mul_expr(std::forward<L>(a), std::forward<R>(b));
}

template<class L, class R, detail::_qd_expr::pub_operand<L, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(L&& a, R&& b) noexcept
{
    return detail::_qd_expr::make_div_expr(std::forward<L>(a), std::forward<R>(b));
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(L&& a, double b) noexcept
{
    return detail::_qd_expr::make_add_double_expr(std::forward<L>(a), b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(double a, R&& b) noexcept
{
    return detail::_qd_expr::make_add_double_expr(std::forward<R>(b), a);
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(L&& a, double b) noexcept
{
    return detail::_qd_expr::make_add_double_expr(std::forward<L>(a), -b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(double a, R&& b) noexcept
{
    return detail::_qd_expr::make_double_sub_expr(a, std::forward<R>(b));
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(L&& a, float b) noexcept
{
    return std::forward<L>(a) + static_cast<double>(b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(float a, R&& b) noexcept
{
    return static_cast<double>(a) + std::forward<R>(b);
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(L&& a, float b) noexcept
{
    return std::forward<L>(a) - static_cast<double>(b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(float a, R&& b) noexcept
{
    return static_cast<double>(a) - std::forward<R>(b);
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(L&& a, double b) noexcept
{
    return detail::_qd_expr::make_mul_double_expr(std::forward<L>(a), b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(double a, R&& b) noexcept
{
    return detail::_qd_expr::make_mul_double_expr(std::forward<R>(b), a);
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(L&& a, float b) noexcept
{
    return std::forward<L>(a) * static_cast<double>(b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(float a, R&& b) noexcept
{
    return static_cast<double>(a) * std::forward<R>(b);
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(L&& a, double b) noexcept
{
    return detail::_qd_expr::make_div_double_expr(std::forward<L>(a), b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(double a, R&& b) noexcept
{
    return detail::_qd_expr::make_double_div_expr(a, std::forward<R>(b));
}

template<class L, detail::_qd_expr::pub_qd<L> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(L&& a, float b) noexcept
{
    return std::forward<L>(a) / static_cast<double>(b);
}

template<class R, detail::_qd_expr::pub_qd<R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(float a, R&& b) noexcept
{
    return static_cast<double>(a) / std::forward<R>(b);
}

template<class L, class T, detail::_qd_expr::pub_qd_int<L, T> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(L&& a, T b) noexcept
{
    return detail::_qd_expr::add_integer_expr(std::forward<L>(a), b);
}

template<class T, class R, detail::_qd_expr::pub_int_qd<T, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator+(T a, R&& b) noexcept
{
    return detail::_qd_expr::integer_add_expr(a, std::forward<R>(b));
}

template<class L, class T, detail::_qd_expr::pub_qd_int<L, T> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(L&& a, T b) noexcept
{
    return detail::_qd_expr::sub_integer_expr(std::forward<L>(a), b);
}

template<class T, class R, detail::_qd_expr::pub_int_qd<T, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator-(T a, R&& b) noexcept
{
    return detail::_qd_expr::integer_sub_expr(a, std::forward<R>(b));
}

template<class L, class T, detail::_qd_expr::pub_qd_int<L, T> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(L&& a, T b) noexcept
{
    return detail::_qd_expr::mul_integer_expr(std::forward<L>(a), b);
}

template<class T, class R, detail::_qd_expr::pub_int_qd<T, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator*(T a, R&& b) noexcept
{
    return detail::_qd_expr::integer_mul_expr(a, std::forward<R>(b));
}

template<class L, class T, detail::_qd_expr::pub_qd_int<L, T> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(L&& a, T b) noexcept
{
    return detail::_qd_expr::div_integer_expr(std::forward<L>(a), b);
}

template<class T, class R, detail::_qd_expr::pub_int_qd<T, R> = 0> [[nodiscard]] BL_FORCE_INLINE constexpr auto operator/(T a, R&& b) noexcept
{
    return detail::_qd_expr::integer_div_expr(a, std::forward<R>(b));
}

#endif // FLTX_ENABLE_FQD_EXPRESSIONS

} // namespace bl

#endif
