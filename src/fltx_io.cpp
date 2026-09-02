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
        const fdd_s& value,
        precision_info precision,
        std::ios_base::fmtflags flags)
    {
        return detail::to_string_impl<detail::_dd::dd_io_traits>(value, precision, flags);
    }

    [[nodiscard]] BL_NO_INLINE std::string to_string(
        const fqd_s& value,
        precision_info precision,
        std::ios_base::fmtflags flags)
    {
        return detail::to_string_impl<detail::_qd::qd_io_traits>(value, precision, flags);
    }

    namespace detail::charconv
    {
        [[nodiscard]] BL_NO_INLINE parse_result<fdd_s> parse_runtime_dd_s(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<fdd_s>(text, fmt);
        }

        [[nodiscard]] BL_NO_INLINE parse_result<fdd> parse_runtime_dd(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<fdd>(text, fmt);
        }

        [[nodiscard]] BL_NO_INLINE parse_result<fqd_s> parse_runtime_qd_s(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<fqd_s>(text, fmt);
        }

        [[nodiscard]] BL_NO_INLINE parse_result<fqd> parse_runtime_qd(
            std::string_view text,
            std::chars_format fmt) noexcept
        {
            return parse_runtime<fqd>(text, fmt);
        }

    } // namespace detail::charconv

} // namespace bl
