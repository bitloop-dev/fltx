/**
 * fltx/f256_conversions.h - f256 assignment conversion helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F256_CONVERSIONS_INCLUDED
#define F256_CONVERSIONS_INCLUDED
#include "fltx/detail/f256_conversions.h"

namespace bl {

BL_FORCE_INLINE constexpr f256_s& f256_s::operator=(uint64_t u) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::assign(*this, u),
        detail::_f256_runtime::assign(*this, u)
    );
}

BL_FORCE_INLINE constexpr f256_s& f256_s::operator=(int64_t v) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::assign(*this, v),
        detail::_f256_runtime::assign(*this, v)
    );
}

} // namespace bl

#endif
