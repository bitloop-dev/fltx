/**
 * fltx/detail/f256_math_basic.h - Basic math implementation details.
 *
 * f256 rounding, decomposition, remainder, hypot, and adjacent-value implementations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F256_DETAIL_MATH_BASIC_INCLUDED
#define F256_DETAIL_MATH_BASIC_INCLUDED
#include "fltx/detail/f256_math_kernels.h"
#include "fltx/detail/pow_tables.h"
#include "fltx/detail/simd.h"

namespace bl {

namespace detail::_f256
{
    inline constexpr int pow10_f256_min_exponent = detail::pow_tables::pow10_min_exponent;
    inline constexpr int pow10_f256_max_exponent = detail::pow_tables::pow10_max_exponent;

    static_assert(sizeof(detail::pow_table_entry) == sizeof(f256_s));
    static_assert(alignof(detail::pow_table_entry) == alignof(f256_s));

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s pow_table_entry_to_f256(const detail::pow_table_entry& row) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            (f256_s{ row.x0, row.x1, row.x2, row.x3 }),
            std::bit_cast<f256_s>(row)
        );
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr double floor_limb(double x) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::fp::floor(x),
            std::floor(x)
        );
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr double ceil_limb(double x) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::fp::ceil(x),
            std::ceil(x)
        );
    }
}

namespace detail::_f256_runtime
{
    [[nodiscard]] BL_FORCE_INLINE double trunc_limb(double x) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(x))
            return x;

        const double ax = detail::fp::absd(x);
        if (ax >= detail::fp::double_integer_threshold)
            return x;

        const double out = static_cast<double>(static_cast<long long>(x));
        return out == 0.0 ? (detail::fp::signbit(x) ? -0.0 : 0.0) : out;
    }
}

namespace detail::_f256
{
    [[nodiscard]] BL_FORCE_INLINE constexpr double trunc_limb(double x) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::fp::trunc(x),
            detail::_f256_runtime::trunc_limb(x)
        );
    }
}

namespace detail::_f256_runtime
{
    [[nodiscard]] BL_FORCE_INLINE double round_nearest_even_limb(double x) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(x))
            return x;

        const double t = detail::_f256::floor_limb(x);
        const double frac = x - t;
        double out = t;
        if (frac > 0.5 || (frac == 0.5 && detail::fp::double_integer_is_odd(t)))
            out = t + 1.0;

        return out == 0.0 ? (detail::fp::signbit(x) ? -0.0 : 0.0) : out;
    }
}

namespace detail::_f256
{
    [[nodiscard]] BL_FORCE_INLINE constexpr double round_nearest_even_limb(double x) noexcept
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::fp::round_nearest_even(x),
            detail::_f256_runtime::round_nearest_even_limb(x)
        );
    }

}

namespace detail::_f256
{
    [[nodiscard]] BL_FORCE_INLINE double round_nearest_away_from_zero_limb_finite_small(
        double x) noexcept
    {
        return detail::fp::round_nearest_away_from_zero(x);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool has_negative_tail(double x1, double x2, double x3) noexcept
    {
        return x1 < 0.0 || (x1 == 0.0 && (x2 < 0.0 || (x2 == 0.0 && x3 < 0.0)));
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool has_positive_tail(double x1, double x2, double x3) noexcept
    {
        return x1 > 0.0 || (x1 == 0.0 && (x2 > 0.0 || (x2 == 0.0 && x3 > 0.0)));
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr int ilogb_finite_fast(const f256_s& x) noexcept
    {
        const double lead =
            x.x0 != 0.0 ? x.x0 :
            x.x1 != 0.0 ? x.x1 :
            x.x2 != 0.0 ? x.x2 : x.x3;
        constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
        const std::uint64_t lead_bits = std::bit_cast<std::uint64_t>(lead);
        const std::uint32_t exponent_bits =
            static_cast<std::uint32_t>((lead_bits >> 52) & 0x7ffu);
        const std::uint64_t fraction = lead_bits & fraction_mask;

        int exponent;
        bool lead_is_power;
        if (exponent_bits != 0)
        {
            exponent = static_cast<int>(exponent_bits) - 1023;
            lead_is_power = fraction == 0;
        }
        else
        {
            exponent = detail::fp::highest_bit_index(fraction) - 1074;
            lead_is_power = (fraction & (fraction - 1)) == 0;
        }

        // Only an exact leading power of two can be pulled into the lower
        // binade by an oppositely signed expansion tail. Keep the ordinary
        // path independent of the more detailed nextafter analysis.
        if (lead_is_power) [[unlikely]]
        {
            const double tail =
                x.x0 != 0.0
                    ? (x.x1 != 0.0 ? x.x1 : (x.x2 != 0.0 ? x.x2 : x.x3))
                    : x.x1 != 0.0
                        ? (x.x2 != 0.0 ? x.x2 : x.x3)
                        : x.x2 != 0.0 ? x.x3 : 0.0;
            if (tail != 0.0 &&
                ((lead_bits ^ std::bit_cast<std::uint64_t>(tail)) >> 63) != 0)
            {
                --exponent;
            }
        }
        return exponent;
    }

    BL_FORCE_INLINE constexpr void adjust_rounded_limb_for_tail(
        double& rounded,
        double value,
        double next1,
        double next2,
        double next3) noexcept
    {
        const double delta = rounded - value;
        if (detail::fp::absd(delta) != 0.5)
            return;

        if (delta > 0.0)
        {
            if (has_negative_tail(next1, next2, next3))
                rounded -= 1.0;
        }
        else if (has_positive_tail(next1, next2, next3))
        {
            rounded += 1.0;
        }
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s signed_zero_like(const f256_s& a) noexcept
    {
        return signed_zero_from(a.x0);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s canonicalize_rounded_zero(f256_s out, const f256_s& a) noexcept
    {
        return out.x0 == 0.0 ? signed_zero_like(a) : out;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s floor_limbwise(const f256_s& a) noexcept
    {
        double x0 = floor_limb(a.x0);
        double x1 = 0.0;
        double x2 = 0.0;
        double x3 = 0.0;

        if (!detail::fp::isfinite(x0))
            return f256_s{ x0, 0.0, 0.0, 0.0 };

        if (x0 == a.x0)
        {
            x1 = floor_limb(a.x1);
            if (x1 == a.x1)
            {
                x2 = floor_limb(a.x2);
                if (x2 == a.x2)
                    x3 = floor_limb(a.x3);
            }

            return canonicalize_rounded_zero(renorm(x0, x1, x2, x3), a);
        }

        return f256_s{ x0 };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s ceil_limbwise(const f256_s& a) noexcept
    {
        double x0 = ceil_limb(a.x0);
        double x1 = 0.0;
        double x2 = 0.0;
        double x3 = 0.0;

        if (!detail::fp::isfinite(x0))
            return f256_s{ x0, 0.0, 0.0, 0.0 };

        if (x0 == a.x0)
        {
            x1 = ceil_limb(a.x1);
            if (x1 == a.x1)
            {
                x2 = ceil_limb(a.x2);
                if (x2 == a.x2)
                    x3 = ceil_limb(a.x3);
            }

            return canonicalize_rounded_zero(renorm(x0, x1, x2, x3), a);
        }

        return f256_s{ x0 };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s trunc_limbwise(const f256_s& a) noexcept
    {
        const double x0 = trunc_limb(a.x0);

        if (!detail::fp::isfinite(x0))
            return f256_s{ x0, 0.0, 0.0, 0.0 };

        if (x0 != a.x0)
            return x0 == 0.0 ? signed_zero_like(a) : f256_s{ x0 };

        if (a.x0 == 0.0)
            return signed_zero_like(a);

        return a.x0 < 0.0 ? ceil_limbwise(a) : floor_limbwise(a);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f256_s round_nearest_even(const f256_s& a) noexcept
    {
        if (detail::fp::iszero_or_inf_or_nan(a.x0)) [[unlikely]]
            return a;

        double x0 = round_nearest_even_limb(a.x0);
        double x1 = 0.0;
        double x2 = 0.0;
        double x3 = 0.0;

        if (x0 == a.x0)
        {
            x1 = round_nearest_even_limb(a.x1);
            if (x1 == a.x1)
            {
                x2 = round_nearest_even_limb(a.x2);
                if (x2 == a.x2)
                    x3 = round_nearest_even_limb(a.x3);
                else
                    adjust_rounded_limb_for_tail(x2, a.x2, a.x3, 0.0, 0.0);
            }
            else
                adjust_rounded_limb_for_tail(x1, a.x1, a.x2, a.x3, 0.0);
        }
        else
        {
            adjust_rounded_limb_for_tail(x0, a.x0, a.x1, a.x2, a.x3);
            return x0 == 0.0 ? signed_zero_like(a) : f256_s{ x0 };
        }

        return canonicalize_rounded_zero(renorm(x0, x1, x2, x3), a);
    }

}

namespace detail::_f256_impl
{
    [[nodiscard]] BL_FORCE_INLINE f256_s round_nearest_away_from_zero_runtime(
        const f256_s& a) noexcept
    {
        if (detail::fp::isinf_or_nan(a.x0)) [[unlikely]]
            return a;

        if (detail::fp::absd(a.x0) < detail::fp::double_integer_threshold)
        {
            if (detail::fp::trunc(a.x0) == a.x0 &&
                (a.x1 != 0.0 || a.x2 != 0.0 || a.x3 != 0.0))
            {
                return detail::_f256::round_nearest_away_from_zero(a);
            }

            double x0 = detail::_f256::round_nearest_away_from_zero_limb_finite_small(a.x0);
            detail::_f256::adjust_rounded_limb_for_tail(x0, a.x0, a.x1, a.x2, a.x3);
            return x0 == 0.0 ? detail::_f256::signed_zero_like(a) : f256_s{ x0 };
        }

        return detail::_f256::round_nearest_away_from_zero(a);
    }

}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::sqrt(const f256_s& a)
{
    using namespace detail::_f256;

    if (detail::fp::iszero_or_negative_or_inf_or_nan(a.x0)) [[unlikely]]
    {
        if (iszero(a))
            return a;

        if (detail::fp::isposinf(a.x0))
            return a;

        return f256_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };
    }

    if (bl::detail::is_constant_evaluated())
        return sqrt_impl(a);

    return sqrt_impl_fast(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::sqrt_accurate(const f256_s& a)
{
    using namespace detail::_f256;

    if (detail::fp::iszero_or_negative_or_inf_or_nan(a.x0)) [[unlikely]]
    {
        if (iszero(a))
            return a;

        if (detail::fp::isposinf(a.x0))
            return a;

        return f256_s{ std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 };
    }

    return sqrt_impl(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::hypot(const f256_s& x, const f256_s& y)
{
    if (detail::fp::isinf_or_nan(x.x0, y.x0)) [[unlikely]]
    {
        if (detail::fp::isinf(x.x0, y.x0))
            return std::numeric_limits<f256_s>::infinity();
        return detail::fp::isnan(x.x0) ? x : y;
    }

    f256_s ax = detail::_f256::mag(x);
    f256_s ay = detail::_f256::mag(y);
    if (ax < ay)
    {
        const f256_s tmp = ax;
        ax = ay;
        ay = tmp;
    }

    if (iszero(ax))
        return f256_s{ 0.0 };
    if (iszero(ay))
        return ax;

    const int ex = detail::fp::frexp_exponent_limb(ax.x0);
    const int ey = detail::fp::frexp_exponent_limb(ay.x0);

    if ((ex - ey) > 110)
        return ax;

    // A 212-bit square needs its leading component at exponent -862 or
    // greater to keep the last result bit above double's 2^-1074 floor.
    // Smaller magnitudes use the ratio form to avoid losing low components.
    if (ex >= -431 && ex < 450)
        return detail::_f256_impl::sqrt_accurate(add_raw5_raw5_inline(sqr_raw5_inline(ax), sqr_raw5_inline(ay)));

    // Power-of-two scaling is exact and avoids both component underflow and
    // the substantially more expensive f256 division in the ratio form.
    const f256_s scaled_ax = detail::_f256::ldexp_terms(ax, -ex);
    const f256_s scaled_ay = detail::_f256::ldexp_terms(ay, -ex);
    const f256_s scaled_result = detail::_f256_impl::sqrt_accurate(
        add_raw5_raw5_inline(
            sqr_raw5_inline(scaled_ax),
            sqr_raw5_inline(scaled_ay)));
    return
        detail::_f256::ldexp_terms(scaled_result, ex);
}

// rounding and decimals
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::trunc(const f256_s& a)
{
    if (detail::fp::isinf_or_nan(a.x0)) [[unlikely]]
        return a;

    return detail::fp::signbit(a.x0) ? detail::_f256::ceil_limbwise(a) : detail::_f256::floor_limbwise(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::round_nearest_away_from_zero(const f256_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f256::round_nearest_away_from_zero(a),
        detail::_f256_impl::round_nearest_away_from_zero_runtime(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::round_decimals(f256_s v, int prec)
{
    // Preserve sparse-expansion decimal functionality beyond the 65-digit
    // nominal default.
    constexpr int local_capacity = 67;

    if (prec <= 0) return v;
    if (prec > local_capacity) prec = local_capacity;
    if (detail::fp::iszero_or_inf_or_nan(v.x0)) return v;

    detail::exact_decimal::biguint coefficient;
    bool neg = false;
    if (!detail::exact_decimal::exact_decimal_places_integer<detail::_f256::f256_significant_decimal_traits>(
            v,
            prec,
            coefficient,
            neg))
    {
        return v;
    }

    const f256_s rounded =
        detail::_f256::round_decimal_exact_to_f256(coefficient, -prec, neg);
    if (iszero(rounded))
        return detail::_f256::signed_zero_from(v.x0);
    return rounded;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::round_significant(f256_s v, int figures)
{
    if (figures <= 0 || detail::fp::iszero_or_inf_or_nan(v.x0))
        return v;
    constexpr int local_capacity = 67;
    if (figures > local_capacity)
        figures = local_capacity;

    const bool neg = v.x0 < 0.0;
    const f256_s ax = neg ? -v : v;

    detail::exact_decimal::biguint coefficient;
    int exp10 = 0;
    if (!detail::exact_decimal::exact_significant_decimal<detail::_f256::f256_significant_decimal_traits>(
            ax,
            figures,
            coefficient,
            exp10))
    {
        return v;
    }

    return detail::_f256::round_decimal_exact_to_f256(coefficient, exp10 - (figures - 1), neg);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::pow10_256(int k)
{
    if (k < detail::_f256::pow10_f256_min_exponent) [[unlikely]]
        return f256_s{ 0.0 };
    if (k > detail::_f256::pow10_f256_max_exponent) [[unlikely]]
        return std::numeric_limits<f256_s>::infinity();

    return detail::_f256::pow_table_entry_to_f256(
        detail::pow_tables::pow10_table[k - detail::_f256::pow10_f256_min_exponent]);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::round_nearest_even(const f256_s& a)
{
    return detail::_f256::round_nearest_even(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr long detail::_f256_impl::lround_nearest_away_from_zero(
    const f256_s& x)
{
    long out = 0;
    if (detail::_f256::try_round_to_signed_integer(x, false, out))
        return out;

    return detail::_f256::to_signed_integer_or_zero<long>(
        detail::_f256::round_nearest_away_from_zero(x));
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long detail::_f256_impl::llround_nearest_away_from_zero(const f256_s& x)
{
    long long out = 0;
    if (detail::_f256::try_round_to_signed_integer(x, false, out))
        return out;

    return detail::_f256::to_signed_integer_or_zero<long long>(
        detail::_f256::round_nearest_away_from_zero(x));
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::fma(const f256_s& x, const f256_s& y, const f256_s& z)
{
    if (detail::fp::isinf_or_nan(x.x0) || detail::fp::isinf_or_nan(y.x0) || detail::fp::isinf_or_nan(z.x0)) [[unlikely]]
        return f256_s{ std::fma(x.x0, y.x0, z.x0), 0.0, 0.0, 0.0 };

    const double leading_product = x.x0 * y.x0;
    if (leading_product != 0.0 && leading_product == -z.x0) [[unlikely]]
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_f256::mul_add_exact_inline(x, y, z),
            detail::_f256_runtime::fma_cancellation(x, y, z)
        );
    }

    #if FLTX_GUARDED_X86_FMA
    if (!bl::detail::is_constant_evaluated())
    {
        const bool use_hardware_fma = detail::fp::runtime_hardware_fma_enabled();
        return
            use_hardware_fma
                ? detail::_f256::mul_add_hardware_inline(x, y, z)
                : detail::_f256::mul_add_dekker_inline(x, y, z);
    }

    return
        detail::_f256::mul_add_dekker_inline(x, y, z);
    #else
    return
        detail::_f256::mul_add_inline(x, y, z);
    #endif
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::fmin(const f256_s& a, const f256_s& b)
{
    if (detail::fp::isnan(a.x0)) [[unlikely]]
        return b;
    if (detail::fp::isnan(b.x0)) [[unlikely]]
        return a;

    if (a.x0 != b.x0)
        return a.x0 < b.x0 ? a : b;

    if (a.x1 != b.x1)
        return a.x1 < b.x1 ? a : b;
    if (a.x2 != b.x2)
        return a.x2 < b.x2 ? a : b;
    if (a.x3 != b.x3)
        return a.x3 < b.x3 ? a : b;

    if (a.x0 == 0.0 && a.x1 == 0.0 && a.x2 == 0.0 && a.x3 == 0.0)
        return detail::fp::signbit(a.x0) ? a : b;

    return a;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::fmax(const f256_s& a, const f256_s& b)
{
    if (detail::fp::isnan(a.x0)) [[unlikely]]
        return b;
    if (detail::fp::isnan(b.x0)) [[unlikely]]
        return a;

    if (a.x0 != b.x0)
        return a.x0 > b.x0 ? a : b;

    if (a.x1 != b.x1)
        return a.x1 > b.x1 ? a : b;
    if (a.x2 != b.x2)
        return a.x2 > b.x2 ? a : b;
    if (a.x3 != b.x3)
        return a.x3 > b.x3 ? a : b;

    if (a.x0 == 0.0 && a.x1 == 0.0 && a.x2 == 0.0 && a.x3 == 0.0)
        return detail::fp::signbit(a.x0) ? b : a;

    return a;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::fdim(const f256_s& x, const f256_s& y)
{
    if (isnan(x) || isnan(y)) [[unlikely]]
        return std::numeric_limits<f256_s>::quiet_NaN();
    if (isinf(x)) [[unlikely]]
    {
        if (signbit(x))
            return f256_s{ 0.0 };
        return (isinf(y) && !signbit(y)) ? f256_s{ 0.0 } : std::numeric_limits<f256_s>::infinity();
    }
    if (isinf(y)) [[unlikely]]
        return signbit(y) ? std::numeric_limits<f256_s>::infinity() : f256_s{ 0.0 };

    return (x > y) ? x - y : f256_s{ 0.0 };
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::copysign(const f256_s& x, const f256_s& y)
{
    return bl::signbit(x) == bl::signbit(y) ? x : -x;
}

// remainders
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::fmod(const f256_s& x, const f256_s& y)
{
    if (detail::fp::isinf_or_nan(x.x0) || detail::fp::iszero_or_nan(y.x0))
        return std::numeric_limits<f256_s>::quiet_NaN();
    if (detail::fp::isinf(y.x0) || x.x0 == 0.0)
        return x;

    const f256_s ax = detail::_f256::mag(x);
    const f256_s ay = detail::_f256::mag(y);

    if (ax < ay)
        return x;

    f256_s fast{};
    if (fmod_fast_small_quotient_abs(ax, ay, fast))
    {
        if (iszero(fast))
            return detail::_f256::signed_zero_like(x);
        const f256_s out = ispositive(x) ? fast : -fast;
        return out;
    }

    return fmod_reduced_or_exact(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::remquo(const f256_s& x, const f256_s& y, int* quo)
{
    if (quo)
        *quo = 0;

    if (detail::fp::isinf_or_nan(x.x0) || detail::fp::iszero_or_nan(y.x0))
        return std::numeric_limits<f256_s>::quiet_NaN();
    if (detail::fp::isinf(y.x0) || x.x0 == 0.0)
        return x;

    const bool x_negative = signbit(x);
    const bool quotient_negative = x_negative != signbit(y);
    const f256_s ax = detail::_f256::mag(x);
    const f256_s ay = detail::_f256::mag(y);

    f256_s r_abs{};
    std::uint64_t quotient_abs = 0;
    bool fast = false;

    if (ax < ay)
    {
        r_abs = ax;
        fast = true;
    }
    else
    {
        fast = fmod_fast_medium_quotient_abs_with_quotient(ax, ay, r_abs, quotient_abs, false);
    }

    if (fast)
    {
        const f256_s half = mul_double_product_inline(ay, 0.5);
        const int half_cmp = detail::_f256::fmod_compare_remainder_to_half(r_abs, half);
        if (half_cmp > 0 || (half_cmp == 0 && ((quotient_abs & 1u) != 0u)))
        {
            r_abs = sub_finite_inline(r_abs, ay);
            ++quotient_abs;
        }

        if (quo)
            *quo = detail::fp::remquo_low_quotient_bits(quotient_abs, quotient_negative);

        f256_s r = x_negative ? -r_abs : r_abs;
        if (iszero(r))
            return detail::_f256::signed_zero_like(x);

        return r;
    }

    std::uint64_t quotient_mod = 0;
    r_abs = fmod_exact_fixed_limb_abs_with_quotient_mod(ax, ay, quotient_mod);
    const f256_s half = mul_double_product_inline(ay, 0.5);
    const int half_cmp = detail::_f256::fmod_compare_remainder_to_half(r_abs, half);

    if (half_cmp > 0)
    {
        r_abs = sub_finite_inline(r_abs, ay);
        ++quotient_mod;
    }
    else if (half_cmp == 0 && ((quotient_mod & 1u) != 0u))
    {
        r_abs = sub_finite_inline(r_abs, ay);
        ++quotient_mod;
    }

    if (quo)
        *quo = detail::fp::remquo_low_quotient_bits(quotient_mod, quotient_negative);

    f256_s r = x_negative ? -r_abs : r_abs;
    if (iszero(r))
        return detail::_f256::signed_zero_like(x);

    return r;
}

// fractional decomposition
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::modf(const f256_s& x, f256_s* iptr) noexcept
{
    if (isnan(x))
    {
        if (iptr)
            *iptr = x;
        return x;
    }
    if (isinf(x))
    {
        if (iptr)
            *iptr = x;
        return detail::_f256::signed_zero_like(x);
    }

    const f256_s i = detail::_f256_impl::trunc(x);
    if (iptr)
        *iptr = i;

    f256_s frac = sub_finite_inline(x, i);
    if (iszero(frac))
        frac = detail::_f256::signed_zero_like(x);
    return frac;
}

// decomposition and scaling
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::ldexp(const f256_s& a, int e)
{
    if (detail::fp::iszero_or_inf_or_nan(a.x0)) [[unlikely]]
        return a;

    return _ldexp(a, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::frexp(const f256_s& x, int* exp) noexcept
{
    if (exp)
        *exp = 0;

    if (detail::fp::iszero_or_inf_or_nan(x.x0))
        return x;

    int e = 0;

    if (bl::detail::is_constant_evaluated())
    {
        e = detail::fp::frexp_exponent(x.x0);
    }
#if BL_FP_BARRIER_ACTIVE
    else
    {
        e = detail::fp::frexp_exponent(x.x0);
    }
#else
    else
    {
        (void)std::frexp(x.x0, &e);
    }
#endif

    const bool safe_fast_scale =
        detail::fp::absd(x.x0) >= std::numeric_limits<double>::min();
    f256_s m = safe_fast_scale
        ? detail::_f256::_ldexp(x, -e)
        : detail::_f256::ldexp_terms(x, -e);
    if ((m.x0 == 0.5 && detail::_f256::has_negative_tail(m.x1, m.x2, m.x3))
        || (m.x0 == -0.5 && detail::_f256::has_positive_tail(m.x1, m.x2, m.x3)))
    {
        m = safe_fast_scale
            ? detail::_f256::_ldexp(m, 1)
            : detail::_f256::ldexp_terms(m, 1);
        --e;
    }
    else if ((m.x0 == 1.0 && !detail::_f256::has_negative_tail(m.x1, m.x2, m.x3))
        || (m.x0 == -1.0 && !detail::_f256::has_positive_tail(m.x1, m.x2, m.x3)))
    {
        m = safe_fast_scale
            ? detail::_f256::_ldexp(m, -1)
            : detail::_f256::ldexp_terms(m, -1);
        ++e;
    }

    if (exp)
        *exp = e;

    return m;
}

[[nodiscard]] BL_FORCE_INLINE constexpr int detail::_f256_impl::ilogb(const f256_s& x) noexcept
{
    if (isnan(x))  return FP_ILOGBNAN;
    if (iszero(x)) return FP_ILOGB0;
    if (isinf(x))  return std::numeric_limits<int>::max();

    return detail::_f256::ilogb_finite_fast(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::logb(const f256_s& x) noexcept
{
    if (isnan(x))  return x;
    if (iszero(x)) return f256_s{ -std::numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 };
    if (isinf(x))  return std::numeric_limits<f256_s>::infinity();

    return f256_s{ static_cast<double>(detail::_f256_impl::ilogb(x)), 0.0, 0.0, 0.0 };
}

// adjacent values
[[nodiscard]] BL_FORCE_INLINE constexpr f256_s detail::_f256_impl::nextafter(const f256_s& from, const f256_s& to) noexcept
{
    if (detail::fp::isnan(from.x0) || detail::fp::isnan(to.x0))
        return std::numeric_limits<f256_s>::quiet_NaN();
    if (from == to)
        return to;
    if (iszero(from))
        return signbit(to)
        ? f256_s{ -std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 }
        : f256_s{ std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 };
    if (isinf(from))
        return signbit(from)
        ? -std::numeric_limits<f256_s>::max()
        : std::numeric_limits<f256_s>::max();

    const bool upward = from < to;
    const bool toward_smaller_magnitude = upward == signbit(from);
    constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
    const std::uint64_t leading_bits =
        std::bit_cast<std::uint64_t>(from.x0) & 0x7fffffffffffffffull;
    const std::uint32_t exponent_bits =
        static_cast<std::uint32_t>(leading_bits >> 52);
    if (exponent_bits > std::numeric_limits<f256_s>::digits - 1 &&
        (leading_bits & fraction_mask) != 0) [[likely]]
    {
        const std::uint64_t step_bits =
            static_cast<std::uint64_t>(
                exponent_bits - (std::numeric_limits<f256_s>::digits - 1)) << 52;
        const double step = std::bit_cast<double>(step_bits);
        const double stepped_x3 = upward ? from.x3 + step : from.x3 - step;
        const std::uint32_t x2_exponent_bits =
            static_cast<std::uint32_t>(
                (std::bit_cast<std::uint64_t>(from.x2) >> 52) & 0x7ffu);
        if (x2_exponent_bits > 53)
        {
            const std::uint64_t half_x2_ulp_bits =
                static_cast<std::uint64_t>(x2_exponent_bits - 53) << 52;
            const std::uint64_t stepped_x3_bits =
                std::bit_cast<std::uint64_t>(stepped_x3) & 0x7fffffffffffffffull;
            if (stepped_x3_bits < half_x2_ulp_bits) [[likely]]
                return f256_s{ from.x0, from.x1, from.x2, stepped_x3 };
        }
        return normalize_nextafter_tail(from, stepped_x3);
    }

    const double leading =
        from.x0 != 0.0 ? from.x0 :
        from.x1 != 0.0 ? from.x1 :
        from.x2 != 0.0 ? from.x2 : from.x3;
    const double trailing =
        from.x0 != 0.0
            ? (from.x1 != 0.0 ? from.x1 : (from.x2 != 0.0 ? from.x2 : from.x3))
            : from.x1 != 0.0
                ? (from.x2 != 0.0 ? from.x2 : from.x3)
                : from.x2 != 0.0 ? from.x3 : 0.0;
    const double step = detail::fp::nominal_ulp_step(
        leading,
        trailing,
        toward_smaller_magnitude,
        std::numeric_limits<f256_s>::digits);

    return normalize_nextafter_tail(
        from,
        upward ? from.x3 + step : from.x3 - step);
}

} // namespace bl

#endif
