/**
 * fltx/round_stream.h - Rounding helpers that mirror stream precision rules.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_ROUND_STREAM_INCLUDED
#define FLTX_ROUND_STREAM_INCLUDED

#include <ios>

#include "fltx/detail/format_flags.h"
#include "fltx/math.h"
#include "fltx/round_options.h"
#include "fltx/traits.h"

namespace bl
{
    // Interpret precision using iostream floatfield rules:
    // - fixed rounds to decimal places
    // - scientific rounds to precision + 1 significant figures
    // - defaultfloat rounds to significant figures
    // - hexfloat leaves the value unchanged
    template<fltx_float T>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto round_to(
        T value,
        int precision,
        std::ios_base::fmtflags flags)
    {
        using result_type = decltype(bl::round_to(value, precision, bl::significant_figures));

        switch (detail::float_format_from_flags(flags))
        {
        case detail::float_format::fixed:
            return result_type{ bl::round_to(value, precision, bl::decimals) };

        case detail::float_format::scientific:
            return result_type{ bl::round_to(value, precision + 1, bl::significant_figures) };

        case detail::float_format::hexfloat:
            return result_type{ value };

        case detail::float_format::defaultfloat:
        default:
            return result_type{ bl::round_to(value, precision, bl::significant_figures) };
        }
    }

} // namespace bl

#endif
