/**
 * fltx/fqd_approx_comparison.h - Approximate comparison for qd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_FQD_APPROX_COMPARISON_INCLUDED
#define FLTX_FQD_APPROX_COMPARISON_INCLUDED

#include "fltx/fqd_arithmetic.h"
#include "fltx/fqd_classification.h"
#include "fltx/traits.h"
#include "fltx/detail/approx_comparison.h"

namespace bl
{
    inline constexpr fqd_s fqd_parity_tolerance{
        0x1p-188, 0.0, 0.0, 0.0
    };

    template<class Value, class Expected>
    requires (fltx_fqd<fltx_expression_value_t<Value>> &&
              fltx_fqd<fltx_expression_value_t<Expected>>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq(
        const Value& value,
        const Expected& expected,
        const fqd_s& relative_tolerance = fqd_parity_tolerance) noexcept
    {
        return detail::approx_eq_impl(
            static_cast<const fqd_s&>(value),
            static_cast<const fqd_s&>(expected),
            relative_tolerance);
    }
}

#endif
