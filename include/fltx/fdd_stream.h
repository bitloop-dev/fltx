/**
 * fltx/fdd_stream.h - Stream formatting for dd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_STREAM_INCLUDED
#define FDD_STREAM_INCLUDED

#include "fltx/detail/common_stream.h"
#include "fltx/fdd_string.h"

namespace bl
{
    inline std::ostream& operator<<(std::ostream& os, const fdd_s& x)
    {
        return detail::write_to_stream<detail::_dd::dd_io_traits>(os, x);
    }

    inline std::istream& operator>>(std::istream& is, fdd_s& x)
    {
        return detail::read_from_stream(is, x, detail::_dd::parse);
    }

    inline std::istream& operator>>(std::istream& is, fdd& x)
    {
        return detail::read_from_stream(is, static_cast<fdd_s&>(x), detail::_dd::parse);
    }

} // namespace bl

#endif
