#include <qd/dd.h>
#include <qd/qd_real.h>

namespace
{
    constexpr dd_real dd_sum = dd_real{1.25} + dd_real{0.75};
    constexpr qd_real qd_product = qd_real{1.5} * qd_real{2.0};

    static_assert(to_double(dd_sum) == 2.0);
    static_assert(to_double(qd_product) == 3.0);
}

int main()
{
    const dd_real dd_root = sqrt(dd_real{4.0});
    const qd_real qd_root = sqrt(qd_real{9.0});
    const qd_real qd_sine = sin(qd_real{0.0});

    return to_double(dd_root) == 2.0 &&
           to_double(qd_root) == 3.0 &&
           to_double(qd_sine) == 0.0
        ? 0
        : 1;
}
