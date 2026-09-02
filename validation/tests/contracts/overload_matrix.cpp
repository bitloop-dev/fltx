#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>

#include <fltx/math.h>

namespace
{
#define FLTX_EXPECT_UNARY(NAME, ARGUMENT, RESULT) \
    static_assert(std::same_as< \
        decltype(bl::NAME(std::declval<ARGUMENT>())), RESULT>)

#define FLTX_EXPECT_BINARY(NAME, LEFT, RIGHT, RESULT) \
    static_assert(std::same_as< \
        decltype(bl::NAME(std::declval<LEFT>(), std::declval<RIGHT>())), \
        RESULT>)

#define FLTX_EXPECT_TERNARY(NAME, FIRST, SECOND, THIRD, RESULT) \
    static_assert(std::same_as< \
        decltype(bl::NAME( \
            std::declval<FIRST>(), \
            std::declval<SECOND>(), \
            std::declval<THIRD>())), \
        RESULT>)

#define FLTX_NUMERIC_UNARY_MATRIX(NAME) \
    FLTX_EXPECT_UNARY(NAME, float, bl::f32); \
    FLTX_EXPECT_UNARY(NAME, double, bl::f64); \
    FLTX_EXPECT_UNARY(NAME, int, bl::f64); \
    FLTX_EXPECT_UNARY(NAME, bl::fdd_s, bl::fdd); \
    FLTX_EXPECT_UNARY(NAME, bl::fdd, bl::fdd); \
    FLTX_EXPECT_UNARY(NAME, bl::fqd_s, bl::fqd); \
    FLTX_EXPECT_UNARY(NAME, bl::fqd, bl::fqd)

#define FLTX_BOOLEAN_UNARY_MATRIX(NAME) \
    FLTX_EXPECT_UNARY(NAME, float, bool); \
    FLTX_EXPECT_UNARY(NAME, double, bool); \
    FLTX_EXPECT_UNARY(NAME, int, bool); \
    FLTX_EXPECT_UNARY(NAME, bl::fdd_s, bool); \
    FLTX_EXPECT_UNARY(NAME, bl::fdd, bool); \
    FLTX_EXPECT_UNARY(NAME, bl::fqd_s, bool); \
    FLTX_EXPECT_UNARY(NAME, bl::fqd, bool)

#define FLTX_INTEGER_UNARY_MATRIX(NAME, RESULT) \
    FLTX_EXPECT_UNARY(NAME, float, RESULT); \
    FLTX_EXPECT_UNARY(NAME, double, RESULT); \
    FLTX_EXPECT_UNARY(NAME, int, RESULT); \
    FLTX_EXPECT_UNARY(NAME, bl::fdd_s, RESULT); \
    FLTX_EXPECT_UNARY(NAME, bl::fdd, RESULT); \
    FLTX_EXPECT_UNARY(NAME, bl::fqd_s, RESULT); \
    FLTX_EXPECT_UNARY(NAME, bl::fqd, RESULT)

#define FLTX_NUMERIC_BINARY_MATRIX(NAME) \
    FLTX_EXPECT_BINARY(NAME, float, float, bl::f32); \
    FLTX_EXPECT_BINARY(NAME, double, double, bl::f64); \
    FLTX_EXPECT_BINARY(NAME, float, double, bl::f64); \
    FLTX_EXPECT_BINARY(NAME, int, float, bl::f64); \
    FLTX_EXPECT_BINARY(NAME, bl::fdd_s, float, bl::fdd); \
    FLTX_EXPECT_BINARY(NAME, double, bl::fdd, bl::fdd); \
    FLTX_EXPECT_BINARY(NAME, bl::fdd, bl::fdd_s, bl::fdd); \
    FLTX_EXPECT_BINARY(NAME, bl::fqd_s, float, bl::fqd); \
    FLTX_EXPECT_BINARY(NAME, bl::fdd, bl::fqd_s, bl::fqd); \
    FLTX_EXPECT_BINARY(NAME, bl::fqd, double, bl::fqd)

#define FLTX_BOOLEAN_BINARY_MATRIX(NAME) \
    FLTX_EXPECT_BINARY(NAME, float, float, bool); \
    FLTX_EXPECT_BINARY(NAME, double, double, bool); \
    FLTX_EXPECT_BINARY(NAME, float, double, bool); \
    FLTX_EXPECT_BINARY(NAME, int, float, bool); \
    FLTX_EXPECT_BINARY(NAME, bl::fdd_s, float, bool); \
    FLTX_EXPECT_BINARY(NAME, double, bl::fdd, bool); \
    FLTX_EXPECT_BINARY(NAME, bl::fdd, bl::fdd_s, bool); \
    FLTX_EXPECT_BINARY(NAME, bl::fqd_s, float, bool); \
    FLTX_EXPECT_BINARY(NAME, bl::fdd, bl::fqd_s, bool); \
    FLTX_EXPECT_BINARY(NAME, bl::fqd, double, bool)

