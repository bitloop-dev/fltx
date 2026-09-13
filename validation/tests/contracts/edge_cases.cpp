#include <catch2/catch_test_macros.hpp>

#include "../../support/domains.hpp"

#include <cmath>
#include <limits>
#include <set>

#include <fltx.h>

TEST_CASE("extended math preserves representable exponential tails",
          "[contracts][edge-case][math]")
{
    const auto check = []<class T>() {
        for (double x : {13.0, 26.0, 27.0625})
        {
            CAPTURE(x);
            const T tail = bl::erfc(T{x});
            CHECK(bl::isfinite(tail));
            CHECK(tail > T{0.0});
            CHECK(tail < T{1.0});
        }
        CHECK(bl::erfc(T{28.0}) == T{0.0});
        CHECK(bl::erfc(T{-28.0}) == T{2.0});
        for (double x : {709.0, 710.0})
        {
            CAPTURE(x);
            const T s = bl::sinh(T{x});
            const T c = bl::cosh(T{x});
            CHECK(bl::isfinite(s));
            CHECK(bl::isfinite(c));
            CHECK(s > T{1.0e307});
            CHECK(c >= s);
            CHECK(bl::sinh(T{-x}) == -s);
            CHECK(bl::cosh(T{-x}) == c);
        }
        CHECK(bl::isinf(bl::sinh(T{711.0})));
        CHECK(bl::isinf(bl::cosh(T{711.0})));
        const T positive_gamma = bl::tgamma(T{-171.5});
        const T negative_gamma = bl::tgamma(T{-172.5});
        CHECK(bl::isfinite(positive_gamma));
        CHECK(positive_gamma > T{0.0});
        CHECK(bl::isfinite(negative_gamma));
        CHECK(negative_gamma < T{0.0});
    };
    check.template operator()<bl::fdd>();
    check.template operator()<bl::fqd>();
}

TEST_CASE("qd sparse tails do not overflow root and logarithm rescaling",
          "[contracts][edge-case][math]")
{
    for (double head : {0x1p-1074, 0x1p-1000, 0x1p510, 0x1p512, 0x1p958, 0x1p960, 0x1p1000})
    {
        CAPTURE(head);
        const bl::fqd sparse{head, head == 0x1p-1074 ? 0.0 : 0x1p-1074, 0.0, 0.0};
        const bl::fqd root = bl::sqrt(sparse);
        const bl::fqd cube_root = bl::cbrt(sparse);
        CHECK(bl::isfinite(root));
        CHECK(root > bl::fqd{0.0});
        CHECK(bl::isfinite(cube_root));
        CHECK(cube_root > bl::fqd{0.0});
        CHECK(bl::isfinite(bl::log(sparse)));
        CHECK(bl::isfinite(bl::log2(sparse)));
        CHECK(bl::isfinite(bl::log10(sparse)));
    }
}

TEST_CASE("large expansion limbs survive floor and truncation", "[contracts][edge-case]")
{
    const auto check_decimal_inputs = []<class T>() {
        const T floor_input = bl::parse<T>(
            "4.6958550912494028428400315673292717414e+19");
        const T trunc_input = bl::parse<T>(
            "-1.5848854675958108400285213569604012722e+23");
        CHECK(bl::floor(floor_input) == bl::parse<T>("46958550912494028428"));
        CHECK(bl::trunc(trunc_input) == bl::parse<T>("-158488546759581084002852"));
    };

    SECTION("dd")
    {
        check_decimal_inputs.template operator()<bl::fdd>();
        const bl::fdd value{ 0x1p+100, 0x1p+40 + 0.75 };
        CHECK(bl::floor(value) == bl::fdd{ 0x1p+100, 0x1p+40 });
        CHECK(bl::trunc(-value) == bl::fdd{ -0x1p+100, -0x1p+40 });
    }

    SECTION("qd")
    {
        check_decimal_inputs.template operator()<bl::fqd>();
        const bl::fqd value{
            0x1p+100, 0x1p+40 + 0.75, 0x1p-20, -0x1p-80
        };
        const bl::fqd integral{ 0x1p+100, 0x1p+40, 0.0, 0.0 };
        CHECK(bl::floor(value) == integral);
        CHECK(bl::trunc(-value) == -integral);
    }
}

TEST_CASE("fmod, remainder, and remquo preserve exact low-limb half ties",
          "[contracts][edge-case][remainder]")
{
    SECTION("dd")
    {
        const bl::fdd half{
            0x1.8000000000000p+0, 0x1.0000000000000p-60
        };
        const bl::fdd divisor{
            -0x1.8000000000000p+1, -0x1.0000000000000p-59
        };
        int quotient = 12345;

        CHECK(bl::fmod(half, divisor) == half);
        CHECK(bl::remainder(half, divisor) == half);
        CHECK(bl::remquo(half, divisor, &quotient) == half);
        CHECK(quotient == 0);
    }

    SECTION("qd")
    {
        const bl::fqd half{
            0x1.8000000000000p+0,
            0x1.0000000000000p-60,
            -0x1.0000000000000p-113,
            0x1.0000000000000p-166
        };
        const bl::fqd divisor{
            -0x1.8000000000000p+1,
            -0x1.0000000000000p-59,
            0x1.0000000000000p-112,
            -0x1.0000000000000p-165
        };
        int quotient = 12345;

        CHECK(bl::fmod(half, divisor) == half);
        CHECK(bl::remainder(half, divisor) == half);
        CHECK(bl::remquo(half, divisor, &quotient) == half);
        CHECK(quotient == 0);
    }
}

