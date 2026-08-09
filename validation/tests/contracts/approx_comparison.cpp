#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <limits>

#include <fltx.h>

namespace
{
    template<class Left, class Right>
    concept has_default_almost_equal = requires (const Left& left, const Right& right)
    {
        { bl::almost_equal(left, right) } -> std::same_as<bool>;
    };

    static_assert(has_default_almost_equal<bl::f128, bl::f128>);
    static_assert(has_default_almost_equal<bl::f128_s, bl::f128>);
    static_assert(has_default_almost_equal<bl::f128, bl::f128_s>);
    static_assert(has_default_almost_equal<bl::f128_s, bl::f128_s>);
    static_assert(has_default_almost_equal<bl::f256, bl::f256>);
    static_assert(has_default_almost_equal<bl::f256_s, bl::f256>);
    static_assert(has_default_almost_equal<bl::f256, bl::f256_s>);
    static_assert(has_default_almost_equal<bl::f256_s, bl::f256_s>);
    static_assert(!has_default_almost_equal<double, double>);
    static_assert(!has_default_almost_equal<bl::f128, double>);
    static_assert(!has_default_almost_equal<bl::f256, double>);
    static_assert(!has_default_almost_equal<bl::f128, bl::f256>);
    static_assert(!has_default_almost_equal<bl::f256, bl::f128>);

    constexpr bl::f128 dd_one{ 1.0 };
    constexpr bl::f128 dd_inside = dd_one + bl::f128{ 0x1p-75 };
    constexpr bl::f128 dd_outside = dd_one + bl::f128{ 0x1p-73 };
    static_assert(bl::almost_equal(dd_one, dd_inside));
    static_assert(!bl::almost_equal(dd_one, dd_outside));

    constexpr bl::f256 qd_one{ 1.0 };
    constexpr bl::f256 qd_inside = qd_one + bl::f256{ 0x1p-170 };
    constexpr bl::f256 qd_outside = qd_one + bl::f256{ 0x1p-168 };
    static_assert(bl::almost_equal(qd_one, qd_inside));
    static_assert(!bl::almost_equal(qd_one, qd_outside));

    constexpr bl::f128 dd_max = std::numeric_limits<bl::f128>::max();
    constexpr bl::f256 qd_max = std::numeric_limits<bl::f256>::max();
    static_assert(!bl::almost_equal(dd_max, -dd_max, bl::f128{ 1.5 }));
    static_assert(bl::almost_equal(dd_max, -dd_max, bl::f128{ 2.0 }));
    static_assert(!bl::almost_equal(qd_max, -qd_max, bl::f256{ 1.5 }));
    static_assert(bl::almost_equal(qd_max, -qd_max, bl::f256{ 2.0 }));
}

TEST_CASE("almost_equal applies relative and absolute tolerances", "[contracts][comparison]")
{
    CHECK(bl::almost_equal(dd_one, dd_inside));
    CHECK(bl::almost_equal(dd_inside, dd_one));
    CHECK_FALSE(bl::almost_equal(dd_one, dd_outside));

    const bl::f128 dd_large = bl::ldexp(bl::f128{ 1.0 }, 400);
    CHECK(bl::almost_equal(
        dd_large,
        dd_large + bl::ldexp(bl::f128{ 1.0 }, 325)));
    CHECK_FALSE(bl::almost_equal(
        dd_large,
        dd_large + bl::ldexp(bl::f128{ 1.0 }, 327)));

    const bl::f128 dd_tiny{ 0x1p-900 };
    CHECK_FALSE(bl::almost_equal(bl::f128{ 0.0 }, dd_tiny));
    CHECK(bl::almost_equal(
        bl::f128{ 0.0 },
        dd_tiny,
        bl::f128{ 0.0 },
        bl::f128{ 0x1p-900 }));
    CHECK_FALSE(bl::almost_equal(
        bl::f128{ 0.0 },
        bl::nextafter(
            dd_tiny,
            std::numeric_limits<bl::f128>::infinity()),
        bl::f128{ 0.0 },
        dd_tiny));

    CHECK(bl::almost_equal(qd_one, qd_inside));
    CHECK(bl::almost_equal(qd_inside, qd_one));
    CHECK_FALSE(bl::almost_equal(qd_one, qd_outside));

    const bl::f256 qd_large = bl::ldexp(bl::f256{ 1.0 }, 400);
    const bl::f256 qd_large_inside =
        qd_large + bl::ldexp(bl::f256{ 1.0 }, 230);
    const bl::f256 qd_large_outside =
        qd_large + bl::ldexp(bl::f256{ 1.0 }, 232);
    CHECK(bl::almost_equal(
        qd_large,
        qd_large_inside));
    CHECK_FALSE(bl::almost_equal(
        qd_large,
        qd_large_outside));

    const bl::f256 qd_tiny{ 0x1p-900 };
    CHECK_FALSE(bl::almost_equal(bl::f256{ 0.0 }, qd_tiny));
    CHECK(bl::almost_equal(
        bl::f256{ 0.0 },
        qd_tiny,
        bl::f256{ 0.0 },
        bl::f256{ 0x1p-900 }));
    CHECK_FALSE(bl::almost_equal(
        bl::f256{ 0.0 },
        bl::nextafter(
            qd_tiny,
            std::numeric_limits<bl::f256>::infinity()),
        bl::f256{ 0.0 },
        qd_tiny));
}

