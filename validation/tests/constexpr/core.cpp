#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstddef>
#include <ios>
#include <limits>
#include <string_view>
#include <type_traits>

#include <fltx.h>

namespace
{
    constexpr bl::fqd add_expression_pair(const auto& x, const auto& y)
    {
        return x + y;
    }

    constexpr bool expression_transparency_is_constant_evaluated()
    {
        using namespace bl::literals;
        constexpr bl::fqd a = 1_qd / 3_qd;
        constexpr bl::fqd b = 2_qd / 3_qd;
        constexpr bl::fqd c = a + b;
        constexpr bl::fqd d = bl::atan2(a, b);
        constexpr bl::fqd e = 5_qd;
        auto left = a * b;
        const auto right = c * d + e;
        const auto combined = left + right;
        const bl::fqd expected = a * b + (c * d + e);
        bl::fqd assigned{};
        assigned = combined;
        left = bl::fqd{ 2.0 } * bl::fqd{ 3.0 };

        const auto three = bl::fqd{ 1.5 } * bl::fqd{ 2.0 };
        const auto four = bl::fqd{ 1.5 } * bl::fqd{ 2.0 } + 1.0;
        using std::sqrt;
        using std::hypot;
        using std::abs;
        return add_expression_pair(a * b, c * d + e) == expected &&
               bl::fqd{ combined } == expected && assigned == expected &&
               sqrt(four) == bl::fqd{ 2.0 } &&
               hypot(three, std::size_t{ 4 }) == bl::fqd{ 5.0 } &&
               abs(three) == bl::fqd{ 3.0 } &&
               bl::pow(three, 2) == bl::fqd{ 9.0 } &&
               bl::fma(three, 2, four) == bl::fqd{ 10.0 };
    }

    template<class T>
    constexpr bool within_roundoff(const T& a, const T& b)
    {
        const T delta = a - b;
        const T difference = bl::abs(delta);
        const T scale = bl::fmax(bl::abs(a), T{ 1.0 });
        return difference <= scale * std::numeric_limits<T>::epsilon() * 64.0;
    }

    template<class T>
    constexpr bool within_bits(const T& actual, const T& expected, int bits)
    {
        const T scale = bl::fmax(bl::abs(expected), T{ 1.0 });
        return bl::abs(actual - expected) <= bl::ldexp(scale, -bits);
    }

    constexpr bool expression_conversions_are_constant_evaluated()
    {
        const bl::fqd numerator{ 3.5, 0x1p-60, -0x1p-120, 0x1p-180 };
        const bl::fqd denominator{ 1.25, -0x1p-61, 0x1p-121, -0x1p-181 };
        const auto expression = numerator / denominator + 0.25;
        const bl::fqd expected = expression;

        return static_cast<bl::fdd_s>(expression) == static_cast<bl::fdd_s>(expected) &&
               static_cast<bl::fdd>(expression) == static_cast<bl::fdd>(expected) &&
               static_cast<double>(expression) == static_cast<double>(expected) &&
               static_cast<float>(expression) == static_cast<float>(expected) &&
               static_cast<long double>(expression) == static_cast<long double>(expected) &&
               static_cast<std::size_t>(expression) == static_cast<std::size_t>(expected) &&
               static_cast<unsigned char>(expression) == static_cast<unsigned char>(expected) &&
               static_cast<bool>(expression) == static_cast<bool>(expected) &&
               static_cast<int>(expression) == static_cast<int>(expected);
    }

