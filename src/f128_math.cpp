/**
 * fltx/f128_math.cpp - Core f128 runtime helpers and common math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/f128_math_basic.h"

namespace bl::detail::_f128_runtime
{
    // roots
    BL_NO_INLINE f128_s sqrt(const f128_s& a)
    {
        return detail::_f128_impl::sqrt(a);
    }

    BL_NO_INLINE f128_s hypot(const f128_s& x, const f128_s& y)
    {
        return detail::_f128_impl::hypot(x, y);
    }

    // rounding and decimals
    BL_NO_INLINE f128_s round_nearest_away_from_zero(const f128_s& a)
    {
        return detail::_f128_impl::round_nearest_away_from_zero_runtime(a);
    }

    BL_NO_INLINE f128_s round_to_decimals(f128_s v, int prec) 
    { 
        return detail::_f128_impl::round_to_decimals(v, prec);
    }

    BL_NO_INLINE f128_s round_to_significant_figures(f128_s v, int figures)
    {
        return detail::_f128_impl::round_to_significant_figures(v, figures);
    }

    BL_NO_INLINE long lround_nearest_away_from_zero(const f128_s& x)
    {
        return detail::_f128_impl::lround_nearest_away_from_zero(x);
    }

    BL_NO_INLINE long long llround_nearest_away_from_zero(const f128_s& x)
    {
        return detail::_f128_impl::llround_nearest_away_from_zero(x);
    }

    // remainders
    BL_NO_INLINE f128_s fmod(const f128_s& x, const f128_s& y)
    {
        return detail::_f128_impl::fmod(x, y);
    }

    BL_NO_INLINE f128_s remquo(const f128_s& x, const f128_s& y, int* quo) 
    { 
        return detail::_f128_impl::remquo(x, y, quo); 
    }

    // fractional decomposition
    BL_NO_INLINE f128_s modf(const f128_s& x, f128_s* iptr) noexcept 
    { 
        return detail::_f128_impl::modf(x, iptr);
    }

} // namespace bl::detail::_f128_runtime
