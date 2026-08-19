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
#if BL_FP_BARRIER_ACTIVE
        if (detail::_f256::has_subnormal_limb(a)) [[unlikely]]
        {
            constexpr int input_scale = 512;
            return detail::_f256::scale_terms_guarded(
                detail::_f256_impl::sqrt(
                    detail::_f256::scale_terms_guarded(a, input_scale)),
                -(input_scale / 2));
        }
#endif
        return detail::_f256_impl::sqrt(a);
    }

    BL_NO_INLINE f256_s hypot(const f256_s& x, const f256_s& y)
    {
#if BL_FP_BARRIER_ACTIVE
        const double largest_head = detail::fp::absd(x.x0) > detail::fp::absd(y.x0)
            ? detail::fp::absd(x.x0)
            : detail::fp::absd(y.x0);
        const bool needs_scale = detail::_f256::has_subnormal_limb(x) ||
            detail::_f256::has_subnormal_limb(y) ||
            (largest_head != 0.0 && largest_head < 0x1p-400);
        if (needs_scale && detail::fp::absd(x.x0) < 0x1p500 &&
            detail::fp::absd(y.x0) < 0x1p500) [[unlikely]]
        {
            constexpr int input_scale = 512;
            return detail::_f256::scale_terms_guarded(
                detail::_f256_impl::hypot(
                    detail::_f256::scale_terms_guarded(x, input_scale),
                    detail::_f256::scale_terms_guarded(y, input_scale)),
                -input_scale);
        }
#endif
        return detail::_f256_impl::hypot(x, y);
    }

    // rounding and decimals
    BL_NO_INLINE f256_s BL_VECTORCALL round_nearest_away_from_zero(const f256_s& a)
    {
        return detail::_f256_impl::round_nearest_away_from_zero_runtime(a);
    }

    BL_NO_INLINE f256_s round_decimals(f256_s v, int prec)
    {
        return detail::_f256_impl::round_decimals(v, prec);
    }

    BL_NO_INLINE f256_s round_significant(f256_s v, int figures)
    {
        return detail::_f256_impl::round_significant(v, figures);
    }

    BL_NO_INLINE long lround_nearest_away_from_zero(const f256_s& x)
    {
        return detail::_f256_impl::lround_nearest_away_from_zero(x);
    }

    BL_NO_INLINE long long llround_nearest_away_from_zero(const f256_s& x)
    {
        return detail::_f256_impl::llround_nearest_away_from_zero(x);
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
#if BL_FP_BARRIER_ACTIVE
        if (!detail::fp::isinf_or_nan(x.x0) && detail::fp::absd(x.x0) < 1.0)
        {
            if (iptr)
                *iptr = detail::_f256::signed_zero(bl::signbit(x));
            return x;
        }
#endif
        return detail::_f256_impl::modf(x, iptr);
    }

} // namespace bl::detail::_f256_runtime