    constexpr bool native_conversions_preserve_expansion_value()
    {
        constexpr std::int64_t exact = (std::int64_t{ 1 } << 60) + 3;
        constexpr long double extended = 1.0L + 0x1p-60L;
        constexpr bl::fdd below_one{ 1.0, -0x1p-60 };
        constexpr bl::fqd above_negative_one{ -1.0, 0x1p-60, 0.0, 0.0 };
        constexpr bl::fdd negative_zero{ -0.0 };

        return static_cast<int>(below_one) == 0 &&
               static_cast<int>(above_negative_one) == 0 &&
               static_cast<std::int64_t>(bl::fdd{ exact }) == exact &&
               static_cast<std::int64_t>(bl::fqd{ exact }) == exact &&
               static_cast<std::uint64_t>(bl::fdd{ std::numeric_limits<std::uint64_t>::max() }) ==
                   std::numeric_limits<std::uint64_t>::max() &&
               static_cast<long double>(bl::fdd{ extended }) == extended &&
               static_cast<long double>(bl::fqd{ extended }) == extended &&
               !static_cast<bool>(negative_zero) &&
               bl::signbit(static_cast<double>(negative_zero));
    }

    constexpr bl::fdd dd_input{ 1.25 };
    constexpr bl::fqd qd_input{ 1.25 };

    constexpr auto dd_sqrt = bl::sqrt(bl::fdd{ 4.0 });
    constexpr auto dd_exp_log = bl::exp(bl::log(dd_input));
    constexpr auto dd_sincos = bl::sincos<bl::fdd>(dd_input);
    constexpr auto qd_sqrt = bl::sqrt(bl::fqd{ 4.0 });
    constexpr auto qd_exp_log = bl::exp(bl::log(qd_input));
    constexpr auto qd_sincos = bl::sincos<bl::fqd>(qd_input);

    // Independently precomputed with a 256-bit MPFR oracle, then split into
    // correctly rounded binary64 limbs. These values prevent an algebraic
    // identity from allowing matching constexpr errors to pass unnoticed.
    constexpr bl::fdd dd_exp_expected{
        0x1.747a513dbef6ap+0,
        0x1.88d1e2d966c25p-54
    };
    constexpr bl::fqd qd_exp_expected{
        0x1.747a513dbef6ap+0,
        0x1.88d1e2d966c25p-54,
        -0x1.bfa3a8705bde1p-108,
        0x1.a5c6dbedc36bbp-163
    };
    constexpr bl::fdd dd_log_expected{
        0x1.4618bc21c5ec2p-2,
        0x1.f42decdeccf1dp-56
    };
    constexpr bl::fqd qd_log_expected{
        0x1.4618bc21c5ec2p-2,
        0x1.f42decdeccf1dp-56,
        -0x1.77d446996da00p-111,
        0x1.68872796bdd6bp-165
    };
    constexpr bl::fdd dd_sin_expected{
        0x1.e5e14fe11418cp-1,
        0x1.f26492c1c25a0p-57
    };
    constexpr bl::fqd qd_sin_expected{
        0x1.e5e14fe11418cp-1,
        0x1.f26492c1c25a0p-57,
        0x1.3931bcfd30bc4p-113,
        0x1.3379a6078b05fp-168
    };
    constexpr bl::fdd dd_cos_expected{
        0x1.42e3dd88bd952p-2,
        -0x1.353a9f74bf255p-57
    };
    constexpr bl::fqd qd_cos_expected{
        0x1.42e3dd88bd952p-2,
        -0x1.353a9f74bf255p-57,
        0x1.f11df0328f9f2p-114,
        -0x1.a3c409c7e5592p-168
    };

    template<class T>
    consteval bool core_is_constant_evaluated()
    {
        T value{ 1.25 };
        value += T{ 0.75 };
        value *= T{ 3.0 };
        value -= T{ 1.0 };
        value /= T{ 2.0 };

        return value == T{ 2.5 } &&
               -value < T{ 0.0 } &&
               bl::clamp(value, T{ 0.0 }, T{ 2.0 }) == T{ 2.0 } &&
               bl::isfinite(value) &&
               bl::isnormal(value) &&
               !bl::isnan(value) &&
               !bl::signbit(value) &&
               bl::fpclassify(value) == FP_NORMAL &&
               bl::recip(T{ 4.0 }) == T{ 0.25 };
    }

