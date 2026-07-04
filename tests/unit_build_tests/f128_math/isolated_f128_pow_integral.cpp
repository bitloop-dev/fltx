#include <cstdint>

#include <fltx/f128_math.h>
#include "isolated_runtime.h"

void isolated_f128_pow_integral()
{
    bl::f128 input = bl::isolated::runtime_f128(1.23456789);

    int signed_exponent = bl::isolated::runtime_i32(-3);
    std::uintmax_t unsigned_exponent =
        static_cast<std::uintmax_t>(bl::isolated::runtime_long(9));

    bl::f128 signed_value   = bl::pow(input, signed_exponent);
    bl::f128 unsigned_value = bl::pow(input, unsigned_exponent);

    bl::isolated::keep_value(signed_value);
    bl::isolated::keep_value(unsigned_value);
}
