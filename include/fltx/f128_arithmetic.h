/**
 * fltx/f128_arithmetic.h - Public arithmetic operators for f128.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F128_ARITHMETIC_INCLUDED
#define F128_ARITHMETIC_INCLUDED
#include "fltx/detail/f128_arithmetic.h"

namespace bl {

BL_PUSH_PRECISE;
// Dispatches full f128 addition to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(const f128_s& a, const f128_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::add_canonical_inline(a, b),
        detail::_f128::add_finite_inline(a, b)
    );
    #else
    return detail::_f128::add_canonical_inline(a, b);
    #endif
}

// Dispatches full f128 subtraction to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(const f128_s& a, const f128_s& b) noexcept
{
    BL_FP_IF_CONSTEVAL_WARNING_PUSH
    BL_FP_IF_CONSTEVAL
    {
        if (detail::fp::isinf_or_nan(a.hi) || detail::fp::isinf_or_nan(b.hi)) [[unlikely]]
            return detail::_f128::sub_special(a, b);
    }
    BL_FP_IF_CONSTEVAL_WARNING_POP

    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::sub_canonical_inline(a, b),
        detail::_f128::sub_finite_inline(a, b)
    );
    #else
    return detail::_f128::sub_canonical_inline(a, b);
    #endif
}
BL_POP_PRECISE;

// Dispatches full f128 multiplication to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(const f128_s& a, const f128_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::mul_canonical_inline(a, b),
        detail::_f128::mul_finite_inline(a, b)
    );
    #else
    return detail::_f128::mul_canonical_inline(a, b);
    #endif
}

// Dispatches full f128 division to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(const f128_s& a, const f128_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::div_canonical_inline(a, b),
        detail::_f128::div_finite_inline(a, b)
    );
    #else
    return detail::_f128::div_canonical_inline(a, b);
    #endif
}

// Dispatches f128-plus-double addition for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(const f128_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::add_double_canonical_inline(a, b),
        detail::_f128::add_double_finite_inline(a, b)
    );
    #else
    return detail::_f128::add_double_canonical_inline(a, b);
    #endif
}

// Dispatches f128-minus-double subtraction for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(const f128_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::sub_double_canonical_inline(a, b),
        detail::_f128::sub_double_finite_inline(a, b)
    );
    #else
    return detail::_f128::sub_double_canonical_inline(a, b);
    #endif
}

// Dispatches f128-by-double multiplication for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(const f128_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::mul_double_canonical_inline(a, b),
        detail::_f128::mul_double_finite_inline(a, b)
    );
    #else
    return detail::_f128::mul_double_canonical_inline(a, b);
    #endif
}

// Dispatches f128-by-double division for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(const f128_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::div_double_canonical_inline(a, b),
        detail::_f128::div_double_finite_inline(a, b)
    );
    #else
    return detail::_f128::div_double_canonical_inline(a, b);
    #endif
}

// Adds double to f128 through the commutative scalar overload.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(double a, const f128_s& b) noexcept { return b + a; }
// Subtracts f128 from double through the existing scalar subtraction path.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(double a, const f128_s& b) noexcept { return -(b - a); }
// Multiplies double by f128 through the commutative scalar overload.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(double a, const f128_s& b) noexcept { return b * a; }
// Divides double by f128 through the full-value division overload.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(double a, const f128_s& b) noexcept { return f128_s{ a } / b; }

// Promotes float and adds it to f128.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(const f128_s& a, float b) noexcept { return a + (double)b; }
// Promotes float and subtracts it from f128.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(const f128_s& a, float b) noexcept { return a - (double)b; }
// Promotes float and multiplies it by f128.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(const f128_s& a, float b) noexcept { return a * (double)b; }
// Promotes float and divides f128 by it.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(const f128_s& a, float b) noexcept { return a / (double)b; }

// Promotes float and adds f128 to it.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(float a, const f128_s& b) noexcept { return (double)a + b; }
// Promotes float and subtracts f128 from it.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(float a, const f128_s& b) noexcept { return (double)a - b; }
// Promotes float and multiplies it by f128.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(float a, const f128_s& b) noexcept { return (double)a * b; }
// Promotes float and divides it by f128.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(float a, const f128_s& b) noexcept { return (double)a / b; }

// Squares an f128 storage value with canonical multiplication semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s sqr(const f128_s& a) noexcept
{
    return detail::_f128::mul_canonical_inline(a, a);
}

// Squares an f128 wrapper value with canonical multiplication semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f128 sqr(const f128& a) noexcept
{
    return f128{ detail::_f128::mul_canonical_inline(a, a) };
}

// Adds an integer to f128, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(const f128_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a + static_cast<double>(b);

    return a + detail::_f128::integer_to_f128(b);
}

// Subtracts an integer from f128, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(const f128_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a - static_cast<double>(b);

    return a - detail::_f128::integer_to_f128(b);
}

// Multiplies f128 by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(const f128_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a * static_cast<double>(b);

    return a * detail::_f128::integer_to_f128(b);
}

// Divides f128 by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(const f128_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a / static_cast<double>(b);

    return a / detail::_f128::integer_to_f128(b);
}

// Adds f128 to an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator+(T a, const f128_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) + b;

    return detail::_f128::integer_to_f128(a) + b;
}

// Subtracts f128 from an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator-(T a, const f128_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) - b;

    return detail::_f128::integer_to_f128(a) - b;
}

// Multiplies an integer by f128, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator*(T a, const f128_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) * b;

    return detail::_f128::integer_to_f128(a) * b;
}

// Divides an integer by f128, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s operator/(T a, const f128_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) / b;

    return detail::_f128::integer_to_f128(a) / b;
}

} // namespace bl

#endif
