#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include <fltx.h>

namespace
{
    constexpr bl::fqd expression_x{
        0x1.2000000000000p+0,
        0x1.09d8792fb4c49p-113,
        0x1.5a5ead789df78p-167,
        0x1.6227d7f2ceee1p-221
    };
    constexpr bl::fqd expression_y{
        -0x1.c000000000000p-1,
        -0x1.09d8792fb4c49p-113,
        -0x1.5a5ead789df78p-167,
        -0x1.6227d7f2ceee1p-221
    };
    constexpr bl::fqd expression_c{
        0x1.5555555555555p-2,
        0x1.5555555555555p-56,
        0x1.4a41a59e03228p-110,
        -0x1.8e6e9c8fb13fap-164
    };

    [[nodiscard]] auto delayed_expression()
    {
        const bl::fqd x = expression_x;
        const bl::fqd y = expression_y;
        const bl::fqd c = expression_c;
        return x * x - y * y + c;
    }

    [[nodiscard]] auto delayed_math_expression()
    {
        const bl::fqd value{ 1.3125, 0x1p-60, -0x1p-120, 0x1p-180 };
        return bl::log(value) - value;
    }

    struct point
    {
        bl::fqd_s x;
        bl::fqd_s y;
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
        std::array<bl::fqd_s, 64> values{};
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

    void require_same_limbs(const bl::fqd& a, const bl::fqd& b)
    {
        CHECK(a.x0 == b.x0);
        CHECK(a.x1 == b.x1);
        CHECK(a.x2 == b.x2);
        CHECK(a.x3 == b.x3);
    }

    struct expression_case
    {
        const char* name;
        bl::fqd value;
        const char* expected;
    };

