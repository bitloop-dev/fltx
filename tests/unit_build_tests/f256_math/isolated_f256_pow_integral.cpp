#include <cstdint>

#include <fltx/f256_math.h>
#include "isolated_runtime.h"

void isolated_f256_pow_integral()
{
    bl::f256 input = bl::isolated::runtime_f256(1.23456789);

    int signed_exponent = bl::isolated::runtime_i32(-3);
    std::uintmax_t unsigned_exponent =
        static_cast<std::uintmax_t>(bl::isolated::runtime_long(9));

    bl::f256 signed_value   = bl::pow(input, signed_exponent);
    bl::f256 unsigned_value = bl::pow(input, unsigned_exponent);

    bl::isolated::keep_value(signed_value);
    bl::isolated::keep_value(unsigned_value);
}
