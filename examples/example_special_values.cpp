#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>

#include <fltx.h>

using namespace bl;
using namespace bl::literals;

const char* class_name(int cls)
{
    switch (cls)
    {
    case FP_INFINITE:  return "infinite";
    case FP_NAN:       return "nan";
    case FP_NORMAL:    return "normal";
    case FP_SUBNORMAL: return "subnormal";
    case FP_ZERO:      return "zero";
    default:           return "unknown";
    }
}

template<class T>
void describe(std::string_view name, T value)
{
    std::cout
        << std::boolalpha
        << name << ":\n"
        << "  class    = " << class_name(bl::fpclassify(value)) << "\n"
        << "  finite   = " << bl::isfinite(value) << "\n"
        << "  infinite = " << bl::isinf(value) << "\n"
        << "  nan      = " << bl::isnan(value) << "\n"
        << "  signbit  = " << bl::signbit(value) << "\n\n";
}

int main()
{
    using T = f256;

    describe("positive zero", T{ 0.0 });
    describe("negative zero", T{ -0.0 });
    describe("denorm_min", std::numeric_limits<T>::denorm_min());
    describe("infinity", std::numeric_limits<T>::infinity());
    describe("quiet_NaN", std::numeric_limits<T>::quiet_NaN());

    const T value = bl::parse<T>("-12.75");

    int exponent{};
    const T significand = bl::frexp(value, &exponent);
    const T rebuilt = bl::ldexp(significand, exponent);

    T integer_part{};
    const T fraction = bl::modf(value, &integer_part);

    std::cout
        << std::setprecision(std::numeric_limits<T>::max_digits10)
        << "frexp(-12.75): significand = " << significand << ", exponent = " << exponent << "\n"
        << "ldexp(significand, exponent): " << rebuilt << "\n"
        << "modf(-12.75): integer = " << integer_part << ", fraction = " << fraction << "\n"
        << "nextafter(0, 1): " << bl::nextafter(T{ 0 }, T{ 1 }) << "\n\n";


    constexpr int counter_width = 3;
    constexpr int zeros_before_counter =
        std::numeric_limits<T>::digits10 - counter_width; // 60 for f256

    constexpr int places = zeros_before_counter + counter_width;
    const T step = bl::pow(10_qd, -places);

    std::cout << std::fixed << std::setprecision(places);

    for (int i = 1; i <= 4; ++i)
    {
        const T v = T{ i } * step;
        std::cout << v << "\n";
    }
}
