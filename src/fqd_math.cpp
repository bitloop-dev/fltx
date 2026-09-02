/**
 * fltx/fqd_math.cpp - Core qd runtime helpers and common math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fqd_math_basic.h"

namespace bl::detail::_qd_runtime
{
    // roots
    BL_NO_INLINE fqd_s sqrt(const fqd_s& a)
    {
#if BL_FP_BARRIER_ACTIVE
        if (detail::_qd::has_subnormal_limb(a)) [[unlikely]]
        {
            constexpr int input_scale = 512;
            return detail::_qd::scale_terms_guarded(
                detail::_qd_impl::sqrt(
                    detail::_qd::scale_terms_guarded(a, input_scale)),
                -(input_scale / 2));
        }
#endif
        return detail::_qd_impl::sqrt(a);
    }

    BL_NO_INLINE fqd_s hypot(const fqd_s& x, const fqd_s& y)
    {
#if BL_FP_BARRIER_ACTIVE
        const double largest_head = detail::fp::absd(x.x0) > detail::fp::absd(y.x0)
            ? detail::fp::absd(x.x0)
            : detail::fp::absd(y.x0);
        const bool needs_scale = detail::_qd::has_subnormal_limb(x) ||
            detail::_qd::has_subnormal_limb(y) ||
            (largest_head != 0.0 && largest_head < 0x1p-400);
        if (needs_scale && detail::fp::absd(x.x0) < 0x1p500 &&
            detail::fp::absd(y.x0) < 0x1p500) [[unlikely]]
        {
            constexpr int input_scale = 512;
            return detail::_qd::scale_terms_guarded(
                detail::_qd_impl::hypot(
                    detail::_qd::scale_terms_guarded(x, input_scale),
                    detail::_qd::scale_terms_guarded(y, input_scale)),
                -input_scale);
        }
#endif
        return detail::_qd_impl::hypot(x, y);
    }

    // rounding and decimals
    BL_NO_INLINE fqd_s BL_VECTORCALL round_nearest_away_from_zero(const fqd_s& a)
    {
        return detail::_qd_impl::round_nearest_away_from_zero_runtime(a);
    }

    BL_NO_INLINE fqd_s round_decimals(fqd_s v, int prec)
    {
        return detail::_qd_impl::round_decimals(v, prec);
    }

    BL_NO_INLINE fqd_s round_significant(fqd_s v, int figures)
    {
        return detail::_qd_impl::round_significant(v, figures);
    }

    BL_NO_INLINE long lround_nearest_away_from_zero(const fqd_s& x)
    {
        return detail::_qd_impl::lround_nearest_away_from_zero(x);
    }

    BL_NO_INLINE long long llround_nearest_away_from_zero(const fqd_s& x)
    {
        return detail::_qd_impl::llround_nearest_away_from_zero(x);
    }

    // remainders
    BL_NO_INLINE fqd_s fmod(const fqd_s& x, const fqd_s& y)
    {
        return detail::_qd_impl::fmod(x, y);
    }

    BL_NO_INLINE fqd_s remquo(const fqd_s& x, const fqd_s& y, int* quo)
    { 
        return detail::_qd_impl::remquo(x, y, quo); 
    }

    // fractional decomposition
    BL_NO_INLINE fqd_s modf(const fqd_s& x, fqd_s* iptr) noexcept
    {
#if BL_FP_BARRIER_ACTIVE
        if (!detail::fp::isinf_or_nan(x.x0) && detail::fp::absd(x.x0) < 1.0)
        {
            if (iptr)
                *iptr = detail::_qd::signed_zero(bl::signbit(x));
            return x;
        }
#endif
        return detail::_qd_impl::modf(x, iptr);
    }

} // namespace bl::detail::_qd_runtime
