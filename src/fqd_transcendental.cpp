/**
 * fltx/fqd_transcendental.cpp - Runtime qd transcendental math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fqd_math_transcendental.h"

namespace bl::detail::_qd_runtime
{
    BL_NO_INLINE fqd_s mul_add_horner_step(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept
    {
        return detail::_qd::mul_add_inline(a, b, c);
    }

    BL_NO_INLINE fqd_s horner_forward(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept
    {
        if (count == 0)
            return {};

        fqd_s p = coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
            p = detail::_qd::mul_add_inline(p, x, coeffs[i]);
        return p;
    }

    BL_NO_INLINE fqd_s horner_reverse(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept
    {
        if (count == 0)
            return {};

        fqd_s p = coeffs[count - 1];
        for (std::size_t i = count - 1; i > 0; --i)
            p = detail::_qd::mul_add_inline(p, x, coeffs[i - 1]);
        return p;
    }

    BL_NO_INLINE void horner_pair_forward(
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
            left = detail::_qd::mul_add_inline(left, x, left_coeffs[i]);
            right = detail::_qd::mul_add_inline(right, x, right_coeffs[i]);
        }

        left_out = left;
        right_out = right;
    }

    BL_NO_INLINE fqd_s cheb_eval(const fqd_s& x, const fqd_s* coeffs, std::size_t count, double shift) noexcept
    {
        if (count == 0)
            return {};

        const fqd_s t = detail::_qd::sub_finite_inline(
            detail::_qd::mul_double_product_inline(x, 2.0),
            fqd_s{ shift });
        fqd_s b1{ 0.0 };
        fqd_s b2{ 0.0 };

        for (std::size_t i = count - 1; i >= 1; --i)
        {
            const fqd_s b0 = detail::_qd::add_finite_inline(
                detail::_qd::mul_double_sub_pow2_inline(detail::_qd::mul_product_inline(t, b1), 2.0, b2),
                coeffs[i]);
            b2 = b1;
            b1 = b0;
        }

        return detail::_qd::add_finite_inline(detail::_qd::mul_sub_inline(t, b1, b2), coeffs[0]);
    }

    BL_NO_INLINE fqd_s log1p_series_reduced(const fqd_s& x) noexcept
    {
        const fqd_s z = detail::_qd::div_add_double_inline(x, x, 2.0);
        const fqd_s z2 = detail::_qd::sqr_inline(z);

        fqd_s term = z;
        fqd_s sum  = z;

        for (int k = 3; k <= 257; k += 2)
        {
            term = detail::_qd::mul_product_inline(term, z2);
            const fqd_s add = detail::_qd::div_double_prechecked_inline(term, static_cast<double>(k));
            sum = detail::_qd::add_finite_inline(sum, add);

            const fqd_s asum  = detail::_qd::mag(sum);
            const fqd_s scale = (asum > fqd_s{ 1.0 }) ? asum : fqd_s{ 1.0 };
            if (detail::_qd::mag(add) <= detail::_qd::mul_product_inline(detail::_qd::convergence_epsilon, scale))
                break;
        }

        return detail::_qd::add_finite_inline(sum, sum);
    }

    // roots
    BL_NO_INLINE fqd_s cbrt(const fqd_s& x)
    {
#if BL_FP_BARRIER_ACTIVE
        if (detail::_qd::has_subnormal_limb(x)) [[unlikely]]
        {
            constexpr int input_scale = 510;
            return detail::_qd::scale_terms_guarded(
                detail::_qd_impl::cbrt(
                    detail::_qd::scale_terms_guarded(x, input_scale)),
                -(input_scale / 3));
        }
#endif
        return detail::_qd_impl::cbrt(x);
    }

    // exponential and logarithmic
    BL_NO_INLINE fqd_s exp(const fqd_s& x)
    {
#if BL_FP_BARRIER_ACTIVE
        if (x.x0 < -450.0 && !detail::fp::isinf_or_nan(x.x0)) [[unlikely]]
        {
            constexpr int result_scale = 512;
            const fqd_s offset = detail::_qd::scale_terms_guarded(
                std::numbers::ln2_v<fqd_s>,
                9);
            const fqd_s adjusted = detail::_qd_runtime::add_finite(x, offset);
            return detail::_qd::scale_terms_guarded(
                detail::_qd_impl::exp(adjusted),
                -result_scale);
        }
#endif
        return detail::_qd_impl::exp(x);
    }

    BL_NO_INLINE fqd_s exp2(const fqd_s& x)
    {
        return detail::_qd_impl::exp2(x);
    }

    BL_NO_INLINE fqd_s log(const fqd_s& a)
    {
#if BL_FP_BARRIER_ACTIVE
        if (detail::_qd::has_subnormal_limb(a)) [[unlikely]]
        {
            constexpr int input_scale = 64;
            const fqd_s scaled = detail::_qd::scale_terms_guarded(a, input_scale);
            const fqd_s correction = detail::_qd::scale_terms_guarded(
                std::numbers::ln2_v<fqd_s>,
                6);
            return detail::_qd_runtime::sub_finite(
                detail::_qd_impl::log(scaled),
                correction);
        }
#endif
        return detail::_qd_impl::log(a);
    }

    BL_NO_INLINE fqd_s log2(const fqd_s& a)
    {
#if BL_FP_BARRIER_ACTIVE
        if (detail::_qd::has_subnormal_limb(a)) [[unlikely]]
        {
            constexpr int input_scale = 64;
            return detail::_qd_runtime::sub_double_finite(
                detail::_qd_impl::log2(
                    detail::_qd::scale_terms_guarded(a, input_scale)),
                static_cast<double>(input_scale));
        }
#endif
        return detail::_qd_impl::log2(a);
    }

    BL_NO_INLINE fqd_s log10(const fqd_s& a)
    {
#if BL_FP_BARRIER_ACTIVE
        if (detail::_qd::has_subnormal_limb(a)) [[unlikely]]
        {
            constexpr int input_scale = 64;
            const fqd_s log10_two = detail::_qd_runtime::mul_finite(
                std::numbers::ln2_v<fqd_s>,
                std::numbers::log10e_v<fqd_s>);
            const fqd_s correction =
                detail::_qd::scale_terms_guarded(log10_two, 6);
            return detail::_qd_runtime::sub_finite(
                detail::_qd_impl::log10(
                    detail::_qd::scale_terms_guarded(a, input_scale)),
                correction);
        }
#endif
        return detail::_qd_impl::log10(a);
    }

    BL_NO_INLINE fqd_s expm1(const fqd_s& x)
    {
#if BL_FP_BARRIER_ACTIVE
        const int exponent = detail::fp::frexp_exponent(x.x0);
        if (exponent != 0 && exponent < -540) [[unlikely]]
            return x;
#endif
        return detail::_qd_impl::expm1(x);
    }

    BL_NO_INLINE fqd_s log1p(const fqd_s& x)
    {
#if BL_FP_BARRIER_ACTIVE
        const int exponent = detail::fp::frexp_exponent(x.x0);
        if (exponent != 0 && exponent < -540) [[unlikely]]
            return x;
#endif
        return detail::_qd_impl::log1p(x);
    }

    // powers
    BL_NO_INLINE fqd_s BL_VECTORCALL pow(const fqd_s& x, const fqd_s& y)
    {
        return detail::_qd_impl::pow(x, y);
    }

    BL_NO_INLINE fqd_s BL_VECTORCALL pow(const fqd_s& x, double y)
    {
        return detail::_qd_impl::pow(x, y);
    }

    BL_NO_INLINE fqd_s BL_VECTORCALL ipow_signed(const fqd_s& x, std::intmax_t y)
    {
        return detail::_qd::ipow_integer(x, y);
    }

    BL_NO_INLINE fqd_s BL_VECTORCALL ipow_unsigned(const fqd_s& x, std::uintmax_t y)
    {
        return detail::_qd::ipow_integer(x, y);
    }

    // trigonometric
    BL_NO_INLINE bool sincos(const fqd_s& x, fqd_s& s_out, fqd_s& c_out)
    {
        return detail::_qd_impl::sincos(x, s_out, c_out);
    }

    BL_NO_INLINE fqd_s sin(const fqd_s& x)
    {
        return detail::_qd_impl::sin(x);
    }

    BL_NO_INLINE fqd_s cos(const fqd_s& x)
    {
        return detail::_qd_impl::cos(x);
    }

    BL_NO_INLINE fqd_s tan(const fqd_s& x)
    {
        return detail::_qd_impl::tan(x);
    }

    BL_NO_INLINE fqd_s atan(const fqd_s& x)
    {
        return detail::_qd_impl::atan(x);
    }

    BL_NO_INLINE fqd_s atan2(const fqd_s& y, const fqd_s& x)
    {
        return detail::_qd_impl::atan2(y, x);
    }

    BL_NO_INLINE fqd_s asin(const fqd_s& x)
    {
        return detail::_qd_impl::asin(x);
    }

    BL_NO_INLINE fqd_s acos(const fqd_s& x)
    {
        return detail::_qd_impl::acos(x);
    }

    // hyperbolic
    BL_NO_INLINE fqd_s sinh(const fqd_s& x)
    {
        return detail::_qd_impl::sinh(x);
    }

    BL_NO_INLINE fqd_s cosh(const fqd_s& x)
    {
        return detail::_qd_impl::cosh(x);
    }

    BL_NO_INLINE fqd_s tanh(const fqd_s& x)
    {
        return detail::_qd_impl::tanh(x);
    }

    BL_NO_INLINE fqd_s asinh(const fqd_s& x)
    {
        return detail::_qd_impl::asinh(x);
    }

    BL_NO_INLINE fqd_s acosh(const fqd_s& x)
    {
        return detail::_qd_impl::acosh(x);
    }

    BL_NO_INLINE fqd_s atanh(const fqd_s& x)
    {
        return detail::_qd_impl::atanh(x);
    }

    // special functions
    BL_NO_INLINE fqd_s erf(const fqd_s& x)
    {
        return detail::_qd_impl::erf(x);
    }

    BL_NO_INLINE fqd_s erfc(const fqd_s& x)
    {
        return detail::_qd_impl::erfc(x);
    }

    BL_NO_INLINE fqd_s lgamma(const fqd_s& x)
    {
        return detail::_qd_impl::lgamma(x);
    }

    BL_NO_INLINE fqd_s tgamma(const fqd_s& x)
    {
        return detail::_qd_impl::tgamma(x);
    }

} // namespace bl::detail::_qd_runtime
