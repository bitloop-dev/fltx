#include <catch2/catch_test_macros.hpp>

#include <ios>
#include <limits>
#include <string_view>
#include <type_traits>

#include <fltx.h>

namespace
{
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

    constexpr bl::f128 dd_input{ 1.25 };
    constexpr bl::f256 qd_input{ 1.25 };

    constexpr auto dd_sqrt = bl::sqrt(bl::f128{ 4.0 });
    constexpr auto dd_exp_log = bl::exp(bl::log(dd_input));
    constexpr auto dd_sincos = bl::sincos<bl::f128>(dd_input);
    constexpr auto qd_sqrt = bl::sqrt(bl::f256{ 4.0 });
    constexpr auto qd_exp_log = bl::exp(bl::log(qd_input));
    constexpr auto qd_sincos = bl::sincos<bl::f256>(qd_input);

    // Independently precomputed with a 256-bit MPFR oracle, then split into
    // correctly rounded binary64 limbs. These values prevent an algebraic
    // identity from allowing matching constexpr errors to pass unnoticed.
    constexpr bl::f128 dd_exp_expected{
        0x1.747a513dbef6ap+0,
        0x1.88d1e2d966c25p-54
    };
    constexpr bl::f256 qd_exp_expected{
        0x1.747a513dbef6ap+0,
        0x1.88d1e2d966c25p-54,
        -0x1.bfa3a8705bde1p-108,
        0x1.a5c6dbedc36bbp-163
    };
    constexpr bl::f128 dd_log_expected{
        0x1.4618bc21c5ec2p-2,
        0x1.f42decdeccf1dp-56
    };
    constexpr bl::f256 qd_log_expected{
        0x1.4618bc21c5ec2p-2,
        0x1.f42decdeccf1dp-56,
        -0x1.77d446996da00p-111,
        0x1.68872796bdd6bp-165
    };
    constexpr bl::f128 dd_sin_expected{
        0x1.e5e14fe11418cp-1,
        0x1.f26492c1c25a0p-57
    };
    constexpr bl::f256 qd_sin_expected{
        0x1.e5e14fe11418cp-1,
        0x1.f26492c1c25a0p-57,
        0x1.3931bcfd30bc4p-113,
        0x1.3379a6078b05fp-168
    };
    constexpr bl::f128 dd_cos_expected{
        0x1.42e3dd88bd952p-2,
        -0x1.353a9f74bf255p-57
    };
    constexpr bl::f256 qd_cos_expected{
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

        const T minimum_zero = bl::fmin(zero, negative_zero);
        const T maximum_zero = bl::fmax(negative_zero, zero);

        return bl::fmin(nan, one) == one &&
               bl::fmin(one, nan) == one &&
               bl::fmax(nan, one) == one &&
               bl::fmax(one, nan) == one &&
               bl::isnan(bl::fmin(nan, nan)) &&
               bl::isnan(bl::fmax(nan, nan)) &&
               bl::iszero(minimum_zero) && bl::signbit(minimum_zero) &&
               bl::iszero(maximum_zero) && !bl::signbit(maximum_zero);
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

        return bl::isinf(positive_infinity) && !bl::signbit(positive_infinity) &&
               bl::isinf(negative_infinity) && bl::signbit(negative_infinity) &&
               bl::isnan(zero / zero) &&
               bl::isnan(infinity - infinity) &&
               bl::isnan(infinity * zero);
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

    template<class T>
    consteval bool nominal_navigation_is_constant_evaluated()
    {
        constexpr T one{ 1.0 };
        constexpr T zero{ 0.0 };
        constexpr T up = bl::nextafter(one, T{ 2.0 });
        constexpr T down = bl::nextafter(one, zero);

        if constexpr (std::is_same_v<T, bl::f128>)
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

    static_assert(dd_sqrt == bl::f128{ 2.0 });
    static_assert(qd_sqrt == bl::f256{ 2.0 });
    static_assert(dd_exp_log > bl::f128{ 1.249 });
    static_assert(dd_exp_log < bl::f128{ 1.251 });
    static_assert(qd_exp_log > bl::f256{ 1.249 });
    static_assert(qd_exp_log < bl::f256{ 1.251 });
    static_assert(dd_sincos.s * dd_sincos.s + dd_sincos.c * dd_sincos.c >
                  bl::f128{ 0.999 });
    static_assert(qd_sincos.s * qd_sincos.s + qd_sincos.c * qd_sincos.c >
                  bl::f256{ 0.999 });
    static_assert(within_bits(bl::exp(bl::f128{ 0.375 }), dd_exp_expected, 78));
    static_assert(within_bits(bl::exp(bl::f256{ 0.375 }), qd_exp_expected, 178));
    static_assert(within_bits(bl::log(bl::f128{ 1.375 }), dd_log_expected, 78));
    static_assert(within_bits(bl::log(bl::f256{ 1.375 }), qd_log_expected, 178));
    static_assert(within_bits(dd_sincos.s, dd_sin_expected, 78));
    static_assert(within_bits(dd_sincos.c, dd_cos_expected, 78));
    static_assert(within_bits(qd_sincos.s, qd_sin_expected, 178));
    static_assert(within_bits(qd_sincos.c, qd_cos_expected, 178));
    static_assert(core_is_constant_evaluated<bl::f128>());
    static_assert(core_is_constant_evaluated<bl::f256>());
    static_assert(native_minmax_special_values_are_constant_evaluated<bl::f32>());
    static_assert(native_minmax_special_values_are_constant_evaluated<bl::f64>());
    static_assert(arithmetic_special_values_are_constant_evaluated<bl::f128>());
    static_assert(arithmetic_special_values_are_constant_evaluated<bl::f256>());
    static_assert(exact_math_is_constant_evaluated<bl::f128>());
    static_assert(exact_math_is_constant_evaluated<bl::f256>());
    static_assert(nominal_navigation_is_constant_evaluated<bl::f128>());
    static_assert(nominal_navigation_is_constant_evaluated<bl::f256>());
    static_assert(io_is_constant_evaluated<bl::f32>());
    static_assert(io_is_constant_evaluated<bl::f64>());
    static_assert(io_is_constant_evaluated<bl::f128>());
    static_assert(io_is_constant_evaluated<bl::f256>());
    static_assert(random_is_constant_evaluated<bl::f128>());
    static_assert(random_is_constant_evaluated<bl::f256>());
}

TEST_CASE("genuine constant evaluation covers representative heavy paths", "[constexpr]")
{
    CHECK(dd_sqrt == bl::sqrt(bl::f128{ 4.0 }));
    CHECK(dd_exp_log == bl::exp(bl::log(dd_input)));
    CHECK(within_roundoff(dd_sincos.s, bl::sin(dd_input)));
    CHECK(within_roundoff(dd_sincos.c, bl::cos(dd_input)));

    CHECK(qd_sqrt == bl::sqrt(bl::f256{ 4.0 }));
    CHECK(qd_exp_log == bl::exp(bl::log(qd_input)));
    CHECK(within_roundoff(qd_sincos.s, bl::sin(qd_input)));
    CHECK(within_roundoff(qd_sincos.c, bl::cos(qd_input)));
    STATIC_CHECK(within_bits(bl::exp(bl::f128{ 0.375 }), dd_exp_expected, 78));
    STATIC_CHECK(within_bits(bl::exp(bl::f256{ 0.375 }), qd_exp_expected, 178));
    STATIC_CHECK(within_bits(bl::log(bl::f128{ 1.375 }), dd_log_expected, 78));
    STATIC_CHECK(within_bits(bl::log(bl::f256{ 1.375 }), qd_log_expected, 178));
    STATIC_CHECK(within_bits(dd_sincos.s, dd_sin_expected, 78));
    STATIC_CHECK(within_bits(dd_sincos.c, dd_cos_expected, 78));
    STATIC_CHECK(within_bits(qd_sincos.s, qd_sin_expected, 178));
    STATIC_CHECK(within_bits(qd_sincos.c, qd_cos_expected, 178));

    STATIC_CHECK(core_is_constant_evaluated<bl::f128>());
    STATIC_CHECK(core_is_constant_evaluated<bl::f256>());
    STATIC_CHECK(native_minmax_special_values_are_constant_evaluated<bl::f32>());
    STATIC_CHECK(native_minmax_special_values_are_constant_evaluated<bl::f64>());
    STATIC_CHECK(arithmetic_special_values_are_constant_evaluated<bl::f128>());
    STATIC_CHECK(arithmetic_special_values_are_constant_evaluated<bl::f256>());
    STATIC_CHECK(exact_math_is_constant_evaluated<bl::f128>());
    STATIC_CHECK(exact_math_is_constant_evaluated<bl::f256>());
    STATIC_CHECK(nominal_navigation_is_constant_evaluated<bl::f128>());
    STATIC_CHECK(nominal_navigation_is_constant_evaluated<bl::f256>());
    STATIC_CHECK(io_is_constant_evaluated<bl::f32>());
    STATIC_CHECK(io_is_constant_evaluated<bl::f64>());
    STATIC_CHECK(io_is_constant_evaluated<bl::f128>());
    STATIC_CHECK(io_is_constant_evaluated<bl::f256>());
    STATIC_CHECK(random_is_constant_evaluated<bl::f128>());
    STATIC_CHECK(random_is_constant_evaluated<bl::f256>());
}
