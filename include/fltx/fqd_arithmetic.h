/**
 * fltx/fqd_arithmetic.h - Public arithmetic operators for qd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_ARITHMETIC_INCLUDED
#define FQD_ARITHMETIC_INCLUDED
#include "fltx/fqd_conversions.h"
#include "fltx/detail/fqd_arithmetic.h"

namespace bl {

// Dispatches full qd addition to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, const fqd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::add_canonical_inline(a, b),
        detail::_qd_runtime::add_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::add_canonical_inline(a, b),
        detail::_qd_runtime::add_canonical(a, b)
    );
    #endif
}

// Dispatches full qd subtraction to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, const fqd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::sub_canonical_inline(a, b),
        detail::_qd_runtime::sub_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::sub_canonical_inline(a, b),
        detail::_qd_runtime::sub_canonical(a, b)
    );
    #endif
}

// Dispatches full qd multiplication to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, const fqd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH) && BL_FP_BARRIER_ACTIVE
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::mul_canonical_inline(a, b),
        detail::_qd::mul_fastmath_guarded(a, b)
    );
    #elif defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::mul_canonical_inline(a, b),
        detail::_qd_runtime::mul_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::mul_canonical_inline(a, b),
        detail::_qd_runtime::mul_canonical(a, b)
    );
    #endif
}

// Dispatches full qd division to canonical or relaxed finite semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, const fqd_s& b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::div_canonical_inline(a, b),
        detail::_qd_runtime::div_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::div_canonical_inline(a, b),
        detail::_qd_runtime::div_canonical(a, b)
    );
    #endif
}

// Dispatches qd-plus-double addition for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::add_double_canonical_inline(a, b),
        detail::_qd_runtime::add_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::add_double_canonical_inline(a, b),
        detail::_qd_runtime::add_double_canonical(a, b)
    );
    #endif
}

// Dispatches qd-minus-double subtraction for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::sub_double_canonical_inline(a, b),
        detail::_qd_runtime::sub_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::sub_double_canonical_inline(a, b),
        detail::_qd_runtime::sub_double_canonical(a, b)
    );
    #endif
}

// Dispatches qd-by-double multiplication for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::mul_double_canonical_inline(a, b),
        detail::_qd_runtime::mul_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::mul_double_canonical_inline(a, b),
        detail::_qd_runtime::mul_double_canonical(a, b)
    );
    #endif
}

// Dispatches qd-by-double division for the active arithmetic semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, double b) noexcept
{
    #if defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::div_double_canonical_inline(a, b),
        detail::_qd_runtime::div_double_finite(a, b)
    );
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::div_double_canonical_inline(a, b),
        detail::_qd_runtime::div_double_canonical(a, b)
    );
    #endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(double a, const fqd_s& b) noexcept { return b + a; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(double a, const fqd_s& b) noexcept { return -(b - a); }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(double a, const fqd_s& b) noexcept { return b * a; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(double a, const fqd_s& b) noexcept { return fqd_s{ a } / b; }

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, float b) noexcept { return a + (double)b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, float b) noexcept { return a - (double)b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, float b) noexcept { return a * (double)b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, float b) noexcept { return a / (double)b; }

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(float a, const fqd_s& b) noexcept { return (double)a + b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(float a, const fqd_s& b) noexcept { return (double)a - b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(float a, const fqd_s& b) noexcept { return (double)a * b; }
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(float a, const fqd_s& b) noexcept { return (double)a / b; }

// Squares with canonical multiplication semantics.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sqr(const fqd_s& a) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd::mul_canonical_inline(a, a),
        detail::_qd_runtime::mul_canonical(a, a)
    );
}

// Preserves the value form for expression-aware callers using fqd.
[[nodiscard]] BL_FORCE_INLINE constexpr fqd sqr(const fqd& a) noexcept
{
    return fqd{ sqr(static_cast<const fqd_s&>(a)) };
}

// Adds an integer to qd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a + static_cast<double>(b);

    return detail::_qd::add_dd(a, detail::_qd::integer_to_double_double(b));
}

// Subtracts an integer from qd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a - static_cast<double>(b);

    return detail::_qd::sub_dd(a, detail::_qd::integer_to_double_double(b));
}

// Multiplies qd by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a * static_cast<double>(b);

    return detail::_qd::mul_dd(a, detail::_qd::integer_to_double_double(b));
}

// Divides qd by an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, T b) noexcept
{
    if (detail::fp::integer_fits_exact_double(b))
        return a / static_cast<double>(b);

    return detail::_qd::div_dd(a, detail::_qd::integer_to_double_double(b));
}

// Adds qd to an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(T a, const fqd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) + b;

    return detail::_qd::add_dd(b, detail::_qd::integer_to_double_double(a));
}

// Subtracts qd from an integer, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(T a, const fqd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) - b;

    return detail::_qd::sub_dd(detail::_qd::integer_to_double_double(a), b);
}

// Multiplies an integer by qd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(T a, const fqd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) * b;

    return detail::_qd::mul_dd(b, detail::_qd::integer_to_double_double(a));
}

// Divides an integer by qd, preserving integers wider than binary64 exactly.
template<class T, std::enable_if_t<detail::fp::is_integer_scalar_v<T>, int> = 0>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(T a, const fqd_s& b) noexcept
{
    if (detail::fp::integer_fits_exact_double(a))
        return static_cast<double>(a) / b;

    return detail::_qd::div_dd(detail::_qd::integer_to_double_double(a), b);
}

namespace detail::_qd
{
    // Operands paired eagerly with fqd; leave expression-capable pairs to the frontend.
    template<class T>
    concept eager_operand = std::is_same_v<T, fqd_s> ||
                            std::is_same_v<T, fdd> || std::is_same_v<T, fdd_s>
    #if !defined(FLTX_ENABLE_FQD_EXPRESSIONS) || !FLTX_ENABLE_FQD_EXPRESSIONS
                         || std::is_same_v<T, fqd> || std::is_same_v<T, float> ||
                            std::is_same_v<T, double> || detail::fp::is_integer_scalar_v<T>
    #endif
                         ;

    template<class L, class R>
    concept eager_pair = ((std::is_same_v<L, fqd> || detail::_qd_expr::is_expr<L>::value) && eager_operand<R>) ||
                         ((std::is_same_v<R, fqd> || detail::_qd_expr::is_expr<R>::value) && eager_operand<L>) ||
                         (std::is_same_v<L, fdd> && std::is_same_v<R, fqd_s>) ||
                         (std::is_same_v<R, fdd> && std::is_same_v<L, fqd_s>);

    // Strip only the scalar wrapper; retain native operands for their existing
    // specialized arithmetic, including exact wide-integer handling.
    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr decltype(auto) eager_value(const T& value) noexcept
    {
        if constexpr (detail::_qd_expr::is_expr<T>::value) return detail::_qd_expr::eval_to_qd_s(value);
        else if constexpr (std::is_same_v<T, fqd>) return static_cast<const fqd_s&>(value);
        else if constexpr (std::is_same_v<T, fdd>) return static_cast<const fdd_s&>(value);
        else return (value);
    }
}

template<class L, class R> requires detail::_qd::eager_pair<L, R>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd operator+(const L& a, const R& b) noexcept
{
    return detail::_qd::eager_value(a) + detail::_qd::eager_value(b);
}

template<class L, class R> requires detail::_qd::eager_pair<L, R>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd operator-(const L& a, const R& b) noexcept
{
    return detail::_qd::eager_value(a) - detail::_qd::eager_value(b);
}

template<class L, class R> requires detail::_qd::eager_pair<L, R>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd operator*(const L& a, const R& b) noexcept
{
    return detail::_qd::eager_value(a) * detail::_qd::eager_value(b);
}

template<class L, class R> requires detail::_qd::eager_pair<L, R>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd operator/(const L& a, const R& b) noexcept
{
    return detail::_qd::eager_value(a) / detail::_qd::eager_value(b);
}

} // namespace bl

#endif
