#include <fltx/f128.h>
#include "isolated_runtime.h"

void isolated_f128_recip()
{
    bl::f128 input = bl::isolated::runtime_f128(2.5);
    bl::f128 value = bl::recip(input);

    bl::isolated::keep_value(value);
}
