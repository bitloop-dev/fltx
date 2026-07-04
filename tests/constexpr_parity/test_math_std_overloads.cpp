#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <type_traits>

#include <fltx/math.h>

namespace
{
#define FLTX_EXPECT_TYPE(EXPR, TYPE) static_assert(std::is_same_v<decltype(EXPR), TYPE>)

    static_assert(bl::fltx_precision_rank_v<float> == 1);
    static_assert(bl::fltx_precision_rank_v<int> == 2);
    static_assert(bl::fltx_precision_rank_v<bool> == 2);
    static_assert(bl::fltx_precision_rank_v<double> == 2);
    static_assert(bl::fltx_precision_rank_v<long double> == 3);
    static_assert(bl::fltx_precision_rank_v<const bl::f128_s&> == 4);
    static_assert(bl::fltx_precision_rank_v<bl::f256> == 5);

    static_assert(std::is_same_v<bl::common_float_type_t<float>, bl::f32>);
    static_assert(std::is_same_v<bl::common_float_type_t<int>, bl::f64>);
    static_assert(std::is_same_v<bl::common_float_type_t<bool>, bl::f64>);
    static_assert(std::is_same_v<bl::common_float_type_t<float, int>, bl::f64>);
    static_assert(std::is_same_v<bl::common_float_type_t<float, double>, bl::f64>);
    static_assert(std::is_same_v<bl::common_float_type_t<double, long double>, long double>);
    static_assert(std::is_same_v<bl::common_float_type_t<const bl::f128_s&, int>, bl::f128>);
    static_assert(std::is_same_v<bl::common_float_type_t<bl::f128, bl::f256_s>, bl::f256>);
    static_assert(!bl::detail::math::promoted_math_arg<long double>);

    FLTX_EXPECT_TYPE(bl::abs(-3), bl::f64);
    FLTX_EXPECT_TYPE(bl::sqrt(true), bl::f64);
    FLTX_EXPECT_TYPE(bl::fabs(-3), bl::f64);
    FLTX_EXPECT_TYPE(bl::signbit(1), bool);
    FLTX_EXPECT_TYPE(bl::isnan(1), bool);
    FLTX_EXPECT_TYPE(bl::isinf(1), bool);
    FLTX_EXPECT_TYPE(bl::isfinite(1), bool);
    FLTX_EXPECT_TYPE(bl::iszero(1), bool);
    FLTX_EXPECT_TYPE(bl::fpclassify(0), int);
    FLTX_EXPECT_TYPE(bl::isnormal(1), bool);

