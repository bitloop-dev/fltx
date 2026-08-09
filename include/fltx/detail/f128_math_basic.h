/**
 * fltx/detail/f128_math_basic.h - Basic math implementation details.
 *
 * f128 rounding, decomposition, remainder, hypot, and adjacent-value implementations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F128_DETAIL_MATH_BASIC_INCLUDED
#define F128_DETAIL_MATH_BASIC_INCLUDED
#include "fltx/detail/f128_math_kernels.h"
#include "fltx/detail/pow_tables.h"
#include "fltx/detail/simd.h"

namespace bl {

namespace detail::_f128
{
    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s hypot_sqrt_sum(const f128_s& sum)
    {
        BL_CONSTEXPR_RUNTIME_DISPATCH(
            detail::_f128_impl::sqrt(sum),
            detail::_f128::sqrt_compensated(sum, std::sqrt(sum.hi))
        );
    }
}

namespace detail::_f128
{
    inline constexpr int pow10_f128_min_exponent = detail::pow_tables::pow10_min_exponent;
    inline constexpr int pow10_f128_max_exponent = detail::pow_tables::pow10_max_exponent;

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s pow_table_entry_to_f128(const detail::pow_table_entry& row) noexcept
    {
        return f128_s{ row.x0, row.x1 };
    }

    struct fma_products
    {
        double p0, q0;
        double p1, q1;
        double p2, q2;
    };

    BL_PUSH_PRECISE;
    [[nodiscard]] BL_NO_INLINE constexpr f128_s fma_cancellation_exact(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z) noexcept
    {
        double expansion[10]{};
        int count = 0;

        const double x_limb[2]{ x.hi, x.lo };
        const double y_limb[2]{ y.hi, y.lo };
        for (double xi : x_limb)
        {
            for (double yi : y_limb)
            {
                double product{}, error{};
                detail::fp::two_prod_precise(xi, yi, product, error);
                count = detail::fp::grow_expansion_zeroelim(count, expansion, error);
                count = detail::fp::grow_expansion_zeroelim(count, expansion, product);
            }
        }

        count = detail::fp::grow_expansion_zeroelim(count, expansion, z.lo);
        count = detail::fp::grow_expansion_zeroelim(count, expansion, z.hi);

        double tail = 0.0;
        for (int i = 0; i + 1 < count; ++i)
            tail += expansion[i];
        return renorm(expansion[count - 1], tail);
    }
    BL_POP_PRECISE;

    [[nodiscard]] BL_FORCE_INLINE fma_products fma_products_hardware(
        const f128_s& x,
        const f128_s& y) noexcept
    {
        fma_products products{};
        detail::fp::two_prod_fma(x.hi, y.hi, products.p0, products.q0);
        detail::fp::two_prod_fma(x.hi, y.lo, products.p1, products.q1);
        detail::fp::two_prod_fma(x.lo, y.hi, products.p2, products.q2);
        return products;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fma_products fma_products_dekker(
        const f128_s& x,
        const f128_s& y) noexcept
    {
        fma_products products{};
        detail::fp::two_prod_precise_dekker(x.hi, y.hi, products.p0, products.q0);
        detail::fp::two_prod_precise_dekker(x.hi, y.lo, products.p1, products.q1);
        detail::fp::two_prod_precise_dekker(x.lo, y.hi, products.p2, products.q2);
        return products;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fma_products fma_products_auto(
        const f128_s& x,
        const f128_s& y) noexcept
    {
        fma_products products{};
        detail::fp::two_prod_precise(x.hi, y.hi, products.p0, products.q0);
        detail::fp::two_prod_precise(x.hi, y.lo, products.p1, products.q1);
        detail::fp::two_prod_precise(x.lo, y.hi, products.p2, products.q2);
        return products;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s fma_finite_from_products(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z,
        fma_products products) noexcept
    {
        const double p0 = products.p0;
        const double q0 = products.q0;
        const double p1 = products.p1;
        const double q1 = products.q1;
        const double p2 = products.p2;
        const double q2 = products.q2;

        double p12{}, e12{};
        double p012{}, e012{};
        two_sum_precise(p1, p2, p12, e12);
        two_sum_precise(q0, p12, p012, e012);

        double hi{}, hi_err{};
        double mid{}, mid_err{};
        double lo{}, lo_err{};
        two_sum_precise(p0, z.hi, hi, hi_err);
        two_sum_precise(p012, z.lo, mid, mid_err);
        two_sum_precise(hi_err, mid, lo, lo_err);

        const double tail = lo_err + mid_err + e12 + e012 + q1 + q2 + (x.lo * y.lo);
        const f128_s rough = renorm(hi, lo + tail);

        return rough;
    }

    [[nodiscard]] BL_FORCE_INLINE f128_s fma_finite_hardware(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z) noexcept
    {
        return fma_finite_from_products(x, y, z, fma_products_hardware(x, y));
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s fma_finite_dekker(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z) noexcept
    {
        return fma_finite_from_products(x, y, z, fma_products_dekker(x, y));
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s fma_finite_auto(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z) noexcept
    {
        return fma_finite_from_products(x, y, z, fma_products_auto(x, y));
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s fma_finite_dispatch(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z,
        double leading_product) noexcept
    {
        if (leading_product != 0.0
            && detail::fp::absd(leading_product + z.hi)
                <= detail::fp::absd(leading_product) * 0x1p-48) [[unlikely]]
        {
            return fma_cancellation_exact(x, y, z);
        }

        #if FLTX_DETAIL_MSVC_GUARDED_X86_FMA
        if (!bl::detail::is_constant_evaluated())
        {
            const bool use_hardware_fma = detail::fp::runtime_hardware_fma_enabled();
            return use_hardware_fma
                ? fma_finite_hardware(x, y, z)
                : fma_finite_dekker(x, y, z);
        }

        return fma_finite_dekker(x, y, z);
        #else
        return fma_finite_auto(x, y, z);
        #endif
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s fma_overflow_scaled(
        const f128_s& x,
        const f128_s& y,
        const f128_s& z) noexcept
    {
        // An overflowing leading product can still cancel with a finite z.
        // Split the common 2^-1024 scale across x and y so all product terms
        // remain normal, then apply the same total scale to z.
        constexpr int half_scale = 512;
        const f128_s scaled_x = _ldexp(x, -half_scale);
        const f128_s scaled_y = _ldexp(y, -half_scale);
        const f128_s scaled_z = _ldexp(z, -2 * half_scale);
        const double scaled_leading_product = scaled_x.hi * scaled_y.hi;
        const f128_s scaled_result =
            fma_finite_dispatch(scaled_x, scaled_y, scaled_z, scaled_leading_product);
        const f128_s result = _ldexp(scaled_result, 2 * half_scale);

        if (detail::fp::isinf(result.hi)) [[unlikely]]
            return f128_s{ result.hi, 0.0 };

        return result;
    }

    [[nodiscard]] BL_NO_INLINE constexpr f128_s round_nearest_even_large(const f128_s& a)
    {
        f128_s t = detail::_f128_impl::floor(a);
        f128_s frac = sub_inline(a, t);

        if (frac < f128_s{ 0.5 })
            return t;

        if (frac > f128_s{ 0.5 })
        {
            t = add_double_inline(t, 1.0);
            if (iszero(t))
                return signed_zero(signbit(a.hi));
            return t;
        }

        if (detail::_f128_impl::fmod(t, f128_s{ 2.0 }) != f128_s{ 0.0 })
            t = add_double_inline(t, 1.0);

        if (iszero(t))
            return signed_zero(signbit(a.hi));

        return t;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr f128_s round_nearest_even(const f128_s& a)
    {
        if (detail::fp::iszero_or_inf_or_nan(a.hi))
            return a;

        if (detail::_f128::absd(a.hi) < 0x1p52)
        {
            auto base = static_cast<long long>(a.hi);
            if (static_cast<double>(base) == a.hi)
            {
                if (a.hi < 0.0 && a.lo > 0.0)
                    ++base;
                else if (a.hi > 0.0 && a.lo < 0.0)
                    --base;
            }

            const double base_d = static_cast<double>(base);
            const double frac_hi = a.hi - base_d;
            const double abs_frac_lo = detail::_f128::absd(a.lo);
            long long rounded = base;
            // Canonical double-double ties can be resolved from the low word
            // directly. Keep the exact expansion fallback for noncanonical input.
            if (frac_hi == 0.5 && abs_frac_lo <= 0.5)
            {
                if (a.lo > 0.0 || (a.lo == 0.0 && (base & 1ll) != 0))
                    ++rounded;
            }
            else if (frac_hi == -0.5 && abs_frac_lo <= 0.5)
            {
                if (a.lo < 0.0 || (a.lo == 0.0 && (base & 1ll) != 0))
                    --rounded;
            }
            else
            {
                const double abs_frac_hi = detail::_f128::absd(frac_hi);
                if (abs_frac_hi > 0.5 + abs_frac_lo)
                {
                    rounded += (frac_hi < 0.0 ||
                                (frac_hi == 0.0 && detail::_f128::signbit(a.lo))) ? -1 : 1;
                }
                else if (abs_frac_hi >= 0.5 - abs_frac_lo)
                {
                    const f128_s frac = sub_double_inline(a, base_d);
                    const f128_s abs_frac = detail::_f128::mag(frac);
                    if (abs_frac > f128_s{ 0.5 } ||
                        (abs_frac == f128_s{ 0.5 } && (base & 1ll) != 0))
                    {
                        rounded += signbit(frac) ? -1 : 1;
                    }
                }
            }

            f128_s out{ static_cast<double>(rounded), 0.0 };
            if (iszero(out))
                return signed_zero(signbit(a));
            return out;
        }

        return round_nearest_even_large(a);
    }
}

namespace detail::_f128_impl
{
    [[nodiscard]] inline BL_NO_INLINE f128_s round_nearest_away_from_zero_integral_head(
        const f128_s& a) noexcept
    {
        return detail::_f128::round_nearest_away_from_zero(a);
    }

    [[nodiscard]] BL_FORCE_INLINE f128_s round_nearest_away_from_zero_runtime(
        const f128_s& a) noexcept
    {
        #if defined(__EMSCRIPTEN__)
        if (detail::fp::isinf_or_nan(a.hi)) [[unlikely]]
            return a;

        if (detail::_f128::absd(a.hi) < detail::fp::double_integer_threshold)
        {
            double rounded = static_cast<double>(static_cast<long long>(detail::fp::absd(a.hi) + 0.5));
            if (detail::fp::signbit(a.hi))
                rounded = -rounded;

            if (rounded == a.hi && a.lo != 0.0)
                return round_nearest_away_from_zero_integral_head(a);

            const double delta = rounded - a.hi;
            if ((delta == 0.5 && a.lo < 0.0) || (delta == -0.5 && a.lo > 0.0))
                rounded += (rounded < 0.0) ? 1.0 : -1.0;

            if (rounded == 0.0)
                return detail::_f128::signed_zero(bl::signbit(a));
            return f128_s{ rounded, 0.0 };
        }
        #endif

        double rounded = detail::fp::round_nearest_away_from_zero(a.hi);
        if (rounded == a.hi)
        {
            if (a.lo != 0.0)
                return round_nearest_away_from_zero_integral_head(a);
            return f128_s{ rounded, 0.0 };
        }

        const double delta = rounded - a.hi;
        if ((delta == 0.5 && a.lo < 0.0) || (delta == -0.5 && a.lo > 0.0))
            rounded += (rounded < 0.0) ? 1.0 : -1.0;

        if (rounded == 0.0)
            return detail::_f128::signed_zero(bl::signbit(a));
        return f128_s{ rounded, 0.0 };
    }

}

// roots
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::sqrt(f128_s a)
{
    if (detail::fp::iszero_or_negative_or_inf_or_nan(a.hi)) [[unlikely]]
    {
        if (a.hi == 0.0 && a.lo == 0.0)
            return a;

        if (detail::fp::isposinf(a.hi))
            return a;

        return f128_s{ std::numeric_limits<double>::quiet_NaN() };
    }

    constexpr double fast_min = 0x1p-900;
    constexpr double fast_max = 0x1p900;

    if (bl::detail::is_constant_evaluated() || a.hi < fast_min || a.hi > fast_max)
    {
        const int exp2 = detail::fp::frexp_exponent_limb(a.hi);
        const int result_scale = exp2 / 2;
        const int input_scale = -2 * result_scale;
        const f128_s scaled_a = input_scale == 0 ? a : ldexp_terms(a, input_scale);

        double seed{};
        if (bl::detail::is_constant_evaluated())
            seed = detail::_f128::sqrt_constexpr_head(scaled_a.hi);
        else
            seed = std::sqrt(scaled_a.hi);

        f128_s y = detail::_f128::sqrt_compensated(scaled_a, seed);

        if (result_scale != 0)
            y = ldexp_terms(y, result_scale);

        return y;
    }

    return detail::_f128::sqrt_compensated(a, std::sqrt(a.hi));
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::hypot(const f128_s& x, const f128_s& y)
{
    using namespace detail::_f128;

    if (detail::fp::isinf_or_nan(x.hi, y.hi)) [[unlikely]]
    {
        if (detail::fp::isinf(x.hi, y.hi))
            return std::numeric_limits<f128_s>::infinity();
        return detail::fp::isnan(x.hi) ? x : y;
    }

    f128_s ax = detail::_f128::mag(x);
    f128_s ay = detail::_f128::mag(y);
    if (ax < ay)
    {
        const f128_s tmp = ax;
        ax = ay;
        ay = tmp;
    }

    if (iszero(ax))
        return f128_s{ 0.0 };
    if (iszero(ay))
        return ax;

    if (ay.hi <= ax.hi * 0x1p-55)
        return ax;

    // Squaring below 2^-484 cannot retain all 106 result bits before the
    // double component floor at 2^-1074. Use the ratio form there so the
    // smaller square is formed near unity instead of losing low components.
    if (ax.hi >= 0x1p-484 && ax.hi < 0x1p500)
    {
        const f128_s sum = add_inline(sqr_inline(ax), sqr_inline(ay));
        return hypot_sqrt_sum(sum);
    }

    const f128_s r = div_inline(ay, ax);
    return mul_inline(ax, detail::_f128_impl::sqrt(add_double_inline(mul_inline(r, r), 1.0)));
}

// rounding and decimals
[[nodiscard]] BL_FORCE_INLINE constexpr double detail::_f128_impl::floor_limb(double x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::fp::floor(x),
        std::floor(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr double detail::_f128_impl::ceil_limb(double x) noexcept
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::fp::ceil(x),
        std::ceil(x)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::floor(const f128_s& a)
{
    double hi = detail::_f128_impl::floor_limb(a.hi);
    double lo = 0.0;

    if (!detail::fp::isfinite(hi))
        return f128_s{ hi, 0.0 };

    if (hi == a.hi)
    {
        lo = detail::_f128_impl::floor_limb(a.lo);
        const f128_s out = detail::_f128::renorm(hi, lo);
        return out.hi == 0.0 ? detail::_f128::signed_zero(detail::fp::signbit(a.hi)) : out;
    }

    return f128_s{ hi, 0.0 };
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::ceil(const f128_s& a)
{
    double hi = detail::_f128_impl::ceil_limb(a.hi);
    double lo = 0.0;

    if (!detail::fp::isfinite(hi))
        return f128_s{ hi, 0.0 };

    if (hi == a.hi)
    {
        lo = detail::_f128_impl::ceil_limb(a.lo);
        const f128_s out = detail::_f128::renorm(hi, lo);
        return out.hi == 0.0 ? detail::_f128::signed_zero(detail::fp::signbit(a.hi)) : out;
    }

    return hi == 0.0 ? detail::_f128::signed_zero(detail::fp::signbit(a.hi)) : f128_s{ hi, 0.0 };
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::trunc(const f128_s& a)
{
    if (detail::fp::iszero_or_inf_or_nan(a.hi)) [[unlikely]]
        return a;

    if (detail::_f128::absd(a.hi) < detail::fp::double_integer_threshold)
    {
        double hi = static_cast<double>(static_cast<long long>(a.hi));
        if (hi == a.hi)
        {
            if (a.hi > 0.0 && a.lo < 0.0)
                hi -= 1.0;
            else if (a.hi < 0.0 && a.lo > 0.0)
                hi += 1.0;
        }

        if (hi == 0.0)
            return detail::_f128::signed_zero(signbit(a));
        return f128_s{ hi, 0.0 };
    }

    return (a.hi < 0.0) ? detail::_f128_impl::ceil(a) : detail::_f128_impl::floor(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::round_nearest_away_from_zero(
    const f128_s& a)
{
    BL_CONSTEXPR_RUNTIME_DISPATCH(
        detail::_f128::round_nearest_away_from_zero(a),
        detail::_f128_impl::round_nearest_away_from_zero_runtime(a)
    );
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::round_decimals(f128_s v, int prec)
{
    // Expansion-aware decimal operations intentionally retain their original
    // ceiling instead of inheriting presentation-oriented numeric metadata.
    constexpr int local_capacity = 33;

    if (prec <= 0) return v;
    if (prec > local_capacity) prec = local_capacity;
    if (detail::fp::iszero_or_inf_or_nan(v.hi)) return v;

    detail::exact_decimal::biguint coefficient;
    bool neg = false;
    if (!detail::exact_decimal::exact_decimal_places_integer<detail::_f128::f128_significant_decimal_traits>(
            v,
            prec,
            coefficient,
            neg))
    {
        return v;
    }

    const f128_s rounded =
        detail::_f128::round_decimal_exact_to_f128(coefficient, -prec, neg);
    if (iszero(rounded))
        return f128_s{ detail::fp::copysign(0.0, v.hi), 0.0 };
    return rounded;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::round_significant(f128_s v, int figures)
{
    if (figures <= 0 || detail::fp::iszero_or_inf_or_nan(v.hi))
        return v;
    constexpr int local_capacity = 33;
    if (figures > local_capacity)
        figures = local_capacity;

    const bool neg = v.hi < 0.0;
    const f128_s ax = neg ? -v : v;

    detail::exact_decimal::biguint coefficient;
    int exp10 = 0;
    if (!detail::exact_decimal::exact_significant_decimal<detail::_f128::f128_significant_decimal_traits>(
            ax,
            figures,
            coefficient,
            exp10))
    {
        return v;
    }

    return detail::_f128::round_decimal_exact_to_f128(coefficient, exp10 - (figures - 1), neg);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::pow10_128(int k)
{
    if (k < detail::_f128::pow10_f128_min_exponent) [[unlikely]]
        return f128_s{ 0.0 };
    if (k > detail::_f128::pow10_f128_max_exponent) [[unlikely]]
        return std::numeric_limits<f128_s>::infinity();

    return detail::_f128::pow_table_entry_to_f128(
        detail::pow_tables::pow10_table[k - detail::_f128::pow10_f128_min_exponent]);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::round_nearest_even(const f128_s& a)
{
    return detail::_f128::round_nearest_even(a);
}

[[nodiscard]] BL_FORCE_INLINE constexpr long detail::_f128_impl::lround_nearest_away_from_zero(
    const f128_s& x)
{
    long out = 0;
    if (detail::_f128::try_round_to_signed_integer(x, false, out))
        return out;

    return to_signed_integer_or_zero<long>(detail::_f128::round_nearest_away_from_zero(x));
}

[[nodiscard]] BL_FORCE_INLINE constexpr long long detail::_f128_impl::llround_nearest_away_from_zero(
    const f128_s& x)
{
    long long out = 0;
    if (detail::_f128::try_round_to_signed_integer(x, false, out))
        return out;

    return to_signed_integer_or_zero<long long>(detail::_f128::round_nearest_away_from_zero(x));
}

// arithmetic and comparisons
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::fma(const f128_s& x, const f128_s& y, const f128_s& z)
{
    if (detail::fp::isinf_or_nan(x.hi) || detail::fp::isinf_or_nan(y.hi) || detail::fp::isinf_or_nan(z.hi)) [[unlikely]]
        return f128_s{ std::fma(x.hi, y.hi, z.hi), 0.0 };

    const double leading_product = x.hi * y.hi;
    if (!detail::fp::isfinite(leading_product)) [[unlikely]]
        return detail::_f128::fma_overflow_scaled(x, y, z);

    return detail::_f128::fma_finite_dispatch(x, y, z, leading_product);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::fmin(const f128_s& a, const f128_s& b)
{
    if (detail::fp::isnan(a.hi)) [[unlikely]]
        return b;
    if (detail::fp::isnan(b.hi)) [[unlikely]]
        return a;

    if (a.hi != b.hi)
        return a.hi < b.hi ? a : b;

    if (a.lo != b.lo)
        return a.lo < b.lo ? a : b;

    if (a.hi == 0.0 && a.lo == 0.0)
        return detail::_f128::signbit(a.hi) ? a : b;

    return a;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::fmax(const f128_s& a, const f128_s& b)
{
    if (detail::fp::isnan(a.hi)) [[unlikely]]
        return b;
    if (detail::fp::isnan(b.hi)) [[unlikely]]
        return a;

    if (a.hi != b.hi)
        return a.hi > b.hi ? a : b;

    if (a.lo != b.lo)
        return a.lo > b.lo ? a : b;

    if (a.hi == 0.0 && a.lo == 0.0)
        return detail::_f128::signbit(a.hi) ? b : a;

    return a;
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::fdim(const f128_s& x, const f128_s& y)
{
    if (!detail::fp::isinf_or_nan(x.hi) && !detail::fp::isinf_or_nan(y.hi))
    {
        const bool x_greater_y = (x.hi > y.hi) || (x.hi == y.hi && x.lo > y.lo);
        return x_greater_y ? sub_inline(x, y) : f128_s{ 0.0 };
    }

    if (detail::fp::isnan(x.hi) || detail::fp::isnan(y.hi))
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (isinf(x))
    {
        if (signbit(x))
            return f128_s{ 0.0 };
        return (isinf(y) && !signbit(y)) ? f128_s{ 0.0 } : std::numeric_limits<f128_s>::infinity();
    }
    if (isinf(y))
        return signbit(y) ? std::numeric_limits<f128_s>::infinity() : f128_s{ 0.0 };

    return (x > y) ? sub_inline(x, y) : f128_s{ 0.0 };
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::copysign(const f128_s& x, const f128_s& y)
{
    return detail::_f128::signbit(x.hi) == detail::_f128::signbit(y.hi) ? x : -x;
}

// remainders
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::fmod(const f128_s& x, const f128_s& y)
{
    if (detail::fp::isinf_or_nan(x.hi) || detail::fp::iszero_or_nan(y.hi))
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (detail::fp::isinf(y.hi) || x.hi == 0.0)
        return x;

    const f128_s ax = detail::_f128::mag(x);
    const f128_s ay = detail::_f128::mag(y);

    if (ax < ay)
        return x;

    f128_s fast{};
    if (fmod_fast_small_quotient_abs(ax, ay, fast))
    {
        if (iszero(fast))
            return detail::_f128::signed_zero(signbit(x.hi));
        return ispositive(x) ? fast : -fast;
    }

    const double q = detail::fp::trunc(ax.hi / ay.hi);
    if (q >= 0x1p50 && q < 0x1p53)
    {
        f128_s exact{};
        if (fmod_exact_candidate_quotient_abs(ax, ay, static_cast<std::uint64_t>(q), exact))
        {
            if (iszero(exact))
                return detail::_f128::signed_zero(signbit(x.hi));
            return ispositive(x) ? exact : -exact;
        }
    }

    return fmod_reduced_or_exact(x, y);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::remquo(const f128_s& x, const f128_s& y, int* quo)
{
    using namespace detail::_f128;

    if (quo)
        *quo = 0;

    if (detail::fp::isinf_or_nan(x.hi) || detail::fp::iszero_or_nan(y.hi))
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (detail::fp::isinf(y.hi) || x.hi == 0.0)
        return x;

    const bool x_negative = signbit(x);
    const bool quotient_negative = x_negative != signbit(y);
    const f128_s ax = mag(x);
    const f128_s ay = mag(y);

    f128_s r_abs{};
    std::uint64_t quotient_abs = 0;
    bool fast = false;

    if (ax < ay)
    {
        r_abs = ax;
        fast = true;
    }
    else
    {
        fast = fmod_fast_small_quotient_abs_with_quotient(ax, ay, r_abs, quotient_abs, true);
    }

    if (fast)
    {
        const f128_s half = mul_double_inline(ay, 0.5);
        const int half_cmp = detail::_f128::fmod_compare_remainder_to_half(r_abs, half);
        if (half_cmp > 0 || (half_cmp == 0 && ((quotient_abs & 1u) != 0u)))
        {
            r_abs = sub_inline(r_abs, ay);
            ++quotient_abs;
        }

        if (quo)
            *quo = detail::fp::remquo_low_quotient_bits(quotient_abs, quotient_negative);

        f128_s r = x_negative ? -r_abs : r_abs;
        if (iszero(r))
            return detail::_f128::signed_zero(x_negative);

        return r;
    }

    std::uint64_t quotient_mod = 0;
    r_abs = fmod_exact_fixed_limb_abs_with_quotient_mod(ax, ay, quotient_mod);
    const f128_s half = mul_double_inline(ay, 0.5);
    const int half_cmp = detail::_f128::fmod_compare_remainder_to_half(r_abs, half);

    if (half_cmp > 0)
    {
        r_abs = sub_inline(r_abs, ay);
        ++quotient_mod;
    }
    else if (half_cmp == 0 && ((quotient_mod & 1u) != 0u))
    {
        r_abs = sub_inline(r_abs, ay);
        ++quotient_mod;
    }

    if (quo)
        *quo = detail::fp::remquo_low_quotient_bits(quotient_mod, quotient_negative);

    f128_s r = x_negative ? -r_abs : r_abs;
    if (iszero(r))
        return detail::_f128::signed_zero(x_negative);

    return r;
}

// fractional decomposition
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::modf(const f128_s& x, f128_s* iptr) noexcept
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
        return detail::_f128::signed_zero(signbit(x));
    }

    const f128_s i = detail::_f128_impl::trunc(x);
    if (iptr)
        *iptr = i;

    f128_s frac = sub_inline(x, i);
    if (iszero(frac))
        frac = detail::_f128::signed_zero(signbit(x));
    return frac;
}

// decomposition and scaling
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::ldexp(const f128_s& x, int e)
{
    if (detail::fp::iszero_or_inf_or_nan(x.hi)) [[unlikely]]
        return x;

    return _ldexp(x, e);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::frexp(const f128_s& x, int* exp) noexcept
{
    if (exp)
        *exp = 0;

    constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
    constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
    constexpr std::uint64_t sign_mask     = 0x8000000000000000ull;

    const std::uint64_t hi_bits = std::bit_cast<std::uint64_t>(x.hi);
    const std::uint32_t hi_exponent =
        static_cast<std::uint32_t>((hi_bits & exponent_mask) >> 52);

    if (hi_exponent == 0x7ffu || ((hi_bits & ~sign_mask) == 0u && x.lo == 0.0))
        return x;

    int e = 0;
    double scaled_hi = 0.0;
    double scaled_lo = 0.0;

    if (hi_exponent != 0u && hi_exponent != 0x7ffu)
    {
        e = static_cast<int>(hi_exponent) - 1022;
        scaled_hi = std::bit_cast<double>(
            (hi_bits & (sign_mask | fraction_mask)) |
            (std::uint64_t{ 1022 } << 52));

        const std::uint64_t lo_bits = std::bit_cast<std::uint64_t>(x.lo);
        const std::uint32_t lo_exponent =
            static_cast<std::uint32_t>((lo_bits & exponent_mask) >> 52);
        const int scaled_lo_exponent = static_cast<int>(lo_exponent) - e;

        if (x.lo == 0.0)
        {
            scaled_lo = x.lo;
        }
        else if (lo_exponent != 0u && lo_exponent != 0x7ffu &&
                 scaled_lo_exponent > 0 && scaled_lo_exponent < 0x7ff)
        {
            scaled_lo = std::bit_cast<double>(
                (lo_bits & (sign_mask | fraction_mask)) |
                (static_cast<std::uint64_t>(scaled_lo_exponent) << 52));
        }
        else
        {
            scaled_lo = detail::fp::ldexp(x.lo, -e);
        }

        if ((scaled_hi == 0.5 && scaled_lo < 0.0) ||
            (scaled_hi == -0.5 && scaled_lo > 0.0))
        {
            scaled_hi *= 2.0;
            scaled_lo *= 2.0;
            --e;
        }

        if (exp)
            *exp = e;

        return f128_s{ scaled_hi, scaled_lo };
    }
    else
    {
        const double lead = (x.hi != 0.0) ? x.hi : x.lo;
        e = detail::fp::frexp_exponent(lead);
        scaled_hi = detail::fp::ldexp(x.hi, -e);
        scaled_lo = detail::fp::ldexp(x.lo, -e);
    }

    const double hi = scaled_hi + scaled_lo;
    f128_s m{hi, (scaled_hi - hi) + scaled_lo};
    const double abs_hi = (m.hi < 0.0) ? -m.hi : m.hi;
    const double abs_lo = (m.hi < 0.0) ? -m.lo : m.lo;

    if (abs_hi == 0.5 && abs_lo < 0.0)
    {
        m.hi *= 2.0;
        m.lo *= 2.0;
        --e;
    }
    else if (abs_hi == 1.0 && abs_lo >= 0.0)
    {
        m.hi *= 0.5;
        m.lo *= 0.5;
        ++e;
    }

    if (exp)
        *exp = e;

    return m;
}

[[nodiscard]] BL_FORCE_INLINE constexpr int detail::_f128_impl::ilogb(const f128_s& x) noexcept
{
    if (isnan(x))
        return FP_ILOGBNAN;
    if (iszero(x))
        return FP_ILOGB0;
    if (isinf(x))
        return std::numeric_limits<int>::max();

    return ilogb_finite_fast(x);
}

[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::logb(const f128_s& x) noexcept
{
    if (isnan(x))
        return x;
    if (iszero(x))
        return f128_s{ -std::numeric_limits<double>::infinity(), 0.0 };
    if (isinf(x))
        return std::numeric_limits<f128_s>::infinity();

    return f128_s{ static_cast<double>(ilogb_finite_fast(x)), 0.0 };
}

// adjacent values
[[nodiscard]] BL_FORCE_INLINE constexpr f128_s detail::_f128_impl::nextafter(const f128_s& from, const f128_s& to) noexcept
{
    if (detail::fp::isnan(from.hi) || detail::fp::isnan(to.hi))
        return std::numeric_limits<f128_s>::quiet_NaN();
    if (from == to)
        return to;
    if (iszero(from))
        return signbit(to)
            ? f128_s{ -std::numeric_limits<double>::denorm_min(), 0.0 }
            : f128_s{  std::numeric_limits<double>::denorm_min(), 0.0 };
    if (isinf(from))
        return signbit(from)
            ? -std::numeric_limits<f128_s>::max()
            :  std::numeric_limits<f128_s>::max();

    const bool upward = from < to;
    const bool toward_smaller_magnitude = upward == signbit(from);
    const double step = detail::fp::nominal_ulp_step(
        from.hi,
        from.lo,
        toward_smaller_magnitude,
        std::numeric_limits<f128_s>::digits);

    return renorm(
        from.hi,
        upward ? from.lo + step : from.lo - step
    );
}

} // namespace bl

#endif
