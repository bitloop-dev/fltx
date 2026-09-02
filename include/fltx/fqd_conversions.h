/**
 * fltx/fqd_conversions.h - qd assignment conversion helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_CONVERSIONS_INCLUDED
#define FQD_CONVERSIONS_INCLUDED
#include "fltx/detail/fqd_conversions.h"

namespace bl {

BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator=(uint64_t u) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::assign(*this, u),
        detail::_qd_runtime::assign(*this, u)
    );
}

BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator=(int64_t v) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::assign(*this, v),
        detail::_qd_runtime::assign(*this, v)
    );
}

} // namespace bl

#endif
