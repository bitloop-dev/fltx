#include <cstdint>

#include <fltx/f256_math.h>
#include "isolated_runtime.h"

void isolated_f256_ipow()
{
    bl::f256 input = bl::isolated::runtime_f256(1.23456789);

    int signed_exponent = bl::isolated::runtime_i32(-3);
    std::intmax_t wide_signed_exponent =
        static_cast<std::intmax_t>(bl::isolated::runtime_long(7));
    std::uintmax_t wide_unsigned_exponent =
        static_cast<std::uintmax_t>(bl::isolated::runtime_long(9));

    bl::f256 signed_value        = bl::ipow(input, signed_exponent);
    bl::f256 wide_signed_value   = bl::ipow(input, wide_signed_exponent);
    bl::f256 wide_unsigned_value = bl::ipow(input, wide_unsigned_exponent);

    bl::isolated::keep_value(signed_value);
    bl::isolated::keep_value(wide_signed_value);
    bl::isolated::keep_value(wide_unsigned_value);
}
