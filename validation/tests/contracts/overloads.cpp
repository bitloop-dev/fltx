#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>

#include <fltx.h>

namespace
{
    template<class Result, class Left, class Right>
    consteval bool arithmetic_returns()
    {
        return
            std::same_as<decltype(std::declval<Left>() + std::declval<Right>()), Result> &&
            std::same_as<decltype(std::declval<Left>() - std::declval<Right>()), Result> &&
            std::same_as<decltype(std::declval<Left>() * std::declval<Right>()), Result> &&
            std::same_as<decltype(std::declval<Left>() / std::declval<Right>()), Result>;
    }

    template<class Left, class Right>
    concept compound_arithmetic = requires (Left left, Right right)
    {
        { left += right } -> std::same_as<Left&>;
        { left -= right } -> std::same_as<Left&>;
        { left *= right } -> std::same_as<Left&>;
        { left /= right } -> std::same_as<Left&>;
    };

    template<class Left, class Right>
    concept assignment_returns_left = requires (Left left, Right right)
    {
        { left = right } -> std::same_as<Left&>;
    };
}

static_assert(arithmetic_returns<bl::f128_s, bl::f128_s, bl::f128_s>());
static_assert(arithmetic_returns<bl::f128_s, bl::f128_s, float>());
static_assert(arithmetic_returns<bl::f128_s, double, bl::f128_s>());
static_assert(arithmetic_returns<bl::f128_s, bl::f128_s, std::int64_t>());
static_assert(arithmetic_returns<bl::f128_s, std::uint32_t, bl::f128_s>());
static_assert(arithmetic_returns<bl::f256_s, bl::f256_s, bl::f256_s>());
static_assert(arithmetic_returns<bl::f256_s, bl::f256_s, float>());
static_assert(arithmetic_returns<bl::f256_s, double, bl::f256_s>());
static_assert(arithmetic_returns<bl::f256_s, bl::f256_s, std::uint64_t>());
static_assert(arithmetic_returns<bl::f256_s, std::int32_t, bl::f256_s>());
static_assert(arithmetic_returns<bl::f256_s, bl::f128_s, bl::f256_s>());
static_assert(arithmetic_returns<bl::f256_s, bl::f256_s, bl::f128_s>());

static_assert(compound_arithmetic<bl::f128_s, bl::f128_s>);
static_assert(compound_arithmetic<bl::f128_s, float>);
static_assert(compound_arithmetic<bl::f128_s, double>);
static_assert(compound_arithmetic<bl::f128_s, std::int32_t>);
static_assert(compound_arithmetic<bl::f128_s, std::uint64_t>);
static_assert(compound_arithmetic<bl::f256_s, bl::f256_s>);
static_assert(compound_arithmetic<bl::f256_s, bl::f128_s>);
static_assert(compound_arithmetic<bl::f256_s, float>);
static_assert(compound_arithmetic<bl::f256_s, double>);
static_assert(compound_arithmetic<bl::f256_s, std::int64_t>);
static_assert(compound_arithmetic<bl::f256_s, std::uint32_t>);

static_assert(std::constructible_from<bl::f128, float>);
static_assert(std::constructible_from<bl::f128, double>);
static_assert(std::constructible_from<bl::f128, std::int32_t>);
static_assert(std::constructible_from<bl::f128, std::uint64_t>);
static_assert(std::constructible_from<bl::f128, bl::f128_s>);
static_assert(std::constructible_from<bl::f256, float>);
static_assert(std::constructible_from<bl::f256, double>);
static_assert(std::constructible_from<bl::f256, std::int64_t>);
static_assert(std::constructible_from<bl::f256, std::uint32_t>);
static_assert(std::constructible_from<bl::f256, bl::f128_s>);
static_assert(std::constructible_from<bl::f256, bl::f256_s>);
static_assert(std::convertible_to<bl::f128, bl::f256>);
static_assert(std::convertible_to<bl::f128_s, bl::f256_s>);
static_assert(!std::convertible_to<bl::f256, bl::f128>);
static_assert(!std::convertible_to<bl::f256_s, bl::f128_s>);
static_assert(assignment_returns_left<bl::f128_s, bl::f256_s>);
static_assert(assignment_returns_left<bl::f256_s, bl::f128_s>);
static_assert(assignment_returns_left<bl::f128_s, std::int16_t>);
static_assert(assignment_returns_left<bl::f256_s, std::uint16_t>);

