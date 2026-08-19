/**
 * fltx/f128_math.h - constexpr <cmath>-style functions for f128.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F128_MATH_INCLUDED
#define F128_MATH_INCLUDED

#include "fltx/detail/f128_math_basic.h"
#include "fltx/detail/f128_math_transcendental.h"
#include "fltx/detail/math_promotion.h"
#include "fltx/traits.h"

namespace bl {

[[nodiscard]] BL_FORCE_INLINE constexpr f128 fabs(const f128_s& a) noexcept
{
    return abs(a);
}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr f128 sqrt(f128_s a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::sqrt(a),
        detail::_f128_runtime::sqrt(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 cbrt(const f128_s& a)
{
    return detail::_f128_impl::cbrt(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 hypot(const f128_s& x, const f128_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::hypot(x, y),
        detail::_f128_runtime::hypot(x, y)
    );
}

// rounding and decimals
[[nodiscard]] BL_FORCE_INLINE constexpr f128 floor(const f128_s& a)
{
    return detail::_f128_impl::floor(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 ceil(const f128_s& a)
{
    return detail::_f128_impl::ceil(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 trunc(const f128_s& a)
{
    return detail::_f128_impl::trunc(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 round(const f128_s& a)
{
#if defined(_MSC_VER) && !defined(__clang__)
    return detail::_f128_impl::round_nearest_away_from_zero(a);
#else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::round_nearest_away_from_zero(a),
        detail::_f128_runtime::round_nearest_away_from_zero(a)
    );
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 roundeven(const f128_s& x)
{
    return detail::_f128_impl::round_nearest_even(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 round_decimals(f128_s v, int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::round_decimals(v, precision),
        detail::_f128_runtime::round_decimals(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 round_significant(
    f128_s v,
    int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::round_significant(v, precision),
        detail::_f128_runtime::round_significant(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long lround(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::lround_nearest_away_from_zero(x),
        detail::_f128_runtime::lround_nearest_away_from_zero(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long llround(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::llround_nearest_away_from_zero(x),
        detail::_f128_runtime::llround_nearest_away_from_zero(x)
    );
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr f128 fma(const f128_s& x, const f128_s& y, const f128_s& z)
{
#if FLTX_HAS_COMPILED_X86_FMA_BACKEND && !FLTX_TU_HAS_X86_FMA
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::fma(x, y, z),
        detail::fp::runtime_hardware_fma_enabled()
            ? detail::_f128_runtime::fma_x86(x, y, z)
            : detail::_f128_impl::fma(x, y, z)
    );
#else
    return detail::_f128_impl::fma(x, y, z);
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 fmin(const f128_s& a, const f128_s& b)
{
    return detail::_f128_impl::fmin(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 fmax(const f128_s& a, const f128_s& b)
{
    return detail::_f128_impl::fmax(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 fdim(const f128_s& x, const f128_s& y)
{
    return detail::_f128_impl::fdim(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 copysign(const f128_s& x, const f128_s& y)
{
    return detail::_f128_impl::copysign(x, y);
}

// remainders
[[nodiscard]] BL_FORCE_INLINE constexpr f128 fmod(const f128_s& x, const f128_s& y)
{
    #if defined(__GNUC__) && !defined(__clang__)
    return detail::_f128_impl::fmod(x, y);
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::fmod(x, y),
        detail::_f128_runtime::fmod(x, y)
    );
    #endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 remainder(const f128_s& x, const f128_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::remquo(x, y, nullptr),
        detail::_f128_runtime::remquo(x, y, nullptr)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 remquo(const f128_s& x, const f128_s& y, int* quo)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::remquo(x, y, quo),
        detail::_f128_runtime::remquo(x, y, quo)
    );
}

// fractional decomposition
[[nodiscard]] BL_FORCE_INLINE constexpr f128 modf(const f128_s& x, f128_s* iptr) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::modf(x, iptr),
        detail::_f128_runtime::modf(x, iptr)
    );
}

// decomposition and scaling
[[nodiscard]] BL_FORCE_INLINE constexpr f128 ldexp(const f128_s& x, int e)
{
    return detail::_f128_impl::ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 frexp(const f128_s& x, int* exp) noexcept
{
    return detail::_f128_impl::frexp(x, exp);
}

[[nodiscard]] BL_FORCE_INLINE constexpr int ilogb(const f128_s& x) noexcept
{
    return detail::_f128_impl::ilogb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 logb(const f128_s& x) noexcept
{
    return detail::_f128_impl::logb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 scalbn(const f128_s& x, int e) noexcept
{
    return detail::_f128_impl::ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 scalbln(const f128_s& x, long e) noexcept
{
    return detail::_f128_impl::ldexp(x, static_cast<int>(e));
}

// adjacent values
[[nodiscard]] BL_FORCE_INLINE constexpr f128 nextafter(const f128_s& from, const f128_s& to) noexcept
{
    return detail::_f128_impl::nextafter(from, to);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 nexttoward(const f128_s& from, long double to) noexcept
{
    const double high = static_cast<double>(to);
    if (detail::fp::isinf_or_nan(high))
        return detail::_f128_impl::nextafter(from, f128_s{ high, 0.0 });
    if (high == 0.0 && to != 0.0L)
    {
        const double target = to < 0.0L
            ? -std::numeric_limits<double>::denorm_min()
            : std::numeric_limits<double>::denorm_min();
        return detail::_f128_impl::nextafter(from, f128_s{ target, 0.0 });
    }

    const double low = static_cast<double>(to - static_cast<long double>(high));
    return detail::_f128_impl::nextafter(from, f128_s{ high, low });
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 nexttoward(const f128_s& from, const f128_s& to) noexcept
{
    return detail::_f128_impl::nextafter(from, to);
}

// exp / log
[[nodiscard]] BL_FORCE_INLINE constexpr double log_as_double(f128_s a)
{
    return detail::_f128_impl::log_as_double(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 exp(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::exp(x),
        detail::_f128_runtime::exp(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 exp2(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::exp2(x),
        detail::_f128_runtime::exp2(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 log(const f128_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::log(a),
        detail::_f128_runtime::log(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 log2(const f128_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::log2(a),
        detail::_f128_runtime::log2(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 log10(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::log10(x),
        detail::_f128_runtime::log10(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 expm1(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::expm1(x),
        detail::_f128_runtime::expm1(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 log1p(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::log1p(x),
        detail::_f128_runtime::log1p(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 pow(const f128_s& x, const f128_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::pow(x, y),
        detail::_f128_runtime::pow(x, y)
    );
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_MSVC_NOINLINE constexpr f128 ipow(const f128_s& x, Exp y)
{
    if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            f128{ detail::_f128::ipow_integer(x, y) },
            f128{ detail::_f128_runtime::ipow_signed(x, static_cast<std::intmax_t>(y)) }
        );
    }
    else
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            f128{ detail::_f128::ipow_integer(x, y) },
            f128{ detail::_f128_runtime::ipow_unsigned(x, static_cast<std::uintmax_t>(y)) }
        );
    }
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_FORCE_INLINE constexpr f128 pow(const f128_s& x, Exp y)
{
    return bl::ipow(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 pow(const f128_s& x, double y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::pow(x, y),
        detail::_f128_runtime::pow(x, y)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 pow(const f128_s&, long double) = delete;

template<class Base, class Exp>
requires (detail::math::f128_promoted_math_args<Base, Exp> &&
    !(fltx_f128<Base> && detail::fp::non_bool_integral<Exp>))
[[nodiscard]] BL_FORCE_INLINE constexpr auto pow(Base x, Exp y)
{
    using P = detail::math::promoted_t<Base, Exp>;
    using E = detail::math::promoted_pow_exponent_t<P, Exp>;
    return bl::pow(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<E>(y));
}

// trig
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const f128_s& x, f128_s& s_out, f128_s& c_out)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::sincos(x, s_out, c_out),
        detail::_f128_runtime::sincos(x, s_out, c_out)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 sin(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::sin(x),
        detail::_f128_runtime::sin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 cos(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::cos(x),
        detail::_f128_runtime::cos(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 tan(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::tan(x),
        detail::_f128_runtime::tan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 atan(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::atan(x),
        detail::_f128_runtime::atan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 atan2(const f128_s& y, const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::atan2(y, x),
        detail::_f128_runtime::atan2(y, x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 asin(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::asin(x),
        detail::_f128_runtime::asin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 acos(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::acos(x),
        detail::_f128_runtime::acos(x)
    );
}

template<class Vec>
    requires detail::fp::sincos_vector_assignable<Vec, f128_s>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const f128_s& x, Vec& out)
{
    f128_s s_out{};
    f128_s c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    detail::fp::assign_sincos_vector(out, s_out, c_out);
    return ok;
}

template<class Value> requires (std::same_as<std::remove_cvref_t<Value>, f128> || std::same_as<std::remove_cvref_t<Value>, f128_s>)
[[nodiscard]] BL_FORCE_INLINE constexpr detail::fp::sincos_vector_result<std::remove_cvref_t<Value>> sincos(const f128_s& x)
{
    using Result = std::remove_cvref_t<Value>;

    Result s_out{};
    Result c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    return detail::fp::make_sincos_result(s_out, c_out, ok);
}

// hyperbolic
[[nodiscard]] BL_FORCE_INLINE constexpr f128 sinh(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::sinh(x),
        detail::_f128_runtime::sinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 cosh(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::cosh(x),
        detail::_f128_runtime::cosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 tanh(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::tanh(x),
        detail::_f128_runtime::tanh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 asinh(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::asinh(x),
        detail::_f128_runtime::asinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 acosh(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::acosh(x),
        detail::_f128_runtime::acosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 atanh(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::atanh(x),
        detail::_f128_runtime::atanh(x)
    );
}

// erf / erfc
[[nodiscard]] BL_FORCE_INLINE constexpr f128 erf(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::erf(x),
        detail::_f128_runtime::erf(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 erfc(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::erfc(x),
        detail::_f128_runtime::erfc(x)
    );
}

// gamma
[[nodiscard]] BL_FORCE_INLINE constexpr f128 lgamma(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::lgamma(x),
        detail::_f128_runtime::lgamma(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 tgamma(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::tgamma(x),
        detail::_f128_runtime::tgamma(x)
    );
}

// type promotions
#define FLTX_F128_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::f128_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define FLTX_F128_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::f128_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define FLTX_F128_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::f128_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

FLTX_F128_PROMOTED_UNARY(abs)
FLTX_F128_PROMOTED_UNARY(fabs)
FLTX_F128_PROMOTED_UNARY(signbit)
FLTX_F128_PROMOTED_UNARY(isnan)
FLTX_F128_PROMOTED_UNARY(isinf)
FLTX_F128_PROMOTED_UNARY(isfinite)
FLTX_F128_PROMOTED_UNARY(iszero)
FLTX_F128_PROMOTED_UNARY(fpclassify)
FLTX_F128_PROMOTED_UNARY(isnormal)

FLTX_F128_PROMOTED_UNARY(floor)
FLTX_F128_PROMOTED_UNARY(ceil)
FLTX_F128_PROMOTED_UNARY(trunc)
FLTX_F128_PROMOTED_UNARY(round)
FLTX_F128_PROMOTED_UNARY(roundeven)
FLTX_F128_PROMOTED_UNARY(lround)
FLTX_F128_PROMOTED_UNARY(llround)

FLTX_F128_PROMOTED_BINARY(fmod)
FLTX_F128_PROMOTED_BINARY(remainder)
FLTX_F128_PROMOTED_TERNARY(fma)
FLTX_F128_PROMOTED_BINARY(fmin)
FLTX_F128_PROMOTED_BINARY(fmax)
FLTX_F128_PROMOTED_BINARY(fdim)
FLTX_F128_PROMOTED_BINARY(copysign)

FLTX_F128_PROMOTED_UNARY(ilogb)
FLTX_F128_PROMOTED_UNARY(logb)
FLTX_F128_PROMOTED_BINARY(nextafter)

FLTX_F128_PROMOTED_UNARY(exp)
FLTX_F128_PROMOTED_UNARY(exp2)
FLTX_F128_PROMOTED_UNARY(expm1)
FLTX_F128_PROMOTED_UNARY(log)
FLTX_F128_PROMOTED_UNARY(log2)
FLTX_F128_PROMOTED_UNARY(log10)
FLTX_F128_PROMOTED_UNARY(log1p)
FLTX_F128_PROMOTED_UNARY(sqrt)
FLTX_F128_PROMOTED_UNARY(cbrt)
FLTX_F128_PROMOTED_BINARY(hypot)

FLTX_F128_PROMOTED_UNARY(sin)
FLTX_F128_PROMOTED_UNARY(cos)
FLTX_F128_PROMOTED_UNARY(tan)
FLTX_F128_PROMOTED_UNARY(atan)
FLTX_F128_PROMOTED_BINARY(atan2)
FLTX_F128_PROMOTED_UNARY(asin)
FLTX_F128_PROMOTED_UNARY(acos)

FLTX_F128_PROMOTED_UNARY(sinh)
FLTX_F128_PROMOTED_UNARY(cosh)
FLTX_F128_PROMOTED_UNARY(tanh)
FLTX_F128_PROMOTED_UNARY(asinh)
FLTX_F128_PROMOTED_UNARY(acosh)
FLTX_F128_PROMOTED_UNARY(atanh)

FLTX_F128_PROMOTED_UNARY(erf)
FLTX_F128_PROMOTED_UNARY(erfc)
FLTX_F128_PROMOTED_UNARY(lgamma)
FLTX_F128_PROMOTED_UNARY(tgamma)

FLTX_F128_PROMOTED_BINARY(isunordered)
FLTX_F128_PROMOTED_BINARY(isgreater)
FLTX_F128_PROMOTED_BINARY(isgreaterequal)
FLTX_F128_PROMOTED_BINARY(isless)
FLTX_F128_PROMOTED_BINARY(islessequal)
FLTX_F128_PROMOTED_BINARY(islessgreater)

#undef FLTX_F128_PROMOTED_TERNARY
#undef FLTX_F128_PROMOTED_BINARY
#undef FLTX_F128_PROMOTED_UNARY

template<class T, class U>
requires detail::math::f128_promoted_math_args<T, U>
[[nodiscard]] BL_FORCE_INLINE constexpr auto remquo(T x, U y, int* quo)
{
    using P = detail::math::promoted_t<T, U>;
    return bl::remquo(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), quo);
}

template<class T>
requires detail::math::f128_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto ldexp(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::ldexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f128_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbn(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbn(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f128_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbln(T x, long exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbln(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f128_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto frexp(T x, int* exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::frexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f128_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto modf(T x, detail::math::promoted_t<T>* iptr)
{
    using P = detail::math::promoted_t<T>;
    return bl::modf(detail::math::promoted_cast<P>(x), iptr);
}

template<class From, class To>
requires detail::math::f128_nexttoward_args<From, To>
[[nodiscard]] BL_FORCE_INLINE constexpr auto nexttoward(From from, To to)
{
    using P = detail::math::promoted_t<From>;
    const P promoted_from = detail::math::promoted_cast<P>(from);

    if constexpr (std::same_as<detail::math::clean_t<To>, long double>)
        return bl::nexttoward(promoted_from, to);
    else
        return bl::nexttoward(promoted_from, detail::math::promoted_cast<P>(to));
}

template<class T>
requires detail::math::f128_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(T x, detail::math::promoted_t<T>& s_out, detail::math::promoted_t<T>& c_out)
{
    using P = detail::math::promoted_t<T>;
    return bl::sincos(detail::math::promoted_cast<P>(x), s_out, c_out);
}

} // namespace bl

#endif
