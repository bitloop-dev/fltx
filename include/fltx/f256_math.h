/**
 * fltx/f256_math.h - constexpr <cmath>-style functions for f256.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F256_MATH_INCLUDED
#define F256_MATH_INCLUDED

#include "fltx/f128.h"
#include "fltx/f256.h"
#include "fltx/detail/f256_math_basic.h"
#include "fltx/detail/f256_math_transcendental.h"
#include "fltx/detail/math_promotion.h"
#include "fltx/traits.h"

namespace bl {

[[nodiscard]] BL_FORCE_INLINE constexpr f256 fabs(const f256_s& a) noexcept
{
    return abs(a);
}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr f256 sqrt(const f256_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::sqrt(a),
        detail::_f256_runtime::sqrt(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 cbrt(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::cbrt(x),
        detail::_f256_runtime::cbrt(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 hypot(const f256_s& x, const f256_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::hypot(x, y),
        detail::_f256_runtime::hypot(x, y)
    );
}

// rounding and decimals
[[nodiscard]] BL_FORCE_INLINE constexpr f256 floor(const f256_s& a)
{
    return detail::_f256::floor_limbwise(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 ceil(const f256_s& a)
{
    return detail::_f256::ceil_limbwise(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 trunc(const f256_s& a)
{
    return detail::_f256_impl::trunc(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 round(const f256_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::round_nearest_away_from_zero(a),
        detail::_f256_runtime::round_nearest_away_from_zero(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 roundeven(const f256_s& x)
{
    return detail::_f256_impl::round_nearest_even(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 round_to_decimals(f256_s v, int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::round_to_decimals(v, precision),
        detail::_f256_runtime::round_to_decimals(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 round_to_significant_figures(
    f256_s v,
    int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::round_to_significant_figures(v, precision),
        detail::_f256_runtime::round_to_significant_figures(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long lround(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::lround_nearest_away_from_zero(x),
        detail::_f256_runtime::lround_nearest_away_from_zero(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long llround(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::llround_nearest_away_from_zero(x),
        detail::_f256_runtime::llround_nearest_away_from_zero(x)
    );
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr f256 fma(const f256_s& x, const f256_s& y, const f256_s& z)
{
#if FLTX_HAS_COMPILED_X86_FMA_BACKEND && !FLTX_TU_HAS_X86_FMA
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::fma(x, y, z),
        detail::fp::runtime_hardware_fma_enabled()
            ? detail::_f256_runtime::fma_x86(x, y, z)
            : detail::_f256_impl::fma(x, y, z)
    );
#else
    return detail::_f256_impl::fma(x, y, z);
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 fmin(const f256_s& a, const f256_s& b)
{
    return detail::_f256_impl::fmin(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 fmax(const f256_s& a, const f256_s& b)
{
    return detail::_f256_impl::fmax(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 fdim(const f256_s& x, const f256_s& y)
{
    return detail::_f256_impl::fdim(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 copysign(const f256_s& x, const f256_s& y)
{
    return detail::_f256_impl::copysign(x, y);
}

// remainders
[[nodiscard]] BL_FORCE_INLINE constexpr f256 fmod(const f256_s& x, const f256_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::fmod(x, y),
        detail::_f256_runtime::fmod(x, y)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 remainder(const f256_s& x, const f256_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::remquo(x, y, nullptr),
        detail::_f256_runtime::remquo(x, y, nullptr)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 remquo(const f256_s& x, const f256_s& y, int* quo)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::remquo(x, y, quo),
        detail::_f256_runtime::remquo(x, y, quo)
    );
}

// fractional decomposition
[[nodiscard]] BL_FORCE_INLINE constexpr f256 modf(const f256_s& x, f256_s* iptr) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::modf(x, iptr),
        detail::_f256_runtime::modf(x, iptr)
    );
}

// decomposition and scaling
[[nodiscard]] BL_FORCE_INLINE constexpr f256 ldexp(const f256_s& a, int e)
{
    return detail::_f256_impl::ldexp(a, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 frexp(const f256_s& x, int* exp) noexcept
{
    return detail::_f256_impl::frexp(x, exp);
}

[[nodiscard]] BL_FORCE_INLINE constexpr int ilogb(const f256_s& x) noexcept
{
    return detail::_f256_impl::ilogb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 logb(const f256_s& x) noexcept
{
    return detail::_f256_impl::logb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 scalbn(const f256_s& x, int e) noexcept
{
    return detail::_f256_impl::ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 scalbln(const f256_s& x, long e) noexcept
{
    return detail::_f256_impl::ldexp(x, static_cast<int>(e));
}

// adjacent values
[[nodiscard]] BL_FORCE_INLINE constexpr f256 nextafter(const f256_s& from, const f256_s& to) noexcept
{
    return detail::_f256_impl::nextafter(from, to);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 nexttoward(const f256_s& from, long double to) noexcept
{
    return detail::_f256_impl::nextafter(from, f256_s{ static_cast<double>(to), 0.0, 0.0, 0.0 });
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 nexttoward(const f256_s& from, const f256_s& to) noexcept
{
    return detail::_f256_impl::nextafter(from, to);
}

// exp / log
[[nodiscard]] BL_FORCE_INLINE constexpr double log_as_double(f256_s a) noexcept
{
    return detail::_f256_impl::log_as_double(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 exp(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::exp(x),
        detail::_f256_runtime::exp(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 exp2(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::exp2(x),
        detail::_f256_runtime::exp2(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 log(const f256_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::log(a),
        detail::_f256_runtime::log(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 log2(const f256_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::log2(a),
        detail::_f256_runtime::log2(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 log10(const f256_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::log10(a),
        detail::_f256_runtime::log10(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 expm1(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::expm1(x),
        detail::_f256_runtime::expm1(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 log1p(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::log1p(x),
        detail::_f256_runtime::log1p(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 pow(const f256_s& x, const f256_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::pow(x, y),
        detail::_f256_runtime::pow(x, y)
    );
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_MSVC_NOINLINE constexpr f256 ipow(const f256_s& x, Exp y)
{
    if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            f256{ detail::_f256::ipow_integer(x, y) },
            f256{ detail::_f256_runtime::ipow_signed(x, static_cast<std::intmax_t>(y)) }
        );
    }
    else
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            f256{ detail::_f256::ipow_integer(x, y) },
            f256{ detail::_f256_runtime::ipow_unsigned(x, static_cast<std::uintmax_t>(y)) }
        );
    }
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_FORCE_INLINE constexpr f256 pow(const f256_s& x, Exp y)
{
    return bl::ipow(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 pow(const f256_s& x, double y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::pow(x, y),
        detail::_f256_runtime::pow(x, y)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 pow(const f256_s& x, const f128_s& y)
{
    f256_s promoted{};
    promoted = y;
    return bl::pow(x, promoted);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 pow(const f256_s&, long double) = delete;

template<class Base, class Exp>
requires (detail::math::f256_promoted_math_args<Base, Exp> &&
    !(fltx_f256<Base> && detail::fp::non_bool_integral<Exp>))
[[nodiscard]] BL_FORCE_INLINE constexpr auto pow(Base x, Exp y)
{
    using P = detail::math::promoted_t<Base, Exp>;
    using E = detail::math::promoted_pow_exponent_t<P, Exp>;
    return bl::pow(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<E>(y));
}

// trig
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const f256_s& x, f256_s& s_out, f256_s& c_out)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::sincos(x, s_out, c_out),
        detail::_f256_runtime::sincos(x, s_out, c_out)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 sin(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::sin(x),
        detail::_f256_runtime::sin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 cos(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::cos(x),
        detail::_f256_runtime::cos(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 tan(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::tan(x),
        detail::_f256_runtime::tan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 atan(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::atan(x),
        detail::_f256_runtime::atan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 atan2(const f256_s& y, const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::atan2(y, x),
        detail::_f256_runtime::atan2(y, x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 asin(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::asin(x),
        detail::_f256_runtime::asin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 acos(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::acos(x),
        detail::_f256_runtime::acos(x)
    );
}

template<class Vec>
    requires detail::fp::sincos_vector_assignable<Vec, f256_s>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const f256_s& x, Vec& out)
{
    f256_s s_out{};
    f256_s c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    if (!ok)
    {
        s_out = bl::sin(x);
        c_out = bl::cos(x);
    }
    detail::fp::assign_sincos_vector(out, s_out, c_out);
    return ok;
}

template<class Value> requires (std::same_as<std::remove_cvref_t<Value>, f256> || std::same_as<std::remove_cvref_t<Value>, f256_s>)
[[nodiscard]] BL_FORCE_INLINE constexpr detail::fp::sincos_vector_result<std::remove_cvref_t<Value>> sincos(const f256_s& x)
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
[[nodiscard]] BL_FORCE_INLINE constexpr f256 sinh(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::sinh(x),
        detail::_f256_runtime::sinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 cosh(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::cosh(x),
        detail::_f256_runtime::cosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 tanh(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::tanh(x),
        detail::_f256_runtime::tanh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 asinh(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::asinh(x),
        detail::_f256_runtime::asinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 acosh(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::acosh(x),
        detail::_f256_runtime::acosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 atanh(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::atanh(x),
        detail::_f256_runtime::atanh(x)
    );
}

// erf / erfc
[[nodiscard]] BL_FORCE_INLINE constexpr f256 erf(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::erf(x),
        detail::_f256_runtime::erf(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 erfc(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::erfc(x),
        detail::_f256_runtime::erfc(x)
    );
}

// gamma
[[nodiscard]] BL_FORCE_INLINE constexpr f256 lgamma(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::lgamma(x),
        detail::_f256_runtime::lgamma(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256 tgamma(const f256_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256_impl::tgamma(x),
        detail::_f256_runtime::tgamma(x)
    );
}

// type promotions
#define FLTX_F256_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::f256_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define FLTX_F256_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::f256_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define FLTX_F256_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::f256_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

FLTX_F256_PROMOTED_UNARY(abs)
FLTX_F256_PROMOTED_UNARY(fabs)
FLTX_F256_PROMOTED_UNARY(signbit)
FLTX_F256_PROMOTED_UNARY(isnan)
FLTX_F256_PROMOTED_UNARY(isinf)
FLTX_F256_PROMOTED_UNARY(isfinite)
FLTX_F256_PROMOTED_UNARY(iszero)
FLTX_F256_PROMOTED_UNARY(fpclassify)
FLTX_F256_PROMOTED_UNARY(isnormal)

FLTX_F256_PROMOTED_UNARY(floor)
FLTX_F256_PROMOTED_UNARY(ceil)
FLTX_F256_PROMOTED_UNARY(trunc)
FLTX_F256_PROMOTED_UNARY(round)
FLTX_F256_PROMOTED_UNARY(roundeven)
FLTX_F256_PROMOTED_UNARY(lround)
FLTX_F256_PROMOTED_UNARY(llround)

FLTX_F256_PROMOTED_BINARY(fmod)
FLTX_F256_PROMOTED_BINARY(remainder)
FLTX_F256_PROMOTED_TERNARY(fma)
FLTX_F256_PROMOTED_BINARY(fmin)
FLTX_F256_PROMOTED_BINARY(fmax)
FLTX_F256_PROMOTED_BINARY(fdim)
FLTX_F256_PROMOTED_BINARY(copysign)

FLTX_F256_PROMOTED_UNARY(ilogb)
FLTX_F256_PROMOTED_UNARY(logb)
FLTX_F256_PROMOTED_BINARY(nextafter)

FLTX_F256_PROMOTED_UNARY(exp)
FLTX_F256_PROMOTED_UNARY(exp2)
FLTX_F256_PROMOTED_UNARY(expm1)
FLTX_F256_PROMOTED_UNARY(log)
FLTX_F256_PROMOTED_UNARY(log2)
FLTX_F256_PROMOTED_UNARY(log10)
FLTX_F256_PROMOTED_UNARY(log1p)
FLTX_F256_PROMOTED_UNARY(sqrt)
FLTX_F256_PROMOTED_UNARY(cbrt)
FLTX_F256_PROMOTED_BINARY(hypot)

FLTX_F256_PROMOTED_UNARY(sin)
FLTX_F256_PROMOTED_UNARY(cos)
FLTX_F256_PROMOTED_UNARY(tan)
FLTX_F256_PROMOTED_UNARY(atan)
FLTX_F256_PROMOTED_BINARY(atan2)
FLTX_F256_PROMOTED_UNARY(asin)
FLTX_F256_PROMOTED_UNARY(acos)

FLTX_F256_PROMOTED_UNARY(sinh)
FLTX_F256_PROMOTED_UNARY(cosh)
FLTX_F256_PROMOTED_UNARY(tanh)
FLTX_F256_PROMOTED_UNARY(asinh)
FLTX_F256_PROMOTED_UNARY(acosh)
FLTX_F256_PROMOTED_UNARY(atanh)

FLTX_F256_PROMOTED_UNARY(erf)
FLTX_F256_PROMOTED_UNARY(erfc)
FLTX_F256_PROMOTED_UNARY(lgamma)
FLTX_F256_PROMOTED_UNARY(tgamma)

FLTX_F256_PROMOTED_BINARY(isunordered)
FLTX_F256_PROMOTED_BINARY(isgreater)
FLTX_F256_PROMOTED_BINARY(isgreaterequal)
FLTX_F256_PROMOTED_BINARY(isless)
FLTX_F256_PROMOTED_BINARY(islessequal)
FLTX_F256_PROMOTED_BINARY(islessgreater)

#undef FLTX_F256_PROMOTED_TERNARY
#undef FLTX_F256_PROMOTED_BINARY
#undef FLTX_F256_PROMOTED_UNARY

template<class T, class U>
requires detail::math::f256_promoted_math_args<T, U>
[[nodiscard]] BL_FORCE_INLINE constexpr auto remquo(T x, U y, int* quo)
{
    using P = detail::math::promoted_t<T, U>;
    return bl::remquo(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), quo);
}

template<class T>
requires detail::math::f256_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto ldexp(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::ldexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f256_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbn(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbn(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f256_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbln(T x, long exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbln(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f256_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto frexp(T x, int* exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::frexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::f256_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto modf(T x, detail::math::promoted_t<T>* iptr)
{
    using P = detail::math::promoted_t<T>;
    return bl::modf(detail::math::promoted_cast<P>(x), iptr);
}

template<class From, class To>
requires detail::math::f256_nexttoward_args<From, To>
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
requires detail::math::f256_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(T x, detail::math::promoted_t<T>& s_out, detail::math::promoted_t<T>& c_out)
{
    using P = detail::math::promoted_t<T>;
    return bl::sincos(detail::math::promoted_cast<P>(x), s_out, c_out);
}

} // namespace bl

#endif
