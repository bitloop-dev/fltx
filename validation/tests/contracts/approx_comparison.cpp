#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <limits>

#include <fltx.h>

namespace
{
    template<class Left, class Right>
    concept has_default_approx_eq = requires (const Left& left, const Right& right)
    {
        { bl::approx_eq(left, right) } -> std::same_as<bool>;
    };

    template<class Left, class Right, class Tolerance>
    concept has_two_tolerances = requires (
        const Left& left,
        const Right& right,
        const Tolerance& tolerance)
    {
        bl::approx_eq(left, right, tolerance, tolerance);
    };

    static_assert(has_default_approx_eq<bl::f32, bl::f32>);
    static_assert(has_default_approx_eq<bl::f64, bl::f64>);
    static_assert(has_default_approx_eq<bl::fdd, bl::fdd>);
    static_assert(has_default_approx_eq<bl::fdd_s, bl::fdd>);
    static_assert(has_default_approx_eq<bl::fdd, bl::fdd_s>);
    static_assert(has_default_approx_eq<bl::fdd_s, bl::fdd_s>);
    static_assert(has_default_approx_eq<bl::fqd, bl::fqd>);
    static_assert(has_default_approx_eq<bl::fqd_s, bl::fqd>);
    static_assert(has_default_approx_eq<bl::fqd, bl::fqd_s>);
    static_assert(has_default_approx_eq<bl::fqd_s, bl::fqd_s>);
    static_assert(!has_default_approx_eq<bl::f32, bl::f64>);
    static_assert(!has_default_approx_eq<bl::f64, bl::f32>);
    static_assert(!has_default_approx_eq<bl::fdd, double>);
    static_assert(!has_default_approx_eq<bl::fqd, double>);
    static_assert(!has_default_approx_eq<bl::fdd, bl::fqd>);
    static_assert(!has_default_approx_eq<bl::fqd, bl::fdd>);
    static_assert(!has_two_tolerances<bl::f32, bl::f32, bl::f32>);
    static_assert(!has_two_tolerances<bl::f64, bl::f64, bl::f64>);
    static_assert(!has_two_tolerances<bl::fdd, bl::fdd, bl::fdd>);
    static_assert(!has_two_tolerances<bl::fqd, bl::fqd, bl::fqd>);

    static_assert(bl::f32_parity_tolerance == 0x1p-20f);
    static_assert(bl::f64_parity_tolerance == 0x1p-48);
    static_assert(bl::fdd_parity_tolerance == bl::fdd_s{ 0x1p-80, 0.0 });
    static_assert(bl::fqd_parity_tolerance ==
        bl::fqd_s{ 0x1p-188, 0.0, 0.0, 0.0 });

    constexpr bl::f32 f32_one = 1.0f;
    constexpr bl::f32 f32_boundary = f32_one + 0x1p-20f;
    constexpr bl::f32 f32_outside = f32_one + 0x1p-19f;
    static_assert(bl::approx_eq(f32_one, f32_boundary));
    static_assert(!bl::approx_eq(f32_one, f32_outside));

    constexpr bl::f64 f64_one = 1.0;
    constexpr bl::f64 f64_boundary = f64_one + 0x1p-48;
    constexpr bl::f64 f64_outside = f64_one + 0x1p-47;
    static_assert(bl::approx_eq(f64_one, f64_boundary));
    static_assert(!bl::approx_eq(f64_one, f64_outside));

    constexpr bl::fdd dd_one{ 1.0 };
    constexpr bl::fdd dd_boundary = dd_one + bl::fdd{ 0x1p-80 };
    constexpr bl::fdd dd_outside = dd_one + bl::fdd{ 0x1p-79 };
    static_assert(bl::approx_eq(dd_one, dd_boundary));
    static_assert(!bl::approx_eq(dd_one, dd_outside));

    constexpr bl::fqd qd_one{ 1.0 };
    constexpr bl::fqd qd_boundary = qd_one + bl::fqd{ 0x1p-188 };
    constexpr bl::fqd qd_outside = qd_one + bl::fqd{ 0x1p-187 };
    static_assert(bl::approx_eq(qd_one, qd_boundary));
    static_assert(!bl::approx_eq(qd_one, qd_outside));

