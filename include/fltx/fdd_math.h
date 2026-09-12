/**
 * fltx/fdd_math.h - constexpr math functions for dd values, following <cmath> conventions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_MATH_INCLUDED
#define FDD_MATH_INCLUDED

#include "fltx/detail/fdd_math_basic.h"
#include "fltx/detail/fdd_math_transcendental.h"
#include "fltx/detail/math_promotion.h"
#include "fltx/traits.h"

namespace bl {

[[nodiscard]] BL_FORCE_INLINE constexpr fdd fabs(const fdd_s& a) noexcept
{
    return abs(a);
}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr fdd sqrt(fdd_s a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::sqrt(a),
        detail::_dd_runtime::sqrt(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd cbrt(const fdd_s& a)
{
    return detail::_dd_impl::cbrt(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd hypot(const fdd_s& x, const fdd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::hypot(x, y),
        detail::_dd_runtime::hypot(x, y)
    );
}

// rounding and decimals
[[nodiscard]] BL_FORCE_INLINE constexpr fdd floor(const fdd_s& a)
{
    return detail::_dd_impl::floor(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd ceil(const fdd_s& a)
{
    return detail::_dd_impl::ceil(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd trunc(const fdd_s& a)
{
    return detail::_dd_impl::trunc(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd round(const fdd_s& a)
{
#if (defined(_MSC_VER) && !defined(__clang__)) || (defined(__APPLE__) && defined(__aarch64__))
    return detail::_dd_impl::round_nearest_away_from_zero(a);
#else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::round_nearest_away_from_zero(a),
        detail::_dd_runtime::round_nearest_away_from_zero(a)
    );
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd roundeven(const fdd_s& x)
{
    return detail::_dd_impl::round_nearest_even(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd round_decimals(fdd_s v, int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::round_decimals(v, precision),
        detail::_dd_runtime::round_decimals(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd round_significant(
    fdd_s v,
    int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::round_significant(v, precision),
        detail::_dd_runtime::round_significant(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long lround(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::lround_nearest_away_from_zero(x),
        detail::_dd_runtime::lround_nearest_away_from_zero(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long llround(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::llround_nearest_away_from_zero(x),
        detail::_dd_runtime::llround_nearest_away_from_zero(x)
    );
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr fdd fma(const fdd_s& x, const fdd_s& y, const fdd_s& z)
{
#if FLTX_HAS_COMPILED_X86_FMA_BACKEND && !FLTX_TU_HAS_X86_FMA
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::fma(x, y, z),
        detail::fp::runtime_hardware_fma_enabled()
            ? detail::_dd_runtime::fma_x86(x, y, z)
            : detail::_dd_impl::fma(x, y, z)
    );
#else
    return detail::_dd_impl::fma(x, y, z);
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd fmin(const fdd_s& a, const fdd_s& b)
{
    return detail::_dd_impl::fmin(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd fmax(const fdd_s& a, const fdd_s& b)
{
    return detail::_dd_impl::fmax(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd fdim(const fdd_s& x, const fdd_s& y)
{
    return detail::_dd_impl::fdim(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd copysign(const fdd_s& x, const fdd_s& y)
{
    return detail::_dd_impl::copysign(x, y);
}

// remainders
[[nodiscard]] BL_FORCE_INLINE constexpr fdd fmod(const fdd_s& x, const fdd_s& y)
{
    #if defined(__GNUC__) && !defined(__clang__)
    return detail::_dd_impl::fmod(x, y);
    #else
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::fmod(x, y),
        detail::_dd_runtime::fmod(x, y)
    );
    #endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd remainder(const fdd_s& x, const fdd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::remquo(x, y, nullptr),
        detail::_dd_runtime::remquo(x, y, nullptr)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd remquo(const fdd_s& x, const fdd_s& y, int* quo)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::remquo(x, y, quo),
        detail::_dd_runtime::remquo(x, y, quo)
    );
}

// fractional decomposition
[[nodiscard]] BL_FORCE_INLINE constexpr fdd modf(const fdd_s& x, fdd_s* iptr) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::modf(x, iptr),
        detail::_dd_runtime::modf(x, iptr)
    );
}

// decomposition and scaling
[[nodiscard]] BL_FORCE_INLINE constexpr fdd ldexp(const fdd_s& x, int e)
{
    return detail::_dd_impl::ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd frexp(const fdd_s& x, int* exp) noexcept
{
    return detail::_dd_impl::frexp(x, exp);
}

[[nodiscard]] BL_FORCE_INLINE constexpr int ilogb(const fdd_s& x) noexcept
{
    return detail::_dd_impl::ilogb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd logb(const fdd_s& x) noexcept
{
    return detail::_dd_impl::logb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd scalbn(const fdd_s& x, int e) noexcept
{
    return detail::_dd_impl::ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd scalbln(const fdd_s& x, long e) noexcept
{
    return detail::_dd_impl::ldexp(x, static_cast<int>(e));
}

// adjacent values
[[nodiscard]] BL_FORCE_INLINE constexpr fdd nextafter(const fdd_s& from, const fdd_s& to) noexcept
{
    return detail::_dd_impl::nextafter(from, to);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd nexttoward(const fdd_s& from, long double to) noexcept
{
    const double high = static_cast<double>(to);
    if (detail::fp::isinf_or_nan(high))
        return detail::_dd_impl::nextafter(from, fdd_s{ high, 0.0 });
    if (high == 0.0 && to != 0.0L)
    {
        const double target = to < 0.0L
            ? -std::numeric_limits<double>::denorm_min()
            : std::numeric_limits<double>::denorm_min();
        return detail::_dd_impl::nextafter(from, fdd_s{ target, 0.0 });
    }

    const double low = static_cast<double>(to - static_cast<long double>(high));
    return detail::_dd_impl::nextafter(from, fdd_s{ high, low });
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd nexttoward(const fdd_s& from, const fdd_s& to) noexcept
{
    return detail::_dd_impl::nextafter(from, to);
}

// exp / log
[[nodiscard]] BL_FORCE_INLINE constexpr double log_as_double(fdd_s a)
{
    return detail::_dd_impl::log_as_double(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd exp(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::exp(x),
        detail::_dd_runtime::exp(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd exp2(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::exp2(x),
        detail::_dd_runtime::exp2(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd log(const fdd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::log(a),
        detail::_dd_runtime::log(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd log2(const fdd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::log2(a),
        detail::_dd_runtime::log2(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd log10(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::log10(x),
        detail::_dd_runtime::log10(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd expm1(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::expm1(x),
        detail::_dd_runtime::expm1(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd log1p(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::log1p(x),
        detail::_dd_runtime::log1p(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd pow(const fdd_s& x, const fdd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::pow(x, y),
        detail::_dd_runtime::pow(x, y)
    );
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_MSVC_NOINLINE constexpr fdd ipow(const fdd_s& x, Exp y)
{
    if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            fdd{ detail::_dd::ipow_integer(x, y) },
            fdd{ detail::_dd_runtime::ipow_signed(x, static_cast<std::intmax_t>(y)) }
        );
    }
    else
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            fdd{ detail::_dd::ipow_integer(x, y) },
            fdd{ detail::_dd_runtime::ipow_unsigned(x, static_cast<std::uintmax_t>(y)) }
        );
    }
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_FORCE_INLINE constexpr fdd pow(const fdd_s& x, Exp y)
{
    return bl::ipow(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd pow(const fdd_s& x, double y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::pow(x, y),
        detail::_dd_runtime::pow(x, y)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd pow(const fdd_s&, long double) = delete;

template<class Base, class Exp>
requires (detail::math::dd_promoted_math_args<Base, Exp> &&
    !(fltx_fdd<Base> && detail::fp::non_bool_integral<Exp>))
[[nodiscard]] BL_FORCE_INLINE constexpr auto pow(Base x, Exp y)
{
    using P = detail::math::promoted_t<Base, Exp>;
    using E = detail::math::promoted_pow_exponent_t<P, Exp>;
    return bl::pow(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<E>(y));
}

// trig
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const fdd_s& x, fdd_s& s_out, fdd_s& c_out)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::sincos(x, s_out, c_out),
        detail::_dd_runtime::sincos(x, s_out, c_out)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd sin(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::sin(x),
        detail::_dd_runtime::sin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd cos(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::cos(x),
        detail::_dd_runtime::cos(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd tan(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::tan(x),
        detail::_dd_runtime::tan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd atan(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::atan(x),
        detail::_dd_runtime::atan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd atan2(const fdd_s& y, const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::atan2(y, x),
        detail::_dd_runtime::atan2(y, x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd asin(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::asin(x),
        detail::_dd_runtime::asin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd acos(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::acos(x),
        detail::_dd_runtime::acos(x)
    );
}

template<class Vec>
    requires detail::fp::sincos_vector_assignable<Vec, fdd_s>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const fdd_s& x, Vec& out)
{
    fdd_s s_out{};
    fdd_s c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    detail::fp::assign_sincos_vector(out, s_out, c_out);
    return ok;
}

template<class Value> requires (std::same_as<std::remove_cvref_t<Value>, fdd> || std::same_as<std::remove_cvref_t<Value>, fdd_s>)
[[nodiscard]] BL_FORCE_INLINE constexpr detail::fp::sincos_vector_result<std::remove_cvref_t<Value>> sincos(const fdd_s& x)
{
    using Result = std::remove_cvref_t<Value>;

    Result s_out{};
    Result c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    return detail::fp::make_sincos_result(s_out, c_out, ok);
}

// hyperbolic
[[nodiscard]] BL_FORCE_INLINE constexpr fdd sinh(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::sinh(x),
        detail::_dd_runtime::sinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd cosh(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::cosh(x),
        detail::_dd_runtime::cosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd tanh(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::tanh(x),
        detail::_dd_runtime::tanh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd asinh(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::asinh(x),
        detail::_dd_runtime::asinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd acosh(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::acosh(x),
        detail::_dd_runtime::acosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd atanh(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::atanh(x),
        detail::_dd_runtime::atanh(x)
    );
}

// erf / erfc
[[nodiscard]] BL_FORCE_INLINE constexpr fdd erf(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::erf(x),
        detail::_dd_runtime::erf(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd erfc(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::erfc(x),
        detail::_dd_runtime::erfc(x)
    );
}

// gamma
[[nodiscard]] BL_FORCE_INLINE constexpr fdd lgamma(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::lgamma(x),
        detail::_dd_runtime::lgamma(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fdd tgamma(const fdd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_dd_impl::tgamma(x),
        detail::_dd_runtime::tgamma(x)
    );
}

// type promotions
#define FLTX_DD_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::dd_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define FLTX_DD_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::dd_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define FLTX_DD_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::dd_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

FLTX_DD_PROMOTED_UNARY(abs)
FLTX_DD_PROMOTED_UNARY(fabs)
FLTX_DD_PROMOTED_UNARY(signbit)
FLTX_DD_PROMOTED_UNARY(isnan)
FLTX_DD_PROMOTED_UNARY(isinf)
FLTX_DD_PROMOTED_UNARY(isfinite)
FLTX_DD_PROMOTED_UNARY(iszero)
FLTX_DD_PROMOTED_UNARY(fpclassify)
FLTX_DD_PROMOTED_UNARY(isnormal)

FLTX_DD_PROMOTED_UNARY(floor)
FLTX_DD_PROMOTED_UNARY(ceil)
FLTX_DD_PROMOTED_UNARY(trunc)
FLTX_DD_PROMOTED_UNARY(round)
FLTX_DD_PROMOTED_UNARY(roundeven)
FLTX_DD_PROMOTED_UNARY(lround)
FLTX_DD_PROMOTED_UNARY(llround)

FLTX_DD_PROMOTED_BINARY(fmod)
FLTX_DD_PROMOTED_BINARY(remainder)
FLTX_DD_PROMOTED_TERNARY(fma)
FLTX_DD_PROMOTED_BINARY(fmin)
FLTX_DD_PROMOTED_BINARY(fmax)
FLTX_DD_PROMOTED_BINARY(fdim)
FLTX_DD_PROMOTED_BINARY(copysign)

FLTX_DD_PROMOTED_UNARY(ilogb)
FLTX_DD_PROMOTED_UNARY(logb)
FLTX_DD_PROMOTED_BINARY(nextafter)

FLTX_DD_PROMOTED_UNARY(exp)
FLTX_DD_PROMOTED_UNARY(exp2)
FLTX_DD_PROMOTED_UNARY(expm1)
FLTX_DD_PROMOTED_UNARY(log)
FLTX_DD_PROMOTED_UNARY(log2)
FLTX_DD_PROMOTED_UNARY(log10)
FLTX_DD_PROMOTED_UNARY(log1p)
FLTX_DD_PROMOTED_UNARY(sqrt)
FLTX_DD_PROMOTED_UNARY(cbrt)
FLTX_DD_PROMOTED_BINARY(hypot)

FLTX_DD_PROMOTED_UNARY(sin)
FLTX_DD_PROMOTED_UNARY(cos)
FLTX_DD_PROMOTED_UNARY(tan)
FLTX_DD_PROMOTED_UNARY(atan)
FLTX_DD_PROMOTED_BINARY(atan2)
FLTX_DD_PROMOTED_UNARY(asin)
FLTX_DD_PROMOTED_UNARY(acos)

FLTX_DD_PROMOTED_UNARY(sinh)
FLTX_DD_PROMOTED_UNARY(cosh)
FLTX_DD_PROMOTED_UNARY(tanh)
FLTX_DD_PROMOTED_UNARY(asinh)
FLTX_DD_PROMOTED_UNARY(acosh)
FLTX_DD_PROMOTED_UNARY(atanh)

FLTX_DD_PROMOTED_UNARY(erf)
FLTX_DD_PROMOTED_UNARY(erfc)
FLTX_DD_PROMOTED_UNARY(lgamma)
FLTX_DD_PROMOTED_UNARY(tgamma)

FLTX_DD_PROMOTED_BINARY(isunordered)
FLTX_DD_PROMOTED_BINARY(isgreater)
FLTX_DD_PROMOTED_BINARY(isgreaterequal)
FLTX_DD_PROMOTED_BINARY(isless)
FLTX_DD_PROMOTED_BINARY(islessequal)
FLTX_DD_PROMOTED_BINARY(islessgreater)

#undef FLTX_DD_PROMOTED_TERNARY
#undef FLTX_DD_PROMOTED_BINARY
#undef FLTX_DD_PROMOTED_UNARY

template<class T, class U>
requires detail::math::dd_promoted_math_args<T, U>
[[nodiscard]] BL_FORCE_INLINE constexpr auto remquo(T x, U y, int* quo)
{
    using P = detail::math::promoted_t<T, U>;
    return bl::remquo(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), quo);
}

template<class T>
requires detail::math::dd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto ldexp(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::ldexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::dd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbn(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbn(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::dd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbln(T x, long exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbln(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::dd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto frexp(T x, int* exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::frexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::dd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto modf(T x, detail::math::promoted_t<T>* iptr)
{
    using P = detail::math::promoted_t<T>;
    return bl::modf(detail::math::promoted_cast<P>(x), iptr);
}

template<class From, class To>
requires detail::math::dd_nexttoward_args<From, To>
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
requires detail::math::dd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(T x, detail::math::promoted_t<T>& s_out, detail::math::promoted_t<T>& c_out)
{
    using P = detail::math::promoted_t<T>;
    return bl::sincos(detail::math::promoted_cast<P>(x), s_out, c_out);
}

} // namespace bl

#endif
