/**
 * fltx/fdd_classification.h - Value utilities and classification predicates for dd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_CLASSIFICATION_INCLUDED
#define FDD_CLASSIFICATION_INCLUDED
#include "fltx/fdd_comparison.h"
#include "fltx/fdd_limits.h"

namespace bl {

namespace detail::_dd // primitives and kernels
{
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s mag(const fdd_s& a) noexcept
    {
        return detail::fp::signbit(a.hi) ? -a : a;
    }
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isnan(const fdd_s& x) noexcept
{
    return detail::fp::isnan(x.hi);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isinf(const fdd_s& x) noexcept
{
    return detail::fp::isinf(x.hi);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isfinite(const fdd_s& x) noexcept
{
    return detail::fp::isfinite(x.hi);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool iszero(const fdd_s& x) noexcept
{
    return x.hi == 0.0 && x.lo == 0.0;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool ispositive(const fdd_s& x) noexcept
{
    return !isnan(x) && !iszero(x) && !detail::fp::signbit(x.hi);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool signbit(const fdd_s& x) noexcept
{
    return detail::_dd::signbit(x.hi);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd abs(const fdd_s& a) noexcept
{
    const std::uint64_t sign = std::bit_cast<std::uint64_t>(a.hi) & (UINT64_C(1) << 63);
    return fdd_s{
        std::bit_cast<double>(std::bit_cast<std::uint64_t>(a.hi) ^ sign),
        std::bit_cast<double>(std::bit_cast<std::uint64_t>(a.lo) ^ sign)
    };
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd clamp(const fdd_s& v, const fdd_s& lo, const fdd_s& hi) noexcept
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

[[nodiscard]] BL_FORCE_INLINE constexpr int fpclassify(const fdd_s& x) noexcept
{
    if (isnan(x))  [[unlikely]] return FP_NAN;
    if (isinf(x))  [[unlikely]] return FP_INFINITE;
    if (iszero(x)) [[unlikely]] return FP_ZERO;

    return abs(x) < std::numeric_limits<fdd_s>::min() ? FP_SUBNORMAL : FP_NORMAL;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isnormal(const fdd_s& x) noexcept
{
    return fpclassify(x) == FP_NORMAL;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isunordered(const fdd_s& a, const fdd_s& b) noexcept
{
    return isnan(a) || isnan(b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isgreater(const fdd_s& a, const fdd_s& b) noexcept
{
    return !isunordered(a, b) && a > b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isgreaterequal(const fdd_s& a, const fdd_s& b) noexcept
{
    return !isunordered(a, b) && a >= b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool isless(const fdd_s& a, const fdd_s& b) noexcept
{
    return !isunordered(a, b) && a < b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool islessequal(const fdd_s& a, const fdd_s& b) noexcept
{
    return !isunordered(a, b) && a <= b;
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool islessgreater(const fdd_s& a, const fdd_s& b) noexcept
{
    return !isunordered(a, b) && a != b;
}

} // namespace bl

#endif
