/**
 * fltx/detail/f32_math_transcendental.h - constexpr transcendental functions for f32.
 *
 * f32 exp/log, roots, pow, trig, hyperbolic, erf, and gamma helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F32_MATH_TRANSCENDENTAL_INCLUDED
#define F32_MATH_TRANSCENDENTAL_INCLUDED

#include "fltx/detail/f32_math_basic.h"
#include "fltx/detail/f64_math_transcendental.h"
#include "fltx/traits.h"


namespace bl {

namespace detail::_f32_runtime
{
    [[nodiscard]] BL_FORCE_INLINE float erfc(float x) noexcept
    {
#if defined(__MINGW32__)
        return static_cast<float>(std::erfc(static_cast<double>(x)));
#else
        return std::erfc(x);
#endif
    }

} // namespace detail::_f32_runtime


// exp / log
[[nodiscard]] BL_FORCE_INLINE constexpr float exp(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::exp(static_cast<double>(x))),
        std::exp(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float exp2(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::exp2(static_cast<double>(x))),
        std::exp2(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float expm1(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::expm1(static_cast<double>(x))),
        std::expm1(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr double log_as_double(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        bl::log(static_cast<double>(x)),
        std::log(static_cast<double>(x))
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float log(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::log(static_cast<double>(x))),
        std::log(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float log2(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::log2(static_cast<double>(x))),
        std::log2(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float log10(float x) noexcept
{
    if (detail::fp::isfinite(x) && x > 0.0f)
    {
        const int exp2 = detail::fp::frexp_exponent(static_cast<double>(x));
        const int k0 = static_cast<int>(detail::fp::floor((exp2 - 1) * 0.30102999566398114));

        for (int k = k0 - 2; k <= k0 + 2; ++k)
        {
            if (0 <= k && k <= 10 && x == detail::_f32_impl::pow10(k))
                return static_cast<float>(k);
        }
    }

    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::log10(static_cast<double>(x))),
        std::log10(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float log1p(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::log1p(static_cast<double>(x))),
        std::log1p(x)
    );
}


// roots
[[nodiscard]] BL_FORCE_INLINE constexpr float sqrt(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::sqrt(static_cast<double>(x))),
        std::sqrt(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float cbrt(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::cbrt(static_cast<double>(x))),
        std::cbrt(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float hypot(float x, float y) noexcept
{
    if (detail::fp::isinf(x) || detail::fp::isinf(y))
        return std::bit_cast<float>(0x7f800000u);
    if (detail::fp::isnan(x) || detail::fp::isnan(y))
        return std::bit_cast<float>(0x7fc00000u);

    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::hypot(static_cast<double>(x), static_cast<double>(y))),
        std::hypot(x, y)
    );
}


// pow
[[nodiscard]] BL_FORCE_INLINE constexpr float pow(float x, float y) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::pow(static_cast<double>(x), static_cast<double>(y))),
        std::pow(x, y)
    );
}

BL_PUSH_PRECISE;
template<detail::fp::non_bool_integral Exp>
[[nodiscard]] BL_FORCE_INLINE constexpr float ipow(float x, Exp y) noexcept
{
    int exponent = 0;
    if (detail::fp::try_int_exponent(y, exponent))
    {
        const bool negative_base = x < 0.0f;
        const float magnitude = negative_base ? -x : x;
        float value{};

        if (magnitude == 2.0f)
        {
            if (exponent >= std::numeric_limits<float>::max_exponent)
                value = std::numeric_limits<float>::infinity();
            else if (exponent < std::numeric_limits<float>::min_exponent - std::numeric_limits<float>::digits)
                value = 0.0f;
            else
                value = static_cast<float>(detail::fp::ldexp(1.0, exponent));

            return (negative_base && detail::fp::is_odd_integral(y)) ? -value : value;
        }

        if (magnitude == 10.0f)
        {
            value = detail::_f32_impl::pow10(exponent);
            return (negative_base && detail::fp::is_odd_integral(y)) ? -value : value;
        }
    }

    using U = std::make_unsigned_t<std::remove_cvref_t<Exp>>;
    const U magnitude = detail::fp::unsigned_abs(y);
    const float powered = detail::fp::ipow_nonneg_fast<float, U>(x, magnitude);

    if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
    {
        if (y < 0)
            return 1.0f / powered;
    }

    return powered;
}
BL_POP_PRECISE;

// trig
[[nodiscard]] BL_FORCE_INLINE constexpr float sin(float x) noexcept
{
    if (bl::detail::is_constant_evaluated())
        return static_cast<float>(bl::sin(static_cast<double>(x)));
#if defined(__MINGW32__)
    if (std::fabs(x) >= 0x1p12f) [[unlikely]]
        return static_cast<float>(
            detail::_f64_runtime::sin_large(static_cast<double>(x)));
#endif
    return std::sin(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr float cos(float x) noexcept
{
    if (bl::detail::is_constant_evaluated())
        return static_cast<float>(bl::cos(static_cast<double>(x)));
#if defined(__MINGW32__)
    if (std::fabs(x) >= 0x1p12f) [[unlikely]]
        return static_cast<float>(
            detail::_f64_runtime::cos_large(static_cast<double>(x)));
#endif
    return std::cos(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(float x, float& s_out, float& c_out) noexcept
{
    s_out = bl::sin(x);
    c_out = bl::cos(x);
    return bl::isfinite(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr float tan(float x) noexcept
{
    if (bl::detail::is_constant_evaluated())
        return static_cast<float>(bl::tan(static_cast<double>(x)));
#if defined(__MINGW32__)
    if (std::fabs(x) >= 0x1p12f) [[unlikely]]
        return static_cast<float>(
            detail::_f64_runtime::tan_large(static_cast<double>(x)));
#endif
    return std::tan(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr float atan(float x) noexcept
{
#if defined(__EMSCRIPTEN__)
    if (!detail::fp::isfinite(x)) [[unlikely]]
        return static_cast<float>(bl::atan(static_cast<double>(x)));
#endif
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::atan(static_cast<double>(x))),
        std::atan(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float atan2(float y, float x) noexcept
{
#if defined(__EMSCRIPTEN__)
    if (!detail::fp::isfinite(x) || !detail::fp::isfinite(y)) [[unlikely]]
        return static_cast<float>(
            bl::atan2(static_cast<double>(y), static_cast<double>(x)));
#endif
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::atan2(static_cast<double>(y), static_cast<double>(x))),
        std::atan2(y, x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float asin(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::asin(static_cast<double>(x))),
        std::asin(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float acos(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::acos(static_cast<double>(x))),
        std::acos(x)
    );
}

template<class Vec> requires detail::fp::sincos_vector_assignable<Vec, float>
[[nodiscard]] BL_FORCE_INLINE constexpr bool sincos(float x, Vec& out)
{
    float s_out{};
    float c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    detail::fp::assign_sincos_vector(out, s_out, c_out);
    return ok;
}

template<class Value> requires std::same_as<std::remove_cvref_t<Value>, float>
[[nodiscard]] BL_FORCE_INLINE constexpr detail::fp::sincos_vector_result<float> sincos(float x)
{
    float s_out{};
    float c_out{};
    const bool ok = bl::sincos(x, s_out, c_out);
    return detail::fp::make_sincos_result(s_out, c_out, ok);
}


// hyperbolic
[[nodiscard]] BL_FORCE_INLINE constexpr float sinh(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::sinh(static_cast<double>(x))),
        std::sinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float cosh(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::cosh(static_cast<double>(x))),
        std::cosh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float tanh(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::tanh(static_cast<double>(x))),
        std::tanh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float asinh(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::asinh(static_cast<double>(x))),
        std::asinh(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float acosh(float x) noexcept
{
    if (bl::detail::is_constant_evaluated())
        return static_cast<float>(bl::acosh(static_cast<double>(x)));
#if defined(__MINGW32__)
    if (x < 1.5f)
        return static_cast<float>(
            detail::_f64_impl::acosh(static_cast<double>(x)));
#endif
    return std::acosh(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr float atanh(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::atanh(static_cast<double>(x))),
        std::atanh(x)
    );
}


// erf / erfc
[[nodiscard]] BL_FORCE_INLINE constexpr float erf(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::erf(static_cast<double>(x))),
        std::erf(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float erfc(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::erfc(static_cast<double>(x))),
        detail::_f32_runtime::erfc(x)
    );
}


// gamma
[[nodiscard]] BL_FORCE_INLINE constexpr float lgamma(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::lgamma(static_cast<double>(x))),
        std::lgamma(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr float tgamma(float x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        static_cast<float>(bl::tgamma(static_cast<double>(x))),
        std::tgamma(x)
    );
}

} // namespace bl

#endif
