/**
 * fltx/fqd_limits.h - std::numeric_limits specializations for qd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_LIMITS_INCLUDED
#define FQD_LIMITS_INCLUDED
#include <limits>

#include "fltx/fqd_type.h"

// C++23 deprecates std::float_denorm_style even though numeric_limits still
// requires has_denorm to use it.
BL_DEPRECATED_DECLARATIONS_WARNING_PUSH

template<>
struct std::numeric_limits<bl::fqd_s>
{
    static constexpr bool is_specialized = true;

    // Public precision and range describe the nominal 212-bit floating-point
    // model. max_digits10 also accounts for the bounded adjacent-tail spacing
    // retained by normalized expansions.
    static constexpr bl::fqd_s min() noexcept
    {
        return { 0x1p-863, 0.0, 0.0, 0.0 };
    }

    static constexpr bl::fqd_s max() noexcept
    {
        return {
            0x1.fffffffffffffp+1023,
            0x1.fffffffffffffp+969,
            0x1.fffffffffffffp+915,
            0x1.fffffffffffffp+861
        };
    }

    static constexpr bl::fqd_s lowest()  noexcept { return -max(); }
    static constexpr bl::fqd_s highest() noexcept { return max(); }

    static constexpr int digits       = 212;
    static constexpr int digits10     = 63;
    static constexpr int max_digits10 = 67;

    static constexpr bool is_signed  = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact   = false;
    static constexpr int radix = 2;

    static constexpr bl::fqd_s epsilon()     noexcept { return bl::fqd_s::eps(); }
    static constexpr bl::fqd_s round_error() noexcept { return { 0.5, 0.0, 0.0, 0.0 }; }

    static constexpr int min_exponent   = -862;
    static constexpr int min_exponent10 = -259;
    static constexpr int max_exponent   = numeric_limits<double>::max_exponent;
    static constexpr int max_exponent10 = numeric_limits<double>::max_exponent10;

    static constexpr bool has_infinity      = true;
    static constexpr bool has_quiet_NaN     = true;
    static constexpr bool has_signaling_NaN = true;
    static constexpr float_denorm_style has_denorm = denorm_present;
    static constexpr bool has_denorm_loss = false;

    static constexpr bl::fqd_s infinity()      noexcept { return { numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 }; }
    static constexpr bl::fqd_s quiet_NaN()     noexcept { return { numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 }; }
    static constexpr bl::fqd_s signaling_NaN() noexcept { return { numeric_limits<double>::signaling_NaN(), 0.0, 0.0, 0.0 }; }
    static constexpr bl::fqd_s denorm_min()    noexcept { return { numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 }; }

    static constexpr bool is_iec559  = false; // not IEEE-754 compliant
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo  = false;

    static constexpr bool traps           = false;
    static constexpr bool tinyness_before = false;

    static constexpr float_round_style round_style = round_to_nearest;
};

BL_DEFINE_FLOAT_WRAPPER_NUMERIC_LIMITS(bl::fqd, bl::fqd_s)

BL_DEPRECATED_DECLARATIONS_WARNING_POP

#endif