    static_assert(bl::fltx_precision_rank_v<float> == 1);
    static_assert(bl::fltx_precision_rank_v<int> == 2);
    static_assert(bl::fltx_precision_rank_v<bool> == 2);
    static_assert(bl::fltx_precision_rank_v<double> == 2);
    static_assert(bl::fltx_precision_rank_v<long double> == 3);
    static_assert(bl::fltx_precision_rank_v<const bl::fdd_s&> == 4);
    static_assert(bl::fltx_precision_rank_v<volatile bl::fqd> == 5);

    static_assert(std::same_as<bl::common_float_type_t<float>, bl::f32>);
    static_assert(std::same_as<bl::common_float_type_t<int>, bl::f64>);
    static_assert(std::same_as<bl::common_float_type_t<bool>, bl::f64>);
    static_assert(std::same_as<bl::common_float_type_t<float, int>, bl::f64>);
    static_assert(std::same_as<bl::common_float_type_t<float, double>, bl::f64>);
    static_assert(std::same_as<bl::common_float_type_t<double, long double>, long double>);
    static_assert(std::same_as<
        bl::common_float_type_t<const bl::fdd_s&, std::uint64_t>, bl::fdd>);
    static_assert(std::same_as<
        bl::common_float_type_t<bl::fdd, bl::fqd_s>, bl::fqd>);

    static_assert(std::same_as<bl::detail::math::promoted_t<int>, bl::f64>);
    static_assert(std::same_as<bl::detail::math::promoted_t<bl::fdd>, bl::fdd_s>);
    static_assert(std::same_as<bl::detail::math::promoted_t<bl::fqd>, bl::fqd_s>);
    static_assert(!bl::detail::math::promoted_math_arg<long double>);

    FLTX_EXPECT_UNARY(abs, float, bl::f32);
    FLTX_EXPECT_UNARY(abs, double, bl::f64);
    FLTX_EXPECT_UNARY(abs, int, bl::f64);
    FLTX_EXPECT_UNARY(abs, bl::fdd_s, bl::fdd);
    FLTX_EXPECT_UNARY(abs, bl::fdd, bl::fdd);
    FLTX_EXPECT_UNARY(abs, bl::fqd_s, bl::fqd);
    FLTX_EXPECT_UNARY(abs, bl::fqd, bl::fqd);
    FLTX_NUMERIC_UNARY_MATRIX(fabs);
    FLTX_NUMERIC_UNARY_MATRIX(floor);
    FLTX_NUMERIC_UNARY_MATRIX(ceil);
    FLTX_NUMERIC_UNARY_MATRIX(trunc);
    FLTX_NUMERIC_UNARY_MATRIX(round);
    FLTX_NUMERIC_UNARY_MATRIX(roundeven);
    FLTX_NUMERIC_UNARY_MATRIX(logb);
    FLTX_NUMERIC_UNARY_MATRIX(exp);
    FLTX_NUMERIC_UNARY_MATRIX(exp2);
    FLTX_NUMERIC_UNARY_MATRIX(expm1);
    FLTX_NUMERIC_UNARY_MATRIX(log);
    FLTX_NUMERIC_UNARY_MATRIX(log2);
    FLTX_NUMERIC_UNARY_MATRIX(log10);
    FLTX_NUMERIC_UNARY_MATRIX(log1p);
    FLTX_NUMERIC_UNARY_MATRIX(sqrt);
    FLTX_NUMERIC_UNARY_MATRIX(cbrt);
    FLTX_NUMERIC_UNARY_MATRIX(sin);
    FLTX_NUMERIC_UNARY_MATRIX(cos);
    FLTX_NUMERIC_UNARY_MATRIX(tan);
    FLTX_NUMERIC_UNARY_MATRIX(atan);
    FLTX_NUMERIC_UNARY_MATRIX(asin);
    FLTX_NUMERIC_UNARY_MATRIX(acos);
    FLTX_NUMERIC_UNARY_MATRIX(sinh);
    FLTX_NUMERIC_UNARY_MATRIX(cosh);
    FLTX_NUMERIC_UNARY_MATRIX(tanh);
    FLTX_NUMERIC_UNARY_MATRIX(asinh);
    FLTX_NUMERIC_UNARY_MATRIX(acosh);
    FLTX_NUMERIC_UNARY_MATRIX(atanh);
    FLTX_NUMERIC_UNARY_MATRIX(erf);
    FLTX_NUMERIC_UNARY_MATRIX(erfc);
    FLTX_NUMERIC_UNARY_MATRIX(lgamma);
    FLTX_NUMERIC_UNARY_MATRIX(tgamma);

