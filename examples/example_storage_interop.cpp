#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <type_traits>

#include <fltx.h>

using namespace bl;
using namespace bl::literals;

struct weighted_point
{
    std::uint32_t id{};
    f128_s x{};
    f128_s y{};
    f256_s weight{};
};

static_assert(sizeof(f128_s) == 16);
static_assert(sizeof(f256_s) == 32);
static_assert(std::is_standard_layout_v<f128_s>);
static_assert(std::is_standard_layout_v<f256_s>);
static_assert(std::is_trivially_copyable_v<f128_s>);
static_assert(std::is_trivially_copyable_v<f256_s>);
static_assert(std::is_standard_layout_v<weighted_point>);
static_assert(std::is_trivially_copyable_v<weighted_point>);

struct center_result
{
    f256 x{};
    f256 y{};
};

template<std::size_t N>
constexpr center_result weighted_center(const std::array<weighted_point, N>& points)
{
    f256 total_x{};
    f256 total_y{};
    f256 total_weight{};

    for (const weighted_point& point : points)
    {
        const f256 x = f256{ point.x };
        const f256 y = f256{ point.y };
        const f256 weight = f256{ point.weight };

        total_x += x * weight;
        total_y += y * weight;
        total_weight += weight;
    }

    return {
        total_x / total_weight,
        total_y / total_weight
    };
}

int main()
{
    constexpr std::array<weighted_point, 3> points{ {
        { 101, 0.125_dd, 1.5_dd, 0.20_qd },
        { 102, 1.250_dd, 2.0_dd, 0.35_qd },
        { 103, 2.750_dd, 3.5_dd, 0.45_qd }
    } };

    constexpr center_result center = weighted_center(points);

    std::cout
        << "weighted_point size: " << sizeof(weighted_point) << " bytes\n"
        << "f128_s size: " << sizeof(f128_s) << " bytes\n"
        << "f256_s size: " << sizeof(f256_s) << " bytes\n\n";

    std::cout
        << std::setprecision(std::numeric_limits<f256>::digits10)
        << "weighted center x: " << center.x << "\n"
        << "weighted center y: " << center.y << "\n";
}
