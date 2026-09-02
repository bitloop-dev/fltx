/**
 * fltx/fdd_math.cpp - Core dd runtime helpers and common math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fdd_math_basic.h"

namespace bl::detail::_dd_runtime
{
    // roots
    BL_NO_INLINE fdd_s BL_VECTORCALL sqrt(const fdd_s& a)
    {
        return detail::_dd_impl::sqrt(a);
    }

    BL_NO_INLINE fdd_s hypot(const fdd_s& x, const fdd_s& y)
    {
        return detail::_dd_impl::hypot(x, y);
    }

    // rounding and decimals
    BL_NO_INLINE fdd_s round_nearest_away_from_zero(const fdd_s& a)
    {
        return detail::_dd_impl::round_nearest_away_from_zero_runtime(a);
    }

    BL_NO_INLINE fdd_s round_decimals(fdd_s v, int prec)
    { 
        return detail::_dd_impl::round_decimals(v, prec);
    }

    BL_NO_INLINE fdd_s round_significant(fdd_s v, int figures)
    {
        return detail::_dd_impl::round_significant(v, figures);
    }

    BL_NO_INLINE long lround_nearest_away_from_zero(const fdd_s& x)
    {
        return detail::_dd_impl::lround_nearest_away_from_zero(x);
    }

    BL_NO_INLINE long long llround_nearest_away_from_zero(const fdd_s& x)
    {
        return detail::_dd_impl::llround_nearest_away_from_zero(x);
    }

    // remainders
    BL_NO_INLINE fdd_s fmod(const fdd_s& x, const fdd_s& y)
    {
        return detail::_dd_impl::fmod(x, y);
    }

    BL_NO_INLINE fdd_s remquo(const fdd_s& x, const fdd_s& y, int* quo)
    { 
        return detail::_dd_impl::remquo(x, y, quo); 
    }

    // fractional decomposition
    BL_NO_INLINE fdd_s modf(const fdd_s& x, fdd_s* iptr) noexcept
    { 
        return detail::_dd_impl::modf(x, iptr);
    }

} // namespace bl::detail::_dd_runtime
