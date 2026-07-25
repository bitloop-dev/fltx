/**
 * fltx/src/fma_x86.cpp - Complete-operation x86 FMA backends.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/f128_math_basic.h"
#include "fltx/detail/f256_math_basic.h"

#if defined(FLTX_BUILD_X86_FMA_BACKEND)

static_assert(FLTX_HAS_X86_FMA, "the x86 FMA backend must be compiled with FMA enabled");

namespace bl::detail::_f128_runtime
{
    BL_NO_INLINE f128_s fma_x86(const f128_s& x, const f128_s& y, const f128_s& z)
    {
        return _f128_impl::fma(x, y, z);
    }
}

namespace bl::detail::_f256_runtime
{
    BL_NO_INLINE f256_s fma_x86(const f256_s& x, const f256_s& y, const f256_s& z)
    {
        return _f256_impl::fma(x, y, z);
    }
}

#endif
