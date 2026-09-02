#include <array>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>

#include <fltx.h>

using namespace bl;

template<std::size_t Radius, class T>
consteval std::array<T, Radius * 2 + 1> gaussian_kernel(T sigma)
{
    std::array<T, Radius * 2 + 1> weights{};
    T sum{};

    for (std::size_t i = 0; i < weights.size(); ++i)
    {
        const int offset = static_cast<int>(i) - static_cast<int>(Radius);
        const T x = T{ offset };
        const T x2 = x * x;
        const T denom = T{ 2 } * sigma * sigma;
        const T exponent = -x2 / denom;
        weights[i] = bl::exp(exponent);
        sum += weights[i];
    }

    for (T& weight : weights)
        weight /= sum;

    return weights;
}

template<std::size_t N, class T>
constexpr T sum_values(const std::array<T, N>& values)
{
    T sum{};
    for (const T& value : values)
        sum += value;
    return sum;
}

int main()
{
    using T = fqd;

    constexpr auto kernel = gaussian_kernel<4>( bl::parse<T>("1.25") );
    constexpr T kernel_sum = sum_values(kernel);

    std::cout << std::setprecision(std::numeric_limits<T>::digits10);

    for (std::size_t i = 0; i < kernel.size(); ++i)
    {
        const int offset = static_cast<int>(i) - 4;
        std::cout << "w[" << offset << "] = " << kernel[i] << "\n";
    }

    std::cout << "\nsum = " << kernel_sum << "\n";
}