    FLTX_BOOLEAN_UNARY_MATRIX(signbit);
    FLTX_BOOLEAN_UNARY_MATRIX(isnan);
    FLTX_BOOLEAN_UNARY_MATRIX(isinf);
    FLTX_BOOLEAN_UNARY_MATRIX(isfinite);
    FLTX_BOOLEAN_UNARY_MATRIX(iszero);
    FLTX_BOOLEAN_UNARY_MATRIX(isnormal);
    FLTX_EXPECT_UNARY(ispositive, bl::fdd_s, bool);
    FLTX_EXPECT_UNARY(ispositive, bl::fdd, bool);
    FLTX_EXPECT_UNARY(ispositive, bl::fqd_s, bool);
    FLTX_EXPECT_UNARY(ispositive, bl::fqd, bool);
    FLTX_INTEGER_UNARY_MATRIX(fpclassify, int);
    FLTX_INTEGER_UNARY_MATRIX(ilogb, int);
    FLTX_INTEGER_UNARY_MATRIX(lround, long);
    FLTX_INTEGER_UNARY_MATRIX(llround, long long);

    FLTX_NUMERIC_BINARY_MATRIX(fmod);
    FLTX_NUMERIC_BINARY_MATRIX(remainder);
    FLTX_NUMERIC_BINARY_MATRIX(fmin);
    FLTX_NUMERIC_BINARY_MATRIX(fmax);
    FLTX_NUMERIC_BINARY_MATRIX(fdim);
    FLTX_NUMERIC_BINARY_MATRIX(copysign);
    FLTX_NUMERIC_BINARY_MATRIX(hypot);
    FLTX_NUMERIC_BINARY_MATRIX(atan2);
    FLTX_NUMERIC_BINARY_MATRIX(nextafter);

    FLTX_BOOLEAN_BINARY_MATRIX(isunordered);
    FLTX_BOOLEAN_BINARY_MATRIX(isgreater);
    FLTX_BOOLEAN_BINARY_MATRIX(isgreaterequal);
    FLTX_BOOLEAN_BINARY_MATRIX(isless);
    FLTX_BOOLEAN_BINARY_MATRIX(islessequal);
    FLTX_BOOLEAN_BINARY_MATRIX(islessgreater);

    FLTX_EXPECT_TERNARY(fma, float, float, float, bl::f32);
    FLTX_EXPECT_TERNARY(fma, float, double, int, bl::f64);
    FLTX_EXPECT_TERNARY(fma, bl::fdd_s, float, int, bl::fdd);
    FLTX_EXPECT_TERNARY(fma, bl::fdd, bl::fqd_s, double, bl::fqd);
    FLTX_EXPECT_TERNARY(fma, bl::fqd, double, float, bl::fqd);

    FLTX_EXPECT_TERNARY(clamp, float, float, float, bl::f32);
    FLTX_EXPECT_TERNARY(clamp, double, double, double, bl::f64);
    FLTX_EXPECT_TERNARY(clamp, bl::fdd_s, bl::fdd_s, bl::fdd_s, bl::fdd);
    FLTX_EXPECT_TERNARY(clamp, bl::fdd, bl::fdd, bl::fdd, bl::fdd);
    FLTX_EXPECT_TERNARY(clamp, bl::fqd_s, bl::fqd_s, bl::fqd_s, bl::fqd);
    FLTX_EXPECT_TERNARY(clamp, bl::fqd, bl::fqd, bl::fqd, bl::fqd);

