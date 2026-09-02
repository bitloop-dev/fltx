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
    fdd_s x{};
    fdd_s y{};
    fqd_s weight{};
};

static_assert(sizeof(fdd_s) == 16);
static_assert(sizeof(fqd_s) == 32);
static_assert(std::is_standard_layout_v<fdd_s>);
static_assert(std::is_standard_layout_v<fqd_s>);
static_assert(std::is_trivially_copyable_v<fdd_s>);
static_assert(std::is_trivially_copyable_v<fqd_s>);
static_assert(std::is_aggregate_v<fdd_s>);
static_assert(std::is_aggregate_v<fqd_s>);
static_assert(std::is_standard_layout_v<weighted_point>);
static_assert(std::is_trivially_copyable_v<weighted_point>);

struct center_result
{
    fqd x{};
    fqd y{};
};

template<std::size_t N>
constexpr center_result weighted_center(const std::array<weighted_point, N>& points)
{
    fqd total_x{};
    fqd total_y{};
    fqd total_weight{};

    for (const weighted_point& point : points)
    {
        const fqd x = fqd{ point.x };
        const fqd y = fqd{ point.y };
        const fqd weight = fqd{ point.weight };

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
        << "fdd_s size: " << sizeof(fdd_s) << " bytes\n"
        << "fqd_s size: " << sizeof(fqd_s) << " bytes\n\n";

    std::cout
        << std::setprecision(std::numeric_limits<fqd>::digits10)
        << "weighted center x: " << center.x << "\n"
        << "weighted center y: " << center.y << "\n";
}