    template<class T>
    consteval bool native_minmax_special_values_are_constant_evaluated()
    {
        constexpr T zero{ 0.0 };
        constexpr T negative_zero{ -0.0 };
        constexpr T one{ 1.0 };
        constexpr T nan = std::numeric_limits<T>::quiet_NaN();

        const T minimum_zero_ab = bl::fmin(zero, negative_zero);
        const T minimum_zero_ba = bl::fmin(negative_zero, zero);
        const T maximum_zero_ab = bl::fmax(zero, negative_zero);
        const T maximum_zero_ba = bl::fmax(negative_zero, zero);

        return bl::fmin(nan, one) == one &&
               bl::fmin(one, nan) == one &&
               bl::fmax(nan, one) == one &&
               bl::fmax(one, nan) == one &&
               bl::isnan(bl::fmin(nan, nan)) &&
               bl::isnan(bl::fmax(nan, nan)) &&
               bl::iszero(minimum_zero_ab) && bl::signbit(minimum_zero_ab) &&
               bl::iszero(minimum_zero_ba) && bl::signbit(minimum_zero_ba) &&
               bl::iszero(maximum_zero_ab) && !bl::signbit(maximum_zero_ab) &&
               bl::iszero(maximum_zero_ba) && !bl::signbit(maximum_zero_ba);
    }

    template<class T>
    consteval bool native_log_pow_atan2_special_values_are_constant_evaluated()
    {
        constexpr T one{ 1.0 };
        constexpr T infinity = std::numeric_limits<T>::infinity();
        constexpr T negative_infinity = -infinity;
        constexpr T nan = std::numeric_limits<T>::quiet_NaN();

        const T log1p_positive_infinity = bl::log1p(infinity);
        const T atan2_positive_infinity = bl::atan2(infinity, one);
        const T atan2_negative_infinity = bl::atan2(negative_infinity, one);
        const T atan2_finite_positive_infinity = bl::atan2(one, infinity);
        const T atan2_finite_negative_infinity = bl::atan2(one, negative_infinity);
        const T atan2_both_positive_infinity = bl::atan2(infinity, infinity);
        const T atan2_mixed_infinity = bl::atan2(infinity, negative_infinity);

        return bl::isinf(log1p_positive_infinity) &&
               !bl::signbit(log1p_positive_infinity) &&
               bl::isnan(bl::log1p(negative_infinity)) &&
               bl::isnan(bl::log1p(nan)) &&
               bl::isinf(bl::pow(infinity, one)) &&
               !bl::signbit(bl::pow(infinity, one)) &&
               bl::isinf(bl::pow(negative_infinity, one)) &&
               bl::signbit(bl::pow(negative_infinity, one)) &&
               bl::pow(one, infinity) == one &&
               bl::pow(one, negative_infinity) == one &&
               bl::isinf(bl::pow(infinity, infinity)) &&
               bl::iszero(bl::pow(infinity, negative_infinity)) &&
               bl::isnan(bl::pow(nan, one)) &&
               bl::pow(one, nan) == one &&
               bl::isnan(bl::pow(nan, nan)) &&
               atan2_positive_infinity > T{ 1.5 } &&
               atan2_positive_infinity < T{ 1.6 } &&
               atan2_negative_infinity < T{ -1.5 } &&
               atan2_negative_infinity > T{ -1.6 } &&
               bl::iszero(atan2_finite_positive_infinity) &&
               !bl::signbit(atan2_finite_positive_infinity) &&
               atan2_finite_negative_infinity > T{ 3.1 } &&
               atan2_finite_negative_infinity < T{ 3.2 } &&
               atan2_both_positive_infinity > T{ 0.7 } &&
               atan2_both_positive_infinity < T{ 0.8 } &&
               atan2_mixed_infinity > T{ 2.3 } &&
               atan2_mixed_infinity < T{ 2.4 } &&
               bl::isnan(bl::atan2(nan, one)) &&
               bl::isnan(bl::atan2(one, nan)) &&
               bl::isnan(bl::atan2(nan, nan));
    }