    static_assert(std::same_as<decltype(bl::remquo(1.0f, 2.0f, nullptr)), bl::f32>);
    static_assert(std::same_as<decltype(bl::remquo(1, 2.0f, nullptr)), bl::f64>);
    static_assert(std::same_as<
        decltype(bl::remquo(bl::fdd{}, 2.0, nullptr)), bl::fdd>);
    static_assert(std::same_as<
        decltype(bl::remquo(bl::fdd{}, bl::fqd{}, nullptr)), bl::fqd>);

#define FLTX_SCALING_MATRIX(NAME, EXPONENT) \
    static_assert(std::same_as<decltype(bl::NAME(1.0f, EXPONENT)), bl::f32>); \
    static_assert(std::same_as<decltype(bl::NAME(1.0, EXPONENT)), bl::f64>); \
    static_assert(std::same_as<decltype(bl::NAME(1, EXPONENT)), bl::f64>); \
    static_assert(std::same_as<decltype(bl::NAME(bl::fdd{}, EXPONENT)), bl::fdd>); \
    static_assert(std::same_as<decltype(bl::NAME(bl::fqd{}, EXPONENT)), bl::fqd>)

    FLTX_SCALING_MATRIX(ldexp, 2);
    FLTX_SCALING_MATRIX(scalbn, 2);
    FLTX_SCALING_MATRIX(scalbln, 2L);

    static_assert(std::same_as<decltype(bl::frexp(1.0f, nullptr)), bl::f32>);
    static_assert(std::same_as<decltype(bl::frexp(1.0, nullptr)), bl::f64>);
    static_assert(std::same_as<decltype(bl::frexp(1, nullptr)), bl::f64>);
    static_assert(std::same_as<decltype(bl::frexp(bl::fdd{}, nullptr)), bl::fdd>);
    static_assert(std::same_as<decltype(bl::frexp(bl::fqd{}, nullptr)), bl::fqd>);

    static_assert(std::same_as<decltype(bl::modf(1.0f, static_cast<float*>(nullptr))), bl::f32>);
    static_assert(std::same_as<decltype(bl::modf(1.0, static_cast<double*>(nullptr))), bl::f64>);
    static_assert(std::same_as<decltype(bl::modf(1, static_cast<double*>(nullptr))), bl::f64>);
    static_assert(std::same_as<
        decltype(bl::modf(bl::fdd{}, static_cast<bl::fdd_s*>(nullptr))), bl::fdd>);
    static_assert(std::same_as<
        decltype(bl::modf(bl::fqd{}, static_cast<bl::fqd_s*>(nullptr))), bl::fqd>);

    static_assert(std::same_as<decltype(bl::nexttoward(1.0f, 2.0L)), bl::f32>);
    static_assert(std::same_as<decltype(bl::nexttoward(1.0, 2.0f)), bl::f64>);
    static_assert(std::same_as<decltype(bl::nexttoward(bl::fdd{}, 2.0L)), bl::fdd>);
    static_assert(std::same_as<
        decltype(bl::nexttoward(bl::fdd{}, bl::fqd{})), bl::fqd>);
    static_assert(std::same_as<
        decltype(bl::nexttoward(bl::fqd{}, bl::fdd{})), bl::fqd>);

    static_assert(std::same_as<decltype(bl::pow(2.0f, 3.0f)), bl::f32>);
    static_assert(std::same_as<decltype(bl::pow(2.0f, 3)), bl::f64>);
    static_assert(std::same_as<decltype(bl::pow(2.0, 3)), bl::f64>);
    static_assert(std::same_as<decltype(bl::pow(2, 3)), bl::f64>);
    static_assert(std::same_as<decltype(bl::pow(bl::fdd{}, 3)), bl::fdd>);
    static_assert(std::same_as<decltype(bl::pow(bl::fdd{}, 3.0)), bl::fdd>);
    static_assert(std::same_as<
        decltype(bl::pow(bl::fdd{}, bl::fqd{})), bl::fqd>);
    static_assert(std::same_as<
        decltype(bl::pow(bl::fqd{}, bl::fdd{})), bl::fqd>);
    static_assert(std::same_as<decltype(bl::pow(bl::fqd{}, 3)), bl::fqd>);