    constexpr bl::f32 f32_max = std::numeric_limits<bl::f32>::max();
    constexpr bl::f64 f64_max = std::numeric_limits<bl::f64>::max();
    constexpr bl::fdd dd_max = std::numeric_limits<bl::fdd>::max();
    constexpr bl::fqd qd_max = std::numeric_limits<bl::fqd>::max();
    static_assert(!bl::approx_eq(f32_max, -f32_max, bl::f32{ 1.5 }));
    static_assert(bl::approx_eq(f32_max, -f32_max, bl::f32{ 2.0 }));
    static_assert(!bl::approx_eq(f64_max, -f64_max, bl::f64{ 1.5 }));
    static_assert(bl::approx_eq(f64_max, -f64_max, bl::f64{ 2.0 }));
    static_assert(!bl::approx_eq(dd_max, -dd_max, bl::fdd{ 1.5 }));
    static_assert(bl::approx_eq(dd_max, -dd_max, bl::fdd{ 2.0 }));
    static_assert(!bl::approx_eq(qd_max, -qd_max, bl::fqd{ 1.5 }));
    static_assert(bl::approx_eq(qd_max, -qd_max, bl::fqd{ 2.0 }));
}

TEST_CASE("approx_eq applies relative tolerances", "[contracts][comparison]")
{
    CHECK(bl::approx_eq(f32_one, f32_boundary));
    CHECK(bl::approx_eq(f32_boundary, f32_one));
    CHECK_FALSE(bl::approx_eq(f32_one, f32_outside));

    CHECK(bl::approx_eq(f64_one, f64_boundary));
    CHECK(bl::approx_eq(f64_boundary, f64_one));
    CHECK_FALSE(bl::approx_eq(f64_one, f64_outside));

    CHECK(bl::approx_eq(dd_one, dd_boundary));
    CHECK(bl::approx_eq(dd_boundary, dd_one));
    CHECK_FALSE(bl::approx_eq(dd_one, dd_outside));

    const bl::fdd dd_large = bl::ldexp(bl::fdd{ 1.0 }, 400);
    CHECK(bl::approx_eq(
        dd_large,
        dd_large + bl::ldexp(bl::fdd{ 1.0 }, 320)));
    CHECK_FALSE(bl::approx_eq(
        dd_large,
        dd_large + bl::ldexp(bl::fdd{ 1.0 }, 321)));

    CHECK(bl::approx_eq(qd_one, qd_boundary));
    CHECK(bl::approx_eq(qd_boundary, qd_one));
    CHECK_FALSE(bl::approx_eq(qd_one, qd_outside));

    const bl::fqd qd_large = bl::ldexp(bl::fqd{ 1.0 }, 400);
    const bl::fqd qd_large_boundary =
        qd_large + bl::ldexp(bl::fqd{ 1.0 }, 212);
    const bl::fqd qd_large_outside =
        qd_large + bl::ldexp(bl::fqd{ 1.0 }, 213);
    CHECK(bl::approx_eq(
        qd_large,
        qd_large_boundary));
    CHECK_FALSE(bl::approx_eq(
        qd_large,
        qd_large_outside));

    CHECK_FALSE(bl::approx_eq(bl::fdd{ 0.0 }, bl::fdd{ 0x1p-900 }));
    CHECK_FALSE(bl::approx_eq(bl::fqd{ 0.0 }, bl::fqd{ 0x1p-900 }));
}

TEST_CASE("approx_eq compares native runtime and constexpr math results", "[contracts][comparison]")
{
    constexpr bl::f32 f32_expected = bl::sin(bl::f32{ 1.0 });
    constexpr bl::f64 f64_expected = bl::sin(bl::f64{ 1.0 });
    volatile bl::f32 f32_input = 1.0f;
    volatile bl::f64 f64_input = 1.0;

    CHECK(bl::approx_eq(bl::sin(f32_input), f32_expected));
    CHECK(bl::approx_eq(bl::sin(f64_input), f64_expected));
}

