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
#include "fltx/round_options.h"
#include "fltx/traits.h"

namespace bl {

[[nodiscard]] BL_FORCE_INLINE constexpr f128 fabs(const f128_s& a) noexcept
{
    return abs(a);
}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr f128 sqrt(f128_s a)
{
    #if defined(BL_FAST_MATH)
    return detail::_f128_impl::sqrt(a);
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::sqrt(a),
        detail::_f128_runtime::sqrt(a)
    );
    #endif
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
    return detail::_f128_impl::round(a);
#else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::round(a),
        detail::_f128_runtime::round(a)
    );
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 round_to(f128_s v, int precision, round_format format)
{
    if (format == round_format::decimals)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_f128_impl::round_to_decimals(v, precision),
            detail::_f128_runtime::round_to_decimals(v, precision)
        );
    }

    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::round_to_significant_figures(v, precision),
        detail::_f128_runtime::round_to_significant_figures(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 nearbyint(const f128_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::nearbyint(a),
        detail::_f128_impl::nearbyint_runtime(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128 rint(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::nearbyint_generic(x),
        detail::_f128_impl::nearbyint_runtime(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long lround(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::lround(x),
        detail::_f128_runtime::lround(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long llround(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::llround(x),
        detail::_f128_runtime::llround(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long lrint(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::lrint(x),
        detail::_f128_runtime::lrint(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long llrint(const f128_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128_impl::llrint(x),
        detail::_f128_runtime::llrint(x)
    );
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr f128 fma(const f128_s& x, const f128_s& y, const f128_s& z)
{
    return detail::_f128_impl::fma(x, y, z);
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
    return detail::_f128_impl::nextafter(from, f128_s{ static_cast<double>(to) });
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
    if (!ok)
    {
        s_out = bl::sin(x);
        c_out = bl::cos(x);
    }
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
    if (!ok)
    {
        s_out = bl::sin(x);
        c_out = bl::cos(x);
    }
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
#define BL_FLTX_F128_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::f128_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define BL_FLTX_F128_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::f128_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define BL_FLTX_F128_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::f128_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

BL_FLTX_F128_PROMOTED_UNARY(abs)
BL_FLTX_F128_PROMOTED_UNARY(fabs)
BL_FLTX_F128_PROMOTED_UNARY(signbit)
BL_FLTX_F128_PROMOTED_UNARY(isnan)
BL_FLTX_F128_PROMOTED_UNARY(isinf)
BL_FLTX_F128_PROMOTED_UNARY(isfinite)
BL_FLTX_F128_PROMOTED_UNARY(iszero)
BL_FLTX_F128_PROMOTED_UNARY(fpclassify)
BL_FLTX_F128_PROMOTED_UNARY(isnormal)

BL_FLTX_F128_PROMOTED_UNARY(floor)
BL_FLTX_F128_PROMOTED_UNARY(ceil)
BL_FLTX_F128_PROMOTED_UNARY(trunc)
BL_FLTX_F128_PROMOTED_UNARY(round)
BL_FLTX_F128_PROMOTED_UNARY(nearbyint)
BL_FLTX_F128_PROMOTED_UNARY(rint)
BL_FLTX_F128_PROMOTED_UNARY(lround)
BL_FLTX_F128_PROMOTED_UNARY(llround)
BL_FLTX_F128_PROMOTED_UNARY(lrint)
BL_FLTX_F128_PROMOTED_UNARY(llrint)

BL_FLTX_F128_PROMOTED_BINARY(fmod)
BL_FLTX_F128_PROMOTED_BINARY(remainder)
BL_FLTX_F128_PROMOTED_TERNARY(fma)
BL_FLTX_F128_PROMOTED_BINARY(fmin)
BL_FLTX_F128_PROMOTED_BINARY(fmax)
BL_FLTX_F128_PROMOTED_BINARY(fdim)
BL_FLTX_F128_PROMOTED_BINARY(copysign)

BL_FLTX_F128_PROMOTED_UNARY(ilogb)
BL_FLTX_F128_PROMOTED_UNARY(logb)
BL_FLTX_F128_PROMOTED_BINARY(nextafter)

BL_FLTX_F128_PROMOTED_UNARY(exp)
BL_FLTX_F128_PROMOTED_UNARY(exp2)
BL_FLTX_F128_PROMOTED_UNARY(expm1)
BL_FLTX_F128_PROMOTED_UNARY(log)
BL_FLTX_F128_PROMOTED_UNARY(log2)
BL_FLTX_F128_PROMOTED_UNARY(log10)
BL_FLTX_F128_PROMOTED_UNARY(log1p)
BL_FLTX_F128_PROMOTED_UNARY(sqrt)
BL_FLTX_F128_PROMOTED_UNARY(cbrt)
BL_FLTX_F128_PROMOTED_BINARY(hypot)

BL_FLTX_F128_PROMOTED_UNARY(sin)
BL_FLTX_F128_PROMOTED_UNARY(cos)
BL_FLTX_F128_PROMOTED_UNARY(tan)
BL_FLTX_F128_PROMOTED_UNARY(atan)
BL_FLTX_F128_PROMOTED_BINARY(atan2)
BL_FLTX_F128_PROMOTED_UNARY(asin)
BL_FLTX_F128_PROMOTED_UNARY(acos)

BL_FLTX_F128_PROMOTED_UNARY(sinh)
BL_FLTX_F128_PROMOTED_UNARY(cosh)
BL_FLTX_F128_PROMOTED_UNARY(tanh)
BL_FLTX_F128_PROMOTED_UNARY(asinh)
BL_FLTX_F128_PROMOTED_UNARY(acosh)
BL_FLTX_F128_PROMOTED_UNARY(atanh)

BL_FLTX_F128_PROMOTED_UNARY(erf)
BL_FLTX_F128_PROMOTED_UNARY(erfc)
BL_FLTX_F128_PROMOTED_UNARY(lgamma)
BL_FLTX_F128_PROMOTED_UNARY(tgamma)

BL_FLTX_F128_PROMOTED_BINARY(isunordered)
BL_FLTX_F128_PROMOTED_BINARY(isgreater)
BL_FLTX_F128_PROMOTED_BINARY(isgreaterequal)
BL_FLTX_F128_PROMOTED_BINARY(isless)
BL_FLTX_F128_PROMOTED_BINARY(islessequal)
BL_FLTX_F128_PROMOTED_BINARY(islessgreater)

#undef BL_FLTX_F128_PROMOTED_TERNARY
#undef BL_FLTX_F128_PROMOTED_BINARY
#undef BL_FLTX_F128_PROMOTED_UNARY

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
