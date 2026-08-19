/**
 * fltx/f256_arithmetic.h - Public arithmetic operators for f256.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F256_ARITHMETIC_INCLUDED
#define F256_ARITHMETIC_INCLUDED
#include "fltx/f256_conversions.h"
#include "fltx/detail/f256_arithmetic.h"

namespace bl {

// Dispatches full f256 addition to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(const f256_s& a, const f256_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::add_canonical_inline(a, b),
        detail::_f256_runtime::add_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::add_canonical_inline(a, b),
        detail::_f256_runtime::add_canonical(a, b)
    );
    #endif
}

// Dispatches full f256 subtraction to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(const f256_s& a, const f256_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::sub_canonical_inline(a, b),
        detail::_f256_runtime::sub_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::sub_canonical_inline(a, b),
        detail::_f256_runtime::sub_canonical(a, b)
    );
    #endif
}

// Dispatches full f256 multiplication to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(const f256_s& a, const f256_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH) && BL_FP_BARRIER_ACTIVE
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::mul_canonical_inline(a, b),
        detail::_f256::mul_fastmath_guarded(a, b)
    );
    #elif defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::mul_canonical_inline(a, b),
        detail::_f256_runtime::mul_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::mul_canonical_inline(a, b),
        detail::_f256_runtime::mul_canonical(a, b)
    );
    #endif
}

// Dispatches full f256 division to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(const f256_s& a, const f256_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::div_canonical_inline(a, b),
        detail::_f256_runtime::div_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::div_canonical_inline(a, b),
        detail::_f256_runtime::div_canonical(a, b)
    );
    #endif
}

// Dispatches f256-plus-double addition for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(const f256_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::add_double_canonical_inline(a, b),
        detail::_f256_runtime::add_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::add_double_canonical_inline(a, b),
        detail::_f256_runtime::add_double_canonical(a, b)
    );
    #endif
}

// Dispatches f256-minus-double subtraction for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(const f256_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::sub_double_canonical_inline(a, b),
        detail::_f256_runtime::sub_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::sub_double_canonical_inline(a, b),
        detail::_f256_runtime::sub_double_canonical(a, b)
    );
    #endif
}

// Dispatches f256-by-double multiplication for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(const f256_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::mul_double_canonical_inline(a, b),
        detail::_f256_runtime::mul_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::mul_double_canonical_inline(a, b),
        detail::_f256_runtime::mul_double_canonical(a, b)
    );
    #endif
}

// Dispatches f256-by-double division for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(const f256_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::div_double_canonical_inline(a, b),
        detail::_f256_runtime::div_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::div_double_canonical_inline(a, b),
        detail::_f256_runtime::div_double_canonical(a, b)
    );
    #endif
}

// Adds double to f256 through the commutative scalar overload.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(double a, const f256_s& b) noexcept { return b + a; }
// Subtracts f256 from double through the existing scalar subtraction path.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(double a, const f256_s& b) noexcept { return -(b - a); }
// Multiplies double by f256 through the commutative scalar overload.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(double a, const f256_s& b) noexcept { return b * a; }
// Divides double by f256 through the full-value division overload.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(double a, const f256_s& b) noexcept { return f256_s{ a } / b; }

// Promotes float and adds it to f256.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(const f256_s& a, float b) noexcept { return a + (double)b; }
// Promotes float and subtracts it from f256.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(const f256_s& a, float b) noexcept { return a - (double)b; }
// Promotes float and multiplies it by f256.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(const f256_s& a, float b) noexcept { return a * (double)b; }
// Promotes float and divides f256 by it.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(const f256_s& a, float b) noexcept { return a / (double)b; }

// Promotes float and adds f256 to it.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(float a, const f256_s& b) noexcept { return (double)a + b; }
// Promotes float and subtracts f256 from it.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(float a, const f256_s& b) noexcept { return (double)a - b; }
// Promotes float and multiplies it by f256.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(float a, const f256_s& b) noexcept { return (double)a * b; }
// Promotes float and divides it by f256.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(float a, const f256_s& b) noexcept { return (double)a / b; }

// Squares an f256 storage value with canonical multiplication semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s sqr(const f256_s& a) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::mul_canonical_inline(a, a),
        detail::_f256_runtime::mul_canonical(a, a)
    );
}

// Squares an f256 wrapper value with canonical multiplication semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr f256 sqr(const f256& a) noexcept
{
    return f256{ sqr(static_cast<const f256_s&>(a)) };
}

// Adds an integer to f256, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(const f256_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a + static_cast<double>(b);

    return detail::_f256::add_dd(a, detail::_f256::integer_to_double_double(b));
}

// Subtracts an integer from f256, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(const f256_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a - static_cast<double>(b);

    return detail::_f256::sub_dd(a, detail::_f256::integer_to_double_double(b));
}

// Multiplies f256 by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(const f256_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a * static_cast<double>(b);

    return detail::_f256::mul_dd(a, detail::_f256::integer_to_double_double(b));
}

// Divides f256 by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(const f256_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a / static_cast<double>(b);

    return detail::_f256::div_dd(a, detail::_f256::integer_to_double_double(b));
}

// Adds f256 to an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator+(T a, const f256_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) + b;

    return detail::_f256::add_dd(b, detail::_f256::integer_to_double_double(a));
}

// Subtracts f256 from an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator-(T a, const f256_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) - b;

    return detail::_f256::sub_dd(detail::_f256::integer_to_double_double(a), b);
}

// Multiplies an integer by f256, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator*(T a, const f256_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) * b;

    return detail::_f256::mul_dd(b, detail::_f256::integer_to_double_double(a));
}

// Divides an integer by f256, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s operator/(T a, const f256_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) / b;

    return detail::_f256::div_dd(detail::_f256::integer_to_double_double(a), b);
}

} // namespace bl

#endif
