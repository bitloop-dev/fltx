/**
 * fltx/f64_math.h - constexpr <cmath>-style functions for f64.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F64_MATH_INCLUDED
#define F64_MATH_INCLUDED

#include "fltx/detail/math_promotion.h"
#include "fltx/detail/f64_math_basic.h"
#include "fltx/detail/f64_math_transcendental.h"

namespace bl
{

#define BL_FLTX_F64_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::f64_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define BL_FLTX_F64_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::f64_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define BL_FLTX_F64_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::f64_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

BL_FLTX_F64_PROMOTED_UNARY(abs)
BL_FLTX_F64_PROMOTED_UNARY(fabs)
BL_FLTX_F64_PROMOTED_UNARY(signbit)
BL_FLTX_F64_PROMOTED_UNARY(isnan)
BL_FLTX_F64_PROMOTED_UNARY(isinf)
BL_FLTX_F64_PROMOTED_UNARY(isfinite)
BL_FLTX_F64_PROMOTED_UNARY(iszero)
BL_FLTX_F64_PROMOTED_UNARY(fpclassify)
BL_FLTX_F64_PROMOTED_UNARY(isnormal)

BL_FLTX_F64_PROMOTED_UNARY(floor)
BL_FLTX_F64_PROMOTED_UNARY(ceil)
BL_FLTX_F64_PROMOTED_UNARY(trunc)
BL_FLTX_F64_PROMOTED_UNARY(round)
BL_FLTX_F64_PROMOTED_UNARY(nearbyint)
BL_FLTX_F64_PROMOTED_UNARY(rint)
BL_FLTX_F64_PROMOTED_UNARY(lround)
BL_FLTX_F64_PROMOTED_UNARY(llround)
BL_FLTX_F64_PROMOTED_UNARY(lrint)
BL_FLTX_F64_PROMOTED_UNARY(llrint)

BL_FLTX_F64_PROMOTED_BINARY(fmod)
BL_FLTX_F64_PROMOTED_BINARY(remainder)
BL_FLTX_F64_PROMOTED_TERNARY(fma)
BL_FLTX_F64_PROMOTED_BINARY(fmin)
BL_FLTX_F64_PROMOTED_BINARY(fmax)
BL_FLTX_F64_PROMOTED_BINARY(fdim)
BL_FLTX_F64_PROMOTED_BINARY(copysign)

BL_FLTX_F64_PROMOTED_UNARY(ilogb)
BL_FLTX_F64_PROMOTED_UNARY(logb)
BL_FLTX_F64_PROMOTED_BINARY(nextafter)

BL_FLTX_F64_PROMOTED_UNARY(exp)
BL_FLTX_F64_PROMOTED_UNARY(exp2)
BL_FLTX_F64_PROMOTED_UNARY(expm1)
BL_FLTX_F64_PROMOTED_UNARY(log)
BL_FLTX_F64_PROMOTED_UNARY(log2)
BL_FLTX_F64_PROMOTED_UNARY(log10)
BL_FLTX_F64_PROMOTED_UNARY(log1p)
BL_FLTX_F64_PROMOTED_UNARY(sqrt)
BL_FLTX_F64_PROMOTED_UNARY(cbrt)
BL_FLTX_F64_PROMOTED_BINARY(hypot)

BL_FLTX_F64_PROMOTED_UNARY(sin)
BL_FLTX_F64_PROMOTED_UNARY(cos)
BL_FLTX_F64_PROMOTED_UNARY(tan)
BL_FLTX_F64_PROMOTED_UNARY(atan)
BL_FLTX_F64_PROMOTED_BINARY(atan2)
BL_FLTX_F64_PROMOTED_UNARY(asin)
BL_FLTX_F64_PROMOTED_UNARY(acos)

BL_FLTX_F64_PROMOTED_UNARY(sinh)
BL_FLTX_F64_PROMOTED_UNARY(cosh)
BL_FLTX_F64_PROMOTED_UNARY(tanh)
BL_FLTX_F64_PROMOTED_UNARY(asinh)
BL_FLTX_F64_PROMOTED_UNARY(acosh)
BL_FLTX_F64_PROMOTED_UNARY(atanh)

BL_FLTX_F64_PROMOTED_UNARY(erf)
BL_FLTX_F64_PROMOTED_UNARY(erfc)
BL_FLTX_F64_PROMOTED_UNARY(lgamma)
BL_FLTX_F64_PROMOTED_UNARY(tgamma)

BL_FLTX_F64_PROMOTED_BINARY(isunordered)
BL_FLTX_F64_PROMOTED_BINARY(isgreater)
BL_FLTX_F64_PROMOTED_BINARY(isgreaterequal)
BL_FLTX_F64_PROMOTED_BINARY(isless)
BL_FLTX_F64_PROMOTED_BINARY(islessequal)
BL_FLTX_F64_PROMOTED_BINARY(islessgreater)

#undef BL_FLTX_F64_PROMOTED_TERNARY
#undef BL_FLTX_F64_PROMOTED_BINARY
#undef BL_FLTX_F64_PROMOTED_UNARY

    template<class T, class U>
    requires detail::math::f64_promoted_math_args<T, U>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto remquo(T x, U y, int* quo)
    {
        using P = detail::math::promoted_t<T, U>;
        return bl::remquo(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), quo);
    }

    template<class T>
    requires detail::math::f64_promoted_math_args<T>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto ldexp(T x, int exp)
    {
        using P = detail::math::promoted_t<T>;
        return bl::ldexp(detail::math::promoted_cast<P>(x), exp);
    }

    template<class T>
    requires detail::math::f64_promoted_math_args<T>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto scalbn(T x, int exp)
    {
        using P = detail::math::promoted_t<T>;
        return bl::scalbn(detail::math::promoted_cast<P>(x), exp);
    }

    template<class T>
    requires detail::math::f64_promoted_math_args<T>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto scalbln(T x, long exp)
    {
        using P = detail::math::promoted_t<T>;
        return bl::scalbln(detail::math::promoted_cast<P>(x), exp);
    }

    template<class T>
    requires detail::math::f64_promoted_math_args<T>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto frexp(T x, int* exp)
    {
        using P = detail::math::promoted_t<T>;
        return bl::frexp(detail::math::promoted_cast<P>(x), exp);
    }

    template<class T>
    requires detail::math::f64_promoted_math_args<T>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto modf(T x, detail::math::promoted_t<T>* iptr)
    {
        using P = detail::math::promoted_t<T>;
        return bl::modf(detail::math::promoted_cast<P>(x), iptr);
    }

    template<class From, class To>
    requires detail::math::f64_nexttoward_args<From, To>
    [[nodiscard]] BL_FORCE_INLINE constexpr auto nexttoward(From from, To to)
    {
        using P = detail::math::promoted_t<From>;
        return bl::nexttoward(detail::math::promoted_cast<P>(from), static_cast<long double>(to));
    }

    template<class T>
    requires detail::math::f64_promoted_math_args<T>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(T x, detail::math::promoted_t<T>& s_out, detail::math::promoted_t<T>& c_out)
    {
        using P = detail::math::promoted_t<T>;
        return bl::sincos(detail::math::promoted_cast<P>(x), s_out, c_out);
    }

} // namespace bl

#endif
