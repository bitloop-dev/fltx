/**
 * fltx/detail/fqd_conversions.h - qd conversion primitives and implementation.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_DETAIL_CONVERSIONS_INCLUDED
#define FQD_DETAIL_CONVERSIONS_INCLUDED
#include "fltx/fqd_type.h"

namespace bl {

namespace detail::_qd_impl
{
    using namespace detail::_qd;

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s to_qd(uint64_t u) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s to_qd(int64_t v) noexcept;

    BL_FORCE_INLINE constexpr fqd_s& assign(fqd_s& out, uint64_t u) noexcept;
    BL_FORCE_INLINE constexpr fqd_s& assign(fqd_s& out, int64_t v) noexcept;

} // namespace detail::_qd_impl

namespace detail::_qd_runtime
{
    BL_NO_INLINE fqd_s to_qd(uint64_t u) noexcept;
    BL_NO_INLINE fqd_s to_qd(int64_t v) noexcept;
    BL_NO_INLINE fqd_s& assign(fqd_s& out, uint64_t u) noexcept;
    BL_NO_INLINE fqd_s& assign(fqd_s& out, int64_t v) noexcept;

} // namespace detail::_qd_runtime

namespace detail::_qd // primitives and kernels
{
    [[nodiscard]] BL_FORCE_INLINE constexpr dd_scalar uint64_to_double_double(uint64_t value) noexcept
    {
        double hi{}, lo{};
        uint64_to_exact_double_pair(value, hi, lo);
        return { hi, lo };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr dd_scalar int64_to_double_double(int64_t value) noexcept
    {
        double hi{}, lo{};
        int64_to_exact_double_pair(value, hi, lo);
        return { hi, lo };
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr dd_scalar integer_to_double_double(T value) noexcept
    {
        if constexpr (std::is_signed_v<std::remove_cv_t<T>>)
            return int64_to_double_double(static_cast<int64_t>(value));
        else
            return uint64_to_double_double(static_cast<uint64_t>(value));
    }

} // namespace detail::_qd

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s detail::_qd_impl::to_qd(uint64_t u) noexcept
{
    const auto value = detail::_qd::uint64_to_double_double(u);
    return fqd_s{ value.hi, value.lo };
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s detail::_qd_impl::to_qd(int64_t v) noexcept
{
    const auto value = detail::_qd::int64_to_double_double(v);
    return fqd_s{ value.hi, value.lo };
}

BL_FORCE_INLINE constexpr fqd_s& detail::_qd_impl::assign(fqd_s& out, uint64_t u) noexcept
{
    out = to_qd(u);
    return out;
}

BL_FORCE_INLINE constexpr fqd_s& detail::_qd_impl::assign(fqd_s& out, int64_t v) noexcept
{
    out = to_qd(v);
    return out;
}

namespace detail::_qd // primitives and kernels
{
    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s integer_to_qd(T value) noexcept
    {
        const auto expanded = integer_to_double_double(value);
        return fqd_s{ expanded.hi, expanded.lo };
    }

} // namespace detail::_qd

} // namespace bl

#endif
