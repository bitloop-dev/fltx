/**
 * fltx/fdd.cpp - Runtime dd conversion and rounding helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#include "fltx/detail/fdd_math_basic.h"

namespace bl::detail::_dd_runtime
{
    fdd_s trunc(const fdd_s& a)
    {
        return detail::_dd_impl::trunc(a);
    }

    fdd_s to_dd(uint64_t u) noexcept
    {
        return detail::_dd_impl::to_dd(u);
    }

    fdd_s to_dd(int64_t v) noexcept
    {
        return detail::_dd_impl::to_dd(v);
    }

    fdd_s& assign(fdd_s& out, uint64_t u) noexcept
    {
        return detail::_dd_impl::assign(out, u);
    }

    fdd_s& assign(fdd_s& out, int64_t v) noexcept
    {
        return detail::_dd_impl::assign(out, v);
    }

} // namespace bl::detail::_dd_runtime
