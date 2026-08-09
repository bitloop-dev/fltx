#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

#include <fltx.h>

namespace
{
    constexpr bl::f256 expression_x{
        0x1.2000000000000p+0,
        0x1.09d8792fb4c49p-113,
        0x1.5a5ead789df78p-167,
        0x1.6227d7f2ceee1p-221
    };
    constexpr bl::f256 expression_y{
        -0x1.c000000000000p-1,
        -0x1.09d8792fb4c49p-113,
        -0x1.5a5ead789df78p-167,
        -0x1.6227d7f2ceee1p-221
    };
    constexpr bl::f256 expression_c{
        0x1.5555555555555p-2,
        0x1.5555555555555p-56,
        0x1.4a41a59e03228p-110,
        -0x1.8e6e9c8fb13fap-164
    };

    [[nodiscard]] auto delayed_expression()
    {
        const bl::f256 x = expression_x;
        const bl::f256 y = expression_y;
        const bl::f256 c = expression_c;
        return x * x - y * y + c;
    }

    [[nodiscard]] auto delayed_math_expression()
    {
        const bl::f256 value{ 1.3125, 0x1p-60, -0x1p-120, 0x1p-180 };
        return bl::log(value) - value;
    }

    struct point
    {
        bl::f256_s x;
        bl::f256_s y;
    };

    [[nodiscard]] auto delayed_storage_expression()
    {
        const point a{
            { 0.875, 0x1p-60, -0x1p-120, 0x1p-180 },
            { -1.125, -0x1p-60, 0x1p-120, -0x1p-180 }
        };
        const point b{
            { 1.375, 0x1p-60, -0x1p-120, 0x1p-180 },
            { -1.625, -0x1p-60, 0x1p-120, -0x1p-180 }
        };
        return ((a.x + a.y * 0.5) - (b.x / 3.0)) +
               ((2.0 * b.y) - (a.x * b.y)) / (a.y + 1.25);
    }

    void clobber_stack()
    {
        std::array<bl::f256_s, 64> values{};
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            const double scale = static_cast<double>(i + 1);
            values[i] = {
                scale,
                std::ldexp(scale, -60),
                -std::ldexp(scale, -120),
                std::ldexp(scale, -180)
            };
        }

        static volatile double sink = 0.0;
        sink = values.back().x0 + values.back().x3;
    }

    void require_same_limbs(const bl::f256& a, const bl::f256& b)
    {
        CHECK(a.x0 == b.x0);
        CHECK(a.x1 == b.x1);
        CHECK(a.x2 == b.x2);
        CHECK(a.x3 == b.x3);
    }

    struct expression_case
    {
        const char* name;
        bl::f256 value;
        const char* expected;
    };

    void check_expression_case(const expression_case& test)
    {
        const bl::f256 expected = bl::parse<bl::f256>(test.expected);
        bl::f256 scale = bl::abs(expected);
        if (scale < bl::f256{ 1.0 })
            scale = bl::f256{ 1.0 };
        const bl::f256 tolerance =
            scale * std::numeric_limits<bl::f256>::epsilon() * 64.0;

        CAPTURE(test.name);
        CHECK(bl::abs(test.value - expected) <= tolerance);
    }
}

using product_expression = std::remove_cvref_t<decltype(
    std::declval<bl::f256>() * std::declval<bl::f256>())>;
static_assert(std::constructible_from<bl::f256, product_expression&&>);
static_assert(!std::constructible_from<bl::f256, product_expression&>);
static_assert(!std::constructible_from<bl::f256, const product_expression&&>);

TEST_CASE("delayed f256 expressions own their operands", "[contracts][expressions]")
{
    auto expression = delayed_expression();
    clobber_stack();
    const bl::f256 delayed = std::move(expression);
    auto direct_expression = delayed_expression();
    const bl::f256 directly_constructed{ std::move(direct_expression) };
    const bl::f256 independently_rounded{
        0x1.aaaaaaaaaaaabp-1,
        -0x1.5555555555555p-55,
        -0x1.5290696780c8ap-109,
        0x1.8e6e9c8fb13fbp-165
    };
    require_same_limbs(delayed, independently_rounded);
    require_same_limbs(directly_constructed, independently_rounded);
}

TEST_CASE("delayed f256 expressions own math temporaries and storage members",
          "[contracts][expressions]")
{
    auto math_expression = delayed_math_expression();
    auto storage_expression = delayed_storage_expression();
    clobber_stack();

    const bl::f256 delayed_math = std::move(math_expression);
    const bl::f256 delayed_storage = std::move(storage_expression);

    const bl::f256 math_input{
        1.3125, 0x1p-60, -0x1p-120, 0x1p-180
    };
    const bl::f256 immediate_math = bl::log(math_input) - math_input;
    require_same_limbs(delayed_math, immediate_math);

    const point a{
        { 0.875, 0x1p-60, -0x1p-120, 0x1p-180 },
        { -1.125, -0x1p-60, 0x1p-120, -0x1p-180 }
    };
    const point b{
        { 1.375, 0x1p-60, -0x1p-120, 0x1p-180 },
        { -1.625, -0x1p-60, 0x1p-120, -0x1p-180 }
    };
    const bl::f256 immediate_storage =
        ((a.x + a.y * 0.5) - (b.x / 3.0)) +
        ((2.0 * b.y) - (a.x * b.y)) / (a.y + 1.25);
    require_same_limbs(delayed_storage, immediate_storage);
}

