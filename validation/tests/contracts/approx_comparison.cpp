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

    static_assert(has_default_approx_eq<bl::f128, bl::f128>);
    static_assert(has_default_approx_eq<bl::f128_s, bl::f128>);
    static_assert(has_default_approx_eq<bl::f128, bl::f128_s>);
    static_assert(has_default_approx_eq<bl::f128_s, bl::f128_s>);
    static_assert(has_default_approx_eq<bl::f256, bl::f256>);
    static_assert(has_default_approx_eq<bl::f256_s, bl::f256>);
    static_assert(has_default_approx_eq<bl::f256, bl::f256_s>);
    static_assert(has_default_approx_eq<bl::f256_s, bl::f256_s>);
    static_assert(!has_default_approx_eq<double, double>);
    static_assert(!has_default_approx_eq<bl::f128, double>);
    static_assert(!has_default_approx_eq<bl::f256, double>);
    static_assert(!has_default_approx_eq<bl::f128, bl::f256>);
    static_assert(!has_default_approx_eq<bl::f256, bl::f128>);
    static_assert(!has_two_tolerances<bl::f128, bl::f128, bl::f128>);
    static_assert(!has_two_tolerances<bl::f256, bl::f256, bl::f256>);

    static_assert(bl::f128_parity_tolerance == bl::f128_s{ 0x1p-80, 0.0 });
    static_assert(bl::f256_parity_tolerance ==
        bl::f256_s{ 0x1p-188, 0.0, 0.0, 0.0 });

    constexpr bl::f128 dd_one{ 1.0 };
    constexpr bl::f128 dd_boundary = dd_one + bl::f128{ 0x1p-80 };
    constexpr bl::f128 dd_outside = dd_one + bl::f128{ 0x1p-79 };
    static_assert(bl::approx_eq(dd_one, dd_boundary));
    static_assert(!bl::approx_eq(dd_one, dd_outside));

    constexpr bl::f256 qd_one{ 1.0 };
    constexpr bl::f256 qd_boundary = qd_one + bl::f256{ 0x1p-188 };
    constexpr bl::f256 qd_outside = qd_one + bl::f256{ 0x1p-187 };
    static_assert(bl::approx_eq(qd_one, qd_boundary));
    static_assert(!bl::approx_eq(qd_one, qd_outside));

    constexpr bl::f128 dd_max = std::numeric_limits<bl::f128>::max();
    constexpr bl::f256 qd_max = std::numeric_limits<bl::f256>::max();
    static_assert(!bl::approx_eq(dd_max, -dd_max, bl::f128{ 1.5 }));
    static_assert(bl::approx_eq(dd_max, -dd_max, bl::f128{ 2.0 }));
    static_assert(!bl::approx_eq(qd_max, -qd_max, bl::f256{ 1.5 }));
    static_assert(bl::approx_eq(qd_max, -qd_max, bl::f256{ 2.0 }));
}

TEST_CASE("approx_eq applies relative tolerances", "[contracts][comparison]")
{
    CHECK(bl::approx_eq(dd_one, dd_boundary));
    CHECK(bl::approx_eq(dd_boundary, dd_one));
    CHECK_FALSE(bl::approx_eq(dd_one, dd_outside));

    const bl::f128 dd_large = bl::ldexp(bl::f128{ 1.0 }, 400);
    CHECK(bl::approx_eq(
        dd_large,
        dd_large + bl::ldexp(bl::f128{ 1.0 }, 320)));
    CHECK_FALSE(bl::approx_eq(
        dd_large,
        dd_large + bl::ldexp(bl::f128{ 1.0 }, 321)));

    CHECK(bl::approx_eq(qd_one, qd_boundary));
    CHECK(bl::approx_eq(qd_boundary, qd_one));
    CHECK_FALSE(bl::approx_eq(qd_one, qd_outside));

    const bl::f256 qd_large = bl::ldexp(bl::f256{ 1.0 }, 400);
    const bl::f256 qd_large_boundary =
        qd_large + bl::ldexp(bl::f256{ 1.0 }, 212);
    const bl::f256 qd_large_outside =
        qd_large + bl::ldexp(bl::f256{ 1.0 }, 213);
    CHECK(bl::approx_eq(
        qd_large,
        qd_large_boundary));
    CHECK_FALSE(bl::approx_eq(
        qd_large,
        qd_large_outside));

    CHECK_FALSE(bl::approx_eq(bl::f128{ 0.0 }, bl::f128{ 0x1p-900 }));
    CHECK_FALSE(bl::approx_eq(bl::f256{ 0.0 }, bl::f256{ 0x1p-900 }));
}

TEST_CASE("approx_eq preserves special-value semantics", "[contracts][comparison]")
{
    const bl::f128 dd_positive_zero{ 0.0 };
    const bl::f128 dd_negative_zero{ -0.0 };
    const bl::f128 dd_infinity = std::numeric_limits<bl::f128>::infinity();
    const bl::f128 dd_nan = std::numeric_limits<bl::f128>::quiet_NaN();

    CHECK(bl::approx_eq(dd_positive_zero, dd_negative_zero));
    CHECK(bl::approx_eq(dd_infinity, dd_infinity));
    CHECK_FALSE(bl::approx_eq(dd_infinity, -dd_infinity));
    CHECK_FALSE(bl::approx_eq(dd_nan, dd_nan));
    CHECK_FALSE(bl::approx_eq(dd_one, dd_one, bl::f128{ -1.0 }));
    CHECK_FALSE(bl::approx_eq(dd_one, dd_one, dd_nan));
    CHECK(bl::approx_eq(dd_one, dd_outside, dd_infinity));

    const bl::f256 qd_positive_zero{ 0.0 };
    const bl::f256 qd_negative_zero{ -0.0 };
    const bl::f256 qd_infinity = std::numeric_limits<bl::f256>::infinity();
    const bl::f256 qd_nan = std::numeric_limits<bl::f256>::quiet_NaN();

    CHECK(bl::approx_eq(qd_positive_zero, qd_negative_zero));
    CHECK(bl::approx_eq(qd_infinity, qd_infinity));
    CHECK_FALSE(bl::approx_eq(qd_infinity, -qd_infinity));
    CHECK_FALSE(bl::approx_eq(qd_nan, qd_nan));
    CHECK_FALSE(bl::approx_eq(qd_one, qd_one, bl::f256{ -1.0 }));
    CHECK_FALSE(bl::approx_eq(qd_one, qd_one, qd_nan));
    CHECK(bl::approx_eq(qd_one, qd_outside, qd_infinity));

    CHECK_FALSE(bl::approx_eq(dd_max, -dd_max, bl::f128{ 1.5 }));
    CHECK(bl::approx_eq(dd_max, -dd_max, bl::f128{ 2.0 }));
    CHECK_FALSE(bl::approx_eq(qd_max, -qd_max, bl::f256{ 1.5 }));
    CHECK(bl::approx_eq(qd_max, -qd_max, bl::f256{ 2.0 }));
}
