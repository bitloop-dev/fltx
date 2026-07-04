/**
 * fltx/f256_math.cpp - Core f256 runtime helpers and common math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/f256_math_basic.h"

namespace bl::detail::_f256_runtime
{
    // roots
    BL_NO_INLINE f256_s sqrt(const f256_s& a)
    {
        return detail::_f256_impl::sqrt(a);
    }

    BL_NO_INLINE f256_s hypot(const f256_s& x, const f256_s& y)
    {
        return detail::_f256_impl::hypot(x, y);
    }

    // rounding and decimals
    BL_NO_INLINE f256_s round(const f256_s& a)
    {
        return detail::_f256_impl::round_runtime(a);
    }

    BL_NO_INLINE f256_s round_to_decimals(f256_s v, int prec)
    {
        return detail::_f256_impl::round_to_decimals(v, prec);
    }

    BL_NO_INLINE f256_s round_to_significant_figures(f256_s v, int figures)
    {
        return detail::_f256_impl::round_to_significant_figures(v, figures);
    }

    BL_NO_INLINE f256_s nearbyint(const f256_s& a)
    {
        return detail::_f256_impl::nearbyint_runtime(a);
    }

    BL_NO_INLINE long lround(const f256_s& x)
    {
        return detail::_f256_impl::lround(x);
    }

    BL_NO_INLINE long long llround(const f256_s& x)
    {
        return detail::_f256_impl::llround(x);
    }

    BL_NO_INLINE long lrint(const f256_s& x)
    {
        return detail::_f256_impl::lrint(x);
    }

    BL_NO_INLINE long long llrint(const f256_s& x)
    {
        return detail::_f256_impl::llrint(x);
    }

    // remainders
    BL_NO_INLINE f256_s fmod(const f256_s& x, const f256_s& y)
    {
        return detail::_f256_impl::fmod(x, y);
    }

    BL_NO_INLINE f256_s remquo(const f256_s& x, const f256_s& y, int* quo)
    { 
        return detail::_f256_impl::remquo(x, y, quo); 
    }

    // fractional decomposition
    BL_NO_INLINE f256_s modf(const f256_s& x, f256_s* iptr) noexcept
    {
        return detail::_f256_impl::modf(x, iptr);
    }

} // namespace bl::detail::_f256_runtime
