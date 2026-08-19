/**
 * fltx/fltx_io.cpp - Runtime IO entry points.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/charconv.h"
#include "fltx/string.h"

namespace bl
{
    [[nodiscard]] BL_NO_INLINE std::string to_string(
        const f128_s& value,
        precision_info precision,
        std::ios_base::fmtflags flags)
    {
        return detail::to_string_impl<detail::_f128::f128_io_traits>(value, precision, flags);
    }

    [[nodiscard]] BL_NO_INLINE std::string to_string(
        const f256_s& value,
        precision_info precision,
        std::ios_base::fmtflags flags)
    {
        return detail::to_string_impl<detail::_f256::f256_io_traits>(value, precision, flags);
    }

    namespace detail::charconv
    {
        [[nodiscard]] BL_NO_INLINE parse_result<f128_s> parse_runtime_f128_s(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<f128_s>(text, fmt);
        }

        [[nodiscard]] BL_NO_INLINE parse_result<f128> parse_runtime_f128(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<f128>(text, fmt);
        }

        [[nodiscard]] BL_NO_INLINE parse_result<f256_s> parse_runtime_f256_s(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<f256_s>(text, fmt);
        }

        [[nodiscard]] BL_NO_INLINE parse_result<f256> parse_runtime_f256(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<f256>(text, fmt);
        }

    } // namespace detail::charconv

} // namespace bl
