/**
 * fltx/f64_math.h - constexpr counterparts to selected <cmath> functions for f64.
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

#define FLTX_F64_PROMOTED_UNARY(NAME) \
    template<class T> \
    requires detail::math::f64_promoted_math_args<T> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x) \
    { \
        using P = detail::math::promoted_t<T>; \
        return bl::NAME(detail::math::promoted_cast<P>(x)); \
    }

#define FLTX_F64_PROMOTED_BINARY(NAME) \
    template<class T, class U> \
    requires detail::math::f64_promoted_math_args<T, U> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y) \
    { \
        using P = detail::math::promoted_t<T, U>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y)); \
    }

#define FLTX_F64_PROMOTED_TERNARY(NAME) \
    template<class T, class U, class V> \
    requires detail::math::f64_promoted_math_args<T, U, V> \
    [[nodiscard]] BL_FORCE_INLINE constexpr auto NAME(T x, U y, V z) \
    { \
        using P = detail::math::promoted_t<T, U, V>; \
        return bl::NAME(detail::math::promoted_cast<P>(x), detail::math::promoted_cast<P>(y), detail::math::promoted_cast<P>(z)); \
    }

FLTX_F64_PROMOTED_UNARY(abs)
FLTX_F64_PROMOTED_UNARY(fabs)
FLTX_F64_PROMOTED_UNARY(signbit)
FLTX_F64_PROMOTED_UNARY(isnan)
FLTX_F64_PROMOTED_UNARY(isinf)
FLTX_F64_PROMOTED_UNARY(isfinite)
FLTX_F64_PROMOTED_UNARY(iszero)
FLTX_F64_PROMOTED_UNARY(fpclassify)
FLTX_F64_PROMOTED_UNARY(isnormal)

FLTX_F64_PROMOTED_UNARY(floor)
FLTX_F64_PROMOTED_UNARY(ceil)
FLTX_F64_PROMOTED_UNARY(trunc)
FLTX_F64_PROMOTED_UNARY(round)
FLTX_F64_PROMOTED_UNARY(roundeven)
FLTX_F64_PROMOTED_UNARY(lround)
FLTX_F64_PROMOTED_UNARY(llround)

FLTX_F64_PROMOTED_BINARY(fmod)
FLTX_F64_PROMOTED_BINARY(remainder)
FLTX_F64_PROMOTED_TERNARY(fma)
FLTX_F64_PROMOTED_BINARY(fmin)
FLTX_F64_PROMOTED_BINARY(fmax)
FLTX_F64_PROMOTED_BINARY(fdim)
FLTX_F64_PROMOTED_BINARY(copysign)

FLTX_F64_PROMOTED_UNARY(ilogb)
FLTX_F64_PROMOTED_UNARY(logb)
FLTX_F64_PROMOTED_BINARY(nextafter)

FLTX_F64_PROMOTED_UNARY(exp)
FLTX_F64_PROMOTED_UNARY(exp2)
FLTX_F64_PROMOTED_UNARY(expm1)
FLTX_F64_PROMOTED_UNARY(log)
FLTX_F64_PROMOTED_UNARY(log2)
FLTX_F64_PROMOTED_UNARY(log10)
FLTX_F64_PROMOTED_UNARY(log1p)
FLTX_F64_PROMOTED_UNARY(sqrt)
FLTX_F64_PROMOTED_UNARY(cbrt)
FLTX_F64_PROMOTED_BINARY(hypot)

FLTX_F64_PROMOTED_UNARY(sin)
FLTX_F64_PROMOTED_UNARY(cos)
FLTX_F64_PROMOTED_UNARY(tan)
FLTX_F64_PROMOTED_UNARY(atan)
FLTX_F64_PROMOTED_BINARY(atan2)
FLTX_F64_PROMOTED_UNARY(asin)
FLTX_F64_PROMOTED_UNARY(acos)

FLTX_F64_PROMOTED_UNARY(sinh)
FLTX_F64_PROMOTED_UNARY(cosh)
FLTX_F64_PROMOTED_UNARY(tanh)
FLTX_F64_PROMOTED_UNARY(asinh)
FLTX_F64_PROMOTED_UNARY(acosh)
FLTX_F64_PROMOTED_UNARY(atanh)

FLTX_F64_PROMOTED_UNARY(erf)
FLTX_F64_PROMOTED_UNARY(erfc)
FLTX_F64_PROMOTED_UNARY(lgamma)
FLTX_F64_PROMOTED_UNARY(tgamma)

FLTX_F64_PROMOTED_BINARY(isunordered)
FLTX_F64_PROMOTED_BINARY(isgreater)
FLTX_F64_PROMOTED_BINARY(isgreaterequal)
FLTX_F64_PROMOTED_BINARY(isless)
FLTX_F64_PROMOTED_BINARY(islessequal)
FLTX_F64_PROMOTED_BINARY(islessgreater)

#undef FLTX_F64_PROMOTED_TERNARY
#undef FLTX_F64_PROMOTED_BINARY
#undef FLTX_F64_PROMOTED_UNARY

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