    template<class T>
    consteval bool arithmetic_special_values_are_constant_evaluated()
    {
        constexpr T zero{ 0.0 };
        constexpr T negative_zero{ -0.0 };
        constexpr T one{ 1.0 };
        constexpr T infinity = std::numeric_limits<T>::infinity();

        const T positive_infinity = one / zero;
        const T negative_infinity = one / negative_zero;
        const T negative_sum = negative_zero + negative_zero;
        const T negative_difference = negative_zero - zero;
        const T positive_cancellation = negative_zero - negative_zero;
        const T negative_scalar_sum = negative_zero + -0.0;
        const T negative_scalar_difference = -0.0 - zero;
        const T negative_fma = bl::fma(negative_zero, one, negative_zero);

        return bl::isinf(positive_infinity) && !bl::signbit(positive_infinity) &&
               bl::isinf(negative_infinity) && bl::signbit(negative_infinity) &&
               bl::isnan(zero / zero) &&
               bl::isnan(infinity - infinity) &&
               bl::isnan(infinity * zero) &&
               bl::iszero(negative_sum) && bl::signbit(negative_sum) &&
               bl::iszero(negative_difference) && bl::signbit(negative_difference) &&
               bl::iszero(positive_cancellation) && !bl::signbit(positive_cancellation) &&
               bl::iszero(negative_scalar_sum) && bl::signbit(negative_scalar_sum) &&
               bl::iszero(negative_scalar_difference) && bl::signbit(negative_scalar_difference) &&
               bl::iszero(negative_fma) && bl::signbit(negative_fma);
    }

    template<class T>
    consteval bool native_fma_signed_zero_is_constant_evaluated()
    {
        constexpr T zero{ 0.0 };
        constexpr T negative_zero{ -0.0 };
        constexpr T one{ 1.0 };

        const T positive = bl::fma(zero, one, zero);
        const T negative = bl::fma(negative_zero, one, negative_zero);
        return bl::iszero(positive) && !bl::signbit(positive) &&
               bl::iszero(negative) && bl::signbit(negative);
    }

    template<class T>
    consteval bool exact_math_is_constant_evaluated()
    {
        T integral{};
        int exponent = 0;
        int quotient = 0;
        T sine{};
        T cosine{};

        const T fraction = bl::modf(T{ 3.25 }, &integral);
        const T mantissa = bl::frexp(T{ 8.0 }, &exponent);
        const T remainder = bl::remquo(T{ 5.0 }, T{ 2.0 }, &quotient);
        const bool sincos_ok = bl::sincos(T{ 0.0 }, sine, cosine);

        return bl::floor(T{ -2.25 }) == T{ -3.0 } &&
               bl::ceil(T{ -2.25 }) == T{ -2.0 } &&
               bl::trunc(T{ -2.75 }) == T{ -2.0 } &&
               bl::round(T{ -2.5 }) == T{ -3.0 } &&
               bl::roundeven(T{ 2.5 }) == T{ 2.0 } &&
               bl::lround(T{ -2.5 }) == -3L &&
               bl::llround(T{ 2.5 }) == 3LL &&
               bl::round_decimals(T{ 1.375 }, 2) > T{ 1.379 } &&
               bl::round_decimals(T{ 1.375 }, 2) < T{ 1.381 } &&
               bl::round_significant(T{ 12345.0 }, 3) > T{ 12299.0 } &&
               bl::round_significant(T{ 12345.0 }, 3) < T{ 12301.0 } &&
               fraction == T{ 0.25 } && integral == T{ 3.0 } &&
               mantissa == T{ 0.5 } && exponent == 4 &&
               bl::ldexp(T{ 0.5 }, 4) == T{ 8.0 } &&
               bl::scalbn(T{ 0.5 }, 4) == T{ 8.0 } &&
               bl::scalbln(T{ 0.5 }, 4L) == T{ 8.0 } &&
               bl::ilogb(T{ 8.0 }) == 3 &&
               bl::logb(T{ 8.0 }) == T{ 3.0 } &&
               remainder == T{ 1.0 } && quotient != 0 &&
               bl::nexttoward(T{ 1.0 }, T{ 2.0 }) > T{ 1.0 } &&
               bl::ipow(T{ 2.0 }, 10) == T{ 1024.0 } &&
               sincos_ok && sine == T{ 0.0 } && cosine == T{ 1.0 };
    }

