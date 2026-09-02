/**
 * fltx/fqd_math.h - constexpr math functions for qd values, following <cmath> conventions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_MATH_INCLUDED
#define FQD_MATH_INCLUDED

#include "fltx/fdd.h"
#include "fltx/fqd.h"
#include "fltx/detail/fqd_math_basic.h"
#include "fltx/detail/fqd_math_transcendental.h"
#include "fltx/detail/math_promotion.h"
#include "fltx/traits.h"

namespace bl {

[[nodiscard]] BL_FORCE_INLINE constexpr fqd fabs(const fqd_s& a) noexcept
{
    return abs(a);
}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr fqd sqrt(const fqd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::sqrt(a),
        detail::_qd_runtime::sqrt(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd cbrt(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::cbrt(x),
        detail::_qd_runtime::cbrt(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd hypot(const fqd_s& x, const fqd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::hypot(x, y),
        detail::_qd_runtime::hypot(x, y)
    );
}

// rounding and decimals
[[nodiscard]] BL_FORCE_INLINE constexpr fqd floor(const fqd_s& a)
{
    return detail::_qd::floor_limbwise(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd ceil(const fqd_s& a)
{
    return detail::_qd::ceil_limbwise(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd trunc(const fqd_s& a)
{
    return detail::_qd_impl::trunc(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd round(const fqd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::round_nearest_away_from_zero(a),
        detail::_qd_runtime::round_nearest_away_from_zero(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd roundeven(const fqd_s& x)
{
    return detail::_qd_impl::round_nearest_even(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd round_decimals(fqd_s v, int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::round_decimals(v, precision),
        detail::_qd_runtime::round_decimals(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd round_significant(
    fqd_s v,
    int precision)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::round_significant(v, precision),
        detail::_qd_runtime::round_significant(v, precision)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long lround(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::lround_nearest_away_from_zero(x),
        detail::_qd_runtime::lround_nearest_away_from_zero(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long llround(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::llround_nearest_away_from_zero(x),
        detail::_qd_runtime::llround_nearest_away_from_zero(x)
    );
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr fqd fma(const fqd_s& x, const fqd_s& y, const fqd_s& z)
{
#if defined(__EMSCRIPTEN__) && defined(__clang__) && defined(__wasm32__) && \
    defined(FLTX_FAST_MATH)
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::fma(x, y, z),
        detail::_qd_runtime::fma(x, y, z)
    );
#elif FLTX_GUARDED_X86_FMA
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::fma(x, y, z),
        detail::_qd_runtime::fma(x, y, z)
    );
#elif FLTX_HAS_COMPILED_X86_FMA_BACKEND && !FLTX_TU_HAS_X86_FMA
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::fma(x, y, z),
        detail::fp::runtime_hardware_fma_enabled()
            ? detail::_qd_runtime::fma_x86(x, y, z)
            : detail::_qd_impl::fma(x, y, z)
    );
#else
    return detail::_qd_impl::fma(x, y, z);
#endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd fmin(const fqd_s& a, const fqd_s& b)
{
    return detail::_qd_impl::fmin(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd fmax(const fqd_s& a, const fqd_s& b)
{
    return detail::_qd_impl::fmax(a, b);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd fdim(const fqd_s& x, const fqd_s& y)
{
    return detail::_qd_impl::fdim(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd copysign(const fqd_s& x, const fqd_s& y)
{
    return detail::_qd_impl::copysign(x, y);
}

// remainders
[[nodiscard]] BL_FORCE_INLINE constexpr fqd fmod(const fqd_s& x, const fqd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::fmod(x, y),
        detail::_qd_runtime::fmod(x, y)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd remainder(const fqd_s& x, const fqd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::remquo(x, y, nullptr),
        detail::_qd_runtime::remquo(x, y, nullptr)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd remquo(const fqd_s& x, const fqd_s& y, int* quo)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::remquo(x, y, quo),
        detail::_qd_runtime::remquo(x, y, quo)
    );
}

// fractional decomposition
[[nodiscard]] BL_FORCE_INLINE constexpr fqd modf(const fqd_s& x, fqd_s* iptr) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::modf(x, iptr),
        detail::_qd_runtime::modf(x, iptr)
    );
}

// decomposition and scaling
[[nodiscard]] BL_FORCE_INLINE constexpr fqd ldexp(const fqd_s& a, int e)
{
    return detail::_qd_impl::ldexp(a, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd frexp(const fqd_s& x, int* exp) noexcept
{
    return detail::_qd_impl::frexp(x, exp);
}

[[nodiscard]] BL_FORCE_INLINE constexpr int ilogb(const fqd_s& x) noexcept
{
    return detail::_qd_impl::ilogb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd logb(const fqd_s& x) noexcept
{
    return detail::_qd_impl::logb(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd scalbn(const fqd_s& x, int e) noexcept
{
    return detail::_qd_impl::ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd scalbln(const fqd_s& x, long e) noexcept
{
    return detail::_qd_impl::ldexp(x, static_cast<int>(e));
}

// adjacent values
[[nodiscard]] BL_FORCE_INLINE constexpr fqd nextafter(const fqd_s& from, const fqd_s& to) noexcept
{
    return detail::_qd_impl::nextafter(from, to);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd nexttoward(const fqd_s& from, long double to) noexcept
{
    const double high = static_cast<double>(to);
    if (detail::fp::isinf_or_nan(high))
        return detail::_qd_impl::nextafter(from, fqd_s{ high, 0.0, 0.0, 0.0 });
    if (high == 0.0 && to != 0.0L)
    {
        const double target = to < 0.0L
            ? -std::numeric_limits<double>::denorm_min()
            : std::numeric_limits<double>::denorm_min();
        return detail::_qd_impl::nextafter(from, fqd_s{ target, 0.0, 0.0, 0.0 });
    }

    const double low = static_cast<double>(to - static_cast<long double>(high));
    return detail::_qd_impl::nextafter(from, fqd_s{ high, low, 0.0, 0.0 });
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd nexttoward(const fqd_s& from, const fqd_s& to) noexcept
{
    return detail::_qd_impl::nextafter(from, to);
}

// exp / log
[[nodiscard]] BL_FORCE_INLINE constexpr double log_as_double(fqd_s a) noexcept
{
    return detail::_qd_impl::log_as_double(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd exp(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::exp(x),
        detail::_qd_runtime::exp(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd exp2(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::exp2(x),
        detail::_qd_runtime::exp2(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd log(const fqd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::log(a),
        detail::_qd_runtime::log(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd log2(const fqd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::log2(a),
        detail::_qd_runtime::log2(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd log10(const fqd_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::log10(a),
        detail::_qd_runtime::log10(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd expm1(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::expm1(x),
        detail::_qd_runtime::expm1(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd log1p(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::log1p(x),
        detail::_qd_runtime::log1p(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd pow(const fqd_s& x, const fqd_s& y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::pow(x, y),
        detail::_qd_runtime::pow(x, y)
    );
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_MSVC_NOINLINE constexpr fqd ipow(const fqd_s& x, Exp y)
{
    if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            fqd{ detail::_qd::ipow_integer(x, y) },
            fqd{ detail::_qd_runtime::ipow_signed(x, static_cast<std::intmax_t>(y)) }
        );
    }
    else
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            fqd{ detail::_qd::ipow_integer(x, y) },
            fqd{ detail::_qd_runtime::ipow_unsigned(x, static_cast<std::uintmax_t>(y)) }
        );
    }
}

template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_FORCE_INLINE constexpr fqd pow(const fqd_s& x, Exp y)
{
    return bl::ipow(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd pow(const fqd_s& x, double y)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::pow(x, y),
        detail::_qd_runtime::pow(x, y)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd pow(const fqd_s& x, const fdd_s& y)
{
    fqd_s promoted{};
    promoted = y;
    return bl::pow(x, promoted);
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd pow(const fqd_s&, long double) = delete;

template<class Base, class Exp>
requires (detail::math::qd_promoted_math_args<Base, Exp> &&
    !(fltx_fqd<Base> && detail::fp::non_bool_integral<Exp>))
[[nodiscard]] BL_FORCE_INLINE constexpr auto pow(Base x, Exp y)
{
    using P = detail::math::promoted_t<Base, Exp>;
    using E = detail::math::promoted_pow_exponent_t<P, Exp>;
    return bl::pow(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<E>(y));
}

// trig
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const fqd_s& x, fqd_s& s_out, fqd_s& c_out)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::sincos(x, s_out, c_out),
        detail::_qd_runtime::sincos(x, s_out, c_out)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd sin(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::sin(x),
        detail::_qd_runtime::sin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd cos(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::cos(x),
        detail::_qd_runtime::cos(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd tan(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::tan(x),
        detail::_qd_runtime::tan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd atan(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::atan(x),
        detail::_qd_runtime::atan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd atan2(const fqd_s& y, const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::atan2(y, x),
        detail::_qd_runtime::atan2(y, x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd asin(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::asin(x),
        detail::_qd_runtime::asin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd acos(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::acos(x),
        detail::_qd_runtime::acos(x)
    );
}

template<class Vec>
    requires detail::fp::sincos_vector_assignable<Vec, fqd_s>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(const fqd_s& x, Vec& out)
{
    fqd_s s_out{};
    fqd_s c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    detail::fp::assign_sincos_vector(out, s_out, c_out);
    return ok;
}

template<class Value> requires (std::same_as<std::remove_cvref_t<Value>, fqd> || std::same_as<std::remove_cvref_t<Value>, fqd_s>)
[[nodiscard]] BL_FORCE_INLINE constexpr detail::fp::sincos_vector_result<std::remove_cvref_t<Value>> sincos(const fqd_s& x)
{
    using Result = std::remove_cvref_t<Value>;

    Result s_out{};
    Result c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    return detail::fp::make_sincos_result(s_out, c_out, ok);
}

// hyperbolic
[[nodiscard]] BL_FORCE_INLINE constexpr fqd sinh(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::sinh(x),
        detail::_qd_runtime::sinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd cosh(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::cosh(x),
        detail::_qd_runtime::cosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd tanh(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::tanh(x),
        detail::_qd_runtime::tanh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd asinh(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::asinh(x),
        detail::_qd_runtime::asinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd acosh(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::acosh(x),
        detail::_qd_runtime::acosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd atanh(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::atanh(x),
        detail::_qd_runtime::atanh(x)
    );
}

// erf / erfc
[[nodiscard]] BL_FORCE_INLINE constexpr fqd erf(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::erf(x),
        detail::_qd_runtime::erf(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd erfc(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::erfc(x),
        detail::_qd_runtime::erfc(x)
    );
}

// gamma
[[nodiscard]] BL_FORCE_INLINE constexpr fqd lgamma(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::lgamma(x),
        detail::_qd_runtime::lgamma(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr fqd tgamma(const fqd_s& x)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_qd_impl::tgamma(x),
        detail::_qd_runtime::tgamma(x)
    );
}

// type promotions
#define FLTX_QD_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::qd_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define FLTX_QD_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::qd_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define FLTX_QD_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::qd_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

FLTX_QD_PROMOTED_UNARY(abs)
FLTX_QD_PROMOTED_UNARY(fabs)
FLTX_QD_PROMOTED_UNARY(signbit)
FLTX_QD_PROMOTED_UNARY(isnan)
FLTX_QD_PROMOTED_UNARY(isinf)
FLTX_QD_PROMOTED_UNARY(isfinite)
FLTX_QD_PROMOTED_UNARY(iszero)
FLTX_QD_PROMOTED_UNARY(fpclassify)
FLTX_QD_PROMOTED_UNARY(isnormal)

FLTX_QD_PROMOTED_UNARY(floor)
FLTX_QD_PROMOTED_UNARY(ceil)
FLTX_QD_PROMOTED_UNARY(trunc)
FLTX_QD_PROMOTED_UNARY(round)
FLTX_QD_PROMOTED_UNARY(roundeven)
FLTX_QD_PROMOTED_UNARY(lround)
FLTX_QD_PROMOTED_UNARY(llround)

FLTX_QD_PROMOTED_BINARY(fmod)
FLTX_QD_PROMOTED_BINARY(remainder)
FLTX_QD_PROMOTED_TERNARY(fma)
FLTX_QD_PROMOTED_BINARY(fmin)
FLTX_QD_PROMOTED_BINARY(fmax)
FLTX_QD_PROMOTED_BINARY(fdim)
FLTX_QD_PROMOTED_BINARY(copysign)

FLTX_QD_PROMOTED_UNARY(ilogb)
FLTX_QD_PROMOTED_UNARY(logb)
FLTX_QD_PROMOTED_BINARY(nextafter)

FLTX_QD_PROMOTED_UNARY(exp)
FLTX_QD_PROMOTED_UNARY(exp2)
FLTX_QD_PROMOTED_UNARY(expm1)
FLTX_QD_PROMOTED_UNARY(log)
FLTX_QD_PROMOTED_UNARY(log2)
FLTX_QD_PROMOTED_UNARY(log10)
FLTX_QD_PROMOTED_UNARY(log1p)
FLTX_QD_PROMOTED_UNARY(sqrt)
FLTX_QD_PROMOTED_UNARY(cbrt)
FLTX_QD_PROMOTED_BINARY(hypot)

FLTX_QD_PROMOTED_UNARY(sin)
FLTX_QD_PROMOTED_UNARY(cos)
FLTX_QD_PROMOTED_UNARY(tan)
FLTX_QD_PROMOTED_UNARY(atan)
FLTX_QD_PROMOTED_BINARY(atan2)
FLTX_QD_PROMOTED_UNARY(asin)
FLTX_QD_PROMOTED_UNARY(acos)

FLTX_QD_PROMOTED_UNARY(sinh)
FLTX_QD_PROMOTED_UNARY(cosh)
FLTX_QD_PROMOTED_UNARY(tanh)
FLTX_QD_PROMOTED_UNARY(asinh)
FLTX_QD_PROMOTED_UNARY(acosh)
FLTX_QD_PROMOTED_UNARY(atanh)

FLTX_QD_PROMOTED_UNARY(erf)
FLTX_QD_PROMOTED_UNARY(erfc)
FLTX_QD_PROMOTED_UNARY(lgamma)
FLTX_QD_PROMOTED_UNARY(tgamma)

FLTX_QD_PROMOTED_BINARY(isunordered)
FLTX_QD_PROMOTED_BINARY(isgreater)
FLTX_QD_PROMOTED_BINARY(isgreaterequal)
FLTX_QD_PROMOTED_BINARY(isless)
FLTX_QD_PROMOTED_BINARY(islessequal)
FLTX_QD_PROMOTED_BINARY(islessgreater)

#undef FLTX_QD_PROMOTED_TERNARY
#undef FLTX_QD_PROMOTED_BINARY
#undef FLTX_QD_PROMOTED_UNARY

template<class T, class U>
requires detail::math::qd_promoted_math_args<T, U>
[[nodiscard]] BL_FORCE_INLINE constexpr auto remquo(T x, U y, int* quo)
{
    using P = detail::math::promoted_t<T, U>;
    return bl::remquo(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), quo);
}

template<class T>
requires detail::math::qd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto ldexp(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::ldexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::qd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbn(T x, int exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbn(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::qd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto scalbln(T x, long exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::scalbln(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::qd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto frexp(T x, int* exp)
{
    using P = detail::math::promoted_t<T>;
    return bl::frexp(detail::math::promoted_cast<P>(x), exp);
}

template<class T>
requires detail::math::qd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr auto modf(T x, detail::math::promoted_t<T>* iptr)
{
    using P = detail::math::promoted_t<T>;
    return bl::modf(detail::math::promoted_cast<P>(x), iptr);
}

template<class From, class To>
requires detail::math::qd_nexttoward_args<From, To>
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
requires detail::math::qd_promoted_math_args<T>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(T x, detail::math::promoted_t<T>& s_out, detail::math::promoted_t<T>& c_out)
{
    using P = detail::math::promoted_t<T>;
    return bl::sincos(detail::math::promoted_cast<P>(x), s_out, c_out);
}

} // namespace bl

#endif
