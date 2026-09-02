/**
 * fltx/detail/fdd_conversions.h - dd conversion primitives and implementation.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_DETAIL_CONVERSIONS_INCLUDED
#define FDD_DETAIL_CONVERSIONS_INCLUDED
#include "fltx/fdd_type.h"

namespace bl {

namespace detail::_dd // primitives and kernels
{
    BL_FORCE_INLINE constexpr fdd_s renorm(double hi, double lo)
    {
        double s{}, e{};
        two_sum_precise(hi, lo, s, e);
        return { s, e };
    }

    BL_FORCE_INLINE constexpr fdd_s uint64_to_dd(uint64_t value) noexcept
    {
        double sum{}, err{};
        uint64_to_exact_double_pair(value, sum, err);
        return renorm(sum, err);
    }

    BL_FORCE_INLINE constexpr fdd_s int64_to_dd(int64_t value) noexcept
    {
        double sum{}, err{};
        int64_to_exact_double_pair(value, sum, err);
        return renorm(sum, err);
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s integer_to_dd(T value) noexcept
    {
        if constexpr (std::is_signed_v<std::remove_cv_t<T>>)
            return int64_to_dd(static_cast<int64_t>(value));
        else
            return uint64_to_dd(static_cast<uint64_t>(value));
    }

} // namespace detail::_dd

namespace detail::_dd_impl
{
    using namespace detail::_dd;

    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s to_dd(uint64_t u) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s to_dd(int64_t v) noexcept;

    BL_FORCE_INLINE constexpr fdd_s& assign(fdd_s& out, uint64_t u) noexcept;
    BL_FORCE_INLINE constexpr fdd_s& assign(fdd_s& out, int64_t v) noexcept;

} // namespace detail::_dd_impl

namespace detail::_dd_runtime
{
    BL_NO_INLINE fdd_s to_dd(uint64_t u) noexcept;
    BL_NO_INLINE fdd_s to_dd(int64_t v) noexcept;
    BL_NO_INLINE fdd_s& assign(fdd_s& out, uint64_t u) noexcept;
    BL_NO_INLINE fdd_s& assign(fdd_s& out, int64_t v) noexcept;

} // namespace detail::_dd_runtime

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s detail::_dd_impl::to_dd(uint64_t u) noexcept
{
    return detail::_dd::uint64_to_dd(u);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s detail::_dd_impl::to_dd(int64_t v) noexcept
{
    return detail::_dd::int64_to_dd(v);
}

BL_FORCE_INLINE constexpr fdd_s& detail::_dd_impl::assign(fdd_s& out, uint64_t u) noexcept
{
    out = to_dd(u);
    return out;
}

BL_FORCE_INLINE constexpr fdd_s& detail::_dd_impl::assign(fdd_s& out, int64_t v) noexcept
{
    out = to_dd(v);
    return out;
}

} // namespace bl

#endif