TEST_CASE("approx_eq preserves special-value semantics", "[contracts][comparison]")
{
    const bl::f32 f32_infinity = std::numeric_limits<bl::f32>::infinity();
    const bl::f32 f32_nan = std::numeric_limits<bl::f32>::quiet_NaN();
    CHECK(bl::approx_eq(bl::f32{ 0.0 }, bl::f32{ -0.0 }));
    CHECK(bl::approx_eq(f32_infinity, f32_infinity));
    CHECK_FALSE(bl::approx_eq(f32_infinity, -f32_infinity));
    CHECK_FALSE(bl::approx_eq(f32_nan, f32_nan));
    CHECK_FALSE(bl::approx_eq(f32_one, f32_one, bl::f32{ -1.0 }));
    CHECK_FALSE(bl::approx_eq(f32_one, f32_one, f32_nan));
    CHECK(bl::approx_eq(f32_one, f32_outside, f32_infinity));

    const bl::f64 f64_infinity = std::numeric_limits<bl::f64>::infinity();
    const bl::f64 f64_nan = std::numeric_limits<bl::f64>::quiet_NaN();
    CHECK(bl::approx_eq(bl::f64{ 0.0 }, bl::f64{ -0.0 }));
    CHECK(bl::approx_eq(f64_infinity, f64_infinity));
    CHECK_FALSE(bl::approx_eq(f64_infinity, -f64_infinity));
    CHECK_FALSE(bl::approx_eq(f64_nan, f64_nan));
    CHECK_FALSE(bl::approx_eq(f64_one, f64_one, bl::f64{ -1.0 }));
    CHECK_FALSE(bl::approx_eq(f64_one, f64_one, f64_nan));
    CHECK(bl::approx_eq(f64_one, f64_outside, f64_infinity));

    const bl::fdd dd_positive_zero{ 0.0 };
    const bl::fdd dd_negative_zero{ -0.0 };
    const bl::fdd dd_infinity = std::numeric_limits<bl::fdd>::infinity();
    const bl::fdd dd_nan = std::numeric_limits<bl::fdd>::quiet_NaN();

    CHECK(bl::approx_eq(dd_positive_zero, dd_negative_zero));
    CHECK(bl::approx_eq(dd_infinity, dd_infinity));
    CHECK_FALSE(bl::approx_eq(dd_infinity, -dd_infinity));
    CHECK_FALSE(bl::approx_eq(dd_nan, dd_nan));
    CHECK_FALSE(bl::approx_eq(dd_one, dd_one, bl::fdd{ -1.0 }));
    CHECK_FALSE(bl::approx_eq(dd_one, dd_one, dd_nan));
    CHECK(bl::approx_eq(dd_one, dd_outside, dd_infinity));

    const bl::fqd qd_positive_zero{ 0.0 };
    const bl::fqd qd_negative_zero{ -0.0 };
    const bl::fqd qd_infinity = std::numeric_limits<bl::fqd>::infinity();
    const bl::fqd qd_nan = std::numeric_limits<bl::fqd>::quiet_NaN();

    CHECK(bl::approx_eq(qd_positive_zero, qd_negative_zero));
    CHECK(bl::approx_eq(qd_infinity, qd_infinity));
    CHECK_FALSE(bl::approx_eq(qd_infinity, -qd_infinity));
    CHECK_FALSE(bl::approx_eq(qd_nan, qd_nan));
    CHECK_FALSE(bl::approx_eq(qd_one, qd_one, bl::fqd{ -1.0 }));
    CHECK_FALSE(bl::approx_eq(qd_one, qd_one, qd_nan));
    CHECK(bl::approx_eq(qd_one, qd_outside, qd_infinity));

    CHECK_FALSE(bl::approx_eq(f32_max, -f32_max, bl::f32{ 1.5 }));
    CHECK(bl::approx_eq(f32_max, -f32_max, bl::f32{ 2.0 }));
    CHECK_FALSE(bl::approx_eq(f64_max, -f64_max, bl::f64{ 1.5 }));
    CHECK(bl::approx_eq(f64_max, -f64_max, bl::f64{ 2.0 }));
    CHECK_FALSE(bl::approx_eq(dd_max, -dd_max, bl::fdd{ 1.5 }));
    CHECK(bl::approx_eq(dd_max, -dd_max, bl::fdd{ 2.0 }));
    CHECK_FALSE(bl::approx_eq(qd_max, -qd_max, bl::fqd{ 1.5 }));
    CHECK(bl::approx_eq(qd_max, -qd_max, bl::fqd{ 2.0 }));
}