TEST_CASE("almost_equal preserves special-value semantics", "[contracts][comparison]")
{
    const bl::f128 dd_positive_zero{ 0.0 };
    const bl::f128 dd_negative_zero{ -0.0 };
    const bl::f128 dd_infinity = std::numeric_limits<bl::f128>::infinity();
    const bl::f128 dd_nan = std::numeric_limits<bl::f128>::quiet_NaN();

    CHECK(bl::almost_equal(dd_positive_zero, dd_negative_zero));
    CHECK(bl::almost_equal(dd_infinity, dd_infinity));
    CHECK_FALSE(bl::almost_equal(dd_infinity, -dd_infinity));
    CHECK_FALSE(bl::almost_equal(dd_nan, dd_nan));
    CHECK_FALSE(bl::almost_equal(dd_one, dd_one, bl::f128{ -1.0 }));
    CHECK_FALSE(bl::almost_equal(dd_one, dd_one, dd_nan));
    CHECK_FALSE(bl::almost_equal(
        dd_one, dd_one, bl::f128{ 0.0 }, bl::f128{ -1.0 }));
    CHECK_FALSE(bl::almost_equal(
        dd_one, dd_one, bl::f128{ 0.0 }, dd_nan));
    CHECK(bl::almost_equal(
        dd_one,
        dd_outside,
        dd_infinity,
        bl::f128{ 0.0 }));
    CHECK(bl::almost_equal(
        dd_one,
        dd_outside,
        bl::f128{ 0.0 },
        dd_infinity));

    const bl::f256 qd_positive_zero{ 0.0 };
    const bl::f256 qd_negative_zero{ -0.0 };
    const bl::f256 qd_infinity = std::numeric_limits<bl::f256>::infinity();
    const bl::f256 qd_nan = std::numeric_limits<bl::f256>::quiet_NaN();

    CHECK(bl::almost_equal(qd_positive_zero, qd_negative_zero));
    CHECK(bl::almost_equal(qd_infinity, qd_infinity));
    CHECK_FALSE(bl::almost_equal(qd_infinity, -qd_infinity));
    CHECK_FALSE(bl::almost_equal(qd_nan, qd_nan));
    CHECK_FALSE(bl::almost_equal(qd_one, qd_one, bl::f256{ -1.0 }));
    CHECK_FALSE(bl::almost_equal(qd_one, qd_one, qd_nan));
    CHECK_FALSE(bl::almost_equal(
        qd_one, qd_one, bl::f256{ 0.0 }, bl::f256{ -1.0 }));
    CHECK_FALSE(bl::almost_equal(
        qd_one, qd_one, bl::f256{ 0.0 }, qd_nan));
    CHECK(bl::almost_equal(
        qd_one,
        qd_outside,
        qd_infinity,
        bl::f256{ 0.0 }));
    CHECK(bl::almost_equal(
        qd_one,
        qd_outside,
        bl::f256{ 0.0 },
        qd_infinity));

    CHECK_FALSE(bl::almost_equal(dd_max, -dd_max, bl::f128{ 1.5 }));
    CHECK(bl::almost_equal(dd_max, -dd_max, bl::f128{ 2.0 }));
    CHECK_FALSE(bl::almost_equal(qd_max, -qd_max, bl::f256{ 1.5 }));
    CHECK(bl::almost_equal(qd_max, -qd_max, bl::f256{ 2.0 }));
}
