#include <iostream>
#include <iomanip>

#include <fltx.h>

using namespace bl;
using namespace bl::literals;

consteval float test()
{
    float ip{};
    static_cast<void>(bl::modf(12.5f, &ip));
    return ip;
}

int main()
{
    constexpr f256 a = 1_qd / 3_qd;
    constexpr f256 b = 2_qd / 3_qd;
    constexpr f256 c = a + b;
    constexpr f256 d = bl::atan2(a, b);

    std::cout
        << std::fixed
        << std::setprecision(std::numeric_limits<f256>::digits10)
        << "a           = " << a << "\n"
        << "b           = " << b << "\n"
        << "a + b       = " << c << "\n"
        << "atan2(a, b) = " << d << "\n";
}