    static_assert(std::same_as<decltype(bl::ipow(2, 3)), int>);
    static_assert(std::same_as<decltype(bl::ipow(2.0f, 3)), bl::f32>);
    static_assert(std::same_as<decltype(bl::ipow(2.0, 3)), bl::f64>);
    static_assert(std::same_as<decltype(bl::ipow(bl::fdd{}, 3)), bl::fdd>);
    static_assert(std::same_as<decltype(bl::ipow(bl::fqd{}, 3)), bl::fqd>);

    FLTX_EXPECT_UNARY(sqr, float, bl::f32);
    FLTX_EXPECT_UNARY(sqr, double, bl::f64);
    FLTX_EXPECT_UNARY(sqr, bl::fdd, bl::fdd);
    FLTX_EXPECT_UNARY(sqr, bl::fqd, bl::fqd);
    FLTX_EXPECT_UNARY(recip, float, bl::f32);
    FLTX_EXPECT_UNARY(recip, double, bl::f64);
    FLTX_EXPECT_UNARY(recip, bl::fdd_s, bl::fdd);
    FLTX_EXPECT_UNARY(recip, bl::fqd_s, bl::fqd);

#define FLTX_DECIMAL_ROUNDING_MATRIX(NAME) \
    static_assert(std::same_as<decltype(bl::NAME(1.25f, 2)), bl::f32>); \
    static_assert(std::same_as<decltype(bl::NAME(1.25, 2)), bl::f64>); \
    static_assert(std::same_as<decltype(bl::NAME(bl::fdd{}, 2)), bl::fdd>); \
    static_assert(std::same_as<decltype(bl::NAME(bl::fqd{}, 2)), bl::fqd>)

    FLTX_DECIMAL_ROUNDING_MATRIX(round_decimals);
    FLTX_DECIMAL_ROUNDING_MATRIX(round_significant);

    static_assert(std::same_as<
        decltype(bl::sincos(std::declval<float>(),
            std::declval<float&>(), std::declval<float&>())), bool>);
    static_assert(std::same_as<
        decltype(bl::sincos(std::declval<double>(),
            std::declval<double&>(), std::declval<double&>())), bool>);
    static_assert(std::same_as<
        decltype(bl::sincos(std::declval<bl::fdd>(),
            std::declval<bl::fdd_s&>(), std::declval<bl::fdd_s&>())), bool>);
    static_assert(std::same_as<
        decltype(bl::sincos(std::declval<bl::fqd>(),
            std::declval<bl::fqd_s&>(), std::declval<bl::fqd_s&>())), bool>);

    template<class Base, class Exponent>
    concept can_pow = requires (Base base, Exponent exponent)
    {
        bl::pow(base, exponent);
    };

    template<class Value>
    concept can_sin = requires (Value value)
    {
        bl::sin(value);
    };

    static_assert(!can_pow<float, long double>);
    static_assert(!can_pow<bl::fdd, long double>);
    static_assert(!can_pow<bl::fqd, long double>);
    static_assert(!can_sin<long double>);

    static_assert(noexcept(bl::isfinite(std::declval<const bl::fdd_s&>())));
    static_assert(noexcept(bl::ispositive(std::declval<const bl::fqd_s&>())));
    static_assert(noexcept(bl::isless(
        std::declval<const bl::fdd_s&>(),
        std::declval<const bl::fdd_s&>())));
    static_assert(noexcept(bl::nextafter(
        std::declval<const bl::fqd_s&>(),
        std::declval<const bl::fqd_s&>())));

#undef FLTX_DECIMAL_ROUNDING_MATRIX
#undef FLTX_SCALING_MATRIX
#undef FLTX_BOOLEAN_BINARY_MATRIX
#undef FLTX_NUMERIC_BINARY_MATRIX
#undef FLTX_INTEGER_UNARY_MATRIX
#undef FLTX_BOOLEAN_UNARY_MATRIX
#undef FLTX_NUMERIC_UNARY_MATRIX
#undef FLTX_EXPECT_TERNARY
#undef FLTX_EXPECT_BINARY
#undef FLTX_EXPECT_UNARY
}