    FLTX_EXPECT_TYPE(bl::floor(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::ceil(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::trunc(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::round(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::nearbyint(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::rint(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::lround(1), long);
    FLTX_EXPECT_TYPE(bl::llround(1), long long);
    FLTX_EXPECT_TYPE(bl::lrint(1), long);
    FLTX_EXPECT_TYPE(bl::llrint(1), long long);

    FLTX_EXPECT_TYPE(bl::fmod(5, 2), bl::f64);
    FLTX_EXPECT_TYPE(bl::remainder(5, 2), bl::f64);
    FLTX_EXPECT_TYPE(bl::remquo(5, 2, static_cast<int*>(nullptr)), bl::f64);
    FLTX_EXPECT_TYPE(bl::fma(1.0f, 2.0, 3.0), bl::f64);
    FLTX_EXPECT_TYPE(bl::fmin(1.0f, 2.0), bl::f64);
    FLTX_EXPECT_TYPE(bl::fmax(1.0f, 2.0), bl::f64);
    FLTX_EXPECT_TYPE(bl::fdim(1.0f, 2.0), bl::f64);
    FLTX_EXPECT_TYPE(bl::copysign(1.0f, 2.0), bl::f64);

    FLTX_EXPECT_TYPE(bl::ldexp(1, 2), bl::f64);
    FLTX_EXPECT_TYPE(bl::scalbn(1, 2), bl::f64);
    FLTX_EXPECT_TYPE(bl::scalbln(1, 2L), bl::f64);
    FLTX_EXPECT_TYPE(bl::frexp(1, static_cast<int*>(nullptr)), bl::f64);
    FLTX_EXPECT_TYPE(bl::modf(1, static_cast<double*>(nullptr)), bl::f64);
    FLTX_EXPECT_TYPE(bl::ilogb(1), int);
    FLTX_EXPECT_TYPE(bl::logb(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::nextafter(1, 2), bl::f64);
    FLTX_EXPECT_TYPE(bl::nexttoward(1, 2.0L), bl::f64);
    FLTX_EXPECT_TYPE(bl::nexttoward(1.0f, 2.0), bl::f32);

    FLTX_EXPECT_TYPE(bl::exp(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::exp2(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::expm1(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::log(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::log2(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::log10(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::log1p(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::sqrt(9), bl::f64);
    FLTX_EXPECT_TYPE(bl::cbrt(8), bl::f64);
    FLTX_EXPECT_TYPE(bl::hypot(3, 4), bl::f64);
    FLTX_EXPECT_TYPE(bl::hypot(3.0f, 4.0), bl::f64);
    FLTX_EXPECT_TYPE(bl::pow(2, 5), bl::f64);

    FLTX_EXPECT_TYPE(bl::sin(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::cos(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::tan(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::atan(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::atan2(1.0f, 2.0), bl::f64);
    FLTX_EXPECT_TYPE(bl::asin(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::acos(1), bl::f64);

    FLTX_EXPECT_TYPE(bl::sinh(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::cosh(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::tanh(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::asinh(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::acosh(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::atanh(0), bl::f64);
    FLTX_EXPECT_TYPE(bl::erf(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::erfc(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::lgamma(1), bl::f64);
    FLTX_EXPECT_TYPE(bl::tgamma(1), bl::f64);

    FLTX_EXPECT_TYPE(bl::isunordered(1, 2.0f), bool);
    FLTX_EXPECT_TYPE(bl::isgreater(1, 2.0f), bool);
    FLTX_EXPECT_TYPE(bl::isgreaterequal(1, 2.0f), bool);
    FLTX_EXPECT_TYPE(bl::isless(1, 2.0f), bool);
    FLTX_EXPECT_TYPE(bl::islessequal(1, 2.0f), bool);
    FLTX_EXPECT_TYPE(bl::islessgreater(1, 2.0f), bool);

    FLTX_EXPECT_TYPE(bl::sqrt(bl::f128{ 4.0 }), bl::f128);
    FLTX_EXPECT_TYPE(bl::hypot(bl::f128{ 3.0 }, 4.0), bl::f128);
    FLTX_EXPECT_TYPE(bl::hypot(bl::f128{ 3.0 }, true), bl::f128);
    FLTX_EXPECT_TYPE(bl::atan2(bl::f128{ 1.0 }, 2.0), bl::f128);
    FLTX_EXPECT_TYPE(bl::fma(bl::f128{ 1.0 }, 2.0, 3.0), bl::f128);
    FLTX_EXPECT_TYPE(bl::fmin(bl::f128{ 1.0 }, bl::f256{ 2.0 }), bl::f256);
    FLTX_EXPECT_TYPE(bl::hypot(bl::f128{ 3.0 }, bl::f256{ 4.0 }), bl::f256);

#undef FLTX_EXPECT_TYPE

    static_assert(bl::sqrt(9) == 3.0);
    static_assert(bl::hypot(3, 4) == 5.0);
    static_assert(bl::fma(1.0f, 2.0, 3.0) == 5.0);
    static_assert(bl::isfinite(1));
    static_assert(!bl::isnan(1));
    static_assert(bl::isless(1, 2.0f));

    TEST_CASE("math promoted overloads cover std-like arithmetic calls", "[fltx][math][overloads]")
    {
        int quotient = 0;
        int std_quotient = 0;

        REQUIRE(bl::sqrt(9) == std::sqrt(9));
        REQUIRE(bl::hypot(3, 4) == std::hypot(3, 4));
        REQUIRE(bl::atan2(1.0f, 2.0) == std::atan2(1.0f, 2.0));
        REQUIRE(bl::fma(1.0f, 2.0, 3.0) == std::fma(1.0f, 2.0, 3.0));
        REQUIRE(bl::remquo(5, 2, &quotient) == std::remquo(5.0, 2.0, &std_quotient));
        REQUIRE(quotient == std_quotient);

        double integral = 0.0;
        REQUIRE(bl::modf(3, &integral) == 0.0);
        REQUIRE(integral == 3.0);
    }
}
