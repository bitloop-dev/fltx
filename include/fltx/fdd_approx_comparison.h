/**
 * fltx/fdd_approx_comparison.h - Approximate comparison for dd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_FDD_APPROX_COMPARISON_INCLUDED
#define FLTX_FDD_APPROX_COMPARISON_INCLUDED

#include "fltx/fdd_arithmetic.h"
#include "fltx/fdd_classification.h"
#include "fltx/traits.h"
#include "fltx/detail/approx_comparison.h"

namespace bl
{
    inline constexpr fdd_s fdd_parity_tolerance{ 0x1p-80, 0.0 };

    template<class Value, class Expected>
    requires (fltx_fdd<Value> && fltx_fdd<Expected>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq(
        const Value& value,
        const Expected& expected,
        const fdd_s& relative_tolerance = fdd_parity_tolerance) noexcept
    {
        return detail::approx_eq_impl(
            static_cast<const fdd_s&>(value),
            static_cast<const fdd_s&>(expected),
            relative_tolerance);
    }
}

#endif
