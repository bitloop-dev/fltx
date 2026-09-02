/**
 * fltx/detail/fdd_declarations.h - Shared dd math declarations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_DETAIL_DECLARATIONS_INCLUDED
#define FDD_DETAIL_DECLARATIONS_INCLUDED
#include <cstddef>

#include "fltx/fdd_arithmetic.h"
#include "fltx/fdd_classification.h"
#include "fltx/detail/fdd_consts.h"
#include "fltx/detail/common_math.h"
#include "fltx/detail/math_utils.h"

namespace bl {

namespace detail::_dd_runtime
{
    // complete-operation x86 FMA backend
    BL_NO_INLINE fdd_s fma_x86(const fdd_s& x, const fdd_s& y, const fdd_s& z);

    // roots
    BL_NO_INLINE fdd_s BL_VECTORCALL sqrt(const fdd_s& a);
    BL_NO_INLINE fdd_s hypot(const fdd_s& x, const fdd_s& y);

    // rounding and decimals
    BL_NO_INLINE fdd_s round_nearest_away_from_zero(const fdd_s& a);
    BL_NO_INLINE fdd_s round_decimals(fdd_s v, int prec);
    BL_NO_INLINE fdd_s round_significant(fdd_s v, int figures);
    BL_NO_INLINE long lround_nearest_away_from_zero(const fdd_s& x);
    BL_NO_INLINE long long llround_nearest_away_from_zero(const fdd_s& x);

    // remainders
    BL_NO_INLINE fdd_s fmod(const fdd_s& x, const fdd_s& y);
    BL_NO_INLINE fdd_s remquo(const fdd_s& x, const fdd_s& y, int* quo);

    // fractional decomposition
    BL_NO_INLINE fdd_s modf(const fdd_s& x, fdd_s* iptr) noexcept;

    // exp / log
    BL_NO_INLINE fdd_s exp(const fdd_s& x);
    BL_NO_INLINE fdd_s exp2(const fdd_s& x);
    BL_NO_INLINE fdd_s log(const fdd_s& a);
    BL_NO_INLINE fdd_s log2(const fdd_s& a);
    BL_NO_INLINE fdd_s log10(const fdd_s& x);
    BL_NO_INLINE fdd_s expm1(const fdd_s& x);
    BL_NO_INLINE fdd_s log1p(const fdd_s& x);

    // pow
    BL_NO_INLINE fdd_s pow(const fdd_s& x, const fdd_s& y);
    BL_NO_INLINE fdd_s pow(const fdd_s& x, double y);
    BL_NO_INLINE fdd_s ipow_signed(const fdd_s& x, std::intmax_t y);
    BL_NO_INLINE fdd_s ipow_unsigned(const fdd_s& x, std::uintmax_t y);

    // trig
    BL_NO_INLINE bool sincos(const fdd_s& x, fdd_s& s_out, fdd_s& c_out);
    BL_NO_INLINE fdd_s BL_VECTORCALL sin(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL cos(const fdd_s& x);
    BL_NO_INLINE fdd_s tan(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL atan(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL atan2(const fdd_s& y, const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL asin(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL acos(const fdd_s& x);

    // hyperbolic
    BL_NO_INLINE fdd_s BL_VECTORCALL sinh(const fdd_s& x);
    BL_NO_INLINE fdd_s cosh(const fdd_s& x);
    BL_NO_INLINE fdd_s tanh(const fdd_s& x);
    BL_NO_INLINE fdd_s asinh(const fdd_s& x);
    BL_NO_INLINE fdd_s acosh(const fdd_s& x);
    BL_NO_INLINE fdd_s atanh(const fdd_s& x);

    // erf/gamma and polynomials
    BL_NO_INLINE fdd_s erf(const fdd_s& x);
    BL_NO_INLINE fdd_s erfc(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL lgamma(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL tgamma(const fdd_s& x);
    BL_NO_INLINE fdd_s BL_VECTORCALL horner_forward(const fdd_s* coeffs, std::size_t count, const fdd_s& x) noexcept;
    BL_NO_INLINE fdd_s BL_VECTORCALL horner_reverse(const fdd_s* coeffs, std::size_t count, const fdd_s& x) noexcept;
    BL_NO_INLINE void horner_pair_forward(const fdd_s* left_coeffs, const fdd_s* right_coeffs, std::size_t count, const fdd_s& x, fdd_s& left_out, fdd_s& right_out) noexcept;

} // namespace detail::_dd_runtime

namespace detail::_dd_impl
{
    using namespace detail::_dd;

    // roots
    BL_FORCE_INLINE constexpr fdd_s sqrt(fdd_s a);
    BL_FORCE_INLINE constexpr fdd_s cbrt(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s hypot(const fdd_s& x, const fdd_s& y);

    // rounding and decimals
    BL_FORCE_INLINE constexpr double floor_limb(double x) noexcept;
    BL_FORCE_INLINE constexpr double ceil_limb(double x) noexcept;
    BL_FORCE_INLINE constexpr fdd_s floor(const fdd_s& a);
    BL_FORCE_INLINE constexpr fdd_s ceil(const fdd_s& a);
    BL_FORCE_INLINE constexpr fdd_s trunc(const fdd_s& a);
    BL_FORCE_INLINE constexpr fdd_s round_nearest_away_from_zero(const fdd_s& a);
    BL_FORCE_INLINE fdd_s round_nearest_away_from_zero_runtime(const fdd_s& a) noexcept;
    BL_FORCE_INLINE constexpr fdd_s round_decimals(fdd_s v, int prec);
    BL_FORCE_INLINE constexpr fdd_s round_significant(fdd_s v, int figures);
    BL_FORCE_INLINE constexpr fdd_s round_nearest_even(const fdd_s& a);
    BL_FORCE_INLINE constexpr long lround_nearest_away_from_zero(const fdd_s& x);
    BL_FORCE_INLINE constexpr long long llround_nearest_away_from_zero(const fdd_s& x);

    // arithmetic and comparisons
    BL_FORCE_INLINE constexpr fdd_s fma(const fdd_s& x, const fdd_s& y, const fdd_s& z);
    BL_FORCE_INLINE constexpr fdd_s fmin(const fdd_s& a, const fdd_s& b);
    BL_FORCE_INLINE constexpr fdd_s fmax(const fdd_s& a, const fdd_s& b);
    BL_FORCE_INLINE constexpr fdd_s fdim(const fdd_s& x, const fdd_s& y);
    BL_FORCE_INLINE constexpr fdd_s copysign(const fdd_s& x, const fdd_s& y);

    // remainders
    BL_FORCE_INLINE constexpr fdd_s fmod(const fdd_s& x, const fdd_s& y);
    BL_FORCE_INLINE constexpr fdd_s remquo(const fdd_s& x, const fdd_s& y, int* quo);

    // fractional decomposition
    BL_FORCE_INLINE constexpr fdd_s modf(const fdd_s& x, fdd_s* iptr) noexcept;

    // decomposition and scaling
    BL_FORCE_INLINE constexpr fdd_s ldexp(const fdd_s& x, int e);
    BL_FORCE_INLINE constexpr fdd_s frexp(const fdd_s& x, int* exp) noexcept;
    BL_FORCE_INLINE constexpr int ilogb(const fdd_s& x) noexcept;
    BL_FORCE_INLINE constexpr fdd_s logb(const fdd_s& x) noexcept;

    // adjacent values
    BL_FORCE_INLINE constexpr fdd_s nextafter(const fdd_s& from, const fdd_s& to) noexcept;

    // exp / log
    BL_FORCE_INLINE constexpr double log_as_double(fdd_s a);
    BL_FORCE_INLINE constexpr fdd_s exp(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s exp2(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s log(const fdd_s& a);
    BL_FORCE_INLINE constexpr fdd_s log2(const fdd_s& a);
    BL_FORCE_INLINE constexpr fdd_s log10(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s expm1(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s log1p(const fdd_s& x);

    // pow
    BL_FORCE_INLINE constexpr fdd_s pow10_fdd(int k);
    BL_MSVC_NOINLINE constexpr fdd_s pow(const fdd_s& x, const fdd_s& y);
    BL_MSVC_NOINLINE constexpr fdd_s pow(const fdd_s& x, double y);

    // trig
    BL_FORCE_INLINE constexpr bool sincos(const fdd_s& x, fdd_s& s_out, fdd_s& c_out);
    BL_FORCE_INLINE constexpr fdd_s sin(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s cos(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s tan(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s atan(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s atan2(const fdd_s& y, const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s asin(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s acos(const fdd_s& x);

    // hyperbolic
    BL_FORCE_INLINE constexpr fdd_s sinh(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s cosh(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s tanh(const fdd_s& x);
    BL_MSVC_NOINLINE constexpr fdd_s asinh(const fdd_s& x);
    BL_MSVC_NOINLINE constexpr fdd_s acosh(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s atanh(const fdd_s& x);

    // erf/gamma
    BL_FORCE_INLINE constexpr fdd_s erf(const fdd_s& x);
    BL_MSVC_NOINLINE constexpr fdd_s erfc(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s lgamma(const fdd_s& x);
    BL_FORCE_INLINE constexpr fdd_s tgamma(const fdd_s& x);

} // namespace detail::_dd_impl

} // namespace bl

#endif
