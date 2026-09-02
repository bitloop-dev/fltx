/**
 * fltx/fdd_transcendental.cpp - Runtime dd transcendental math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fdd_math_transcendental.h"

 // MinGW f64/f32 libm fallbacks implemented through the dd kernels.
 // Kept in this translation unit to avoid compiling the heavyweight dd
 // transcendental implementation in a second source file.
namespace bl::detail::_f64_runtime
{
    BL_NO_INLINE double sin_large(double x) noexcept
    {
        if (x == 0.0)
            return x;

        return static_cast<double>(
            detail::_dd_impl::sin(fdd_s{ x }));
    }

    BL_NO_INLINE double cos_large(double x) noexcept
    {
        return static_cast<double>(
            detail::_dd_impl::cos(fdd_s{ x }));
    }

    BL_NO_INLINE double tan_large(double x) noexcept
    {
        return static_cast<double>(
            detail::_dd_impl::tan(fdd_s{ x }));
    }
}

namespace bl::detail::_dd_runtime
{
    BL_NO_INLINE fdd_s BL_VECTORCALL horner_forward(const fdd_s* coeffs, std::size_t count, const fdd_s& x) noexcept
    {
        if (count == 0)
            return {};

        fdd_s p = coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
            p = detail::_dd::mul_add_inline(p, x, coeffs[i]);
        return p;
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL horner_reverse(const fdd_s* coeffs, std::size_t count, const fdd_s& x) noexcept
    {
        if (count == 0)
            return {};

        fdd_s p = coeffs[count - 1];
        for (std::size_t i = count - 1; i > 0; --i)
            p = detail::_dd::mul_add_inline(p, x, coeffs[i - 1]);
        return p;
    }

    BL_NO_INLINE void horner_pair_forward(
        const fdd_s* left_coeffs,
        const fdd_s* right_coeffs,
        std::size_t count,
        const fdd_s& x,
        fdd_s& left_out,
        fdd_s& right_out) noexcept
    {
        if (count == 0)
        {
            left_out = fdd_s{};
            right_out = fdd_s{};
            return;
        }

        fdd_s left  = left_coeffs[0];
        fdd_s right = right_coeffs[0];
        for (std::size_t i = 1; i < count; ++i)
            detail::_dd::mul_add_pair_same_rhs_inline(left, right, x, left_coeffs[i], right_coeffs[i], left, right);

        left_out = left;
        right_out = right;
    }

    // exponential and logarithmic
    BL_NO_INLINE fdd_s exp(const fdd_s& x)
    {
        return detail::_dd_impl::exp(x);
    }

    BL_NO_INLINE fdd_s exp2(const fdd_s& x)
    {
        return detail::_dd_impl::exp2(x);
    }

    BL_NO_INLINE fdd_s log(const fdd_s& a)
    {
        return detail::_dd_impl::log(a);
    }

    BL_NO_INLINE fdd_s log2(const fdd_s& a)
    {
        return detail::_dd_impl::log2(a);
    }

    BL_NO_INLINE fdd_s log10(const fdd_s& x)
    {
        return detail::_dd_impl::log10(x);
    }

    BL_NO_INLINE fdd_s expm1(const fdd_s& x)
    {
        return detail::_dd_impl::expm1(x);
    }

    BL_NO_INLINE fdd_s log1p(const fdd_s& x)
    {
        return detail::_dd_impl::log1p(x);
    }

    // powers
    BL_NO_INLINE fdd_s pow(const fdd_s& x, const fdd_s& y)
    {
        return detail::_dd_impl::pow(x, y);
    }

    BL_NO_INLINE fdd_s pow(const fdd_s& x, double y)
    {
        return detail::_dd_impl::pow(x, y);
    }

    BL_NO_INLINE fdd_s ipow_signed(const fdd_s& x, std::intmax_t y)
    {
        return detail::_dd::ipow_integer(x, y);
    }

    BL_NO_INLINE fdd_s ipow_unsigned(const fdd_s& x, std::uintmax_t y)
    {
        return detail::_dd::ipow_integer(x, y);
    }

    // trigonometric
    BL_NO_INLINE bool sincos(const fdd_s& x, fdd_s& s_out, fdd_s& c_out)
    {
        return detail::_dd_impl::sincos(x, s_out, c_out);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL sin(const fdd_s& x)
    {
        return detail::_dd_impl::sin(x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL cos(const fdd_s& x)
    {
        return detail::_dd_impl::cos(x);
    }

    BL_NO_INLINE fdd_s tan(const fdd_s& x)
    {
        return detail::_dd_impl::tan(x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL atan(const fdd_s& x)
    {
        return detail::_dd_impl::atan(x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL atan2(const fdd_s& y, const fdd_s& x)
    {
        return detail::_dd_impl::atan2(y, x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL asin(const fdd_s& x)
    {
        return detail::_dd_impl::asin(x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL acos(const fdd_s& x)
    {
        return detail::_dd_impl::acos(x);
    }

    // hyperbolic
    BL_NO_INLINE fdd_s BL_VECTORCALL sinh(const fdd_s& x)
    {
        return detail::_dd_impl::sinh(x);
    }

    BL_NO_INLINE fdd_s cosh(const fdd_s& x)
    {
        return detail::_dd_impl::cosh(x);
    }

    BL_NO_INLINE fdd_s tanh(const fdd_s& x)
    {
        return detail::_dd_impl::tanh(x);
    }

    BL_NO_INLINE fdd_s asinh(const fdd_s& x)
    {
        return detail::_dd_impl::asinh(x);
    }

    BL_NO_INLINE fdd_s acosh(const fdd_s& x)
    {
        return detail::_dd_impl::acosh(x);
    }

    BL_NO_INLINE fdd_s atanh(const fdd_s& x)
    {
        return detail::_dd_impl::atanh(x);
    }

    // special functions
    BL_NO_INLINE fdd_s erf(const fdd_s& x)
    {
        return detail::_dd_impl::erf(x);
    }

    BL_NO_INLINE fdd_s erfc(const fdd_s& x)
    {
        return detail::_dd_impl::erfc(x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL lgamma(const fdd_s& x)
    {
        return detail::_dd_impl::lgamma(x);
    }

    BL_NO_INLINE fdd_s BL_VECTORCALL tgamma(const fdd_s& x)
    {
        return detail::_dd_impl::tgamma(x);
    }

} // namespace bl::detail::_dd_runtime