    template<class T>
    consteval bool io_is_constant_evaluated()
    {
        const auto parsed = bl::try_parse<T>("12.5");
        const auto rejected = bl::try_parse<T>("12.5tail");
        const T fallback = bl::parse<T>("bad", T{ 7.0 });
        const auto text = bl::to_static_string(T{ 12.5 }, 1, std::ios_base::fixed);

        return parsed && parsed.consumed == 4 && parsed.value == T{ 12.5 } &&
               !rejected && rejected.consumed == 4 &&
               fallback == T{ 7.0 } && text.view() == std::string_view{ "12.5" };
    }

    consteval bool maximum_fqd_io_is_constant_evaluated()
    {
        constexpr bl::fqd maximum = std::numeric_limits<bl::fqd>::max();
        constexpr bl::fqd lowest = std::numeric_limits<bl::fqd>::lowest();
        constexpr auto maximum_text = bl::to_static_string(maximum);
        constexpr auto lowest_text = bl::to_static_string(lowest);
        return bl::parse<bl::fqd>(maximum_text) == maximum &&
               bl::parse<bl::fqd>(lowest_text) == lowest;
    }

    consteval bool sparse_fdd_io_is_constant_evaluated()
    {
        constexpr bl::fdd value{ 1.0, 0x1p-55 };
        constexpr auto text = bl::to_static_string(value);
        return bl::parse<bl::fdd>(text) == value;
    }

    template<class T>
    consteval bool nominal_navigation_is_constant_evaluated()
    {
        constexpr T one{ 1.0 };
        constexpr T zero{ 0.0 };
        constexpr T up = bl::nextafter(one, T{ 2.0 });
        constexpr T down = bl::nextafter(one, zero);

        if constexpr (std::is_same_v<T, bl::fdd>)
        {
            return up == T{ 1.0, 0x1p-105 } &&
                   down == T{ 1.0, -0x1p-106 } &&
                   up - one == std::numeric_limits<T>::epsilon();
        }
        else
        {
            return up == T{ 1.0, 0x1p-211, 0.0, 0.0 } &&
                   down == T{ 1.0, -0x1p-212, 0.0, 0.0 } &&
                   up - one == std::numeric_limits<T>::epsilon();
        }
    }

    template<class T>
    consteval bool random_is_constant_evaluated()
    {
        const auto first = bl::uniform_real_array<4>(
            T{ -2.0 },
            T{ 3.0 },
            bl::mt19937_64{ 0x1020304050607080ull });
        const auto second = bl::random_array<4>(
            bl::mt19937_64{ 0x1020304050607080ull },
            bl::uniform_real_distribution<T>{ T{ -2.0 }, T{ 3.0 } });
        bl::mt19937 canonical_engine{ 1234u };
        const T canonical =
            bl::generate_canonical<T, std::numeric_limits<T>::digits>(canonical_engine);

        bl::mt19937_64 distribution_engine{ 777u };
        bl::exponential_distribution<T> exponential{ T{ 0.75 } };
        bl::normal_distribution<T> normal{ T{ 1.0 }, T{ 0.5 } };
        bl::lognormal_distribution<T> lognormal{ T{ 0.0 }, T{ 0.5 } };
        const T exponential_sample = exponential(distribution_engine);
        const T normal_sample = normal(distribution_engine);
        const T lognormal_sample = lognormal(distribution_engine);

        return first == second &&
               first.front() >= T{ -2.0 } && first.front() < T{ 3.0 } &&
               first.back() >= T{ -2.0 } && first.back() < T{ 3.0 } &&
               canonical >= T{ 0.0 } && canonical < T{ 1.0 } &&
               exponential_sample >= T{ 0.0 } &&
               bl::isfinite(normal_sample) &&
               lognormal_sample > T{ 0.0 };
    }

