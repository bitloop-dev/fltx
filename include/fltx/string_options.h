/**
 * fltx/string_options.h - Options for fltx string formatting.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_STRING_OPTIONS_INCLUDED
#define FLTX_STRING_OPTIONS_INCLUDED

namespace bl
{
    enum class trailing_zero_policy
    {
        standard,
        strip
    };

    struct precision_info
    {
        int digits = -1;
        int leading_digits = 0;
        int trailing_digits = 0;

        constexpr precision_info() noexcept = default;

        constexpr precision_info(int precision_digits) noexcept :
            digits(precision_digits)
        {
        }

        constexpr precision_info(int precision_digits, int leading, int trailing) noexcept :
            digits(precision_digits),
            leading_digits(leading),
            trailing_digits(trailing)
        {
        }
    };

} // namespace bl

#endif
