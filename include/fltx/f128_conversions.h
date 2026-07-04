/**
 * fltx/f128_conversions.h - f128 assignment conversion helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F128_CONVERSIONS_INCLUDED
#define F128_CONVERSIONS_INCLUDED
#include "fltx/detail/f128_conversions.h"

namespace bl {

BL_FORCE_INLINE constexpr f128_s& f128_s::operator=(uint64_t u) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::assign(*this, u),
        detail::_f128_runtime::assign(*this, u)
    );
}

BL_FORCE_INLINE constexpr f128_s& f128_s::operator=(int64_t v) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::assign(*this, v),
        detail::_f128_runtime::assign(*this, v)
    );
}

} // namespace bl

#endif