TEST_CASE("remainder operations handle huge double-divisor quotients",
          "[contracts][edge-case][remainder]")
{
    const auto check = []<class T>(const T& lhs, const T& rhs) {
        const T expected{ 0.5 };
        int quotient = 0;

        CHECK(bl::fmod(lhs, rhs) == expected);
        CHECK(bl::remainder(lhs, rhs) == expected);
        CHECK(bl::remquo(lhs, rhs, &quotient) == expected);
        CHECK(quotient < 0);
        CHECK((static_cast<unsigned>(-quotient) & 0x7u) == 1u);
    };

    const double large = std::nextafter(std::ldexp(1.0, 900), 0.0);
    check(bl::fdd{ large, 0.0 }, bl::fdd{ -7.5, 0.0 });
    check(
        bl::fqd{ large, 0.0, 0.0, 0.0 },
        bl::fqd{ -7.5, 0.0, 0.0, 0.0 });
}

TEST_CASE("nextafter crosses normal and subnormal boundaries", "[contracts][edge-case]")
{
    const bl::fdd minimum = std::numeric_limits<bl::fdd>::min();
    const bl::fdd below = bl::nextafter(minimum, bl::fdd{ 0.0 });
    CHECK(below < minimum);
    CHECK(bl::fpclassify(below) == FP_SUBNORMAL);
    CHECK(bl::nextafter(below, minimum) == minimum);

    const bl::fqd zero{ 0.0 };
    const bl::fqd negative = bl::nextafter(zero, bl::fqd{ -1.0 });
    CHECK(negative < zero);
    CHECK(bl::signbit(negative));
}

TEST_CASE("cancellation corpus preserves values below a binary64 ulp",
          "[contracts][support]")
{
    const auto domain = fltx::tests::domains::near_equal_cancellation(64);
    std::set<std::array<double, 4>> unique;
    std::size_t low_limb_values = 0;
    for (const auto& value : domain.values)
    {
        unique.insert(value.limb);
        if (value.limb[1] != 0.0)
            ++low_limb_values;
    }

    CHECK(unique.size() > 48);
    CHECK(low_limb_values > 40);
}

TEST_CASE("ternary accuracy domains are prefix-stable",
          "[contracts][support]")
{
    const auto check_prefix = [](const auto& small, const auto& large)
    {
        REQUIRE(small.values.size() < large.values.size());
        for (std::size_t i = 0; i < small.values.size(); ++i)
        {
            CHECK(small.values[i].x.limb == large.values[i].x.limb);
            CHECK(small.values[i].y.limb == large.values[i].y.limb);
            CHECK(small.values[i].z.limb == large.values[i].z.limb);
        }
    };

    check_prefix(
        fltx::tests::domains::moderate_ternary(8),
        fltx::tests::domains::moderate_ternary(16));
    check_prefix(
        fltx::tests::domains::wide_exponent_ternary(8),
        fltx::tests::domains::wide_exponent_ternary(16));

    const auto cancellation = fltx::tests::domains::fma_cancellation(8);
    check_prefix(
        cancellation,
        fltx::tests::domains::fma_cancellation(16));

    CHECK(cancellation.values.front().x.limb == std::array<double, 4>{
        0x1.000000000069dp+0, 0x1.aff8095000000p-54, 0.0, 0.0
    });
    CHECK(cancellation.values.front().y.limb == std::array<double, 4>{
        0x1.0000000000001p+0,
        0x1.0000000000001p-55,
        -0x1.0000000000001p-109,
        0x1.0000000000001p-163
    });
}

TEST_CASE("fdd fma handles an overflowing leading product",
          "[contracts][edge-case][fma]")
{
    const bl::fdd huge{ 0x1p+1000 };
    const bl::fdd overflow = bl::fma(huge, huge, bl::fdd{ 1.0 });
    CHECK(bl::isinf(overflow));
    CHECK_FALSE(bl::signbit(overflow));
    CHECK(overflow.lo == 0.0);

    const bl::fdd negative_overflow = bl::fma(-huge, huge, bl::fdd{ -1.0 });
    CHECK(bl::isinf(negative_overflow));
    CHECK(bl::signbit(negative_overflow));
    CHECK(negative_overflow.lo == 0.0);

    const bl::fdd exact_power{ 0x1p+512 };
    const bl::fdd cancelled = bl::fma(
        exact_power,
        exact_power,
        bl::fdd{ -std::numeric_limits<double>::max() });
    CHECK(cancelled == bl::fdd{ 0x1p+971 });
}

TEST_CASE("fqd Payne-Hanek reduction preserves quadrants for deep expansions",
          "[contracts][edge-case][trig]")
{
    const bl::fqd near_negative_29_pi{
        -91.106186954104004,
        1.2379612731767154e-18,
        -1.1116010032489903e-35,
        -6.0911710209670782e-52
    };
    CHECK(bl::sin(near_negative_29_pi) > bl::fqd{ 0.0 });
    CHECK(bl::cos(near_negative_29_pi) < bl::fqd{ 0.0 });

    const bl::fqd above_25_pi_over_2{
        39.269908169872416,
        -2.4554834046605899e-16,
        -3.385941079041688e-34,
        -8.7933276384616772e-54
    };
    CHECK(bl::tan(above_25_pi_over_2) < bl::fqd{ 0.0 });
    CHECK(bl::abs(bl::tan(above_25_pi_over_2)) > bl::fqd{ 1.0e30 });
}