    static_assert(dd_sqrt == bl::fdd{ 2.0 });
    static_assert(qd_sqrt == bl::fqd{ 2.0 });
    static_assert(expression_conversions_are_constant_evaluated());
    static_assert(native_conversions_preserve_expansion_value());
    static_assert(dd_exp_log > bl::fdd{ 1.249 });
    static_assert(dd_exp_log < bl::fdd{ 1.251 });
    static_assert(qd_exp_log > bl::fqd{ 1.249 });
    static_assert(qd_exp_log < bl::fqd{ 1.251 });
    static_assert(dd_sincos.s * dd_sincos.s + dd_sincos.c * dd_sincos.c >
                  bl::fdd{ 0.999 });
    static_assert(qd_sincos.s * qd_sincos.s + qd_sincos.c * qd_sincos.c >
                  bl::fqd{ 0.999 });
    static_assert(within_bits(bl::exp(bl::fdd{ 0.375 }), dd_exp_expected, 78));
    static_assert(within_bits(bl::exp(bl::fqd{ 0.375 }), qd_exp_expected, 178));
    static_assert(within_bits(bl::log(bl::fdd{ 1.375 }), dd_log_expected, 78));
    static_assert(within_bits(bl::log(bl::fqd{ 1.375 }), qd_log_expected, 178));
    static_assert(within_bits(dd_sincos.s, dd_sin_expected, 78));
    static_assert(within_bits(dd_sincos.c, dd_cos_expected, 78));
    static_assert(within_bits(qd_sincos.s, qd_sin_expected, 178));
    static_assert(within_bits(qd_sincos.c, qd_cos_expected, 178));
    static_assert(core_is_constant_evaluated<bl::fdd>());
    static_assert(core_is_constant_evaluated<bl::fqd>());
    static_assert(native_minmax_special_values_are_constant_evaluated<bl::f32>());
    static_assert(native_minmax_special_values_are_constant_evaluated<bl::f64>());
    static_assert(native_log_pow_atan2_special_values_are_constant_evaluated<bl::f32>());
    static_assert(native_log_pow_atan2_special_values_are_constant_evaluated<bl::f64>());
    static_assert(native_fma_signed_zero_is_constant_evaluated<bl::f32>());
    static_assert(native_fma_signed_zero_is_constant_evaluated<bl::f64>());
    static_assert(arithmetic_special_values_are_constant_evaluated<bl::fdd>());
    static_assert(arithmetic_special_values_are_constant_evaluated<bl::fqd>());
    static_assert(exact_math_is_constant_evaluated<bl::fdd>());
    static_assert(exact_math_is_constant_evaluated<bl::fqd>());
    static_assert(nominal_navigation_is_constant_evaluated<bl::fdd>());
    static_assert(nominal_navigation_is_constant_evaluated<bl::fqd>());
    static_assert(io_is_constant_evaluated<bl::f32>());
    static_assert(io_is_constant_evaluated<bl::f64>());
    static_assert(io_is_constant_evaluated<bl::fdd>());
    static_assert(io_is_constant_evaluated<bl::fqd>());
    static_assert(maximum_fqd_io_is_constant_evaluated());
    static_assert(sparse_fdd_io_is_constant_evaluated());
    static_assert(random_is_constant_evaluated<bl::fdd>());
    static_assert(random_is_constant_evaluated<bl::fqd>());
}

