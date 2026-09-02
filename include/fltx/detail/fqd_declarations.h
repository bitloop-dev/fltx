/**
 * fltx/detail/fqd_declarations.h - Shared qd math declarations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_DETAIL_DECLARATIONS_INCLUDED
#define FQD_DETAIL_DECLARATIONS_INCLUDED
#include <cstddef>

#include "fltx/detail/fqd_expressions.h"
#include "fltx/detail/fqd_consts.h"
#include "fltx/detail/common_math.h"
#include "fltx/detail/math_utils.h"

namespace bl {

namespace detail::_qd_runtime
{
    // complete-operation x86 FMA backend
    BL_NO_INLINE fqd_s fma_x86(const fqd_s& x, const fqd_s& y, const fqd_s& z);
    BL_NO_INLINE fqd_s fma(const fqd_s& x, const fqd_s& y, const fqd_s& z);
    BL_NO_INLINE fqd_s fma_cancellation(const fqd_s& x, const fqd_s& y, const fqd_s& z);

    // roots
    BL_NO_INLINE fqd_s sqrt(const fqd_s& a);
    BL_NO_INLINE fqd_s cbrt(const fqd_s& x);
    BL_NO_INLINE fqd_s hypot(const fqd_s& x, const fqd_s& y);

    // rounding and decimals
    BL_NO_INLINE fqd_s BL_VECTORCALL round_nearest_away_from_zero(const fqd_s& a);
    BL_NO_INLINE fqd_s round_decimals(fqd_s v, int prec);
    BL_NO_INLINE fqd_s round_significant(fqd_s v, int figures);
    BL_NO_INLINE long lround_nearest_away_from_zero(const fqd_s& x);
    BL_NO_INLINE long long llround_nearest_away_from_zero(const fqd_s& x);

    // remainders
    BL_NO_INLINE fqd_s fmod(const fqd_s& x, const fqd_s& y);
    BL_NO_INLINE fqd_s remquo(const fqd_s& x, const fqd_s& y, int* quo);

    // fractional decomposition
    BL_NO_INLINE fqd_s modf(const fqd_s& x, fqd_s* iptr) noexcept;

    // exp / log
    BL_NO_INLINE fqd_s exp(const fqd_s& x);
    BL_NO_INLINE fqd_s exp2(const fqd_s& x);
    BL_NO_INLINE fqd_s log(const fqd_s& a);
    BL_NO_INLINE fqd_s log2(const fqd_s& a);
    BL_NO_INLINE fqd_s log10(const fqd_s& a);
    BL_NO_INLINE fqd_s expm1(const fqd_s& x);
    BL_NO_INLINE fqd_s log1p(const fqd_s& x);

    // pow
    BL_NO_INLINE fqd_s BL_VECTORCALL pow(const fqd_s& x, const fqd_s& y);
    BL_NO_INLINE fqd_s BL_VECTORCALL pow(const fqd_s& x, double y);
    BL_NO_INLINE fqd_s BL_VECTORCALL ipow_signed(const fqd_s& x, std::intmax_t y);
    BL_NO_INLINE fqd_s BL_VECTORCALL ipow_unsigned(const fqd_s& x, std::uintmax_t y);

    // trig
    BL_NO_INLINE bool sincos(const fqd_s& x, fqd_s& s_out, fqd_s& c_out);
    BL_NO_INLINE fqd_s sin(const fqd_s& x);
    BL_NO_INLINE fqd_s cos(const fqd_s& x);
    BL_NO_INLINE fqd_s tan(const fqd_s& x);
    BL_NO_INLINE fqd_s atan(const fqd_s& x);
    BL_NO_INLINE fqd_s atan2(const fqd_s& y, const fqd_s& x);
    BL_NO_INLINE fqd_s asin(const fqd_s& x);
    BL_NO_INLINE fqd_s acos(const fqd_s& x);

    // hyperbolic
    BL_NO_INLINE fqd_s sinh(const fqd_s& x);
    BL_NO_INLINE fqd_s cosh(const fqd_s& x);
    BL_NO_INLINE fqd_s tanh(const fqd_s& x);
    BL_NO_INLINE fqd_s asinh(const fqd_s& x);
    BL_NO_INLINE fqd_s acosh(const fqd_s& x);
    BL_NO_INLINE fqd_s atanh(const fqd_s& x);

    // erf/gamma and polynomials
    BL_NO_INLINE fqd_s erf(const fqd_s& x);
    BL_NO_INLINE fqd_s erfc(const fqd_s& x);
    BL_NO_INLINE fqd_s lgamma(const fqd_s& x);
    BL_NO_INLINE fqd_s tgamma(const fqd_s& x);
    BL_NO_INLINE fqd_s mul_add_horner_step(const fqd_s& a, const fqd_s& b, const fqd_s& c) noexcept;
    BL_NO_INLINE fqd_s horner_forward(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept;
    BL_NO_INLINE fqd_s horner_reverse(const fqd_s* coeffs, std::size_t count, const fqd_s& x) noexcept;
    BL_NO_INLINE void horner_pair_forward(const fqd_s* left_coeffs, const fqd_s* right_coeffs, std::size_t count, const fqd_s& x, fqd_s& left_out, fqd_s& right_out) noexcept;
    BL_NO_INLINE fqd_s cheb_eval(const fqd_s& x, const fqd_s* coeffs, std::size_t count, double shift) noexcept;
    BL_NO_INLINE fqd_s log1p_series_reduced(const fqd_s& x) noexcept;

} // namespace detail::_qd_runtime

namespace detail::_qd_impl
{
    using namespace detail::_qd;

    // roots
    BL_FORCE_INLINE constexpr fqd_s sqrt(const fqd_s& a);
    BL_FORCE_INLINE constexpr fqd_s sqrt_accurate(const fqd_s& a);
    BL_FORCE_INLINE constexpr fqd_s cbrt(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s hypot(const fqd_s& x, const fqd_s& y);

    // rounding and decimals
    BL_FORCE_INLINE constexpr fqd_s trunc(const fqd_s& a);
    BL_FORCE_INLINE constexpr fqd_s round_nearest_away_from_zero(const fqd_s& a);
    BL_FORCE_INLINE fqd_s round_nearest_away_from_zero_runtime(const fqd_s& a) noexcept;
    BL_FORCE_INLINE constexpr fqd_s round_decimals(fqd_s v, int prec);
    BL_FORCE_INLINE constexpr fqd_s round_significant(fqd_s v, int figures);
    BL_FORCE_INLINE constexpr fqd_s round_nearest_even(const fqd_s& a);
    BL_FORCE_INLINE constexpr long lround_nearest_away_from_zero(const fqd_s& x);
    BL_FORCE_INLINE constexpr long long llround_nearest_away_from_zero(const fqd_s& x);

    // arithmetic and comparisons
    BL_FORCE_INLINE constexpr fqd_s fma(const fqd_s& x, const fqd_s& y, const fqd_s& z);
    BL_FORCE_INLINE constexpr fqd_s fmin(const fqd_s& a, const fqd_s& b);
    BL_FORCE_INLINE constexpr fqd_s fmax(const fqd_s& a, const fqd_s& b);
    BL_FORCE_INLINE constexpr fqd_s fdim(const fqd_s& x, const fqd_s& y);
    BL_FORCE_INLINE constexpr fqd_s copysign(const fqd_s& x, const fqd_s& y);

    // remainders
    BL_FORCE_INLINE constexpr fqd_s fmod(const fqd_s& x, const fqd_s& y);
    BL_FORCE_INLINE constexpr fqd_s remquo(const fqd_s& x, const fqd_s& y, int* quo);

    // fractional decomposition
    BL_FORCE_INLINE constexpr fqd_s modf(const fqd_s& x, fqd_s* iptr) noexcept;

    // decomposition and scaling
    BL_FORCE_INLINE constexpr fqd_s ldexp(const fqd_s& a, int e);
    BL_FORCE_INLINE constexpr fqd_s frexp(const fqd_s& x, int* exp) noexcept;
    BL_FORCE_INLINE constexpr int ilogb(const fqd_s& x) noexcept;
    BL_FORCE_INLINE constexpr fqd_s logb(const fqd_s& x) noexcept;

    // adjacent values
    BL_FORCE_INLINE constexpr fqd_s nextafter(const fqd_s& from, const fqd_s& to) noexcept;

    // exp / log
    BL_FORCE_INLINE constexpr double log_as_double(fqd_s a) noexcept;
    BL_FORCE_INLINE constexpr fqd_s exp(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s exp2(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s log(const fqd_s& a);
    BL_FORCE_INLINE constexpr fqd_s log2(const fqd_s& a);
    BL_FORCE_INLINE constexpr fqd_s log10(const fqd_s& a);
    BL_FORCE_INLINE constexpr fqd_s expm1(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s log1p(const fqd_s& x);

    // pow
    BL_FORCE_INLINE constexpr fqd_s pow10_fqd(int k);
    BL_MSVC_NOINLINE constexpr fqd_s pow(const fqd_s& x, const fqd_s& y);
    BL_MSVC_NOINLINE constexpr fqd_s pow(const fqd_s& x, double y);

    // trig
    BL_FORCE_INLINE constexpr bool sincos(const fqd_s& x, fqd_s& s_out, fqd_s& c_out);
    BL_FORCE_INLINE constexpr fqd_s sin(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s cos(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s tan(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s atan(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s atan2(const fqd_s& y, const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s asin(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s acos(const fqd_s& x);

    // hyperbolic
    BL_FORCE_INLINE constexpr fqd_s sinh(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s cosh(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s tanh(const fqd_s& x);
    BL_MSVC_NOINLINE constexpr fqd_s asinh(const fqd_s& x);
    BL_MSVC_NOINLINE constexpr fqd_s acosh(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s atanh(const fqd_s& x);

    // erf/gamma
    BL_FORCE_INLINE constexpr fqd_s erf(const fqd_s& x);
    BL_MSVC_NOINLINE constexpr fqd_s erfc(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s lgamma(const fqd_s& x);
    BL_FORCE_INLINE constexpr fqd_s tgamma(const fqd_s& x);

} // namespace detail::_qd_impl

} // namespace bl

#endif
