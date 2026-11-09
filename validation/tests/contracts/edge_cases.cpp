#include <catch2/catch_test_macros.hpp>

#include "../../support/domains.hpp"

#include <cmath>
#include <limits>
#include <set>

#include <fltx.h>

#if !defined(FLTX_FAST_MATH)
TEST_CASE("extreme range results retain canonical limbs",
          "[contracts][edge-case][range-canonical]")
{
    const auto check_scaling = []<class T>() {
        for (double sign : {-1.0, 1.0})
        {
            for (int exponent : {1, 30, 1024, 2048, std::numeric_limits<int>::max()})
            {
                const T input{sign * (exponent < 1024 ? 0x1p1023 : 1.0)};
                const T out = bl::ldexp(input, exponent);
                CAPTURE(sign, exponent);
                CHECK(bl::isinf(out));
                CHECK(bl::signbit(out) == (sign < 0));
                if constexpr (sizeof(T) == sizeof(bl::fdd)) CHECK(out.lo == 0.0);
                else { CHECK(out.x1 == 0.0); CHECK(out.x2 == 0.0); CHECK(out.x3 == 0.0); }
                CHECK(bl::scalbn(input, exponent) == out);
                CHECK(bl::scalbln(input, exponent) == out);
            }
        }
    };
    check_scaling.template operator()<bl::fdd>();
    check_scaling.template operator()<bl::fdd_s>();
    check_scaling.template operator()<bl::fqd>();
    check_scaling.template operator()<bl::fqd_s>();

    for (double sign : {-1.0, 1.0})
    {
        for (double magnitude : {0x1p-1074, 0x1p-1030, 0x1p-1024})
        {
            const bl::fqd out = bl::recip(bl::fqd{sign * magnitude});
            CHECK(bl::isinf(out));
            CHECK(bl::signbit(out) == (sign < 0));
            CHECK(out.x1 == 0.0); CHECK(out.x2 == 0.0); CHECK(out.x3 == 0.0);
        }
        const bl::fqd boundary = bl::recip(bl::fqd{sign * 0x1.0000000000004p-1024});
        CHECK(bl::isfinite(boundary));
        CHECK(boundary == bl::fqd{boundary + bl::fqd{0.0}});
    }
}

#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
TEST_CASE("dd division protects the residual splitter at extreme exponents",
          "[contracts][edge-case][range-canonical]")
{
    for (int exponent : {109, 110, 111, 995, 996, 997, 1000, 1022})
    {
        for (double sign : {-1.0, 1.0})
        {
            const bl::fdd a{sign * std::ldexp(1.0, exponent),
                            sign * std::ldexp(1.0, exponent - 55)};
            CAPTURE(exponent, sign);
            CHECK(bl::fdd{a / bl::fdd{1.0}} == a);
            CHECK(bl::fdd{a / 1.0} == a);
            CHECK(bl::fdd{a / a} == bl::fdd{1.0});
            CHECK(bl::detail::_dd::div_finite_inline(a, bl::fdd_s{1.0}) == a);
            CHECK(bl::atan(a) == (sign < 0 ? -bl::detail::_dd::pi_2 : bl::detail::_dd::pi_2));
        }
    }
}
#endif

