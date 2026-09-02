/**
 * fltx/fdd_limits.h - std::numeric_limits specializations for dd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_LIMITS_INCLUDED
#define FDD_LIMITS_INCLUDED
#include <limits>

#include "fltx/fdd_type.h"

// C++23 deprecates std::float_denorm_style even though numeric_limits still
// requires has_denorm to use it.
BL_DEPRECATED_DECLARATIONS_WARNING_PUSH

template<>
struct std::numeric_limits<bl::fdd_s>
{
    static constexpr bool is_specialized = true;

    // Public precision and range describe the nominal 106-bit floating-point
    // model. max_digits10 also accounts for the bounded adjacent-tail spacing
    // retained by normalized expansions.
    static constexpr bl::fdd_s min() noexcept
    {
        return { 0x1p-969, 0.0 };
    }

    static constexpr bl::fdd_s max() noexcept
    {
        return {
            0x1.fffffffffffffp+1023,
            0x1.fffffffffffffp+969
        };
    }

    static constexpr bl::fdd_s lowest()  noexcept { return -max(); }
    static constexpr bl::fdd_s highest() noexcept { return max(); }

    static constexpr int digits       = 106; // ~53 bits * 2
    static constexpr int digits10     = 31;  // log10(2^106) ~= 31.9
    static constexpr int max_digits10 = 34;

    static constexpr bool is_signed  = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact   = false;
    static constexpr int radix = 2;

    static constexpr bl::fdd_s epsilon()     noexcept { return { 0x1p-105, 0.0 }; }
    static constexpr bl::fdd_s round_error() noexcept { return { 0.5, 0.0 }; }

    static constexpr int min_exponent   = -968;
    static constexpr int min_exponent10 = -291;
    static constexpr int max_exponent   = numeric_limits<double>::max_exponent;
    static constexpr int max_exponent10 = numeric_limits<double>::max_exponent10;

    static constexpr bool has_infinity      = true;
    static constexpr bool has_quiet_NaN     = true;
    static constexpr bool has_signaling_NaN = true;
    static constexpr float_denorm_style has_denorm = denorm_present;
    static constexpr bool has_denorm_loss = false;

    static constexpr bl::fdd_s infinity()      noexcept { return { numeric_limits<double>::infinity(), 0.0 }; }
    static constexpr bl::fdd_s quiet_NaN()     noexcept { return { numeric_limits<double>::quiet_NaN(), 0.0 }; }
    static constexpr bl::fdd_s signaling_NaN() noexcept { return { numeric_limits<double>::signaling_NaN(), 0.0 }; }
    static constexpr bl::fdd_s denorm_min()    noexcept { return { numeric_limits<double>::denorm_min(), 0.0 }; }

    static constexpr bool is_iec559  = false; // not IEEE-754 compliant
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo  = false;

    static constexpr bool traps           = false;
    static constexpr bool tinyness_before = false;

    static constexpr float_round_style round_style = round_to_nearest;
};

BL_DEFINE_FLOAT_WRAPPER_NUMERIC_LIMITS(bl::fdd, bl::fdd_s)

BL_DEPRECATED_DECLARATIONS_WARNING_POP

#endif
