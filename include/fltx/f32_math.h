/**
 * fltx/f32_math.h - constexpr <cmath>-style functions for f32.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F32_MATH_INCLUDED
#define F32_MATH_INCLUDED

#include "fltx/f64_math.h"
#include "fltx/detail/f32_math_basic.h"
#include "fltx/detail/f32_math_transcendental.h"

namespace bl
{
    template<class To>
    requires detail::math::native_nexttoward_target<To>
    [[nodiscard]] BL_FORCE_INLINE constexpr float nexttoward(float from, To to) noexcept
    {
        return bl::nexttoward(from, static_cast<long double>(to));
    }

} // namespace bl

#endif
