#include <iostream>
#include <iomanip>

#include <fltx.h>
using namespace bl::types;
using namespace bl::literals;

int main()
{
    constexpr fqd a = 1_qd / 3_qd;
    constexpr fqd b = 2_qd / 3_qd;
    constexpr fqd c = a + b;
    constexpr fqd d = bl::atan2(a, b);

    std::cout
        << std::fixed
        << std::setprecision(std::numeric_limits<fqd>::digits10)
        << "a           = " << a << "\n"
        << "b           = " << b << "\n"
        << "a + b       = " << c << "\n"
        << "atan2(a, b) = " << d << "\n";
}
