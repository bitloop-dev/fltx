/**
 * fltx/fqd_classification.h - Value utilities and classification predicates for qd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_CLASSIFICATION_INCLUDED
#define FQD_CLASSIFICATION_INCLUDED
#include "fltx/fqd_comparison.h"
#include "fltx/fqd_limits.h"

namespace bl {

namespace detail::_qd // primitives and kernels
{
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mag(const fqd_s& a) noexcept
    {
        return detail::fp::signbit(a.x0) ? -a : a;
    }
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isnan(const fqd_s& a) noexcept
{
    return detail::_qd::isnan(a.x0);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isinf(const fqd_s& a) noexcept
{
    return detail::_qd::isinf(a.x0);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isfinite(const fqd_s& x) noexcept
{
    return detail::_qd::isfinite(x.x0);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool iszero(const fqd_s& a) noexcept
{
    return a.x0 == 0 && a.x1 == 0 && a.x2 == 0 && a.x3 == 0;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool ispositive(const fqd_s& x) noexcept
{
    return !isnan(x) && !iszero(x) && !detail::fp::signbit(x.x0);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool signbit(const fqd_s& x) noexcept
{
    return detail::_qd::signbit(x.x0);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd abs(const fqd_s& a) noexcept
{
    if (a.x0 < 0.0)
        return -a;
    return (a.x0 == 0.0 && signbit(a)) ? -a : a;
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd clamp(const fqd_s& v, const fqd_s& lo, const fqd_s& hi) noexcept
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

[[nodiscard]] BL_FORCE_INLINE constexpr int fpclassify(const fqd_s& x) noexcept
{
    if (isnan(x))  [[unlikely]] return FP_NAN;
    if (isinf(x))  [[unlikely]] return FP_INFINITE;
    if (iszero(x)) [[unlikely]] return FP_ZERO;

    return abs(x) < std::numeric_limits<fqd_s>::min() ? FP_SUBNORMAL : FP_NORMAL;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isnormal(const fqd_s& x) noexcept
{
    return fpclassify(x) == FP_NORMAL;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isunordered(const fqd_s& a, const fqd_s& b) noexcept
{
    return isnan(a) || isnan(b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isgreater(const fqd_s& a, const fqd_s& b) noexcept
{
    return !isunordered(a, b) && a > b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isgreaterequal(const fqd_s& a, const fqd_s& b) noexcept
{
    return !isunordered(a, b) && a >= b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isless(const fqd_s& a, const fqd_s& b) noexcept
{
    return !isunordered(a, b) && a < b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool islessequal(const fqd_s& a, const fqd_s& b) noexcept
{
    return !isunordered(a, b) && a <= b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool islessgreater(const fqd_s& a, const fqd_s& b) noexcept
{
    return !isunordered(a, b) && a != b;
}

} // namespace bl

#endif
