/**
 * fltx/fdd_conversions.h - dd assignment conversion helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_CONVERSIONS_INCLUDED
#define FDD_CONVERSIONS_INCLUDED
#include "fltx/detail/fdd_conversions.h"

namespace bl {

BL_FORCE_INLINE constexpr fdd_s& fdd_s::operator=(uint64_t u) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::assign(*this, u),
        detail::_dd_runtime::assign(*this, u)
    );
}

BL_FORCE_INLINE constexpr fdd_s& fdd_s::operator=(int64_t v) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::assign(*this, v),
        detail::_dd_runtime::assign(*this, v)
    );
}

} // namespace bl

#endif
