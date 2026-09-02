/**
 * fltx/fqd_stream.h - Stream formatting for qd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_STREAM_INCLUDED
#define FQD_STREAM_INCLUDED

#include "fltx/detail/common_stream.h"
#include "fltx/fqd_string.h"

namespace bl
{
    inline std::ostream& operator<<(std::ostream& os, const fqd_s& x)
    {
        return detail::write_to_stream<detail::_qd::qd_io_traits>(os, x);
    }

    inline std::istream& operator>>(std::istream& is, fqd_s& x)
    {
        return detail::read_from_stream(is, x, detail::_qd::parse);
    }

    inline std::istream& operator>>(std::istream& is, fqd& x)
    {
        return detail::read_from_stream(is, static_cast<fqd_s&>(x), detail::_qd::parse);
    }

} // namespace bl

#endif