TEST_CASE("genuine constant evaluation covers representative heavy paths", "[constexpr]")
{
    CHECK(dd_sqrt == bl::sqrt(bl::fdd{ 4.0 }));
    CHECK(dd_exp_log == bl::exp(bl::log(dd_input)));
    CHECK(within_roundoff(dd_sincos.s, bl::sin(dd_input)));
    CHECK(within_roundoff(dd_sincos.c, bl::cos(dd_input)));

    CHECK(qd_sqrt == bl::sqrt(bl::fqd{ 4.0 }));
    STATIC_CHECK(expression_conversions_are_constant_evaluated());
    STATIC_CHECK(expression_transparency_is_constant_evaluated());
    STATIC_CHECK(native_conversions_preserve_expansion_value());
    CHECK(qd_exp_log == bl::exp(bl::log(qd_input)));
    CHECK(within_roundoff(qd_sincos.s, bl::sin(qd_input)));
    CHECK(within_roundoff(qd_sincos.c, bl::cos(qd_input)));
    STATIC_CHECK(within_bits(bl::exp(bl::fdd{ 0.375 }), dd_exp_expected, 78));
    STATIC_CHECK(within_bits(bl::exp(bl::fqd{ 0.375 }), qd_exp_expected, 178));
    STATIC_CHECK(within_bits(bl::log(bl::fdd{ 1.375 }), dd_log_expected, 78));
    STATIC_CHECK(within_bits(bl::log(bl::fqd{ 1.375 }), qd_log_expected, 178));
    STATIC_CHECK(within_bits(dd_sincos.s, dd_sin_expected, 78));
    STATIC_CHECK(within_bits(dd_sincos.c, dd_cos_expected, 78));
    STATIC_CHECK(within_bits(qd_sincos.s, qd_sin_expected, 178));
    STATIC_CHECK(within_bits(qd_sincos.c, qd_cos_expected, 178));

    STATIC_CHECK(core_is_constant_evaluated<bl::fdd>());
    STATIC_CHECK(core_is_constant_evaluated<bl::fqd>());
    STATIC_CHECK(native_minmax_special_values_are_constant_evaluated<bl::f32>());
    STATIC_CHECK(native_minmax_special_values_are_constant_evaluated<bl::f64>());
    STATIC_CHECK(native_log_pow_atan2_special_values_are_constant_evaluated<bl::f32>());
    STATIC_CHECK(native_log_pow_atan2_special_values_are_constant_evaluated<bl::f64>());
    STATIC_CHECK(native_fma_signed_zero_is_constant_evaluated<bl::f32>());
    STATIC_CHECK(native_fma_signed_zero_is_constant_evaluated<bl::f64>());
    STATIC_CHECK(arithmetic_special_values_are_constant_evaluated<bl::fdd>());
    STATIC_CHECK(arithmetic_special_values_are_constant_evaluated<bl::fqd>());
    STATIC_CHECK(exact_math_is_constant_evaluated<bl::fdd>());
    STATIC_CHECK(exact_math_is_constant_evaluated<bl::fqd>());
    STATIC_CHECK(nominal_navigation_is_constant_evaluated<bl::fdd>());
    STATIC_CHECK(nominal_navigation_is_constant_evaluated<bl::fqd>());
    STATIC_CHECK(io_is_constant_evaluated<bl::f32>());
    STATIC_CHECK(io_is_constant_evaluated<bl::f64>());
    STATIC_CHECK(io_is_constant_evaluated<bl::fdd>());
    STATIC_CHECK(io_is_constant_evaluated<bl::fqd>());
    STATIC_CHECK(maximum_fqd_io_is_constant_evaluated());
    STATIC_CHECK(sparse_fdd_io_is_constant_evaluated());
    STATIC_CHECK(random_is_constant_evaluated<bl::fdd>());
    STATIC_CHECK(random_is_constant_evaluated<bl::fqd>());
}
