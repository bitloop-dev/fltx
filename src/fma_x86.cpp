/**
 * fltx/src/fma_x86.cpp - Complete-operation x86 FMA backends.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fdd_math_basic.h"
#include "fltx/detail/fqd_math_basic.h"

#if defined(FLTX_BUILD_X86_FMA_BACKEND)

static_assert(FLTX_HAS_X86_FMA, "the x86 FMA backend must be compiled with FMA enabled");

namespace bl::detail::_dd_runtime
{
    BL_NO_INLINE fdd_s fma_x86(const fdd_s& x, const fdd_s& y, const fdd_s& z)
    {
        return _dd_impl::fma(x, y, z);
    }
}

namespace bl::detail::_qd_runtime
{
    BL_NO_INLINE fqd_s fma_x86(const fqd_s& x, const fqd_s& y, const fqd_s& z)
    {
        return _qd_impl::fma(x, y, z);
    }
}

#endif