TEST_CASE("f256 multiplication remains fused with a following subtraction",
          "[contracts][expressions]")
{
    const bl::f256 rounded_product{
        -0x1.f800000000000p-1,
        -0x1.09d8792fb4c49p-112,
        -0x1.5a5ead789df78p-166,
        -0x1.66781f7a94e69p-220
    };
    const bl::f256 independently_rounded_residual{
        0x1.d529c8e2c2693p-274,
        0x1.f25380232caccp-328,
        -0x1.9cd5b6c082469p-386,
        -0x1.5eb53f5b41559p-441
    };

    const bl::f256 residual = expression_x * expression_y - rounded_product;
    const bl::f256 error = bl::abs(residual - independently_rounded_residual);
    const bl::f256 tolerance =
        bl::abs(independently_rounded_residual) * bl::f256{ 0x1p-50 };

    CHECK(residual != bl::f256{ 0.0 });
    CHECK(error <= tolerance);
}

TEST_CASE("f256 product expression shapes preserve mathematical value",
          "[contracts][expressions][fusion]")
{
    const bl::f256 x = bl::parse<bl::f256>(
        "1.1250000000000000000000000000000001");
    const bl::f256 y = bl::parse<bl::f256>(
        "-0.8750000000000000000000000000000001");
    const bl::f256 a = bl::parse<bl::f256>(
        "0.3333333333333333333333333333333333");

    auto xy0 = x * y;
    auto xy1 = x * y;
    const bl::f256 doubled = std::move(xy0) + std::move(xy1);

    auto xy2 = x * y;
    auto xy3 = x * y;
    const bl::f256 doubled_plus =
        a + (std::move(xy2) + std::move(xy3));

    auto xy4 = x * y;
    auto xy5 = x * y;
    const bl::f256 associated_plus =
        (std::move(xy4) + a) + std::move(xy5);

    auto xy6 = x * y;
    auto xy7 = x * y;
    const bl::f256 associated_minus =
        (std::move(xy6) - a) + std::move(xy7);

    auto xx0 = x * x;
    auto yy0 = y * y;
    const bl::f256 commuted_difference =
        a + (std::move(xx0) - std::move(yy0));

    auto xy8 = x * y;
    auto xx1 = x * x;
    auto yy1 = y * y;
    const bl::f256 product_reassociated =
        std::move(xy8) + (std::move(xx1) + std::move(yy1));

    auto xx2 = x * x;
    auto yy2 = y * y;
    const bl::f256 product_plus_value_sub_product =
        std::move(xx2) + (a - std::move(yy2));

    auto xx3 = x * x;
    auto yy3 = y * y;
    const bl::f256 value_sub_product_plus_product =
        (a - std::move(xx3)) + std::move(yy3);

    auto xx4 = x * x;
    auto yy4 = y * y;
    const bl::f256 product_minus_value_sub_product =
        std::move(xx4) - (a - std::move(yy4));

    const bl::f256 scaled_right_associated =
        (x * 1.25) + (a + y * -0.75);
    const bl::f256 scaled_value_sub_product =
        (x * 1.25) + (a - y * 0.75);
    const bl::f256 scaled_sub_associated =
        (x * 1.25) - (a + y * 0.75);
    const bl::f256 scaled_left_associated =
        (a + x * 1.25) - (y * 0.75);

    const std::array cases{
        expression_case{
            "doubled product", doubled,
            "-1.96875000000000000000000000000000040000000000000000000000000000000002"
        },
        expression_case{
            "doubled product plus value", doubled_plus,
            "-1.63541666666666666666666666666666710000000000000000000000000000000002"
        },
        expression_case{
            "associated plus", associated_plus,
            "-1.63541666666666666666666666666666710000000000000000000000000000000002"
        },
        expression_case{
            "associated minus", associated_minus,
            "-2.30208333333333333333333333333333370000000000000000000000000000000002"
        },
        expression_case{
            "commuted difference", commuted_difference,
            "0.83333333333333333333333333333333335000000000000000000000000000000000"
        },
        expression_case{
            "product reassociated", product_reassociated,
            "1.04687500000000000000000000000000020000000000000000000000000000000001"
        },
        expression_case{
            "product plus value minus product", product_plus_value_sub_product,
            "0.83333333333333333333333333333333335000000000000000000000000000000000"
        },
        expression_case{
            "value minus product plus product", value_sub_product_plus_product,
            "-0.16666666666666666666666666666666675000000000000000000000000000000000"
        },
        expression_case{
            "product minus value minus product", product_minus_value_sub_product,
            "1.69791666666666666666666666666666710000000000000000000000000000000002"
        },
        expression_case{
            "scaled right associated", scaled_right_associated,
            "2.395833333333333333333333333333333500"
        },
        expression_case{
            "scaled value minus product", scaled_value_sub_product,
            "2.395833333333333333333333333333333500"
        },
        expression_case{
            "scaled subtraction associated", scaled_sub_associated,
            "1.729166666666666666666666666666666900"
        },
        expression_case{
            "scaled left associated", scaled_left_associated,
            "2.395833333333333333333333333333333500"
        }
    };

    for (const auto& test : cases)
        check_expression_case(test);
}

TEST_CASE("f256 storage assignment materializes a complete expression", "[contracts][expressions]")
{
    const bl::f256 x{ 1.125 };
    const bl::f256 y{ 0.875 };
    const bl::f256 value = x * x + y * y;
    const bl::f256_s storage = value;
    CHECK(storage.x0 == value.x0);
    CHECK(storage.x1 == value.x1);
    CHECK(storage.x2 == value.x2);
    CHECK(storage.x3 == value.x3);
}
