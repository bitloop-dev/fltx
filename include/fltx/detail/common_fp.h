/**
 * fltx/detail/common_fp.h - Shared low-level constexpr floating-point logic.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_COMMON_FP_INCLUDED
#define FLTX_DETAIL_COMMON_FP_INCLUDED
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "fltx/config.h"

// Discard arithmetic-only constant-evaluation guards structurally when the compiler supports it.
#if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
  #define BL_FP_IF_CONSTEVAL_WARNING_PUSH
  #define BL_FP_IF_CONSTEVAL_WARNING_POP
  #define BL_FP_IF_CONSTEVAL if (true)
#elif !BL_HAS_IF_CONSTEVAL && defined(__clang__) && (__clang_major__ >= 14)
  #define BL_FP_IF_CONSTEVAL_WARNING_PUSH \
      _Pragma("clang diagnostic push")                  \
      _Pragma("clang diagnostic ignored \"-Wc++23-extensions\"")
  #define BL_FP_IF_CONSTEVAL_WARNING_POP _Pragma("clang diagnostic pop")
  #define BL_FP_IF_CONSTEVAL if consteval
#else
  #define BL_FP_IF_CONSTEVAL_WARNING_PUSH BL_IF_CONSTEVAL_WARNING_PUSH
  #define BL_FP_IF_CONSTEVAL_WARNING_POP BL_IF_CONSTEVAL_WARNING_POP
  #define BL_FP_IF_CONSTEVAL BL_IF_CONSTEVAL
#endif

#if FLTX_X86_FMA_RUNTIME_CHECK
#  include <atomic>
#endif

#if FLTX_USE_SCALAR_X86_FMA || FLTX_GUARDED_X86_FMA
#  include <immintrin.h>
#endif

namespace bl::detail::fp
{

inline constexpr std::uint64_t exact_double_integer_limit = 9007199254740992ull;
inline constexpr double exact_double_integer_limit_double = 9007199254740992.0;
inline constexpr double double_integer_threshold          = 4503599627370496.0;

template<class T>
inline constexpr bool is_integer_scalar_v = std::is_integral_v<std::remove_cv_t<T>> && (sizeof(std::remove_cv_t<T>) <= 8);

template<class T>
inline constexpr bool is_native_arithmetic_scalar_v =
    is_integer_scalar_v<T> || std::is_floating_point_v<std::remove_cv_t<T>>;

template<class T>
inline constexpr bool integer_type_fits_exact_double_v = std::is_integral_v<std::remove_cv_t<T>> && (sizeof(std::remove_cv_t<T>) < 8);

struct double_double
{
    double hi, lo;
};

BL_FORCE_INLINE constexpr bool isinf(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "is_inf bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
    return (bits & 0x7fffffffffffffffULL) == 0x7ff0000000000000ULL;
}

BL_FORCE_INLINE constexpr bool isinf(float value) noexcept
{
    static_assert(std::numeric_limits<float>::is_iec559,
        "is_inf bit-pattern check requires IEEE 754 / IEC 559 float");

    const std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
    return (bits & 0x7fffffffu) == 0x7f800000u;
}

BL_FORCE_INLINE constexpr bool isfinite(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "isfinite bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
    return (bits & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL;
}

BL_FORCE_INLINE constexpr bool isfinite(float value) noexcept
{
    static_assert(std::numeric_limits<float>::is_iec559,
        "isfinite bit-pattern check requires IEEE 754 / IEC 559 float");

    const std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
    return (bits & 0x7f800000u) != 0x7f800000u;
}

BL_FORCE_INLINE constexpr bool isinf_or_nan(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "isinf_or_nan bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
    return (bits & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL;
}

BL_FORCE_INLINE constexpr bool isinf_or_nan(double first, double second) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "isinf_or_nan bit-pattern check requires IEEE 754 / IEC 559 double");

    constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
    constexpr std::uint64_t inf_bits = 0x7ff0000000000000ull;
    const std::uint64_t first_abs_bits = std::bit_cast<std::uint64_t>(first) & ~sign_mask;
    const std::uint64_t second_abs_bits = std::bit_cast<std::uint64_t>(second) & ~sign_mask;

    return first_abs_bits >= inf_bits || second_abs_bits >= inf_bits;
}

BL_FORCE_INLINE constexpr bool isinf(double first, double second) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "isinf bit-pattern check requires IEEE 754 / IEC 559 double");

    constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
    constexpr std::uint64_t inf_bits = 0x7ff0000000000000ull;
    const std::uint64_t first_abs_bits = std::bit_cast<std::uint64_t>(first) & ~sign_mask;
    const std::uint64_t second_abs_bits = std::bit_cast<std::uint64_t>(second) & ~sign_mask;

    return first_abs_bits == inf_bits || second_abs_bits == inf_bits;
}

BL_FORCE_INLINE constexpr bool iszero_or_nan(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "iszero_or_nan bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t abs_bits = std::bit_cast<std::uint64_t>(value) & 0x7fffffffffffffffULL;
    return (abs_bits - 1ULL) > 0x7fefffffffffffffULL;
}

BL_FORCE_INLINE constexpr bool iszero_or_inf_or_nan(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "iszero_or_inf_or_nan bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t abs_bits = std::bit_cast<std::uint64_t>(value) & 0x7fffffffffffffffULL;
    return (abs_bits - 1ULL) >= 0x7fefffffffffffffULL;
}

BL_FORCE_INLINE constexpr bool iszero_or_negative_or_inf_or_nan(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "iszero_or_negative_or_inf_or_nan bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
    return (bits - 1ULL) >= 0x7fefffffffffffffULL;
}

BL_FORCE_INLINE constexpr bool isposinf(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "isposinf bit-pattern check requires IEEE 754 / IEC 559 double");

    return std::bit_cast<std::uint64_t>(value) == 0x7ff0000000000000ULL;
}

BL_FORCE_INLINE constexpr bool isnan(double value) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559,
        "is_nan bit-pattern check requires IEEE 754 / IEC 559 double");

    const std::uint64_t bits     = std::bit_cast<std::uint64_t>(value);
    const std::uint64_t abs_bits = bits & 0x7fffffffffffffffULL;
    return abs_bits > 0x7ff0000000000000ULL;
}

BL_FORCE_INLINE constexpr bool isnan(float value) noexcept
{
    static_assert(std::numeric_limits<float>::is_iec559,
        "is_nan bit-pattern check requires IEEE 754 / IEC 559 float");

    const std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
    return (bits & 0x7fffffffu) > 0x7f800000u;
}

BL_FORCE_INLINE constexpr double absd(double x) noexcept
{
    return std::bit_cast<double>(
        std::bit_cast<std::uint64_t>(x) & 0x7fffffffffffffffULL);
}

BL_FORCE_INLINE constexpr int frexp_exponent(double x) noexcept
{
    if (iszero_or_inf_or_nan(x))
        return 0;

    const std::uint64_t bits = std::bit_cast<std::uint64_t>(x);
    const std::uint32_t exp_bits = static_cast<std::uint32_t>((bits >> 52) & 0x7ffu);
    if (exp_bits != 0)
        return static_cast<int>(exp_bits) - 1022;

    std::uint64_t frac = bits & ((std::uint64_t{ 1 } << 52) - 1);
    int e = -1022;
    while ((frac & (std::uint64_t{ 1 } << 52)) == 0)
    {
        frac <<= 1;
        --e;
    }
    return e + 1;
}

BL_FORCE_INLINE constexpr int frexp_exponent_limb(double value) noexcept
{
    if (bl::detail::is_constant_evaluated())
    {
        return frexp_exponent(value);
    }

    int exponent = 0;
    (void)std::frexp(value, &exponent);
    return exponent;
}

BL_FORCE_INLINE constexpr int highest_bit_index(std::uint64_t value) noexcept
{
    int index = -1;
    while (value != 0)
    {
        value >>= 1;
        ++index;
    }
    return index;
}

[[nodiscard]] BL_FORCE_INLINE constexpr int bit_length_u64(std::uint64_t value) noexcept
{
    int bits = 0;
    while (value != 0)
    {
        ++bits;
        value >>= 1;
    }
    return bits;
}

struct pow2_scale_info
{
    bool valid = false;
    bool negative = false;
    int exponent = 0;
};

BL_FORCE_INLINE constexpr pow2_scale_info exact_pow2_scale_info(double value) noexcept
{
    constexpr std::uint64_t sign_mask     = 0x8000000000000000ull;
    constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
    constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;

    const std::uint64_t bits     = std::bit_cast<std::uint64_t>(value);
    const std::uint64_t abs_bits = bits & ~sign_mask;
    const bool negative = (bits & sign_mask) != 0;

    if (abs_bits == 0 || abs_bits >= exponent_mask)
        return { false, negative, 0 };

    const std::uint32_t exponent_bits = static_cast<std::uint32_t>((abs_bits & exponent_mask) >> 52);
    const std::uint64_t fraction = abs_bits & fraction_mask;

    if (exponent_bits != 0)
        return { fraction == 0, negative, static_cast<int>(exponent_bits) - 1023 };

    if ((fraction & (fraction - 1)) != 0)
        return { false, negative, 0 };

    return { true, negative, highest_bit_index(fraction) - 1074 };
}

BL_FORCE_INLINE constexpr bool abs_double_is_power_of_two(double value) noexcept
{
    return exact_pow2_scale_info(value).valid;
}

[[nodiscard]] BL_FORCE_INLINE constexpr double positive_power_of_two(int exponent) noexcept
{
    if (exponent >= -1022)
    {
        return std::bit_cast<double>(
            static_cast<std::uint64_t>(exponent + 1023) << 52);
    }
    return std::bit_cast<double>(
        std::uint64_t{ 1 } << static_cast<unsigned>(exponent + 1074));
}

[[nodiscard]] BL_FORCE_INLINE constexpr double nominal_ulp_step(
    double leading,
    double trailing,
    bool toward_smaller_magnitude,
    int precision_bits) noexcept
{
    constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
    constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;

    const std::uint64_t bits = std::bit_cast<std::uint64_t>(leading);
    const std::uint32_t exponent_bits =
        static_cast<std::uint32_t>((bits >> 52) & 0x7ffu);
    const std::uint64_t fraction = bits & fraction_mask;

    if (exponent_bits != 0 && fraction != 0)
    {
        const int step_exponent_bits =
            static_cast<int>(exponent_bits) - (precision_bits - 1);
        if (step_exponent_bits > 0)
        {
            return std::bit_cast<double>(
                static_cast<std::uint64_t>(step_exponent_bits) << 52);
        }

        const int step_exponent =
            static_cast<int>(exponent_bits) - 1023 + 1 - precision_bits;
        return positive_power_of_two(step_exponent < -1074 ? -1074 : step_exponent);
    }

    const bool opposite_trailing_sign =
        trailing != 0.0 &&
        ((bits & sign_mask) !=
         (std::bit_cast<std::uint64_t>(trailing) & sign_mask));
    const int lower_binade_adjustment =
        opposite_trailing_sign ||
        (trailing == 0.0 && toward_smaller_magnitude);

    if (exponent_bits != 0)
    {
        const int step_exponent_bits =
            static_cast<int>(exponent_bits) - (precision_bits - 1) -
            lower_binade_adjustment;
        if (step_exponent_bits > 0)
        {
            return std::bit_cast<double>(
                static_cast<std::uint64_t>(step_exponent_bits) << 52);
        }

        const int step_exponent =
            static_cast<int>(exponent_bits) - 1023 + 1 - precision_bits -
            lower_binade_adjustment;
        return positive_power_of_two(step_exponent < -1074 ? -1074 : step_exponent);
    }

    const int binade = highest_bit_index(fraction) - 1074;
    const int step_exponent = binade + 1 - precision_bits;
    return positive_power_of_two(step_exponent < -1074 ? -1074 : step_exponent);
}

BL_FORCE_INLINE constexpr double scalbn(double value, int exp) noexcept
{
    if (exp == 0 || iszero_or_inf_or_nan(value))
        return value;

    constexpr std::uint64_t sign_mask     = 0x8000000000000000ull;
    constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;
    constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
    constexpr std::uint64_t hidden_bit    = 0x0010000000000000ull;

    const std::uint64_t bits     = std::bit_cast<std::uint64_t>(value);
    const std::uint64_t sign     = bits & sign_mask;
    const std::uint64_t fraction = bits & fraction_mask;
    const std::uint32_t exponent_bits = static_cast<std::uint32_t>((bits & exponent_mask) >> 52);

    std::uint64_t significand = 0;
    long long unbiased_exponent = 0;

    if (exponent_bits != 0)
    {
        significand = hidden_bit | fraction;
        unbiased_exponent = static_cast<int>(exponent_bits) - 1023;
    }
    else
    {
        const int msb_index = highest_bit_index(fraction);
        significand = fraction << (52 - msb_index);
        unbiased_exponent = static_cast<long long>(msb_index) - 1074ll;
    }

    const long long new_unbiased_exponent = unbiased_exponent + static_cast<long long>(exp);

    if (new_unbiased_exponent > 1023)
        return std::bit_cast<double>(sign | exponent_mask);

    if (new_unbiased_exponent >= -1022)
    {
        const std::uint64_t new_exponent_bits =
            static_cast<std::uint64_t>(new_unbiased_exponent + 1023) << 52;
        const std::uint64_t new_fraction = significand & fraction_mask;
        return std::bit_cast<double>(sign | new_exponent_bits | new_fraction);
    }

    const long long shift = -1022ll - new_unbiased_exponent;
    if (shift >= 64)
        return std::bit_cast<double>(sign);

    const unsigned shift_u = static_cast<unsigned>(shift);

    std::uint64_t subnormal_fraction = 0;
    if (shift_u == 0)
    {
        subnormal_fraction = significand;
    }
    else
    {
        const std::uint64_t truncated = significand >> shift_u;
        const std::uint64_t remainder_mask = (std::uint64_t{ 1 } << shift_u) - 1;
        const std::uint64_t remainder = significand & remainder_mask;
        const std::uint64_t halfway = std::uint64_t{ 1 } << (shift_u - 1);
        const bool round_up =
            (remainder > halfway) ||
            (remainder == halfway && (truncated & 1u) != 0);

        subnormal_fraction = truncated + static_cast<std::uint64_t>(round_up);
    }

    if (subnormal_fraction >= hidden_bit)
        return std::bit_cast<double>(sign | (std::uint64_t{ 1 } << 52));

    if (subnormal_fraction == 0)
        return std::bit_cast<double>(sign);

    return std::bit_cast<double>(sign | subnormal_fraction);
}

BL_FORCE_INLINE constexpr double ldexp(double value, int exp) noexcept
{
    return scalbn(value, exp);
}

BL_FORCE_INLINE constexpr double ldexp_limb(double value, int exponent) noexcept
{
    if (bl::detail::is_constant_evaluated())
    {
        return ldexp(value, exponent);
    }

    return std::ldexp(value, exponent);
}

BL_FORCE_INLINE constexpr bool signbit(double x) noexcept
{
    const std::uint64_t bits = std::bit_cast<std::uint64_t>(x);
    return (bits >> 63) != 0;
}

BL_FORCE_INLINE constexpr bool signbit(float x) noexcept
{
    return (std::bit_cast<std::uint32_t>(x) & 0x80000000u) != 0u;
}

BL_FORCE_INLINE constexpr double fabs(double x) noexcept
{
    return absd(x);
}

BL_FORCE_INLINE constexpr float fabs(float x) noexcept
{
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(x) & 0x7fffffffu);
}

BL_FORCE_INLINE constexpr double copysign(double magnitude, double sign_source) noexcept
{
    const std::uint64_t magnitude_bits = std::bit_cast<std::uint64_t>(magnitude) & 0x7fffffffffffffffULL;
    const std::uint64_t sign_bits = std::bit_cast<std::uint64_t>(sign_source) & 0x8000000000000000ULL;
    return std::bit_cast<double>(magnitude_bits | sign_bits);
}

BL_FORCE_INLINE constexpr float copysign(float magnitude, float sign_source) noexcept
{
    const std::uint32_t magnitude_bits = std::bit_cast<std::uint32_t>(magnitude) & 0x7fffffffu;
    const std::uint32_t sign_bits = std::bit_cast<std::uint32_t>(sign_source) & 0x80000000u;
    return std::bit_cast<float>(magnitude_bits | sign_bits);
}

BL_FORCE_INLINE constexpr double floor(double x) noexcept
{
    if (iszero_or_inf_or_nan(x))
        return x;

    const double ax = absd(x);
    if (ax >= double_integer_threshold)
        return x;

    const long long i = static_cast<long long>(x);
    double di = static_cast<double>(i);
    if (di > x) di -= 1.0;
    if (di == 0.0) return signbit(x) ? -0.0 : 0.0;
    return di;
}

BL_FORCE_INLINE constexpr double ceil(double x) noexcept
{
    if (iszero_or_inf_or_nan(x))
        return x;

    const double ax = absd(x);
    if (ax >= double_integer_threshold)
        return x;

    const long long i = static_cast<long long>(x);
    double di = static_cast<double>(i);
    if (di < x)
        di += 1.0;
    if (di == 0.0)
        return signbit(x) ? -0.0 : 0.0;
    return di;
}

BL_FORCE_INLINE constexpr double trunc(double x) noexcept
{
    return signbit(x) ? ceil(x) : floor(x);
}

namespace conversion
{
    struct signed_integer_accumulator
    {
        std::uint64_t low{};
        int high{};

        BL_FORCE_INLINE constexpr void add(double value) noexcept
        {
            if (value == 0.0)
                return;

            const bool negative = signbit(value);
            const double magnitude = absd(value);
            const bool high_unit = magnitude >= 0x1p64;
            const std::uint64_t term = high_unit
                ? std::uint64_t{ 0 }
                : static_cast<std::uint64_t>(magnitude);

            if (negative)
            {
                const std::uint64_t previous = low;
                low -= term;
                high -= static_cast<int>(high_unit) + static_cast<int>(previous < term);
            }
            else
            {
                const std::uint64_t previous = low;
                low += term;
                high += static_cast<int>(high_unit) + static_cast<int>(low < previous);
            }
        }
    };

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr T accumulator_to_integer(
        signed_integer_accumulator value) noexcept
    {
        using target_type = std::remove_cv_t<T>;
        static_assert(is_integer_scalar_v<target_type> && !std::is_same_v<target_type, bool>);

        if constexpr (std::is_unsigned_v<target_type>)
        {
            return static_cast<target_type>(value.low);
        }
        else
        {
            if (value.high >= 0)
                return static_cast<target_type>(value.low);

            const std::uint64_t magnitude = std::uint64_t{ 0 } - value.low;
            if constexpr (sizeof(target_type) == sizeof(std::int64_t))
            {
                if (magnitude == (std::uint64_t{ 1 } << 63))
                    return std::numeric_limits<target_type>::lowest();
            }
            return static_cast<target_type>(-static_cast<std::int64_t>(magnitude));
        }
    }

    template<class T, class... Tail>
    [[nodiscard]] BL_FORCE_INLINE constexpr T expansion_to_integer(
        double leading,
        Tail... tail) noexcept
    {
        static_assert(is_integer_scalar_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>);
        static_assert((std::is_same_v<double, std::remove_cv_t<Tail>> && ...));

        const double limbs[] = { leading, tail... };
        bool negative = signbit(leading);
        if (leading == 0.0)
        {
            for (double limb : limbs)
            {
                if (limb != 0.0)
                {
                    negative = signbit(limb);
                    break;
                }
            }
        }

        signed_integer_accumulator result{};
        for (double limb : limbs)
        {
            const double integral = negative ? ceil(limb) : floor(limb);
            result.add(integral);
            if (integral != limb)
                break;
        }
        return accumulator_to_integer<T>(result);
    }

    template<class T, class... Tail>
    [[nodiscard]] BL_FORCE_INLINE constexpr T expansion_to_floating(
        double leading,
        Tail... tail) noexcept
    {
        static_assert(std::is_floating_point_v<std::remove_cv_t<T>>);
        static_assert((std::is_same_v<double, std::remove_cv_t<Tail>> && ...));

        if (leading == 0.0 && ((tail == 0.0) && ...))
            return static_cast<T>(leading);

        long double result = static_cast<long double>(leading);
        ((result += static_cast<long double>(tail)), ...);
        return static_cast<T>(result);
    }

} // namespace conversion

template<class T, class... Tail>
[[nodiscard]] BL_FORCE_INLINE constexpr T expansion_to_native(
    double leading,
    Tail... tail) noexcept
{
    using target_type = std::remove_cv_t<T>;
    static_assert(is_native_arithmetic_scalar_v<target_type>);

    if constexpr (std::is_same_v<target_type, bool>)
        return ((std::bit_cast<std::uint64_t>(leading) | ... |
                 std::bit_cast<std::uint64_t>(tail)) & 0x7fffffffffffffffULL) != 0;
    else if constexpr (std::is_integral_v<target_type>)
        return conversion::expansion_to_integer<target_type>(leading, tail...);
    else
        return conversion::expansion_to_floating<target_type>(leading, tail...);
}

template<int LimbCount>
BL_FORCE_INLINE constexpr void long_double_to_double_expansion(
    long double value,
    double (&out)[LimbCount]) noexcept
{
    static_assert(LimbCount > 0);

    out[0] = static_cast<double>(value);
    for (int i = 1; i < LimbCount; ++i)
        out[i] = 0.0;

    if (!isfinite(out[0]) || out[0] == 0.0)
        return;

    long double remainder = value - static_cast<long double>(out[0]);
    for (int i = 1; i < LimbCount && remainder != 0.0L; ++i)
    {
        out[i] = static_cast<double>(remainder);
        remainder -= static_cast<long double>(out[i]);
    }
}

BL_FORCE_INLINE constexpr bool double_integer_is_odd(double x) noexcept
{
    const double ax = absd(x);
    if (!isfinite(x) || ax < 1.0 || ax >= exact_double_integer_limit_double)
        return false;
    const long long i = static_cast<long long>(x);
    return (i & 1ll) != 0;
}

// GCC can reassociate error-free transforms under -ffast-math even inside a
// no-fast-math function scope. Empty XMM constraints preserve the required
// rounding points without emitting instructions or affecting constant evaluation.
#if defined(__GNUC__) && !defined(__clang__) && defined(__FAST_MATH__) && defined(__SSE2__)
#define FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER 1
#define FLTX_DETAIL_EFT_BARRIER(value) __asm__ __volatile__("" : "+x"(value))
#else
#define FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER 0
#endif

BL_PUSH_PRECISE
#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
BL_FORCE_INLINE void two_sum_precise_runtime(
    double a,
    double b,
    double& s,
    double& e) noexcept
{
    s = a + b;
    FLTX_DETAIL_EFT_BARRIER(s);
    double bv = s - a;
    FLTX_DETAIL_EFT_BARRIER(bv);
    double av = s - bv;
    FLTX_DETAIL_EFT_BARRIER(av);
    double ar = a - av;
    FLTX_DETAIL_EFT_BARRIER(ar);
    double br = b - bv;
    FLTX_DETAIL_EFT_BARRIER(br);
    e = ar + br;
    FLTX_DETAIL_EFT_BARRIER(e);
}
#endif

BL_FORCE_INLINE constexpr void two_sum_precise(double a, double b, double& s, double& e) noexcept
{
#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
    if (!std::is_constant_evaluated())
    {
        two_sum_precise_runtime(a, b, s, e);
        return;
    }
#endif
    s = a + b;
    double bv = s - a;
    e = (a - (s - bv)) + (b - bv);
}

#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
BL_FORCE_INLINE void two_diff_precise_runtime(
    double a,
    double b,
    double& s,
    double& e) noexcept
{
    s = a - b;
    FLTX_DETAIL_EFT_BARRIER(s);
    double bv = s - a;
    FLTX_DETAIL_EFT_BARRIER(bv);
    double av = s - bv;
    FLTX_DETAIL_EFT_BARRIER(av);
    double ar = a - av;
    FLTX_DETAIL_EFT_BARRIER(ar);
    double br = b + bv;
    FLTX_DETAIL_EFT_BARRIER(br);
    e = ar - br;
    FLTX_DETAIL_EFT_BARRIER(e);
}
#endif

BL_FORCE_INLINE constexpr void two_diff_precise(double a, double b, double& s, double& e) noexcept
{
#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
    if (!std::is_constant_evaluated())
    {
        two_diff_precise_runtime(a, b, s, e);
        return;
    }
#endif
    s = a - b;
    double bv = s - a;
    e = (a - (s - bv)) - (b + bv);
}

#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
BL_FORCE_INLINE void quick_two_sum_precise_runtime(
    double a,
    double b,
    double& s,
    double& e) noexcept
{
    s = a + b;
    FLTX_DETAIL_EFT_BARRIER(s);
    double delta = s - a;
    FLTX_DETAIL_EFT_BARRIER(delta);
    e = b - delta;
    FLTX_DETAIL_EFT_BARRIER(e);
}
#endif

BL_FORCE_INLINE constexpr void quick_two_sum_precise(double a, double b, double& s, double& e) noexcept
{
#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
    if (!std::is_constant_evaluated())
    {
        quick_two_sum_precise_runtime(a, b, s, e);
        return;
    }
#endif
    s = a + b;
    e = b - (s - a);
}

// Adds one value to a nonoverlapping expansion ordered from small to large.
// Input and output may be the same array.
BL_FORCE_INLINE constexpr int grow_expansion_zeroelim(
    int length,
    double* expansion,
    double value) noexcept
{
    double sum = value;
    int out = 0;

    for (int i = 0; i < length; ++i)
    {
        double next{}, error{};
        two_sum_precise(sum, expansion[i], next, error);
        if (error != 0.0)
            expansion[out++] = error;
        sum = next;
    }

    if (sum != 0.0 || out == 0)
        expansion[out++] = sum;
    return out;
}

#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
BL_FORCE_INLINE void two_prod_precise_dekker_runtime(
    double a,
    double b,
    double& p,
    double& err) noexcept
{
    constexpr double split = 134217729.0;

    double a_c = a * split;
    FLTX_DETAIL_EFT_BARRIER(a_c);
    double a_delta = a_c - a;
    FLTX_DETAIL_EFT_BARRIER(a_delta);
    double a_hi = a_c - a_delta;
    FLTX_DETAIL_EFT_BARRIER(a_hi);
    double a_lo = a - a_hi;
    FLTX_DETAIL_EFT_BARRIER(a_lo);

    double b_c = b * split;
    FLTX_DETAIL_EFT_BARRIER(b_c);
    double b_delta = b_c - b;
    FLTX_DETAIL_EFT_BARRIER(b_delta);
    double b_hi = b_c - b_delta;
    FLTX_DETAIL_EFT_BARRIER(b_hi);
    double b_lo = b - b_hi;
    FLTX_DETAIL_EFT_BARRIER(b_lo);

    p = a * b;
    FLTX_DETAIL_EFT_BARRIER(p);
    double residual = a_hi * b_hi;
    FLTX_DETAIL_EFT_BARRIER(residual);
    residual -= p;
    FLTX_DETAIL_EFT_BARRIER(residual);
    double term = a_hi * b_lo;
    FLTX_DETAIL_EFT_BARRIER(term);
    residual += term;
    FLTX_DETAIL_EFT_BARRIER(residual);
    term = a_lo * b_hi;
    FLTX_DETAIL_EFT_BARRIER(term);
    residual += term;
    FLTX_DETAIL_EFT_BARRIER(residual);
    term = a_lo * b_lo;
    FLTX_DETAIL_EFT_BARRIER(term);
    err = residual + term;
    FLTX_DETAIL_EFT_BARRIER(err);
}
#endif

BL_FORCE_INLINE constexpr void two_prod_precise_dekker(double a, double b, double& p, double& err) noexcept
{
#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
    if (!std::is_constant_evaluated())
    {
        two_prod_precise_dekker_runtime(a, b, p, err);
        return;
    }
#endif

    constexpr double split = 134217729.0;
    double a_c = a * split;
    double a_hi = a_c - (a_c - a);
    double a_lo = a - a_hi;
    double b_c = b * split;
    double b_hi = b_c - (b_c - b);
    double b_lo = b - b_hi;

    p = a * b;
    err = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
}

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
inline constexpr double dekker_split_overflow_threshold = 0x1p996;
inline constexpr double dekker_split_underflow_threshold = 0x1p-968;

// Tests whether Dekker splitting needs temporary operand scaling.
[[nodiscard]] BL_FORCE_INLINE constexpr bool dekker_product_needs_scaling(double a, double b) noexcept
{
    const double aa = absd(a);
    const double ab = absd(b);
    return aa > dekker_split_overflow_threshold || ab > dekker_split_overflow_threshold ||
           (aa != 0.0 && aa < dekker_split_underflow_threshold) ||
           (ab != 0.0 && ab < dekker_split_underflow_threshold);
}

template<class ExpUnsigned>
// Predicts whether repeated powers can leave the ordinary Dekker split range.
[[nodiscard]] BL_FORCE_INLINE constexpr bool ipow_loop_needs_range_safe_dekker(double head, ExpUnsigned exp) noexcept
{
    if (exp <= ExpUnsigned{ 4 } || iszero_or_inf_or_nan(head)) [[likely]]
        return false;

    const double magnitude = absd(head);
    if (magnitude <= 1.0)
        return false;

    const int frexp_exponent = frexp_exponent_limb(magnitude);
    const int bits_per_power = frexp_exponent > 1 ? frexp_exponent : 1;
    // Bound the exponent before multiplying: this is the same range test
    // without an integer division, and the product cannot overflow unsigned.
    return exp > 996u || static_cast<unsigned>(exp) * static_cast<unsigned>(bits_per_power) > 996u;
}

// Forms an exact product after scaling operands into Dekker's safe range.
BL_MSVC_NOINLINE constexpr void two_prod_precise_dekker_scaled(double a, double b, double& p, double& err) noexcept
{
    p = a * b;
    if (isinf_or_nan(a) || isinf_or_nan(b) || isinf_or_nan(p))
    {
        err = 0.0;
        return;
    }

    constexpr int scale = 28;
    int a_scale = 0;
    int b_scale = 0;
    if (absd(a) > dekker_split_overflow_threshold)
        a_scale = -scale;
    else if (a != 0.0 && absd(a) < dekker_split_underflow_threshold)
        a_scale = scale;

    if (absd(b) > dekker_split_overflow_threshold)
        b_scale = -scale;
    else if (b != 0.0 && absd(b) < dekker_split_underflow_threshold)
        b_scale = scale;

    const int product_scale = a_scale + b_scale;
    double scaled_product{};
    double scaled_error{};
    two_prod_precise_dekker(ldexp(a, a_scale), ldexp(b, b_scale), scaled_product, scaled_error);

    const double direct_scaled_product = ldexp(p, product_scale);
    err = ldexp((scaled_product - direct_scaled_product) + scaled_error, -product_scale);
}

// Forms an exact Dekker product with scaling only when the operand range requires it.
BL_FORCE_INLINE constexpr void two_prod_precise_dekker_range_safe(double a, double b, double& p, double& err) noexcept
{
    if (dekker_product_needs_scaling(a, b)) [[unlikely]]
    {
        two_prod_precise_dekker_scaled(a, b, p, err);
    }
    else
    {
        two_prod_precise_dekker(a, b, p, err);
    }
}
#else
// Reports that scaling is disabled when range-safe Dekker support is not configured.
[[nodiscard]] BL_FORCE_INLINE constexpr bool dekker_product_needs_scaling(double a, double b) noexcept
{
    (void)a;
    (void)b;
    return false;
}

template<class ExpUnsigned>
// Reports that power-loop scaling is disabled without range-safe Dekker support.
[[nodiscard]] BL_FORCE_INLINE constexpr bool ipow_loop_needs_range_safe_dekker(double head, ExpUnsigned exp) noexcept
{
    (void)head;
    (void)exp;
    return false;
}
#endif
BL_POP_PRECISE

#if FLTX_X86_FMA_RUNTIME_CHECK
[[nodiscard]] bool runtime_x86_fma_available_uncached() noexcept;

inline constinit std::atomic<std::int8_t> x86_fma_available_state{-1};
static_assert(decltype(x86_fma_available_state)::is_always_lock_free);

[[nodiscard]] BL_NO_INLINE inline bool initialize_x86_fma_available() noexcept
{
    const bool available = runtime_x86_fma_available_uncached();
    x86_fma_available_state.store(
        static_cast<std::int8_t>(available),
        std::memory_order_relaxed);
    return available;
}

[[nodiscard]] BL_FORCE_INLINE bool x86_fma_available_cached() noexcept
{
    #if defined(FLTX_TEST_RUNTIME_X86_FMA_AVAILABLE)
    return FLTX_TEST_RUNTIME_X86_FMA_AVAILABLE != 0;
    #else
    const std::int8_t state =
        x86_fma_available_state.load(std::memory_order_relaxed);
    return state >= 0
        ? state != 0
        : initialize_x86_fma_available();
    #endif
}
#endif

[[nodiscard]] BL_FORCE_INLINE bool runtime_hardware_fma_enabled() noexcept
{
    #if FLTX_X86_FMA_RUNTIME_CHECK
    return x86_fma_available_cached();
    #elif FLTX_USE_SCALAR_X86_FMA || FLTX_USE_BASELINE_ARM64_FMA
    return true;
    #else
    return false;
    #endif
}

BL_PUSH_PRECISE
FLTX_X86_FMA_LEAF_INLINE double fmsub_fma(double a, double b, double c) noexcept
{
    #if FLTX_USE_SCALAR_X86_FMA || FLTX_GUARDED_X86_FMA
    const __m128d aw = _mm_set_sd(a);
    const __m128d bw = _mm_set_sd(b);
    const __m128d cw = _mm_set_sd(c);
    return _mm_cvtsd_f64(_mm_fmsub_sd(aw, bw, cw));
    #elif FLTX_USE_BASELINE_ARM64_FMA && (defined(__clang__) || defined(__GNUC__))
    return __builtin_fma(a, b, -c);
    #elif FLTX_USE_BASELINE_ARM64_FMA
    return std::fma(a, b, -c);
    #elif defined(__clang__) || defined(__GNUC__)
    return __builtin_fma(a, b, -c);
    #else
    return std::fma(a, b, -c);
    #endif
}

FLTX_X86_FMA_LEAF_INLINE double fmadd_fma(double a, double b, double c) noexcept
{
    #if FLTX_USE_SCALAR_X86_FMA || FLTX_GUARDED_X86_FMA
    const __m128d aw = _mm_set_sd(a);
    const __m128d bw = _mm_set_sd(b);
    const __m128d cw = _mm_set_sd(c);
    return _mm_cvtsd_f64(_mm_fmadd_sd(aw, bw, cw));
    #elif FLTX_USE_BASELINE_ARM64_FMA && (defined(__clang__) || defined(__GNUC__))
    return __builtin_fma(a, b, c);
    #elif FLTX_USE_BASELINE_ARM64_FMA
    return std::fma(a, b, c);
    #elif defined(__clang__) || defined(__GNUC__)
    return __builtin_fma(a, b, c);
    #else
    return std::fma(a, b, c);
    #endif
}

FLTX_X86_FMA_LEAF_INLINE float fmadd_fma(float a, float b, float c) noexcept
{
    #if FLTX_USE_SCALAR_X86_FMA || FLTX_GUARDED_X86_FMA
    const __m128 aw = _mm_set_ss(a);
    const __m128 bw = _mm_set_ss(b);
    const __m128 cw = _mm_set_ss(c);
    return _mm_cvtss_f32(_mm_fmadd_ss(aw, bw, cw));
    #elif FLTX_USE_BASELINE_ARM64_FMA && (defined(__clang__) || defined(__GNUC__))
    return __builtin_fmaf(a, b, c);
    #elif FLTX_USE_BASELINE_ARM64_FMA
    return std::fma(a, b, c);
    #elif defined(__clang__) || defined(__GNUC__)
    return __builtin_fmaf(a, b, c);
    #else
    return std::fma(a, b, c);
    #endif
}

BL_FORCE_INLINE double fmadd_auto(double a, double b, double c) noexcept
{
    return runtime_hardware_fma_enabled()
        ? fmadd_fma(a, b, c)
        : std::fma(a, b, c);
}

BL_FORCE_INLINE float fmadd_auto(float a, float b, float c) noexcept
{
    return runtime_hardware_fma_enabled()
        ? fmadd_fma(a, b, c)
        : std::fma(a, b, c);
}

BL_FORCE_INLINE void two_prod_fma(
    double a,
    double b,
    double& product,
    double& error) noexcept
{
    product = a * b;
#if FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
    FLTX_DETAIL_EFT_BARRIER(product);
#endif
    error = fmsub_fma(a, b, product);
}
BL_POP_PRECISE
#undef FLTX_DETAIL_GNU_FAST_MATH_EFT_BARRIER
#undef FLTX_DETAIL_EFT_BARRIER

// Selects the available exact-product implementation for ordinary operand ranges.
BL_FORCE_INLINE constexpr void two_prod_precise(double a, double b, double& p, double& err) noexcept
{
    #if FLTX_HAS_RUNTIME_FMA_PATH
    if (bl::detail::is_constant_evaluated()) [[unlikely]]
    {
        two_prod_precise_dekker(a, b, p, err);
    }
    else if (runtime_hardware_fma_enabled())
    {
        two_prod_fma(a, b, p, err);
    }
    else
    {
        two_prod_precise_dekker(a, b, p, err);
    }
    #else
    two_prod_precise_dekker(a, b, p, err);
    #endif
}

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
// Selects the available exact-product implementation with Dekker range protection.
BL_FORCE_INLINE constexpr void two_prod_precise_range_safe(double a, double b, double& p, double& err) noexcept
{
    #if FLTX_HAS_RUNTIME_FMA_PATH
    if (bl::detail::is_constant_evaluated()) [[unlikely]]
    {
        two_prod_precise_dekker_range_safe(a, b, p, err);
    }
    else if (runtime_hardware_fma_enabled())
    {
        two_prod_fma(a, b, p, err);
    }
    else
    {
        two_prod_precise_dekker_range_safe(a, b, p, err);
    }
    #else
    two_prod_precise_dekker_range_safe(a, b, p, err);
    #endif
}
#endif

BL_FORCE_INLINE constexpr void split_uint64_to_doubles(std::uint64_t value, double& hi, double& lo) noexcept
{
    hi = static_cast<double>(value >> 32) * 4294967296.0;
    lo = static_cast<double>(value & 0xFFFFFFFFull);
}

BL_FORCE_INLINE constexpr std::uint64_t magnitude_u64(std::int64_t value) noexcept
{
    return (value < 0) ? (std::uint64_t{0} - static_cast<std::uint64_t>(value)) : static_cast<std::uint64_t>(value);
}

BL_FORCE_INLINE constexpr void uint64_to_exact_double_pair(std::uint64_t value, double& sum, double& err) noexcept
{
    double hi{}, lo{};
    split_uint64_to_doubles(value, hi, lo);
    two_sum_precise(hi, lo, sum, err);
}

BL_FORCE_INLINE constexpr void int64_to_exact_double_pair(std::int64_t value, double& sum, double& err) noexcept
{
    uint64_to_exact_double_pair(magnitude_u64(value), sum, err);
    if (value < 0)
    {
        sum = -sum;
        err = -err;
    }
}

template<class T> [[nodiscard]]
BL_FORCE_INLINE constexpr bool integer_fits_exact_double(T value) noexcept
{
    using clean_t = std::remove_cv_t<T>;
    static_assert(is_integer_scalar_v<clean_t>);

    if constexpr (sizeof(clean_t) < 8)
        return true;
    else if constexpr (std::is_signed_v<clean_t>)
        return magnitude_u64(static_cast<std::int64_t>(value)) <= exact_double_integer_limit;
    else
        return static_cast<std::uint64_t>(value) <= exact_double_integer_limit;
}

} // namespace bl::detail::fp

#endif
