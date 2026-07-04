#include <fltx/f256.h>
#include "isolated_runtime.h"

void isolated_f256_recip()
{
    bl::f256 input = bl::isolated::runtime_f256(2.5);
    bl::f256 value = bl::recip(input);

    bl::isolated::keep_value(value);
}
