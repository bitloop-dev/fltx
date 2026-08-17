/**
 * fltx/detail/approx_comparison.h - Shared approximate-comparison mechanics.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_APPROX_COMPARISON_INCLUDED
#define FLTX_DETAIL_APPROX_COMPARISON_INCLUDED

namespace bl::detail
{
    template<class Float>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool approx_eq_impl(
        const Float& value,
        const Float& expected,
        const Float& relative_tolerance) noexcept
    {
        const Float zero{ 0.0 };
        if (isnan(relative_tolerance) || relative_tolerance < zero) [[unlikely]]
        {
            return false;
        }

        if (value == expected)
            return true;

        if (!isfinite(value) || !isfinite(expected)) [[unlikely]]
            return false;

        const Float magnitude_value = abs(value);
        const Float magnitude_expected = abs(expected);
        const Float scale = magnitude_value < magnitude_expected
            ? magnitude_expected
            : magnitude_value;
        const Float one{ 1.0 };
        const Float two{ 2.0 };

        // Opposite-sign values need an overflow-safe sum of magnitudes. Work
        // relative to the larger magnitude so no intermediate can exceed the
        // finite range.
        if ((value < zero) != (expected < zero))
        {
            const Float smaller = magnitude_value < magnitude_expected
                ? magnitude_value
                : magnitude_expected;
            return relative_tolerance >= two ||
                (relative_tolerance >= one &&
                 smaller <= (relative_tolerance - one) * scale);
        }

        const Float difference = abs(value - expected);
        // Same-sign finite values differ by at most `scale`. Tolerances below
        // one cannot overflow when scaled; tolerances of one or more already
        // cover the complete same-sign interval.
        return relative_tolerance >= one ||
            difference <= relative_tolerance * scale;
    }
}

#endif
