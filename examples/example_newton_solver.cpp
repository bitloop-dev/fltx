#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>

#include <fltx.h>

using namespace bl;

template<class T>
struct solve_result
{
    T root{};
    T residual{};
    int iterations{};
};

template<class T>
constexpr T equation(T x)
{
    return bl::cos(x) - x;
}

template<class T>
constexpr T derivative(T x)
{
    return -bl::sin(x) - T{ 1 };
}

template<class T>
constexpr solve_result<T> solve_newton(T initial)
{
    T x = initial;
    const T tolerance = std::numeric_limits<T>::epsilon() * T{ 32 };

    for (int i = 0; i < 20; ++i)
    {
        const T step = equation(x) / derivative(x);
        x -= step;

        T scale = bl::abs(x);
        if (scale < T{ 1 })
            scale = T{ 1 };

        if (bl::abs(step) <= tolerance * scale)
            return { x, equation(x), i + 1 };
    }

    return { x, equation(x), 20 };
}

template<class T>
void print_result(std::string_view name)
{
    const auto result = solve_newton<T>(1);

    std::cout
        << name << ":\n"
        << "  iterations = " << result.iterations << "\n"
        << std::setprecision(std::numeric_limits<T>::max_digits10)
        << "  root       = " << result.root << "\n"
        << "  residual   = " << result.residual << "\n\n";
}

int main()
{
    print_result<f64>("f64");
    print_result<fdd>("fdd");
    print_result<fqd>("fqd");
}
