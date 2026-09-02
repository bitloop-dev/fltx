/**
 * fltx/fdd_arithmetic.h - Public arithmetic operators for dd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_ARITHMETIC_INCLUDED
#define FDD_ARITHMETIC_INCLUDED
#include "fltx/detail/fdd_arithmetic.h"

namespace bl {

BL_PUSH_PRECISE;
// Dispatches full dd addition to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, const fdd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::add_canonical_inline(a, b),
        detail::_dd::add_finite_inline(a, b)
    );
    #else
    return detail::_dd::add_canonical_inline(a, b);
    #endif
}

// Dispatches full dd subtraction to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, const fdd_s& b) noexcept
{
    BL_FP_IF_CONSTEVAL_WARNING_PUSH
    BL_FP_IF_CONSTEVAL
    {
        if (detail::fp::isinf_or_nan(a.hi) || detail::fp::isinf_or_nan(b.hi)) [[unlikely]]
            return detail::_dd::sub_special(a, b);
    }
    BL_FP_IF_CONSTEVAL_WARNING_POP

    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::sub_canonical_inline(a, b),
        detail::_dd::sub_finite_inline(a, b)
    );
    #else
    return detail::_dd::sub_canonical_inline(a, b);
    #endif
}
BL_POP_PRECISE;

// Dispatches full dd multiplication to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, const fdd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::mul_canonical_inline(a, b),
        detail::_dd::mul_finite_inline(a, b)
    );
    #else
    return detail::_dd::mul_canonical_inline(a, b);
    #endif
}

// Dispatches full dd division to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, const fdd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::div_canonical_inline(a, b),
        detail::_dd::div_finite_inline(a, b)
    );
    #else
    return detail::_dd::div_canonical_inline(a, b);
    #endif
}

// Dispatches dd-plus-double addition for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::add_double_canonical_inline(a, b),
        detail::_dd::add_double_finite_inline(a, b)
    );
    #else
    return detail::_dd::add_double_canonical_inline(a, b);
    #endif
}

// Dispatches dd-minus-double subtraction for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::sub_double_canonical_inline(a, b),
        detail::_dd::sub_double_finite_inline(a, b)
    );
    #else
    return detail::_dd::sub_double_canonical_inline(a, b);
    #endif
}

// Dispatches dd-by-double multiplication for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::mul_double_canonical_inline(a, b),
        detail::_dd::mul_double_finite_inline(a, b)
    );
    #else
    return detail::_dd::mul_double_canonical_inline(a, b);
    #endif
}

// Dispatches dd-by-double division for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd::div_double_canonical_inline(a, b),
        detail::_dd::div_double_finite_inline(a, b)
    );
    #else
    return detail::_dd::div_double_canonical_inline(a, b);
    #endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(double a, const fdd_s& b) noexcept { return b + a; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(double a, const fdd_s& b) noexcept { return -(b - a); }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(double a, const fdd_s& b) noexcept { return b * a; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(double a, const fdd_s& b) noexcept { return fdd_s{ a } / b; }

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, float b) noexcept { return a + (double)b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, float b) noexcept { return a - (double)b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, float b) noexcept { return a * (double)b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, float b) noexcept { return a / (double)b; }

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(float a, const fdd_s& b) noexcept { return (double)a + b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(float a, const fdd_s& b) noexcept { return (double)a - b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(float a, const fdd_s& b) noexcept { return (double)a * b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(float a, const fdd_s& b) noexcept { return (double)a / b; }

// Squares with canonical multiplication semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s sqr(const fdd_s& a) noexcept
{
    return detail::_dd::mul_canonical_inline(a, a);
}

// Preserves the value form for callers using fdd.
[[nodiscard]] BL_FORCE_INLINE constexpr fdd sqr(const fdd& a) noexcept
{
    return fdd{ detail::_dd::mul_canonical_inline(a, a) };
}

// Adds an integer to dd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a + static_cast<double>(b);

    return a + detail::_dd::integer_to_dd(b);
}

// Subtracts an integer from dd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a - static_cast<double>(b);

    return a - detail::_dd::integer_to_dd(b);
}

// Multiplies dd by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a * static_cast<double>(b);

    return a * detail::_dd::integer_to_dd(b);
}

// Divides dd by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a / static_cast<double>(b);

    return a / detail::_dd::integer_to_dd(b);
}

// Adds dd to an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(T a, const fdd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) + b;

    return detail::_dd::integer_to_dd(a) + b;
}

// Subtracts dd from an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(T a, const fdd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) - b;

    return detail::_dd::integer_to_dd(a) - b;
}

// Multiplies an integer by dd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(T a, const fdd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) * b;

    return detail::_dd::integer_to_dd(a) * b;
}

// Divides an integer by dd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(T a, const fdd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) / b;

    return detail::_dd::integer_to_dd(a) / b;
}

} // namespace bl

#endif