    void check_expression_case(const expression_case& test)
    {
        const bl::fqd expected = bl::parse<bl::fqd>(test.expected);
        bl::fqd scale = bl::abs(expected);
        if (scale < bl::fqd{ 1.0 })
            scale = bl::fqd{ 1.0 };
        const bl::fqd tolerance =
            scale * std::numeric_limits<bl::fqd>::epsilon() * 64.0;

        CAPTURE(test.name);
        CHECK(bl::abs(test.value - expected) <= tolerance);
    }
}

using product_expression = std::remove_cvref_t<decltype(
    std::declval<bl::fqd>() * std::declval<bl::fqd>())>;
using abs_product_expression = std::remove_cvref_t<decltype(
    bl::abs(std::declval<bl::fqd>()) * std::declval<bl::fqd>())>;
using clamp_product_expression = std::remove_cvref_t<decltype(
    bl::clamp(
        std::declval<bl::fqd>(),
        std::declval<bl::fqd>(),
        std::declval<bl::fqd>()) * std::declval<bl::fqd>())>;

template<class Expr>
concept has_direct_qd_expression_conversions = requires(Expr&& expression)
{
    static_cast<bl::fdd_s>(std::move(expression));
    static_cast<bl::fdd>(std::move(expression));
    static_cast<bool>(std::move(expression));
    static_cast<char>(std::move(expression));
    static_cast<signed char>(std::move(expression));
    static_cast<unsigned char>(std::move(expression));
    static_cast<wchar_t>(std::move(expression));
    static_cast<char8_t>(std::move(expression));
    static_cast<char16_t>(std::move(expression));
    static_cast<char32_t>(std::move(expression));
    static_cast<short>(std::move(expression));
    static_cast<unsigned short>(std::move(expression));
    static_cast<int>(std::move(expression));
    static_cast<unsigned int>(std::move(expression));
    static_cast<long>(std::move(expression));
    static_cast<unsigned long>(std::move(expression));
    static_cast<long long>(std::move(expression));
    static_cast<unsigned long long>(std::move(expression));
    static_cast<std::size_t>(std::move(expression));
    static_cast<float>(std::move(expression));
    static_cast<double>(std::move(expression));
    static_cast<long double>(std::move(expression));
};

template<class Expr>
concept has_lvalue_double_conversion = requires(Expr& expression)
{
    static_cast<double>(expression);
};

template<class Expr>
concept has_const_rvalue_double_conversion = requires(const Expr&& expression)
{
    static_cast<double>(std::move(expression));
};

using qd_leaf_expression = bl::detail::_qd_expr::leaf_expr;
using qd_mul_expression = bl::detail::_qd_expr::mul_expr<qd_leaf_expression, qd_leaf_expression>;
using qd_expression_nodes = std::tuple<
    qd_leaf_expression,
    bl::detail::_qd_expr::mul_double_expr<qd_leaf_expression>,
    bl::detail::_qd_expr::add_double_expr<qd_leaf_expression>,
    bl::detail::_qd_expr::double_sub_expr<qd_leaf_expression>,
    bl::detail::_qd_expr::add_expr<qd_leaf_expression, qd_leaf_expression>,
    bl::detail::_qd_expr::sub_expr<qd_leaf_expression, qd_leaf_expression>,
    qd_mul_expression,
    bl::detail::_qd_expr::div_expr<qd_leaf_expression, qd_leaf_expression>,
    bl::detail::_qd_expr::div_double_expr<qd_leaf_expression>,
    bl::detail::_qd_expr::double_div_expr<qd_leaf_expression>,
    bl::detail::_qd_expr::prod_pair_expr<qd_mul_expression, qd_mul_expression, 1>,
    bl::detail::_qd_expr::prod_value_expr<qd_mul_expression, qd_leaf_expression, 1>,
    bl::detail::_qd_expr::prod_pair_value_expr<qd_mul_expression, qd_mul_expression, qd_leaf_expression, 1, 1>,
    bl::detail::_qd_expr::prod_triple_add_expr<qd_mul_expression, qd_mul_expression, qd_mul_expression>>;

template<class... Expr>
consteval bool all_expression_nodes_have_direct_conversions(std::tuple<Expr...>*)
{
    return (has_direct_qd_expression_conversions<Expr> && ...);
}

template<class... Expr>
consteval bool all_expression_nodes_publish_their_value_type(std::tuple<Expr...>*)
{
    return ((bl::fltx_expression<Expr> &&
             std::same_as<bl::fltx_expression_value_t<Expr>, bl::fqd> &&
             std::same_as<bl::fltx_expression_storage_t<Expr>, bl::fqd_s>) && ...);
}

static_assert(std::constructible_from<bl::fqd, product_expression&&>);
static_assert(std::constructible_from<bl::fqd, product_expression&>);
static_assert(std::constructible_from<bl::fqd, const product_expression&&>);
static_assert(all_expression_nodes_have_direct_conversions(static_cast<qd_expression_nodes*>(nullptr)));
static_assert(all_expression_nodes_publish_their_value_type(static_cast<qd_expression_nodes*>(nullptr)));
static_assert(std::same_as<bl::fltx_expression_value_t<double>, double>);
static_assert(std::same_as<bl::fltx_expression_value_t<bl::fdd>, bl::fdd>);
static_assert(std::same_as<bl::fltx_expression_storage_t<double>, double>);
static_assert(std::same_as<bl::fltx_expression_storage_t<bl::fdd>, bl::fdd>);
static_assert(!std::convertible_to<product_expression&&, double>);
static_assert(has_lvalue_double_conversion<product_expression>);
static_assert(has_const_rvalue_double_conversion<product_expression>);
static_assert(bl::detail::_qd_expr::is_expr<abs_product_expression>::value);
static_assert(bl::detail::_qd_expr::is_expr<clamp_product_expression>::value);

TEST_CASE("delayed fqd expressions own their operands", "[contracts][expressions]")
{
    auto expression = delayed_expression();
    clobber_stack();
    const bl::fqd delayed = std::move(expression);
    auto direct_expression = delayed_expression();
    const bl::fqd directly_constructed{ std::move(direct_expression) };
    const bl::fqd independently_rounded{
        0x1.aaaaaaaaaaaabp-1,
        -0x1.5555555555555p-55,
        -0x1.5290696780c8ap-109,
        0x1.8e6e9c8fb13fbp-165
    };
    require_same_limbs(delayed, independently_rounded);
    require_same_limbs(directly_constructed, independently_rounded);
}

TEST_CASE("delayed fqd expressions own math temporaries and storage members",
          "[contracts][expressions]")
{
    auto math_expression = delayed_math_expression();
    auto storage_expression = delayed_storage_expression();
    clobber_stack();

    const bl::fqd delayed_math = std::move(math_expression);
    const bl::fqd delayed_storage = std::move(storage_expression);

    const bl::fqd math_input{
        1.3125, 0x1p-60, -0x1p-120, 0x1p-180
    };
    const bl::fqd immediate_math = bl::log(math_input) - math_input;
    require_same_limbs(delayed_math, immediate_math);

    const point a{
        { 0.875, 0x1p-60, -0x1p-120, 0x1p-180 },
        { -1.125, -0x1p-60, 0x1p-120, -0x1p-180 }
    };
    const point b{
        { 1.375, 0x1p-60, -0x1p-120, 0x1p-180 },
        { -1.625, -0x1p-60, 0x1p-120, -0x1p-180 }
    };
    const bl::fqd immediate_storage =
        ((a.x + a.y * 0.5) - (b.x / 3.0)) +
        ((2.0 * b.y) - (a.x * b.y)) / (a.y + 1.25);
    require_same_limbs(delayed_storage, immediate_storage);
}

TEST_CASE("owning fqd expressions convert directly to value and narrower scalar types",
          "[contracts][expressions][conversion]")
{
    const auto make_expression = [] {
        return expression_x / expression_c + 0.25;
    };
    const bl::fqd expected = make_expression();

    CHECK(static_cast<bl::fdd_s>(make_expression()) == static_cast<bl::fdd_s>(expected));
    CHECK(static_cast<bl::fdd>(make_expression()) == static_cast<bl::fdd>(expected));
    CHECK(static_cast<double>(make_expression()) == static_cast<double>(expected));
    CHECK(static_cast<float>(make_expression()) == static_cast<float>(expected));
    CHECK(static_cast<long double>(make_expression()) == static_cast<long double>(expected));
    CHECK(static_cast<int>(make_expression()) == static_cast<int>(expected));
    CHECK(static_cast<std::size_t>(make_expression()) == static_cast<std::size_t>(expected));
    CHECK(static_cast<unsigned char>(make_expression()) == static_cast<unsigned char>(expected));
    CHECK(static_cast<bool>(make_expression()) == static_cast<bool>(expected));

    const auto expression = make_expression();
    CHECK(bl::fqd{ expression } == expected);
    CHECK(static_cast<bl::fdd_s>(expression) == static_cast<bl::fdd_s>(expected));
    CHECK(static_cast<bl::fdd>(expression) == static_cast<bl::fdd>(expected));
    CHECK(static_cast<double>(expression) == static_cast<double>(expected));
    CHECK(static_cast<float>(expression) == static_cast<float>(expected));
    CHECK(static_cast<long double>(expression) == static_cast<long double>(expected));
    CHECK(static_cast<int>(expression) == static_cast<int>(expected));
    CHECK(static_cast<std::size_t>(expression) == static_cast<std::size_t>(expected));
    CHECK(static_cast<unsigned char>(expression) == static_cast<unsigned char>(expected));
    CHECK(static_cast<bool>(expression) == static_cast<bool>(expected));
}

TEST_CASE("fqd multiplication remains fused with a following subtraction",
          "[contracts][expressions]")
{
    const bl::fqd rounded_product{
        -0x1.f800000000000p-1,
        -0x1.09d8792fb4c49p-112,
        -0x1.5a5ead789df78p-166,
        -0x1.66781f7a94e69p-220
    };
    const bl::fqd independently_rounded_residual{
        0x1.d529c8e2c2693p-274,
        0x1.f25380232caccp-328,
        -0x1.9cd5b6c082469p-386,
        -0x1.5eb53f5b41559p-441
    };

    const bl::fqd residual = expression_x * expression_y - rounded_product;
    const bl::fqd error = bl::abs(residual - independently_rounded_residual);
    const bl::fqd tolerance =
        bl::abs(independently_rounded_residual) * bl::fqd{ 0x1p-50 };

    CHECK(residual != bl::fqd{ 0.0 });
    CHECK(error <= tolerance);
}

TEST_CASE("fqd product expression shapes preserve mathematical value",
          "[contracts][expressions][fusion]")
{
    const bl::fqd x = bl::parse<bl::fqd>(
        "1.1250000000000000000000000000000001");
    const bl::fqd y = bl::parse<bl::fqd>(
        "-0.8750000000000000000000000000000001");
    const bl::fqd a = bl::parse<bl::fqd>(
        "0.3333333333333333333333333333333333");

    auto xy0 = x * y;
    auto xy1 = x * y;
    const bl::fqd doubled = std::move(xy0) + std::move(xy1);

    auto xy2 = x * y;
    auto xy3 = x * y;
    const bl::fqd doubled_plus =
        a + (std::move(xy2) + std::move(xy3));

    auto xy4 = x * y;
    auto xy5 = x * y;
    const bl::fqd associated_plus =
        (std::move(xy4) + a) + std::move(xy5);

    auto xy6 = x * y;
    auto xy7 = x * y;
    const bl::fqd associated_minus =
        (std::move(xy6) - a) + std::move(xy7);

    auto xx0 = x * x;
    auto yy0 = y * y;
    const bl::fqd commuted_difference =
        a + (std::move(xx0) - std::move(yy0));

    auto xy8 = x * y;
    auto xx1 = x * x;
    auto yy1 = y * y;
    const bl::fqd product_reassociated =
        std::move(xy8) + (std::move(xx1) + std::move(yy1));

    auto xx2 = x * x;
    auto yy2 = y * y;
    const bl::fqd product_plus_value_sub_product =
        std::move(xx2) + (a - std::move(yy2));

    auto xx3 = x * x;
    auto yy3 = y * y;
    const bl::fqd value_sub_product_plus_product =
        (a - std::move(xx3)) + std::move(yy3);

    auto xx4 = x * x;
    auto yy4 = y * y;
    const bl::fqd product_minus_value_sub_product =
        std::move(xx4) - (a - std::move(yy4));

    const bl::fqd scaled_right_associated =
        (x * 1.25) + (a + y * -0.75);
    const bl::fqd scaled_value_sub_product =
        (x * 1.25) + (a - y * 0.75);
    const bl::fqd scaled_sub_associated =
        (x * 1.25) - (a + y * 0.75);
    const bl::fqd scaled_left_associated =
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

TEST_CASE("fqd expression assignment materializes into fqd_s", "[contracts][expressions]")
{
    const bl::fqd x{ 1.125 };
    const bl::fqd y{ 0.875 };
    const bl::fqd value = x * x + y * y;
    const bl::fqd_s storage = value;
    CHECK(storage.x0 == value.x0);
    CHECK(storage.x1 == value.x1);
    CHECK(storage.x2 == value.x2);
    CHECK(storage.x3 == value.x3);
}
