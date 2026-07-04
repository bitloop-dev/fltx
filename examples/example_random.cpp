#include <iostream>
#include <iomanip>
#include <limits>
#include <array>

#include <fltx.h>

using namespace bl;
using namespace bl::literals;

template<std::size_t N, typename T>
consteval T avg(const std::array<T, N>& values)
{
    T total = 0;
    for (const auto& v : values)
        total += v;
    return total / N;
}

int main()
{
    // f256: random uniform arrays (min=0, max=100)
    constexpr auto a = bl::uniform_real_array<100>(0_qd, 100_qd, bl::mt19937_64{ 0x123ull }); // explicit engine / custom seed
    constexpr auto b = bl::uniform_real_array<100>(0_qd, 100_qd, 0x123ull);                   // default engine  / custom seed
    constexpr auto c = bl::uniform_real_array<100>(0_qd, 100_qd);                             // default engine  / default seed

    // f128: random normal array (mean=50, stddev=50)
    constexpr auto d = bl::normal_array<100>(50_dd, 50_dd, bl::mt19937_64{ 0x123ull }); // explicit engine / custom seed
    constexpr auto e = bl::normal_array<100>(50_dd, 50_dd, 0x123ull);                   // default engine  / custom seed
    constexpr auto f = bl::normal_array<100>(50_dd, 50_dd);                             // default engine  / default seed

    constexpr f256 a_avg = avg(a);
    constexpr f256 b_avg = avg(b);
    constexpr f256 c_avg = avg(c);

    constexpr f128 d_avg = avg(d);
    constexpr f128 e_avg = avg(e);
    constexpr f128 f_avg = avg(f);

    std::cout
        << std::fixed
        << std::setprecision(std::numeric_limits<f256>::digits10)
        << "a_avg: " << a_avg << "\n"
        << "b_avg: " << b_avg << "\n"
        << "c_avg: " << c_avg << "\n\n"
        << "d_avg: " << d_avg << "\n"
        << "e_avg: " << e_avg << "\n"
        << "f_avg: " << f_avg << "\n";
}
