/**
 * fltx/f256_limits.h - std::numeric_limits specializations for f256.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F256_LIMITS_INCLUDED
#define F256_LIMITS_INCLUDED
#include <limits>

#include "fltx/f256_type.h"

// C++23 deprecates std::float_denorm_style even though numeric_limits still
// requires has_denorm to use it.
BL_DEPRECATED_DECLARATIONS_WARNING_PUSH

template<>
struct std::numeric_limits<bl::f256_s>
{
    static constexpr bool is_specialized = true;

    // Public limits describe the nominal 212-bit floating-point model. The
    // expansion representation may retain additional sparse-tail information.
    static constexpr bl::f256_s min() noexcept
    {
        return { 0x1p-863, 0.0, 0.0, 0.0 };
    }

    static constexpr bl::f256_s max() noexcept
    {
        return {
            0x1.fffffffffffffp+1023,
            0x1.fffffffffffffp+969,
            0x1.fffffffffffffp+915,
            0x1.fffffffffffffp+861
        };
    }

    static constexpr bl::f256_s lowest()  noexcept { return -max(); }
    static constexpr bl::f256_s highest() noexcept { return max(); }

    static constexpr int digits       = 212;
    static constexpr int digits10     = 63;
    static constexpr int max_digits10 = 65;

    static constexpr bool is_signed  = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact   = false;
    static constexpr int radix = 2;

    static constexpr bl::f256_s epsilon()     noexcept { return bl::f256_s::eps(); }
    static constexpr bl::f256_s round_error() noexcept { return { 0.5, 0.0, 0.0, 0.0 }; }

    static constexpr int min_exponent   = -862;
    static constexpr int min_exponent10 = -259;
    static constexpr int max_exponent   = numeric_limits<double>::max_exponent;
    static constexpr int max_exponent10 = numeric_limits<double>::max_exponent10;

    static constexpr bool has_infinity      = true;
    static constexpr bool has_quiet_NaN     = true;
    static constexpr bool has_signaling_NaN = true;
    static constexpr float_denorm_style has_denorm = denorm_present;
    static constexpr bool has_denorm_loss = false;

    static constexpr bl::f256_s infinity()      noexcept { return { numeric_limits<double>::infinity(), 0.0, 0.0, 0.0 }; }
    static constexpr bl::f256_s quiet_NaN()     noexcept { return { numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0 }; }
    static constexpr bl::f256_s signaling_NaN() noexcept { return { numeric_limits<double>::signaling_NaN(), 0.0, 0.0, 0.0 }; }
    static constexpr bl::f256_s denorm_min()    noexcept { return { numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 }; }

    static constexpr bool is_iec559  = false; // not IEEE-754 compliant
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo  = false;

    static constexpr bool traps           = false;
    static constexpr bool tinyness_before = false;

    static constexpr float_round_style round_style = round_to_nearest;
};

BL_DEFINE_FLOAT_WRAPPER_NUMERIC_LIMITS(bl::f256, bl::f256_s)

BL_DEPRECATED_DECLARATIONS_WARNING_POP

#endif