static_assert(std::same_as<decltype(bl::sqrt(4)), bl::f64>);
static_assert(std::same_as<decltype(bl::sqrt(bl::f128{ 4.0 })), bl::f128>);
static_assert(std::same_as<decltype(bl::sqrt(bl::f256{ 4.0 })), bl::f256>);
static_assert(std::same_as<decltype(bl::hypot(3.0f, 4.0)), bl::f64>);
static_assert(std::same_as<decltype(bl::hypot(bl::f128{ 3.0 }, 4.0)), bl::f128>);
static_assert(std::same_as<decltype(bl::hypot(bl::f128{ 3.0 }, bl::f256{ 4.0 })), bl::f256>);
static_assert(std::same_as<decltype(bl::fma(bl::f128{ 1.0 }, 2.0, 3.0)), bl::f128>);
static_assert(std::same_as<decltype(bl::fmin(bl::f128{ 1.0 }, bl::f256{ 2.0 })), bl::f256>);
static_assert(std::same_as<decltype(bl::pow(bl::f128{ 2.0 }, 3)), bl::f128>);
static_assert(std::same_as<decltype(bl::pow(bl::f256{ 2.0 }, bl::f128{ 3.0 })), bl::f256>);
static_assert(std::same_as<decltype(bl::sqr(bl::f128{ 2.0 })), bl::f128>);
static_assert(std::same_as<decltype(bl::sqr(bl::f256{ 2.0 })), bl::f256>);
static_assert(std::same_as<decltype(bl::fabs(-1)), bl::f64>);
static_assert(std::same_as<decltype(bl::fabs(bl::f128{ -1.0 })), bl::f128>);
static_assert(std::same_as<decltype(bl::fabs(bl::f256{ -1.0 })), bl::f256>);
static_assert(std::same_as<decltype(bl::isless(bl::f128{ 1.0 }, bl::f256{ 2.0 })), bool>);
static_assert(std::same_as<decltype(bl::isgreaterequal(1.0f, 1.0)), bool>);
static_assert(std::same_as<decltype(bl::nexttoward(1.0f, 2.0)), bl::f32>);
static_assert(std::same_as<decltype(bl::nexttoward(bl::f128{ 1.0 }, bl::f256{ 2.0 })), bl::f256>);
static_assert(noexcept(bl::isfinite(std::declval<const bl::f128_s&>())));
static_assert(noexcept(bl::ispositive(std::declval<const bl::f256_s&>())));
static_assert(noexcept(bl::isless(
    std::declval<const bl::f128_s&>(),
    std::declval<const bl::f128_s&>())));
static_assert(noexcept(bl::nextafter(
    std::declval<const bl::f256_s&>(),
    std::declval<const bl::f256_s&>())));

TEST_CASE("promoted overloads behave like their standard-shaped counterparts", "[contracts][overloads]")
{
    CHECK(bl::sqrt(9) == 3.0);
    CHECK(bl::hypot(3, 4) == 5.0);
    CHECK(bl::fma(1.0f, 2.0, 3.0) == 5.0);
    CHECK(bl::pow(bl::f128{ 2.0 }, 10) == bl::f128{ 1024.0 });
    CHECK(bl::hypot(bl::f128{ 3.0 }, bl::f256{ 4.0 }) == bl::f256{ 5.0 });
    CHECK(bl::sqr(bl::f128{ -3.0 }) == bl::f128{ 9.0 });
    CHECK(bl::fabs(bl::f256{ -3.0 }) == bl::f256{ 3.0 });
    CHECK(bl::isless(bl::f128{ 1.0 }, bl::f256{ 2.0 }));
    CHECK(bl::isgreaterequal(bl::f256{ 2.0 }, 2.0));

    int quotient = 0;
    CHECK(bl::remquo(5, 2, &quotient) == 1.0);
    CHECK((quotient & 1) == 0);

    double integral = 0.0;
    CHECK(bl::modf(3, &integral) == 0.0);
    CHECK(integral == 3.0);
}