TEST_CASE("qd tiny stepping and decimal tails preserve representation identities",
          "[contracts][edge-case][range-canonical]")
{
    const auto stable = [](bl::fqd x) {
        const bl::fqd y = x + bl::fqd{0.0};
        CHECK(x == y);
        CHECK(x == bl::fqd{x - bl::fqd{0.0}});
        CHECK(x == bl::fqd{x * bl::fqd{1.0}});
        CHECK(x == bl::fqd{x / bl::fqd{1.0}});
        CHECK(std::hash<bl::fqd>{}(x) == std::hash<bl::fqd>{}(y));
    };
    for (double magnitude : {0x1p-1074, 0x1p-1030, 0x1p-1022, 0x1p-1021, 0x1p-1020})
        for (double sign : {-1.0, 1.0})
            for (double direction : {-1.0, 1.0})
            {
                const bl::fqd x{sign * magnitude};
                const bl::fqd out = bl::nextafter(x, bl::fqd{direction});
                stable(out);
                CHECK(bl::nextafter(out, x) == x);
                CHECK((direction > 0 ? out > x : out < x));
            }
    CHECK(bl::iszero(bl::nextafter(bl::fqd{-0x1p-1074}, bl::fqd{1.0})));
    for (const char* text : {"6.002e-291", "-6.002e-291",
        "9.764828507356783252210253403079124617387068392850063287793767521841862072699e-274",
        "6.002195961969672823482481250658182829807816806007646023760837288967435639935e-291"})
    {
        CAPTURE(text);
        const auto parsed = bl::try_parse<bl::fqd>(text);
        REQUIRE(parsed);
        stable(parsed.value);
        CHECK(bl::parse<bl::fqd_s>(text) == parsed.value);
    }
}

TEST_CASE("mixed qd division recovers from an overflowing reciprocal",
          "[contracts][edge-case][mixed-division]")
{
    const auto check = []<class QD, class DD>() {
        for (double magnitude : {0x1p-1074, 0x1p-1050, 0x1p-1030, 0x1p-1024,
                                 0x1.0000000000004p-1024, 0x1p-1023, 0x1p-1022})
        {
            for (double sign : {-1.0, 1.0})
            {
                const double denominator = sign * magnitude;
                for (double factor : {-4.0, -1.0, 1.0, 4.0})
                {
                    CAPTURE(magnitude, sign, factor);
                    const double numerator = factor * denominator;
                    const QD expected{ factor };
                    CHECK(QD{ QD{ numerator } / denominator } == expected);
                    CHECK(QD{ numerator / QD{ denominator } } == expected);
                    CHECK(QD{ QD{ numerator } / DD{ denominator } } == expected);
                    CHECK(QD{ DD{ numerator } / QD{ denominator } } == expected);
                    CHECK(bl::detail::_qd_runtime::div_double_finite(
                        bl::fqd_s{ numerator }, denominator) == expected);
                }
                for (double zero : {0.0, -0.0})
                {
                    const QD quotient = QD{ zero } / denominator;
                    CHECK(bl::iszero(quotient));
                    CHECK(bl::signbit(quotient) == (std::signbit(zero) != std::signbit(denominator)));
                }
            }
        }

        // A normalized DD numerator can have a nonzero low limb even when the
        // divisor is tiny. Exercise the route that bypasses its scalar shortcut.
        const DD numerator{ 0x1p-100, 0x1p-155 };
        const QD expected{ 0x1p930, 0x1p875, 0.0, 0.0 };
        CHECK(QD{ numerator / QD{ 0x1p-1030 } } == expected);
        CHECK(QD{ -numerator / QD{ 0x1p-1030 } } == -expected);
        const QD wide_numerator{ 0x1p-100, 0x1p-155, 0x1p-210, 0x1p-265 };
        const QD wide_expected{ 0x1p930, 0x1p875, 0x1p820, 0x1p765 };
        CHECK(QD{ wide_numerator / 0x1p-1030 } == wide_expected);
        CHECK(QD{ wide_numerator / DD{ 0x1p-1030 } } == wide_expected);
        CHECK(bl::detail::_qd_runtime::div_double_finite(wide_numerator, 0x1p-1030) == wide_expected);

        const QD overflow = QD{ 1.0 } / 0x1p-1030;
        CHECK(bl::isinf(overflow));
        CHECK_FALSE(bl::signbit(overflow));
        CHECK(overflow.x1 == 0.0);
        CHECK(overflow.x2 == 0.0);
        CHECK(overflow.x3 == 0.0);
    };
    check.template operator()<bl::fqd_s, bl::fdd_s>();
    check.template operator()<bl::fqd, bl::fdd>();
}
#endif

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
