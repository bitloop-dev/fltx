#include <fltx.h>

#include <array>
#include <string_view>

#if FLTX_BENCHMARK_SIZE_PRECISION == 128
using value_type = bl::fdd;
#elif FLTX_BENCHMARK_SIZE_PRECISION == 256
using value_type = bl::fqd;
#else
#error "FLTX_BENCHMARK_SIZE_PRECISION must be 128 or 256"
#endif

#if defined(_MSC_VER)
#define FLTX_BENCHMARK_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define FLTX_BENCHMARK_NOINLINE __attribute__((noinline))
#else
#define FLTX_BENCHMARK_NOINLINE
#endif

template <int Index>
FLTX_BENCHMARK_NOINLINE value_type arithmetic(value_type x)
{
    const value_type a{1.0 + (Index % 7) * 0.001};
    return (x * a + value_type{0.125}) / (a + value_type{0.5});
}

template <int Index>
FLTX_BENCHMARK_NOINLINE value_type math(value_type x)
{
    const value_type a{0.001 * (Index + 1)};
    return bl::sin(x + a) + bl::exp(a) + bl::sqrt(x * x + value_type{1.0});
}

template <int Index>
FLTX_BENCHMARK_NOINLINE value_type io(value_type x)
{
    std::array<char, 256> buffer{};
    const auto result = bl::to_chars(buffer.data(), buffer.data() + buffer.size(), x);
    if (result.ec != std::errc{})
        return x + value_type{Index * 0.001};
    return bl::parse<value_type>(
        std::string_view{buffer.data(), static_cast<std::size_t>(result.ptr - buffer.data())});
}

template <int Index>
FLTX_BENCHMARK_NOINLINE value_type random(value_type x)
{
    bl::mt19937_64 engine{0x1020304050607080ull + static_cast<unsigned long long>(Index)};
    bl::uniform_real_distribution<value_type> distribution{value_type{0.0}, value_type{1.0}};
    return x + distribution(engine);
}

template <int Index>
value_type repeat(value_type x)
{
    if constexpr (Index == 0)
        return x;
    else if constexpr (FLTX_BENCHMARK_SIZE_CASE == 1)
        return repeat<Index - 1>(arithmetic<Index>(x));
    else if constexpr (FLTX_BENCHMARK_SIZE_CASE == 2)
        return repeat<Index - 1>(math<Index>(x));
    else if constexpr (FLTX_BENCHMARK_SIZE_CASE == 3)
        return repeat<Index - 1>(io<Index>(x));
    else
        return repeat<Index - 1>(random<Index>(x));
}

int main(int argc, char**)
{
    value_type value{1.25 + argc * 0.001};
#if FLTX_BENCHMARK_SIZE_CASE != 0
    value = repeat<FLTX_BENCHMARK_SIZE_REPEATS>(value);
#endif
    volatile double sink = static_cast<double>(value);
    return sink == 0.0;
}
