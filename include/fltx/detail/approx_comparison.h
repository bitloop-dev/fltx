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
    [[nodiscard]] BL_FORCE_INLINE constexpr bool almost_equal_impl(
        const Float& value,
        const Float& expected,
        const Float& relative_tolerance,
        const Float& absolute_tolerance) noexcept
    {
        const Float zero{ 0.0 };
        if (isnan(relative_tolerance) || isnan(absolute_tolerance) ||
            relative_tolerance < zero || absolute_tolerance < zero) [[unlikely]]
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
        // relative to the larger magnitude, and test the absolute limit with
        // subtraction, so no intermediate can exceed the finite range.
        if ((value < zero) != (expected < zero))
        {
            const Float smaller = magnitude_value < magnitude_expected
                ? magnitude_value
                : magnitude_expected;
            const bool within_absolute =
                isinf(absolute_tolerance) ||
                (absolute_tolerance >= scale &&
                 smaller <= absolute_tolerance - scale);
            const bool within_relative =
                relative_tolerance >= two ||
                (relative_tolerance >= one &&
                 smaller <= (relative_tolerance - one) * scale);
            return within_absolute || within_relative;
        }

        const Float difference = abs(value - expected);
        if (difference <= absolute_tolerance)
            return true;

        // Same-sign finite values differ by at most `scale`. Tolerances below
        // one cannot overflow when scaled; tolerances of one or more already
        // cover the complete same-sign interval.
        return relative_tolerance >= one ||
            difference <= relative_tolerance * scale;
    }
}

#endif
