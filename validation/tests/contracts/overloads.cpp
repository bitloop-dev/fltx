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

static_assert(arithmetic_returns<bl::fdd_s, bl::fdd_s, bl::fdd_s>());
static_assert(arithmetic_returns<bl::fdd_s, bl::fdd_s, float>());
static_assert(arithmetic_returns<bl::fdd_s, double, bl::fdd_s>());
static_assert(arithmetic_returns<bl::fdd_s, bl::fdd_s, std::int64_t>());
static_assert(arithmetic_returns<bl::fdd_s, std::uint32_t, bl::fdd_s>());
static_assert(arithmetic_returns<bl::fqd_s, bl::fqd_s, bl::fqd_s>());
static_assert(arithmetic_returns<bl::fqd_s, bl::fqd_s, float>());
static_assert(arithmetic_returns<bl::fqd_s, double, bl::fqd_s>());
static_assert(arithmetic_returns<bl::fqd_s, bl::fqd_s, std::uint64_t>());
static_assert(arithmetic_returns<bl::fqd_s, std::int32_t, bl::fqd_s>());
static_assert(arithmetic_returns<bl::fqd_s, bl::fdd_s, bl::fqd_s>());
static_assert(arithmetic_returns<bl::fqd_s, bl::fqd_s, bl::fdd_s>());

static_assert(compound_arithmetic<bl::fdd_s, bl::fdd_s>);
static_assert(compound_arithmetic<bl::fdd_s, float>);
static_assert(compound_arithmetic<bl::fdd_s, double>);
static_assert(compound_arithmetic<bl::fdd_s, std::int32_t>);
static_assert(compound_arithmetic<bl::fdd_s, std::uint64_t>);
static_assert(compound_arithmetic<bl::fqd_s, bl::fqd_s>);
static_assert(compound_arithmetic<bl::fqd_s, bl::fdd_s>);
static_assert(compound_arithmetic<bl::fqd_s, float>);
static_assert(compound_arithmetic<bl::fqd_s, double>);
static_assert(compound_arithmetic<bl::fqd_s, std::int64_t>);
static_assert(compound_arithmetic<bl::fqd_s, std::uint32_t>);

static_assert(std::constructible_from<bl::fdd, float>);
static_assert(std::constructible_from<bl::fdd, double>);
static_assert(std::constructible_from<bl::fdd, std::int32_t>);
static_assert(std::constructible_from<bl::fdd, std::uint64_t>);
static_assert(std::constructible_from<bl::fdd, bl::fdd_s>);
static_assert(std::constructible_from<bl::fqd, float>);
static_assert(std::constructible_from<bl::fqd, double>);
static_assert(std::constructible_from<bl::fqd, std::int64_t>);
static_assert(std::constructible_from<bl::fqd, std::uint32_t>);
static_assert(std::constructible_from<bl::fqd, bl::fdd_s>);
static_assert(std::constructible_from<bl::fqd, bl::fqd_s>);
static_assert(std::convertible_to<bl::fdd, bl::fqd>);
static_assert(std::convertible_to<bl::fdd_s, bl::fqd_s>);
static_assert(!std::convertible_to<bl::fqd, bl::fdd>);
static_assert(!std::convertible_to<bl::fqd_s, bl::fdd_s>);
static_assert(assignment_returns_left<bl::fdd_s, bl::fqd_s>);
static_assert(assignment_returns_left<bl::fqd_s, bl::fdd_s>);
static_assert(assignment_returns_left<bl::fdd_s, std::int16_t>);
static_assert(assignment_returns_left<bl::fqd_s, std::uint16_t>);

static_assert(std::same_as<decltype(bl::sqrt(4)), bl::f64>);
static_assert(std::same_as<decltype(bl::sqrt(bl::fdd{ 4.0 })), bl::fdd>);
static_assert(std::same_as<decltype(bl::sqrt(bl::fqd{ 4.0 })), bl::fqd>);
static_assert(std::same_as<decltype(bl::hypot(3.0f, 4.0)), bl::f64>);
static_assert(std::same_as<decltype(bl::hypot(bl::fdd{ 3.0 }, 4.0)), bl::fdd>);
static_assert(std::same_as<decltype(bl::hypot(bl::fdd{ 3.0 }, bl::fqd{ 4.0 })), bl::fqd>);
static_assert(std::same_as<decltype(bl::fma(bl::fdd{ 1.0 }, 2.0, 3.0)), bl::fdd>);
static_assert(std::same_as<decltype(bl::fmin(bl::fdd{ 1.0 }, bl::fqd{ 2.0 })), bl::fqd>);
static_assert(std::same_as<decltype(bl::pow(bl::fdd{ 2.0 }, 3)), bl::fdd>);
static_assert(std::same_as<decltype(bl::pow(bl::fqd{ 2.0 }, bl::fdd{ 3.0 })), bl::fqd>);
static_assert(std::same_as<decltype(bl::sqr(bl::fdd{ 2.0 })), bl::fdd>);
static_assert(std::same_as<decltype(bl::sqr(bl::fqd{ 2.0 })), bl::fqd>);
static_assert(std::same_as<decltype(bl::fabs(-1)), bl::f64>);
static_assert(std::same_as<decltype(bl::fabs(bl::fdd{ -1.0 })), bl::fdd>);
static_assert(std::same_as<decltype(bl::fabs(bl::fqd{ -1.0 })), bl::fqd>);
static_assert(std::same_as<decltype(bl::isless(bl::fdd{ 1.0 }, bl::fqd{ 2.0 })), bool>);
static_assert(std::same_as<decltype(bl::isgreaterequal(1.0f, 1.0)), bool>);
static_assert(std::same_as<decltype(bl::nexttoward(1.0f, 2.0)), bl::f32>);
static_assert(std::same_as<decltype(bl::nexttoward(bl::fdd{ 1.0 }, bl::fqd{ 2.0 })), bl::fqd>);
static_assert(noexcept(bl::isfinite(std::declval<const bl::fdd_s&>())));
static_assert(noexcept(bl::ispositive(std::declval<const bl::fqd_s&>())));
static_assert(noexcept(bl::isless(
    std::declval<const bl::fdd_s&>(),
    std::declval<const bl::fdd_s&>())));
static_assert(noexcept(bl::nextafter(
    std::declval<const bl::fqd_s&>(),
    std::declval<const bl::fqd_s&>())));

TEST_CASE("promoted overloads behave like their standard-shaped counterparts", "[contracts][overloads]")
{
    CHECK(bl::sqrt(9) == 3.0);
    CHECK(bl::hypot(3, 4) == 5.0);
    CHECK(bl::fma(1.0f, 2.0, 3.0) == 5.0);
    CHECK(bl::pow(bl::fdd{ 2.0 }, 10) == bl::fdd{ 1024.0 });
    CHECK(bl::hypot(bl::fdd{ 3.0 }, bl::fqd{ 4.0 }) == bl::fqd{ 5.0 });
    CHECK(bl::sqr(bl::fdd{ -3.0 }) == bl::fdd{ 9.0 });
    CHECK(bl::fabs(bl::fqd{ -3.0 }) == bl::fqd{ 3.0 });
    CHECK(bl::isless(bl::fdd{ 1.0 }, bl::fqd{ 2.0 }));
    CHECK(bl::isgreaterequal(bl::fqd{ 2.0 }, 2.0));

    int quotient = 0;
    CHECK(bl::remquo(5, 2, &quotient) == 1.0);
    CHECK((quotient & 1) == 0);

    double integral = 0.0;
    CHECK(bl::modf(3, &integral) == 0.0);
    CHECK(integral == 3.0);
}
