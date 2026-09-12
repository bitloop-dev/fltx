/**
 * fltx/fqd.cpp - Hot qd runtime helpers and optimized fused expression bodies.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fqd_math_basic.h"

namespace bl::detail::_qd_runtime
{
    // assignment
    fqd_s to_qd(uint64_t u) noexcept
    {
        return detail::_qd_impl::to_qd(u);
    }

    fqd_s to_qd(int64_t v) noexcept
    {
        return detail::_qd_impl::to_qd(v);
    }

    fqd_s& assign(fqd_s& out, uint64_t u) noexcept
    {
        return detail::_qd_impl::assign(out, u);
    }

    fqd_s& assign(fqd_s& out, int64_t v) noexcept
    {
        return detail::_qd_impl::assign(out, v);
    }

    // fma
    BL_NO_INLINE fqd_s fma(const fqd_s& x, const fqd_s& y, const fqd_s& z)
    {
        return detail::_qd_impl::fma(x, y, z);
    }

    BL_NO_INLINE fqd_s fma_cancellation(const fqd_s& x, const fqd_s& y, const fqd_s& z)
    {
        return detail::_qd::mul_add_exact_inline(x, y, z);
    }

    namespace
    {
        // Adds finite double-double to qd without canonical special-value handling.
        [[nodiscard]] BL_FORCE_INLINE fqd_s add_dd_finite_inline(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
        {
            using namespace detail::_qd;

            if (b.lo == 0.0)
                return add_double_finite_inline(a, b.hi);

            double s0{}, e0{};
            double s1{}, e1{};
            double s2 = a.x2, e2 = 0.0;
            double s3 = a.x3, e3 = 0.0;

            two_sum_precise(a.x0, b.hi, s0, e0);
            two_sum_precise(a.x1, b.lo, s1, e1);
            two_sum_precise(s1, e0, s1, e0);
            three_sum(s2, e0, e1);
            three_sum2(s3, e0, e2);

            e0 += e1 + e3;

            if (e0 == 0.0)
                return renorm4(s0, s1, s2, s3);

            return renorm5(s0, s1, s2, s3, e0);
        }

        // Subtracts finite double-double from qd without canonical special handling.
        [[nodiscard]] BL_FORCE_INLINE fqd_s sub_dd_finite_inline(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
        {
            using namespace detail::_qd;

            if (b.lo == 0.0)
                return sub_double_finite_inline(a, b.hi);

            b.hi = -b.hi;
            b.lo = -b.lo;
            return add_dd_finite_inline(a, b);
        }

        // Subtracts finite qd from double-double without canonical special handling.
        [[nodiscard]] BL_FORCE_INLINE fqd_s sub_dd_finite_inline(detail::_qd::dd_scalar a, const fqd_s& b) noexcept
        {
            using namespace detail::_qd;

            if (a.lo == 0.0)
                return sub_double_finite_inline(a.hi, b);

            double s0{}, e0{};
            double s1{}, e1{};
            double s2{}, e2{};
            double s3{}, e3{};

            two_sum_precise(a.hi, -b.x0, s0, e0);
            two_sum_precise(a.lo, -b.x1, s1, e1);
            two_sum_precise(0.0, -b.x2, s2, e2);
            two_sum_precise(0.0, -b.x3, s3, e3);
            two_sum_precise(s1, e0, s1, e0);
            three_sum(s2, e0, e1);
            three_sum2(s3, e0, e2);

            e0 += e1 + e3;

            if (e0 == 0.0)
                return renorm4(s0, s1, s2, s3);

            return renorm5(s0, s1, s2, s3, e0);
        }

        // Multiplies finite qd by double-double without canonical special handling.
        [[nodiscard]] BL_FORCE_INLINE fqd_s mul_dd_finite_inline(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
        {
            using namespace detail::_qd;

            if (b.lo == 0.0)
                return mul_double_product_inline(a, b.hi);
            if (!isfinite(a.x0) || !isfinite(b.hi) || !isfinite(b.lo))
                return mul_product_inline(a, fqd_s{ b.hi, b.lo });

            double p0{}, p1{}, p2{}, p3 = 0.0, p4{}, p5{};
            double q0{}, q1{}, q2{}, q3 = 0.0, q4{}, q5{};
            double p6 = 0.0, p7 = 0.0, p8{}, p9{};
            double q6 = 0.0, q7 = 0.0, q8{}, q9{};
            double r0{}, r1{};
            double t0{}, t1{};
            double s0{}, s1{}, s2{};

            two_prod_precise(a.x0, b.hi, p0, q0);
            two_prod_precise(a.x0, b.lo, p1, q1);
            two_prod_precise(a.x1, b.hi, p2, q2);
            two_prod_precise(a.x1, b.lo, p4, q4);
            two_prod_precise(a.x2, b.hi, p5, q5);
            two_prod_precise(a.x2, b.lo, p8, q8);
            two_prod_precise(a.x3, b.hi, p9, q9);

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

            two_sum_precise(q0, q4, t0, t1);  t1 += (q3 + q5);
            two_sum_precise(p6, p8, r0, r1);  r1 += (p7 + p9);
            two_sum_precise(t0, r0, q3, q4);  q4 += (t1 + r1);

            two_sum_precise(q3, s1, t0, t1);
            t1 += q4;
            t1 += a.x3 * b.lo + q6 + q7 + q8 + q9 + s2;

            return renorm5(p0, p1, s0, t0, t1);
        }

        BL_PUSH_PRECISE
        // Computes an exact residual for long division by a double-double denominator.
        [[nodiscard]] BL_FORCE_INLINE fqd_s div_dd_residual_exact_inline(const fqd_s& r, detail::_qd::dd_scalar b, double q) noexcept
        {
            using namespace detail::_qd;

            double p0{}, p1{}, p2 = 0.0, p3 = 0.0;
            double q0{}, q1{}, q2 = 0.0;
            double s0{}, s1{}, s2{}, s3{}, s4{};

            two_prod_precise(b.hi, q, p0, q0);
            two_prod_precise(b.lo, q, p1, q1);

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
        BL_POP_PRECISE

        // Divides qd by a prechecked double-double denominator.
        [[nodiscard]] BL_FORCE_INLINE fqd_s div_dd_prechecked_inline(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
        {
            using namespace detail::_qd;

            if (b.lo == 0.0)
                return div_double_prechecked_inline(a, b.hi);
            if (b.hi == 0.0 || !isfinite(b.hi) || !isfinite(b.lo))
                return div_prechecked_inline(a, fqd_s{ b.hi, b.lo });

            const double inv_b0 = 1.0 / b.hi;

            const double q0 = a.x0 * inv_b0;
			
            if (!isfinite(q0)) [[unlikely]]
                return fqd_s{ q0, 0.0, 0.0, 0.0 };
            if (q0 == 0.0 && bl::iszero(a)) [[unlikely]]
                return signed_zero(bl::signbit(a) != signbit(b.hi));

            fqd_s r = div_dd_residual_exact_inline(a, b, q0);

            const double q1 = r.x0 * inv_b0;
            r = div_dd_residual_exact_inline(r, b, q1);

            const double q2 = r.x0 * inv_b0;
            r = div_dd_residual_exact_inline(r, b, q2);

            const double q3 = r.x0 * inv_b0;
            r = div_dd_residual_exact_inline(r, b, q3);

            const double q4 = r.x0 * inv_b0;

            return renorm5(q0, q1, q2, q3, q4);
        }

        // Divides double-double by a prechecked qd denominator.
        [[nodiscard]] BL_FORCE_INLINE fqd_s div_dd_prechecked_inline(detail::_qd::dd_scalar a, const fqd_s& b) noexcept
        {
            using namespace detail::_qd;

            if (a.lo == 0.0)
                return div_double_prechecked_inline(a.hi, b);

            return div_prechecked_inline(fqd_s{ a.hi, a.lo }, b);
        }

        // Divides by a qd-plus-double denominator without materializing it when exact.
        [[nodiscard]] BL_FORCE_INLINE fqd_s div_add_double_kernel_inline(const fqd_s& numerator, const fqd_s& base_denominator, double scalar) noexcept
        {
            double head{}, carry{};
            detail::fp::two_sum_precise(base_denominator.x0, scalar, head, carry);

            const double head_abs = head < 0.0 ? -head : head;
            const double base_abs = base_denominator.x0 < 0.0 ? -base_denominator.x0 : base_denominator.x0;
            if (carry == 0.0 && head_abs >= base_abs)
                return detail::_qd::div_prechecked_inline(numerator, fqd_s{ head, base_denominator.x1, base_denominator.x2, base_denominator.x3 });

            return detail::_qd::div_prechecked_inline(numerator, detail::_qd::add_double_finite_inline(base_denominator, scalar));
        }

    } // namespace

    // Adds finite qd values through the compiled runtime boundary.
    fqd_s BL_VECTORCALL add_finite(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_finite_inline(a, b);
    }

    // Adds qd values and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL add_canonical(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_canonical_inline(a, b);
    }

    // Adds a double-double value to qd with canonical special-value handling.
    fqd_s BL_VECTORCALL add_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
    {
        const fqd_s rhs{ b.hi, b.lo, 0.0, 0.0 };
        const fqd_s out = add_dd_finite_inline(a, b);
        if (detail::_qd::limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return detail::_qd::finish_add_canonical(a, rhs, out.x0);
        return out;
    }

    // Adds a finite double to qd through the compiled runtime boundary.
    fqd_s BL_VECTORCALL add_double_finite(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::add_double_finite_inline(a, b);
    }

    // Adds a double to qd and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL add_double_canonical(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::add_double_canonical_inline(a, b);
    }

    // Subtracts finite qd values through the compiled runtime boundary.
    fqd_s BL_VECTORCALL sub_finite(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::sub_finite_inline(a, b);
    }

    // Subtracts qd values and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL sub_canonical(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::sub_canonical_inline(a, b);
    }

    // Subtracts a double-double value from qd with canonical special-value handling.
    fqd_s BL_VECTORCALL sub_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
    {
        const fqd_s out = sub_dd_finite_inline(a, b);
        const fqd_s rhs{ b.hi, b.lo, 0.0, 0.0 };
        if (detail::_qd::limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return detail::_qd::finish_sub_canonical(a, rhs, out.x0);
        return out;
    }

    // Subtracts qd from a double-double value with canonical special-value handling.
    fqd_s BL_VECTORCALL sub_dd(detail::_qd::dd_scalar a, const fqd_s& b) noexcept
    {
        const fqd_s out = sub_dd_finite_inline(a, b);
        const fqd_s lhs{ a.hi, a.lo, 0.0, 0.0 };
        if (detail::_qd::limb_is_zero_or_nonfinite(out.x0)) [[unlikely]]
            return detail::_qd::finish_sub_canonical(lhs, b, out.x0);
        return out;
    }

    // Subtracts a finite double from qd through the compiled runtime boundary.
    fqd_s BL_VECTORCALL sub_double_finite(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::sub_double_finite_inline(a, b);
    }

    // Subtracts a double from qd and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL sub_double_canonical(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::sub_double_canonical_inline(a, b);
    }

    // Subtracts qd from a double and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL sub_double_canonical(double a, const fqd_s& b) noexcept
    {
        return detail::_qd::sub_double_canonical_inline(a, b);
    }

    // Multiplies finite qd values through the compiled runtime boundary.
    fqd_s mul_finite(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::mul_finite_inline(a, b);
    }

    // Multiplies qd values and returns the canonical special-value representation.
    fqd_s mul_canonical(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::mul_canonical_inline(a, b);
    }

    // Multiplies qd by a double-double value with canonical special-value handling.
    fqd_s mul_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
    {
        const fqd_s rhs{ b.hi, b.lo, 0.0, 0.0 };
        const fqd_s out = mul_dd_finite_inline(a, b);
        if (!detail::_qd::isfinite(out.x0)) [[unlikely]]
        {
            if (!detail::_qd::isfinite(a.x0) || !detail::_qd::isfinite(b.hi)) [[unlikely]]
                return detail::_qd::mul_special(a, rhs);
            return detail::_qd::signed_infinity(bl::signbit(a) != bl::signbit(rhs));
        }
        return out;
    }

    // Multiplies qd by a finite double through the compiled runtime boundary.
    fqd_s mul_double_finite(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::mul_double_finite_inline(a, b);
    }

    // Multiplies qd by a double and returns the canonical special-value representation.
    fqd_s mul_double_canonical(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::mul_double_canonical_inline(a, b);
    }

    // Divides finite qd values through the compiled runtime boundary.
    fqd_s BL_VECTORCALL div_finite(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::div_finite_inline(a, b);
    }

    // Divides qd values and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL div_canonical(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::div_canonical_inline(a, b);
    }

    // Divides qd by a double-double value with canonical special-value handling.
    fqd_s BL_VECTORCALL div_dd(const fqd_s& a, detail::_qd::dd_scalar b) noexcept
    {
        const fqd_s rhs{ b.hi, b.lo, 0.0, 0.0 };
        if (!detail::_qd::isfinite(b.hi) || b.hi == 0.0) [[unlikely]]
            return detail::_qd::div_special(a, rhs);

        return div_dd_prechecked_inline(a, b);
    }

    // Divides a double-double value by qd with canonical special-value handling.
    fqd_s BL_VECTORCALL div_dd(detail::_qd::dd_scalar a, const fqd_s& b) noexcept
    {
        const fqd_s lhs{ a.hi, a.lo, 0.0, 0.0 };
        if (!detail::_qd::isfinite(b.x0) || b.x0 == 0.0) [[unlikely]]
            return detail::_qd::div_special(lhs, b);

        return div_dd_prechecked_inline(a, b);
    }

    // Divides qd by a finite double through the compiled runtime boundary.
    fqd_s BL_VECTORCALL div_double_finite(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::div_double_finite_inline(a, b);
    }

    // Divides qd by a double and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL div_double_canonical(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::div_double_canonical_inline(a, b);
    }

    // Divides a double by qd and returns the canonical special-value representation.
    fqd_s BL_VECTORCALL div_double_canonical(double a, const fqd_s& b) noexcept
    {
        return detail::_qd::div_double_canonical_inline(a, b);
    }

    // Squares qd through the compiled runtime boundary.
    fqd_s sqr(const fqd_s& a) noexcept
    {
        return detail::_qd::sqr_inline(a);
    }

    fqd_s sqr_canonical(const fqd_s& a) noexcept
    {
        return detail::_qd::sqr_canonical_inline(a);
    }

    // Multiplies qd by a double with an exact power-of-two fast path.
    fqd_s mul_pow2_or_double(const fqd_s& a, double b) noexcept
    {
        return detail::_qd::mul_pow2_or_double_canonical_inline(a, b);
    }

    fqd_s BL_VECTORCALL mul_add(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::mul_add_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL mul_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::mul_sub_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL value_sub_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::value_sub_mul_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL mul_add_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::add_finite_inline(detail::_qd::mul_add_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL mul_add_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::sub_finite_inline(detail::_qd::mul_add_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL mul_sub_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::add_finite_inline(detail::_qd::mul_sub_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL mul_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::sub_finite_inline(detail::_qd::mul_sub_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL mul_add_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::mul_add_mul_inline(a, b, c, d);
    }

    fqd_s BL_VECTORCALL mul_sub_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::mul_sub_mul_inline(a, b, c, d);
    }

    fqd_s BL_VECTORCALL mul_add_mul_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return detail::_qd::mul_add_mul_add_inline(a, b, c, d, e);
    }

    fqd_s BL_VECTORCALL mul_add_mul_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return detail::_qd::mul_add_mul_sub_inline(a, b, c, d, e);
    }

    fqd_s BL_VECTORCALL mul_sub_mul_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return detail::_qd::mul_sub_mul_add_inline(a, b, c, d, e);
    }

    fqd_s BL_VECTORCALL mul_sub_mul_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e) noexcept
    {
        return detail::_qd::mul_sub_mul_sub_inline(a, b, c, d, e);
    }

    fqd_s BL_VECTORCALL mul_add_mul_add_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e, const fqd_s& f) noexcept
    {
        return detail::_qd::add_finite_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), detail::_qd::mul_product_inline(e, f));
    }

    fqd_s BL_VECTORCALL mul_add_mul_add_mul_add_mul(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& e, const fqd_s& f, const fqd_s& g, const fqd_s& h) noexcept
    {
        return detail::_qd::add_finite_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), detail::_qd::mul_add_mul_inline(e, f, g, h));
    }

    fqd_s BL_VECTORCALL add_add_add(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::add_add_add_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL add_sub_add(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::add_sub_add_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL add_add_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::add_add_sub_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL add_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::add_sub_sub_inline(a, b, c);
    }

    fqd_s BL_VECTORCALL add_add_add_add(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::add_finite_inline(detail::_qd::add_add_add_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL add_add_add_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::sub_finite_inline(detail::_qd::add_add_add_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL add_add_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::sub_finite_inline(detail::_qd::add_add_sub_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL add_sub_sub_sub(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d) noexcept
    {
        return detail::_qd::sub_finite_inline(detail::_qd::add_sub_sub_inline(a, b, c), d);
    }

    fqd_s BL_VECTORCALL add_scaled_2_1(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_scaled_inline<2, 1>(a, b);
    }

    fqd_s BL_VECTORCALL add_scaled_1_2(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_scaled_inline<1, 2>(a, b);
    }

    fqd_s BL_VECTORCALL add_scaled_2_neg1(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_scaled_inline<2, -1>(a, b);
    }

    fqd_s BL_VECTORCALL add_scaled_1_neg2(const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_scaled_inline<1, -2>(a, b);
    }

    fqd_s BL_VECTORCALL add_mul_double(const fqd_s& addend, const fqd_s& value, double scalar) noexcept
    {
        return detail::_qd::add_mul_double_inline(addend, value, scalar);
    }

    fqd_s BL_VECTORCALL sub_mul_double(const fqd_s& minuend, const fqd_s& value, double scalar) noexcept
    {
        return detail::_qd::sub_mul_double_inline(minuend, value, scalar);
    }

    fqd_s BL_VECTORCALL mul_double_sub(const fqd_s& value, double scalar, const fqd_s& subtrahend) noexcept
    {
        return detail::_qd::mul_double_sub_inline(value, scalar, subtrahend);
    }

    fqd_s BL_VECTORCALL mul_double_add_mul_double(const fqd_s& a, double a_scalar, const fqd_s& b, double b_scalar) noexcept
    {
        return detail::_qd::add_raw5_raw5_inline(
            detail::_qd::mul_double_raw5_inline(a, a_scalar),
            detail::_qd::mul_double_raw5_inline(b, b_scalar));
    }

    fqd_s BL_VECTORCALL mul_double_add_mul_double_add(const fqd_s& a, double a_scalar, const fqd_s& b, double b_scalar, const fqd_s& c) noexcept
    {
        return detail::_qd::add_raw5_raw5_value_inline(
            detail::_qd::mul_double_raw5_inline(a, a_scalar),
            detail::_qd::mul_double_raw5_inline(b, b_scalar),
            c);
    }

    fqd_s BL_VECTORCALL div_add(const fqd_s& numerator, const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::div_prechecked_inline(numerator, detail::_qd::add_finite_inline(a, b));
    }

    fqd_s BL_VECTORCALL div_sub(const fqd_s& numerator, const fqd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::div_prechecked_inline(numerator, detail::_qd::sub_finite_inline(a, b));
    }

    fqd_s BL_VECTORCALL div_add_double(const fqd_s& numerator, const fqd_s& base_denominator, double scalar) noexcept
    {
        return div_add_double_kernel_inline(numerator, base_denominator, scalar);
    }

    fqd_s BL_VECTORCALL div_double_sub(const fqd_s& numerator, double scalar, const fqd_s& base_denominator) noexcept
    {
        return detail::_qd::div_double_sub_inline(numerator, scalar, base_denominator);
    }

    fqd_s BL_VECTORCALL mul_add_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_add_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL mul_sub_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL value_sub_mul_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::value_sub_mul_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL mul_add_mul_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), denominator);
    }

    fqd_s BL_VECTORCALL mul_sub_mul_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_mul_inline(a, b, c, d), denominator);
    }

    fqd_s BL_VECTORCALL add_add_add_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_add_add_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL add_sub_add_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_sub_add_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL add_add_sub_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_add_sub_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL add_sub_sub_div(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_sub_sub_inline(a, b, c), denominator);
    }

    fqd_s BL_VECTORCALL add_mul_double_div(const fqd_s& addend, const fqd_s& value, double scalar, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_mul_double_inline(addend, value, scalar), denominator);
    }

    fqd_s BL_VECTORCALL sub_mul_double_div(const fqd_s& minuend, const fqd_s& value, double scalar, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::sub_mul_double_inline(minuend, value, scalar), denominator);
    }

    fqd_s BL_VECTORCALL mul_double_sub_div(const fqd_s& value, double scalar, const fqd_s& subtrahend, const fqd_s& denominator) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_double_sub_inline(value, scalar, subtrahend), denominator);
    }

    fqd_s BL_VECTORCALL mul_add_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_add_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL mul_sub_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL value_sub_mul_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::value_sub_mul_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL mul_add_mul_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_add_mul_inline(a, b, c, d), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL mul_sub_mul_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& d, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_sub_mul_inline(a, b, c, d), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL add_add_add_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_add_add_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL add_sub_add_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_sub_add_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL add_add_sub_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_add_sub_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL add_sub_sub_div_add_double(const fqd_s& a, const fqd_s& b, const fqd_s& c, const fqd_s& denominator, double scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_sub_sub_inline(a, b, c), detail::_qd::add_double_finite_inline(denominator, scalar));
    }

    fqd_s BL_VECTORCALL add_mul_double_div_add_double(const fqd_s& addend, const fqd_s& value, double value_scalar, const fqd_s& denominator, double denominator_scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::add_mul_double_inline(addend, value, value_scalar), detail::_qd::add_double_finite_inline(denominator, denominator_scalar));
    }

    fqd_s BL_VECTORCALL sub_mul_double_div_add_double(const fqd_s& minuend, const fqd_s& value, double value_scalar, const fqd_s& denominator, double denominator_scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::sub_mul_double_inline(minuend, value, value_scalar), detail::_qd::add_double_finite_inline(denominator, denominator_scalar));
    }

    fqd_s BL_VECTORCALL mul_double_sub_div_add_double(const fqd_s& value, double value_scalar, const fqd_s& subtrahend, const fqd_s& denominator, double denominator_scalar) noexcept
    {
        return detail::_qd::div_prechecked_inline(detail::_qd::mul_double_sub_inline(value, value_scalar, subtrahend), detail::_qd::add_double_finite_inline(denominator, denominator_scalar));
    }

} // namespace bl::detail::_qd_runtime
