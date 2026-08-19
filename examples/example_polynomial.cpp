#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>

#include <fltx.h>

using namespace bl;

template<class T>
constexpr T expanded_shifted_power(T x)
{
    T y = T{ 1 };
    y = bl::fma(y, x, T{ -8 });
    y = bl::fma(y, x, T{ 28 });
    y = bl::fma(y, x, T{ -56 });
    y = bl::fma(y, x, T{ 70 });
    y = bl::fma(y, x, T{ -56 });
    y = bl::fma(y, x, T{ 28 });
    y = bl::fma(y, x, T{ -8 });
    y = bl::fma(y, x, T{ 1 });
    return y;
}

template<class T>
constexpr T factored_shifted_power(T x)
{
    const T delta = x - T{ 1 };
    return bl::ipow(delta, 8);
}

template<class T>
void print_case(std::string_view name)
{
    const T x = bl::parse<T>("1.0001");
    const T expanded = expanded_shifted_power(x);
    const T factored = factored_shifted_power(x);
    const T diff = expanded - factored;
    const T error = bl::abs(diff) / bl::abs(factored);

    std::cout
        << name << ":\n"
        << std::setprecision(std::numeric_limits<T>::max_digits10)
        << "  expanded Horner = " << expanded << "\n"
        << "  factored form   = " << factored << "\n"
        << "  relative error  = " << error << "\n\n";
}

int main()
{
    print_case<f64>("f64");
    print_case<f128>("f128");
    print_case<f256>("f256");
}
