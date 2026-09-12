#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cfloat>
#include <cfenv>
#include <cmath>
#include <limits>
#include <string_view>
#include <type_traits>

#include <fltx/charconv.h>
#include <fltx/math.h>
#include <fltx/string.h>

namespace
{
    constexpr bool reciprocal_keeps_exact_powers_of_two(double unit = 1.0)
    {
        for (double input : {0x1p-1000, 0x1p-500, 0.5, 1.0, 2.0, 0x1p500, 0x1p1000})
        {
            input *= unit;
            const auto positive = bl::recip(bl::fdd{ input });
            const auto negative = bl::recip(bl::fdd{ -input });
            if (positive.hi != 1.0 / input || positive.lo != 0.0 ||
                negative.hi != -1.0 / input || negative.lo != 0.0)
                return false;
        }
        return true;
    }

    template<class T>
    constexpr bool rounding_keeps_tail_direction(double unit = 1.0)
    {
        struct witness { double head, tail, expected; };
        constexpr witness cases[]{
            { 2.5, 0x1p-80, 3.0 }, { 2.5, -0x1p-80, 2.0 },
            { -2.5, 0x1p-80, -2.0 }, { -2.5, -0x1p-80, -3.0 },
            { 3.0, 0x1p-80, 3.0 }, { 3.0, -0x1p-80, 3.0 },
            { -3.0, 0x1p-80, -3.0 }, { -3.0, -0x1p-80, -3.0 },
            { 0.5, -0x1p-80, 0.0 }, { -0.5, 0x1p-80, -0.0 },
            { 0x1.fffffffffffffp-2, 0.0, 0.0 },
            { -0x1.fffffffffffffp-2, 0.0, -0.0 },
            { 0x1p52, 0.5, 0x1p52 + 1.0 },
            { -0x1p52, -0.5, -0x1p52 - 1.0 }
        };
        for (const auto& row : cases)
        {
            T input{ row.head * unit };
            if constexpr (bl::fltx_fdd<T>) input.lo = row.tail;
            else input.x1 = row.tail;
            const T rounded = bl::round(input);
            if (rounded != T{ row.expected } ||
                (row.expected == 0.0 && bl::signbit(rounded) != bl::signbit(row.expected)))
                return false;
        }
        return true;
    }

    template<class T>
    constexpr bool integer_powers_keep_binary_values(double unit = 1.0)
    {
        double expected = 1.0;
        for (unsigned exponent = 0; exponent <= 32; ++exponent)
        {
            if (bl::ipow(T{ -0.5 * unit }, exponent) != T{ expected } ||
                bl::ipow(T{ -2.0 * unit }, -static_cast<int>(exponent)) != T{ expected })
                return false;
            expected *= -0.5;
        }
        return bl::ipow(T{ -unit }, std::numeric_limits<std::uint64_t>::max()) == T{ -1.0 } &&
               bl::ipow(T{ -unit }, std::numeric_limits<std::int64_t>::min()) == T{ 1.0 };
    }

    template<class T>
    void check_selection_and_interpolation()
    {
        CAPTURE(sizeof(T), std::numeric_limits<T>::digits);
        const T zero{ 0.0 }, negative_zero{ -0.0 }, one{ 1.0 };
        const T maximum = std::numeric_limits<T>::max();
        const T tiny = std::numeric_limits<T>::denorm_min();
        const T nan = std::numeric_limits<T>::quiet_NaN();
        const T infinity = std::numeric_limits<T>::infinity();
        CHECK(bl::min(T{ 3.0 }, T{ 4.0 }) == T{ 3.0 });
        CHECK(bl::max(T{ 3.0 }, T{ 4.0 }) == T{ 4.0 });
        CHECK(bl::clamp(T{ 3.0 }, 0, 2) == T{ 2.0 });
        CHECK(bl::clamp(T{ -3.0 }, 0.0, 2.0) == zero);
        CHECK(bl::signbit(bl::min(negative_zero, zero)));
        CHECK_FALSE(bl::signbit(bl::min(zero, negative_zero)));
        CHECK(bl::signbit(bl::max(negative_zero, zero)));
        CHECK_FALSE(bl::signbit(bl::max(zero, negative_zero)));
        CHECK(bl::isnan(bl::min(nan, one)));
        CHECK(bl::isnan(bl::max(nan, one)));
        CHECK(bl::min(one, nan) == one);
        CHECK(bl::max(one, nan) == one);
        CHECK(bl::min({ one }) == one);
        CHECK(bl::max({ one }) == one);
        const std::initializer_list<T> values{ T{ 3.0 }, one, T{ 4.0 }, T{ 2.0 } };
        CHECK(bl::min(values) == one);
        CHECK(bl::max(values) == T{ 4.0 });
        CHECK(bl::signbit(bl::min({ negative_zero, zero })));
        CHECK_FALSE(bl::signbit(bl::min({ zero, negative_zero })));
        CHECK(bl::signbit(bl::max({ negative_zero, zero })));
        CHECK_FALSE(bl::signbit(bl::max({ zero, negative_zero })));
        CHECK(bl::isnan(bl::min({ nan, one, zero })));
        CHECK(bl::isnan(bl::max({ nan, one, zero })));
        CHECK(bl::min({ one, nan, zero }) == zero);
        CHECK(bl::max({ zero, nan, one }) == one);
        CHECK(bl::min({ infinity, one, T{ -infinity } }) == T{ -infinity });
        CHECK(bl::max({ T{ -infinity }, one, infinity }) == infinity);
        CHECK(bl::isnan(bl::clamp(nan, zero, one)));
        CHECK(bl::signbit(bl::clamp(negative_zero, zero, one)));
        CHECK(bl::signbit(bl::clamp(negative_zero, T{ -1.0 }, zero)));
        CHECK_FALSE(bl::signbit(bl::clamp(zero, negative_zero, negative_zero)));
        const auto ties = bl::minmax(negative_zero, zero);
        CHECK(bl::signbit(ties.first));
        CHECK_FALSE(bl::signbit(ties.second));

        CHECK(bl::midpoint(T{ 3.0 }, T{ 4.0 }) == T{ 3.5 });
        CHECK(bl::midpoint(maximum, maximum) == maximum);
        CHECK(bl::midpoint(T{ -maximum }, maximum) == zero);
        CHECK(bl::midpoint(tiny, tiny) == tiny);
        CHECK(bl::midpoint(zero, tiny) == zero);
        CHECK(bl::midpoint(tiny, maximum) == T{ maximum * T{ 0.5 } });
        CHECK(bl::midpoint(maximum, tiny) == T{ maximum * T{ 0.5 } });
        CHECK(bl::isnan(bl::midpoint(nan, one)));
        CHECK(bl::midpoint(infinity, infinity) == infinity);

        CHECK(bl::lerp(T{ 3.0 }, T{ 4.0 }, T{ 0.25 }) == T{ 3.25 });
        CHECK(bl::lerp(T{ 3.0 }, T{ 4.0 }, T{ -1.0 }) == T{ 2.0 });
        CHECK(bl::lerp(T{ 3.0 }, T{ 4.0 }, T{ 2.0 }) == T{ 5.0 });
        CHECK(bl::lerp(maximum, maximum, maximum) == maximum);
        CHECK(bl::lerp(T{ -maximum }, maximum, T{ 0.5 }) == zero);
        CHECK(bl::signbit(bl::lerp(negative_zero, one, zero)));
        CHECK(bl::signbit(bl::lerp(one, negative_zero, one)));
        CHECK(bl::isnan(bl::lerp(one, T{ 2.0 }, nan)));
        CHECK(bl::lerp(zero, one, infinity) == infinity);
        CHECK(bl::lerp(zero, one, T{ -infinity }) == T{ -infinity });

        if constexpr (std::floating_point<T>)
        {
            CHECK(bl::isnan(bl::lerp(zero, one, nan)));
            CHECK(bl::isnan(bl::lerp(one, zero, nan)));
            CHECK(bl::lerp(one, zero, infinity) == T{ -infinity });
            CHECK(bl::lerp(one, zero, T{ -infinity }) == infinity);
            CHECK(bl::lerp(T{ -maximum }, maximum, infinity) == infinity);
            CHECK(bl::lerp(maximum, T{ -maximum }, infinity) == T{ -infinity });
        }

        if constexpr (bl::fltx_extended_float<T>)
        {
            const T a = one + T{ 0x1p-60 };
            const T b = one + T{ 0x1p-58 };
            const T center = one + T{ 0x1.4p-59 };
            const T quarter = one + T{ 0x1.cp-60 };
            CHECK(bl::midpoint(a, b) == center);
            CHECK(bl::lerp(a, b, T{ 0.25 }) == quarter);
        }

        const std::array<T, 7> endpoints{ T{ -maximum }, T{ -1.0 }, T{ -tiny }, zero, tiny, one, maximum };
        for (const T& a : endpoints)
        {
            for (const T& b : endpoints)
            {
                CHECK(bl::lerp(a, b, zero) == a);
                CHECK(bl::lerp(a, b, one) == b);
                T previous = a;
                for (const T& weight : std::array<T, 3>{ T{ 0.25 }, T{ 0.5 }, T{ 0.75 } })
                {
                    const T value = bl::lerp(a, b, weight);
                    CHECK(bl::isfinite(value));
                    CHECK(value >= bl::min(a, b));
                    CHECK(value <= bl::max(a, b));
                    CHECK((a < b ? value >= previous : value <= previous));
                    previous = value;
                }
            }
        }
    }

    [[nodiscard]] bool nonoverlapping(double high, double low) noexcept
    {
        if (low == 0.0)
            return true;
        if (high == 0.0 || !std::isfinite(high) || !std::isfinite(low))
            return false;
        return high + low == high;
    }

    [[nodiscard]] bool is_canonical(const bl::fdd_s& value) noexcept
    {
        return nonoverlapping(value.hi, value.lo);
    }

    [[nodiscard]] bool is_canonical(const bl::fqd_s& value) noexcept
    {
        return nonoverlapping(value.x0, value.x1) &&
               nonoverlapping(value.x1, value.x2) &&
               nonoverlapping(value.x2, value.x3);
    }

    template<class T>
    void check_canonical(std::string_view operation, const T& value)
    {
        INFO("operation: " << operation);
        CHECK(is_canonical(value));
    }

    template<class T>
    [[nodiscard]] T expansion(double leading, double trailing)
    {
        if constexpr (std::is_same_v<T, bl::fdd>)
            return T{ leading, trailing };
        else
            return T{ leading, trailing, 0.0, 0.0 };
    }

    template<class T>
    constexpr bool nominal_nextafter_contract()
    {
        constexpr T zero{ 0.0 };
        constexpr T one{ 1.0 };
        constexpr T two{ 2.0 };
        constexpr T eight{ 8.0 };
        constexpr T epsilon = std::numeric_limits<T>::epsilon();

        const T above_one = bl::nextafter(one, two);
        const T below_one = bl::nextafter(one, zero);
        const T above_eight = bl::nextafter(eight, T{ 16.0 });
        const T below_eight = bl::nextafter(eight, zero);

        if constexpr (std::is_same_v<T, bl::fdd>)
        {
            if (above_one != T{ 1.0, 0x1p-105 } ||
                below_one != T{ 1.0, -0x1p-106 } ||
                above_eight != T{ 8.0, 0x1p-102 } ||
                below_eight != T{ 8.0, -0x1p-103 } ||
                bl::nextafter(T{ -1.0 }, zero) != T{ -1.0, 0x1p-106 } ||
                bl::nextafter(T{ -1.0 }, T{ -2.0 }) != T{ -1.0, -0x1p-105 })
            {
                return false;
            }
        }
        else
        {
            if (above_one != T{ 1.0, 0x1p-211, 0.0, 0.0 } ||
                below_one != T{ 1.0, -0x1p-212, 0.0, 0.0 } ||
                above_eight != T{ 8.0, 0x1p-208, 0.0, 0.0 } ||
                below_eight != T{ 8.0, -0x1p-209, 0.0, 0.0 } ||
                bl::nextafter(T{ -1.0 }, zero) != T{ -1.0, 0x1p-212, 0.0, 0.0 } ||
                bl::nextafter(T{ -1.0 }, T{ -2.0 }) != T{ -1.0, -0x1p-211, 0.0, 0.0 })
            {
                return false;
            }
        }

        return above_one - one == epsilon &&
               bl::nextafter(above_one, zero) == one &&
               bl::nextafter(below_one, two) == one;
    }

    template<class T>
    void check_exact_math()
    {
        CHECK(bl::abs(T{ -2.0 }) == T{ 2.0 });
        CHECK(bl::fabs(T{ -2.0 }) == T{ 2.0 });
        CHECK_FALSE(bl::signbit(bl::fabs(T{ -0.0 })));
        CHECK(bl::sqr(T{ -3.0 }) == T{ 9.0 });
        CHECK(bl::recip(T{ 4.0 }) == T{ 0.25 });
        CHECK(bl::sqrt(T{ 4.0 }) == T{ 2.0 });
        CHECK(bl::cbrt(T{ 8.0 }) == T{ 2.0 });
        CHECK(bl::hypot(T{ 3.0 }, T{ 4.0 }) == T{ 5.0 });
        CHECK(bl::pow(T{ 2.0 }, 10) == T{ 1024.0 });
        CHECK(bl::fma(T{ 2.0 }, T{ 3.0 }, T{ 4.0 }) == T{ 10.0 });
        CHECK(bl::fmin(T{ 2.0 }, T{ 3.0 }) == T{ 2.0 });
        CHECK(bl::fmax(T{ 2.0 }, T{ 3.0 }) == T{ 3.0 });
        CHECK(bl::fdim(T{ 2.0 }, T{ 3.0 }) == T{ 0.0 });
        CHECK(bl::copysign(T{ 2.0 }, T{ -1.0 }) == T{ -2.0 });
        CHECK(bl::clamp(T{ 3.0 }, T{ -1.0 }, T{ 2.0 }) == T{ 2.0 });
    }

    template<class T>
    void check_decomposition()
    {
        T integral{};
        int exponent = 0;
        int quotient = 0;

        CHECK(bl::modf(T{ 3.25 }, &integral) == T{ 0.25 });
        CHECK(integral == T{ 3.0 });
        CHECK(bl::frexp(T{ 8.0 }, &exponent) == T{ 0.5 });
        CHECK(exponent == 4);
        CHECK(bl::ldexp(T{ 0.5 }, 4) == T{ 8.0 });
        CHECK(bl::scalbn(T{ 0.5 }, 4) == T{ 8.0 });
        CHECK(bl::scalbln(T{ 0.5 }, 4L) == T{ 8.0 });
        CHECK(bl::logb(T{ 8.0 }) == T{ 3.0 });
        CHECK(bl::ilogb(T{ 8.0 }) == 3);
        CHECK(bl::fmod(T{ 5.5 }, T{ 2.0 }) == T{ 1.5 });
        CHECK(bl::remainder(T{ 5.0 }, T{ 2.0 }) == T{ 1.0 });
        CHECK(bl::remquo(T{ 5.0 }, T{ 2.0 }, &quotient) == T{ 1.0 });
        CHECK((quotient & 1) == 0);

        const T one{ 1.0 };
        const T next = bl::nextafter(one, T{ 2.0 });
        CHECK(next > one);
        CHECK(bl::nextafter(next, T{ 0.0 }) == one);
        CHECK(bl::nexttoward(one, T{ 2.0 }) == next);
        CHECK(bl::nexttoward(one, 2.0L) == next);
    }

    template<class T>
    void check_sincos()
    {
        const T input{ 0.375 };
        T sine{};
        T cosine{};
        const bool ok = bl::sincos(input, sine, cosine);
        const auto result = bl::sincos<T>(input);

        CHECK(ok);
        CHECK(result.ok == ok);
        CHECK(result.s == sine);
        CHECK(result.c == cosine);

        const T identity = sine * sine + cosine * cosine;
        const T delta = identity - T{ 1.0 };
        const T error = bl::abs(delta);
        const T tolerance = std::numeric_limits<T>::epsilon() * 64.0;
        CHECK(error < tolerance);
    }

    template<class T>
    void check_nan(std::string_view operation, const T& value)
    {
        INFO("operation: " << operation);
        CHECK(bl::isnan(value));
    }

    template<class T>
    void check_zero(
        std::string_view operation,
        const T& value)
    {
        INFO("operation: " << operation);
        CHECK(bl::iszero(value));
    }

    template<class T>
    void check_zero(
        std::string_view operation,
        const T& value,
        bool negative)
    {
        INFO("operation: " << operation);
        CHECK(bl::iszero(value));
        CHECK(bl::signbit(value) == negative);
    }

    template<class T>
    void check_infinity(
        std::string_view operation,
        const T& value,
        bool negative)
    {
        INFO("operation: " << operation);
        CHECK(bl::isinf(value));
        CHECK(bl::signbit(value) == negative);
    }

    template<class T>
    [[nodiscard]] T divide_without_constant_folding(T numerator, T denominator)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            volatile T runtime_numerator = numerator;
            volatile T runtime_denominator = denominator;
            return runtime_numerator / runtime_denominator;
        }
        else
        {
            return numerator / denominator;
        }
    }

    template<class T>
    void check_arithmetic_special_values(std::string_view precision)
    {
        INFO("precision: " << precision);
        const T zero{ 0.0 };
        const T negative_zero{ -0.0 };
        const T one{ 1.0 };
        const T negative_one{ -1.0 };
        const T infinity = std::numeric_limits<T>::infinity();
        const T negative_infinity = -infinity;
        const T nan = std::numeric_limits<T>::quiet_NaN();

        #if defined(FLTX_FAST_MATH)
        constexpr bool relaxed_basic_arithmetic =
            std::is_same_v<T, bl::fdd> || std::is_same_v<T, bl::fqd>;
        #else
        constexpr bool relaxed_basic_arithmetic = false;
        #endif

        if constexpr (!relaxed_basic_arithmetic)
        {
            check_zero("+zero + +zero", T{ zero + zero }, false);
            check_zero("-zero + -zero", T{ negative_zero + negative_zero }, true);
            check_zero("+zero + -zero", T{ zero + negative_zero }, false);
            check_zero("-zero + +zero", T{ negative_zero + zero }, false);
            check_zero("+zero - +zero", T{ zero - zero }, false);
            check_zero("-zero - +zero", T{ negative_zero - zero }, true);
            check_zero("+zero - -zero", T{ zero - negative_zero }, false);
            check_zero("-zero - -zero", T{ negative_zero - negative_zero }, false);
            check_zero("-zero + scalar -zero",
                static_cast<T>(negative_zero + -0.0), true);
            check_zero("-zero - scalar +zero",
                static_cast<T>(negative_zero - 0.0), true);
            check_zero("scalar -zero - +zero",
                static_cast<T>(-0.0 - zero), true);

            check_infinity("infinity + finite", T{ infinity + one }, false);
            check_infinity("finite + -infinity", T{ one + negative_infinity }, true);
            check_nan("infinity + -infinity", T{ infinity + negative_infinity });
            check_nan("infinity - infinity", T{ infinity - infinity });
            check_nan("NaN arithmetic propagation", T{ nan + one });

            check_nan("infinity * zero", T{ infinity * zero });
            check_zero("zero * negative", T{ zero * negative_one }, true);
            check_zero("-zero * negative", T{ negative_zero * negative_one }, false);
            check_infinity("finite / +zero",
                divide_without_constant_folding(one, zero), false);
            check_infinity("finite / -zero",
                divide_without_constant_folding(one, negative_zero), true);
            check_nan("zero / zero",
                divide_without_constant_folding(zero, zero));
            check_nan("infinity / infinity",
                divide_without_constant_folding(infinity, infinity));
        }

        check_zero("abs(-zero)", T{ bl::abs(negative_zero) }, false);
        check_infinity("fabs(-infinity)", T{ bl::fabs(negative_infinity) }, false);
        check_nan("abs(NaN)", T{ bl::abs(nan) });
        check_zero("sqr(-zero)", T{ bl::sqr(negative_zero) }, false);
        check_infinity("sqr(-infinity)", T{ bl::sqr(negative_infinity) }, false);
        check_infinity("recip(+zero)", T{ bl::recip(zero) }, false);
        check_infinity("recip(-zero)", T{ bl::recip(negative_zero) }, true);
        check_zero("recip(+infinity)", T{ bl::recip(infinity) }, false);
        check_zero("recip(-infinity)", T{ bl::recip(negative_infinity) }, true);

        check_nan("fma(infinity, zero, one)",
            T{ bl::fma(infinity, zero, one) });
        check_nan("fma(infinity, one, -infinity)",
            T{ bl::fma(infinity, one, negative_infinity) });
        check_nan("fma NaN propagation", T{ bl::fma(nan, one, one) });
        check_infinity("fma infinity result",
            T{ bl::fma(infinity, one, one) }, false);
        if constexpr (!relaxed_basic_arithmetic)
        {
            check_zero("fma(+zero, one, +zero)",
                T{ bl::fma(zero, one, zero) }, false);
            check_zero("fma(-zero, one, -zero)",
                T{ bl::fma(negative_zero, one, negative_zero) }, true);
        }

        CHECK(T{ bl::fmin(nan, one) } == one);
        CHECK(T{ bl::fmin(one, nan) } == one);
        CHECK(T{ bl::fmax(nan, one) } == one);
        CHECK(T{ bl::fmax(one, nan) } == one);
        check_nan("fmin(NaN, NaN)", T{ bl::fmin(nan, nan) });
        check_nan("fmax(NaN, NaN)", T{ bl::fmax(nan, nan) });
        check_zero("fmin(+zero, -zero)",
            T{ bl::fmin(zero, negative_zero) }, true);
        check_zero("fmin(-zero, +zero)",
            T{ bl::fmin(negative_zero, zero) }, true);
        check_zero("fmax(+zero, -zero)",
            T{ bl::fmax(zero, negative_zero) }, false);
        check_zero("fmax(-zero, +zero)",
            T{ bl::fmax(negative_zero, zero) }, false);

        check_nan("fdim NaN propagation", T{ bl::fdim(nan, one) });
        check_infinity("fdim(+infinity, finite)",
            T{ bl::fdim(infinity, one) }, false);
        check_zero("fdim(finite, +infinity)",
            T{ bl::fdim(one, infinity) }, false);
        check_zero("copysign(+zero, -zero)",
            T{ bl::copysign(zero, negative_zero) }, true);
        check_infinity("copysign(+infinity, -one)",
            T{ bl::copysign(infinity, negative_one) }, true);
    }

    template<class T>
    void check_root_and_power_special_values(std::string_view precision)
    {
        INFO("precision: " << precision);
        const T zero{ 0.0 };
        const T negative_zero{ -0.0 };
        const T one{ 1.0 };
        const T negative_one{ -1.0 };
        const T infinity = std::numeric_limits<T>::infinity();
        const T negative_infinity = -infinity;
        const T nan = std::numeric_limits<T>::quiet_NaN();

        check_zero("sqrt(-zero)", T{ bl::sqrt(negative_zero) }, true);
        check_infinity("sqrt(+infinity)", T{ bl::sqrt(infinity) }, false);
        check_nan("sqrt(negative)", T{ bl::sqrt(negative_one) });
        check_nan("sqrt(NaN)", T{ bl::sqrt(nan) });
        check_zero("cbrt(-zero)", T{ bl::cbrt(negative_zero) }, true);
        check_infinity("cbrt(-infinity)",
            T{ bl::cbrt(negative_infinity) }, true);
        check_infinity("hypot(infinity, NaN)",
            T{ bl::hypot(infinity, nan) }, false);
        check_nan("hypot(finite, NaN)", T{ bl::hypot(one, nan) });
        check_zero("hypot(+zero, -zero)",
            T{ bl::hypot(zero, negative_zero) }, false);

        check_infinity("exp(+infinity)", T{ bl::exp(infinity) }, false);
        check_zero("exp(-infinity)", T{ bl::exp(negative_infinity) }, false);
        check_infinity("exp2(+infinity)", T{ bl::exp2(infinity) }, false);
        check_zero("exp2(-infinity)", T{ bl::exp2(negative_infinity) }, false);
        check_infinity("expm1(+infinity)", T{ bl::expm1(infinity) }, false);
        CHECK(T{ bl::expm1(negative_infinity) } == negative_one);
        check_zero("expm1(-zero)", T{ bl::expm1(negative_zero) }, true);
        check_nan("exp(NaN)", T{ bl::exp(nan) });

        check_infinity("log(+zero)", T{ bl::log(zero) }, true);
        check_infinity("log(-zero)", T{ bl::log(negative_zero) }, true);
        check_nan("log(negative)", T{ bl::log(negative_one) });
        check_infinity("log(+infinity)", T{ bl::log(infinity) }, false);
        check_infinity("log2(+zero)", T{ bl::log2(zero) }, true);
        check_nan("log2(negative)", T{ bl::log2(negative_one) });
        check_infinity("log10(+infinity)",
            T{ bl::log10(infinity) }, false);
        check_infinity("log1p(-one)", T{ bl::log1p(negative_one) }, true);
        check_nan("log1p(less than -one)", T{ bl::log1p(T{ -2.0 }) });
        check_zero("log1p(-zero)", T{ bl::log1p(negative_zero) }, true);

        CHECK(T{ bl::pow(one, nan) } == one);
        CHECK(T{ bl::pow(nan, zero) } == one);
        CHECK(T{ bl::pow(negative_one, infinity) } == one);
        check_nan("pow(negative, fractional)",
            T{ bl::pow(T{ -2.0 }, T{ 0.5 }) });
        check_zero("pow(-zero, positive odd)",
            T{ bl::pow(negative_zero, T{ 3.0 }) }, true);
        check_infinity("pow(-zero, negative odd)",
            T{ bl::pow(negative_zero, T{ -3.0 }) }, true);
        check_zero("pow(+infinity, negative)",
            T{ bl::pow(infinity, T{ -2.0 }) }, false);
    }

    template<class T>
    void check_transcendental_special_values(std::string_view precision)
    {
        INFO("precision: " << precision);
        const T zero{ 0.0 };
        const T negative_zero{ -0.0 };
        const T one{ 1.0 };
        const T negative_one{ -1.0 };
        const T infinity = std::numeric_limits<T>::infinity();
        const T negative_infinity = -infinity;
        const T nan = std::numeric_limits<T>::quiet_NaN();

        check_zero("sin(-zero)", T{ bl::sin(negative_zero) }, true);
        CHECK(T{ bl::cos(negative_zero) } == one);
        check_zero("tan(-zero)", T{ bl::tan(negative_zero) }, true);
        check_nan("sin(infinity)", T{ bl::sin(infinity) });
        check_nan("cos(-infinity)", T{ bl::cos(negative_infinity) });
        check_nan("tan(infinity)", T{ bl::tan(infinity) });
        check_nan("sin(NaN)", T{ bl::sin(nan) });

        check_zero("asin(-zero)", T{ bl::asin(negative_zero) }, true);
        check_zero("atan(-zero)", T{ bl::atan(negative_zero) }, true);
        check_nan("asin(outside domain)", T{ bl::asin(T{ 2.0 }) });
        check_nan("acos(outside domain)", T{ bl::acos(T{ -2.0 }) });
        check_zero("atan2(-zero, positive)",
            T{ bl::atan2(negative_zero, one) }, true);
        check_zero("atan2(+zero, positive)",
            T{ bl::atan2(zero, one) }, false);
        CHECK(bl::signbit(T{ bl::atan2(negative_zero, negative_one) }));
        CHECK_FALSE(bl::signbit(T{ bl::atan2(zero, negative_one) }));

        check_zero("sinh(-zero)", T{ bl::sinh(negative_zero) }, true);
        CHECK(T{ bl::cosh(negative_zero) } == one);
        check_zero("tanh(-zero)", T{ bl::tanh(negative_zero) }, true);
        check_zero("asinh(-zero)", T{ bl::asinh(negative_zero) }, true);
        check_zero("acosh(one)", T{ bl::acosh(one) }, false);
        check_nan("acosh(below one)", T{ bl::acosh(zero) });
        check_infinity("atanh(+one)", T{ bl::atanh(one) }, false);
        check_infinity("atanh(-one)", T{ bl::atanh(negative_one) }, true);
        check_nan("atanh(outside domain)", T{ bl::atanh(T{ 2.0 }) });

        check_zero("erf(-zero)", T{ bl::erf(negative_zero) }, true);
        CHECK(T{ bl::erf(infinity) } == one);
        CHECK(T{ bl::erf(negative_infinity) } == negative_one);
        check_zero("erfc(+infinity)", T{ bl::erfc(infinity) }, false);
        CHECK(T{ bl::erfc(negative_infinity) } == T{ 2.0 });
        check_infinity("lgamma(-infinity)",
            T{ bl::lgamma(negative_infinity) }, false);
        check_infinity("tgamma(+zero)", T{ bl::tgamma(zero) }, false);
        check_infinity("tgamma(-zero)", T{ bl::tgamma(negative_zero) }, true);
        check_infinity("tgamma(+infinity)", T{ bl::tgamma(infinity) }, false);
        check_nan("tgamma(-infinity)", T{ bl::tgamma(negative_infinity) });
    }

    template<class T>
    void check_rounding_and_remainder_special_values(std::string_view precision)
    {
        INFO("precision: " << precision);
        const T zero{ 0.0 };
        const T negative_zero{ -0.0 };
        const T one{ 1.0 };
        const T infinity = std::numeric_limits<T>::infinity();
        const T negative_infinity = -infinity;
        const T nan = std::numeric_limits<T>::quiet_NaN();

        check_zero("floor(-zero)", T{ bl::floor(negative_zero) }, true);
        check_zero("ceil(-zero)", T{ bl::ceil(negative_zero) }, true);
        check_zero("trunc(-zero)", T{ bl::trunc(negative_zero) }, true);
        check_zero("round(-zero)", T{ bl::round(negative_zero) }, true);
        check_zero("roundeven(-zero)", T{ bl::roundeven(negative_zero) }, true);
        check_zero("round(positive fraction)", T{ bl::round(T{ 0.375 }) }, false);
        check_zero("round(negative fraction)", T{ bl::round(T{ -0.375 }) }, true);
        check_infinity("floor(+infinity)", T{ bl::floor(infinity) }, false);
        check_infinity("round(-infinity)",
            T{ bl::round(negative_infinity) }, true);
        check_infinity("roundeven(+infinity)",
            T{ bl::roundeven(infinity) }, false);
        check_nan("roundeven(NaN)", T{ bl::roundeven(nan) });
        check_zero("round_decimals(-zero)",
            T{ bl::round_decimals(negative_zero, 3) }, true);
        check_infinity("round_significant(infinity)",
            T{ bl::round_significant(infinity, 4) }, false);

        check_zero("fmod(-zero, finite)",
            T{ bl::fmod(negative_zero, one) }, true);
        check_zero("remainder(-zero, finite)",
            T{ bl::remainder(negative_zero, one) }, true);
        CHECK(T{ bl::fmod(one, infinity) } == one);
        CHECK(T{ bl::remainder(one, infinity) } == one);
        check_nan("fmod(infinity, finite)", T{ bl::fmod(infinity, one) });
        check_nan("remainder(finite, zero)",
            T{ bl::remainder(one, zero) });
        check_nan("fmod NaN propagation", T{ bl::fmod(nan, one) });

        int quotient = 123;
        check_nan("remquo(infinity, finite)",
            T{ bl::remquo(infinity, one, &quotient) });

        T integral{};
        check_zero("modf(+infinity) fraction",
            T{ bl::modf(infinity, &integral) }, false);
        check_infinity("modf(+infinity) integral", integral, false);
        check_zero("modf(-infinity) fraction",
            T{ bl::modf(negative_infinity, &integral) }, true);
        check_infinity("modf(-infinity) integral", integral, true);
        check_zero("modf(-zero) fraction",
            T{ bl::modf(negative_zero, &integral) }, true);
        check_zero("modf(-zero) integral", integral, true);
        check_nan("modf(NaN) fraction", T{ bl::modf(nan, &integral) });
        check_nan("modf(NaN) integral", integral);

        int exponent = 99;
        check_zero("frexp(-zero)",
            T{ bl::frexp(negative_zero, &exponent) }, true);
        CHECK(exponent == 0);
        check_infinity("frexp(+infinity)",
            T{ bl::frexp(infinity, &exponent) }, false);
        check_zero("ldexp(-zero)", T{ bl::ldexp(negative_zero, 12) }, true);
        check_infinity("scalbn(-infinity)",
            T{ bl::scalbn(negative_infinity, -12) }, true);
        check_infinity("ldexp(+infinity)",
            T{ bl::ldexp(infinity, 12) }, false);
        check_nan("ldexp(NaN)", T{ bl::ldexp(nan, 12) });
        check_infinity("ipow(+infinity, odd)",
            T{ bl::ipow(infinity, 7) }, false);
        check_infinity("ipow(-infinity, odd)",
            T{ bl::ipow(negative_infinity, 7) }, true);
        check_nan("ipow(NaN, odd)", T{ bl::ipow(nan, 7) });
        check_infinity("logb(+zero)", T{ bl::logb(zero) }, true);
        check_infinity("logb(+infinity)", T{ bl::logb(infinity) }, false);
        check_nan("logb(NaN)", T{ bl::logb(nan) });

        const T below_infinity = bl::nextafter(infinity, zero);
        CHECK(bl::isfinite(below_infinity));
        CHECK_FALSE(bl::signbit(below_infinity));
        const T below_zero = bl::nextafter(zero, T{ -1.0 });
        CHECK_FALSE(bl::iszero(below_zero));
        CHECK(bl::signbit(below_zero));
        check_zero("nextafter(-zero, +zero)",
            T{ bl::nextafter(negative_zero, zero) }, false);
    }

    template<class T>
    void check_expansion_precision_rounding()
    {
        const T decimal = bl::parse<T>("1.2345");
        const T decimal_tie = bl::parse<T>("1.1875");
        const T negative_decimal_tie = bl::parse<T>("-1.1875");
        const T two_decimals = bl::parse<T>("1.23");
        const T three_decimals = bl::parse<T>("1.234");
        const T tied_decimals = bl::parse<T>("1.188");
        const T negative_tied_decimals = bl::parse<T>("-1.188");
        const T unchanged = bl::parse<T>("1.25");
        CHECK(bl::round_decimals(decimal, 2) == two_decimals);
        CHECK(bl::round_decimals(decimal, 3) == three_decimals);
        CHECK(bl::round_decimals(decimal_tie, 3) == tied_decimals);
        CHECK(bl::round_decimals(
            negative_decimal_tie, 3) == negative_tied_decimals);
        CHECK(bl::round_decimals(unchanged, 0) == unchanged);

        const T positive_below_half = bl::parse<T>(
            "0.0002719634966936723775485033971720674");
        const T negative_below_half = -positive_below_half;
        const T positive_zero = bl::round_decimals(positive_below_half, 3);
        const T negative_zero = bl::round_decimals(negative_below_half, 3);
        CHECK(bl::iszero(positive_zero));
        CHECK_FALSE(bl::signbit(positive_zero));
        CHECK(bl::iszero(negative_zero));
        CHECK(bl::signbit(negative_zero));

        const T large = bl::parse<T>("12345");
        const T small = bl::parse<T>("0.012345");
        const T tie_down = bl::parse<T>("12500");
        const T tie_up = bl::parse<T>("13500");
        const T rounded_large = bl::parse<T>("12300");
        const T rounded_small = bl::parse<T>("0.0123");
        const T rounded_down = bl::parse<T>("12000");
        const T rounded_up = bl::parse<T>("14000");
        CHECK(bl::round_significant(
            large, 3) == rounded_large);
        CHECK(bl::round_significant(
            small, 3) == rounded_small);
        CHECK(bl::round_significant(
            tie_down, 2) == rounded_down);
        CHECK(bl::round_significant(
            tie_up, 2) == rounded_up);
    }
}

TEST_CASE("selection and interpolation preserve ordering and finite range",
          "[contracts][math][selection]")
{
    check_selection_and_interpolation<bl::f32>();
    check_selection_and_interpolation<bl::f64>();
    check_selection_and_interpolation<bl::fdd>();
    check_selection_and_interpolation<bl::fqd>();
    const std::uint64_t largest = std::numeric_limits<std::uint64_t>::max();
    CHECK(bl::min(largest, largest - 1) == largest - 1);
    CHECK(bl::max(largest, largest - 1) == largest);
    CHECK(bl::min({ largest, largest - 1, largest - 2 }) == largest - 2);
    CHECK(bl::max({ largest - 2, largest - 1, largest }) == largest);
    CHECK(bl::min<int>({ 3, 1, 4, 2 }) == 1);
    CHECK(bl::max<double>({ 3, 1.5, 4, 2.0 }) == 4.0);
    CHECK(bl::clamp(largest, largest - 2, largest - 1) == largest - 1);
    CHECK(bl::midpoint(largest, largest - 1) == largest);
    CHECK(bl::midpoint(largest - 1, largest) == largest - 1);
    CHECK(bl::midpoint(largest, std::uint64_t{ 0 }) == (largest / 2 + 1));
    CHECK(bl::midpoint(std::numeric_limits<std::int64_t>::min(),
                       std::numeric_limits<std::int64_t>::max()) == -1);
    CHECK(bl::midpoint(std::numeric_limits<std::int64_t>::max(),
                       std::numeric_limits<std::int64_t>::min()) == 0);
    CHECK(bl::clamp(std::string_view{ "b" }, std::string_view{ "a" }, std::string_view{ "c" }) == "b");
}

TEST_CASE("exact math contracts hold for both expansion types", "[contracts][math]")
{
    STATIC_CHECK(reciprocal_keeps_exact_powers_of_two());
    volatile double unit = 1.0;
    CHECK(reciprocal_keeps_exact_powers_of_two(unit));
    check_exact_math<bl::f32>();
    check_exact_math<bl::f64>();
    check_exact_math<bl::fdd>();
    check_exact_math<bl::fqd>();
}

TEST_CASE("nextafter follows the nominal 106 and 212 bit models",
          "[contracts][math][nextafter]")
{
    STATIC_CHECK(nominal_nextafter_contract<bl::fdd>());
    STATIC_CHECK(nominal_nextafter_contract<bl::fqd>());

    const auto check_edges = []<class T>()
    {
        using limits = std::numeric_limits<T>;
        const T zero{ 0.0 };
        const T negative_zero{ -0.0 };
        const T denorm = limits::denorm_min();
        const T infinity = limits::infinity();
        const T nan = limits::quiet_NaN();

        CHECK(bl::nextafter(zero, T{ 1.0 }) == denorm);
        CHECK(bl::nextafter(zero, T{ -1.0 }) == -denorm);
        CHECK_FALSE(bl::signbit(bl::nextafter(negative_zero, zero)));
        CHECK(bl::nextafter(infinity, zero) == limits::max());
        CHECK(bl::nextafter(-infinity, zero) == limits::lowest());
        CHECK(bl::isinf(bl::nextafter(limits::max(), infinity)));
        CHECK(bl::isinf(bl::nextafter(limits::lowest(), -infinity)));
        CHECK(bl::isnan(bl::nextafter(nan, zero)));

        const T below_minimum = bl::nextafter(limits::min(), zero);
        CHECK(below_minimum < limits::min());
        CHECK(bl::fpclassify(below_minimum) == FP_SUBNORMAL);
        CHECK(bl::nextafter(below_minimum, limits::min()) == limits::min());
    };
    check_edges.template operator()<bl::fdd>();
    check_edges.template operator()<bl::fqd>();

    constexpr bl::fdd sparse_dd{ 1.0, 0x1.8p-106 };
    constexpr bl::fqd sparse_qd{
        1.0, 0x1p-55, -0x1p-120, 0x1.8p-212
    };
    STATIC_CHECK(bl::nextafter(sparse_dd, bl::fdd{ 2.0 }) ==
                 sparse_dd + bl::fdd{ 0x1p-105 });
    STATIC_CHECK(bl::nextafter(sparse_qd, bl::fqd{ 2.0 }) ==
                 sparse_qd + bl::fqd{ 0x1p-211 });
    STATIC_CHECK(bl::ilogb(bl::fdd{ 1.0, -0x1p-107 }) == -1);
    STATIC_CHECK(bl::ilogb(bl::fqd{ 1.0, -0x1p-213, 0.0, 0.0 }) == -1);

#if LDBL_MANT_DIG > DBL_MANT_DIG
    constexpr long double extended_target = 1.0L + 0x1p-60L;
    STATIC_CHECK(bl::nexttoward(bl::fdd{ 1.0 }, extended_target) > bl::fdd{ 1.0 });
    STATIC_CHECK(bl::nexttoward(bl::fqd{ 1.0 }, extended_target) > bl::fqd{ 1.0 });
#endif
}

TEST_CASE("integer powers preserve exponent width and expansion tails", "[contracts][math][ipow]")
{
    STATIC_CHECK(integer_powers_keep_binary_values<bl::fdd>());
    STATIC_CHECK(integer_powers_keep_binary_values<bl::fqd>());
    volatile double unit = 1.0;
    CHECK(integer_powers_keep_binary_values<bl::fdd>(unit));
    CHECK(integer_powers_keep_binary_values<bl::fqd>(unit));

    const auto check_tails = [&]<class T>() {
        T base{ 1.25 * unit };
        if constexpr (bl::fltx_fdd<T>) base.lo = 0x1p-60;
        else { base.x1 = 0x1p-60; base.x2 = -0x1p-120; }
        T repeated{ 1.0 };
        for (int exponent = 0; exponent <= 16; ++exponent)
        {
            CAPTURE(exponent, sizeof(T));
            const T powered = bl::ipow(base, exponent);
            const T tolerance = bl::abs(repeated) * std::numeric_limits<T>::epsilon() * 64.0;
            CHECK(bl::abs(powered - repeated) <= tolerance);
            repeated *= base;
        }
    };
    check_tails.template operator()<bl::fdd>();
    check_tails.template operator()<bl::fqd>();
}

TEST_CASE("rounding and decomposition preserve defined semantics", "[contracts][math]")
{
    STATIC_CHECK(rounding_keeps_tail_direction<bl::fdd>());
    STATIC_CHECK(rounding_keeps_tail_direction<bl::fqd>());
    volatile double unit = 1.0;
    CHECK(rounding_keeps_tail_direction<bl::fdd>(unit));
    CHECK(rounding_keeps_tail_direction<bl::fqd>(unit));
    check_decomposition<bl::f32>();
    check_decomposition<bl::f64>();
    check_decomposition<bl::fdd>();
    check_decomposition<bl::fqd>();

    CHECK(bl::floor(bl::fdd{ -2.25 }) == bl::fdd{ -3.0 });
    CHECK(bl::ceil(bl::fdd{ -2.25 }) == bl::fdd{ -2.0 });
    CHECK(bl::trunc(bl::fqd{ -2.75 }) == bl::fqd{ -2.0 });
    CHECK(bl::round(bl::fqd{ -2.5 }) == bl::fqd{ -3.0 });
    CHECK(bl::roundeven(bl::fdd{ 2.5 }) == bl::fdd{ 2.0 });
    CHECK(bl::roundeven(bl::fdd{ 3.5 }) == bl::fdd{ 4.0 });
    CHECK(bl::lround(bl::fdd{ -2.5 }) == -3L);
    CHECK(bl::llround(bl::fqd{ 2.5 }) == 3LL);
    CHECK(bl::to_string(
        bl::round_decimals(bl::fdd{ 1.2345 }, 2),
        2,
        std::ios_base::fixed) == "1.23");
    CHECK(bl::to_string(
        bl::round_decimals(bl::fqd{ 1.2345 }, 2),
        2,
        std::ios_base::fixed) == "1.23");
    CHECK(bl::round_significant(bl::fdd{ 1234.5 }, 3) == bl::fdd{ 1230.0 });
    CHECK(bl::round_significant(bl::fqd{ 1234.5 }, 3) == bl::fqd{ 1230.0 });
}

TEST_CASE("decimal and significant-figure rounding preserve tie rules", "[contracts][math][rounding]")
{
    CHECK(bl::round_decimals(1.2345f, 2) == 1.23f);
    CHECK(bl::round_decimals(1.125f, 2) == 1.12f);
    CHECK(bl::round_decimals(1.375f, 2) == 1.38f);
    CHECK(bl::round_decimals(-1.375f, 2) == -1.38f);
    CHECK(bl::round_significant(12345.0f, 3) == 12300.0f);
    CHECK(bl::round_significant(0.012345f, 3) == 0.0123f);
    CHECK(bl::round_significant(12500.0f, 2) == 12000.0f);
    CHECK(bl::round_significant(13500.0f, 2) == 14000.0f);

    CHECK(bl::round_decimals(1.2345, 2) == 1.23);
    CHECK(bl::round_decimals(1.2345, 3) == 1.234);
    CHECK(bl::round_decimals(1.125, 2) == 1.12);
    CHECK(bl::round_decimals(1.375, 2) == 1.38);
    CHECK(bl::round_decimals(-1.375, 2) == -1.38);
    CHECK(bl::round_significant(12345.0, 3) == 12300.0);
    CHECK(bl::round_significant(0.012345, 3) == 0.0123);
    CHECK(bl::round_significant(12500.0, 2) == 12000.0);
    CHECK(bl::round_significant(13500.0, 2) == 14000.0);

    check_expansion_precision_rounding<bl::fdd>();
    check_expansion_precision_rounding<bl::fqd>();
}

TEST_CASE("integer rounding covers expansion limbs at signed boundaries", "[contracts][math][rounding]")
{
    const bl::fdd long_min_dd{
        static_cast<double>(std::numeric_limits<long>::min()), 0.5
    };
    const bl::fqd long_min_qd{
        static_cast<double>(std::numeric_limits<long>::min()), 0.5, 0.0, 0.0
    };
    CHECK(bl::lround(long_min_dd) == std::numeric_limits<long>::min());
    CHECK(bl::lround(long_min_qd) == std::numeric_limits<long>::min());

    if constexpr (std::numeric_limits<long>::digits <= 53)
    {
        const bl::fdd long_max_dd{
            static_cast<double>(std::numeric_limits<long>::max()), -0.49
        };
        const bl::fqd long_max_qd{
            static_cast<double>(std::numeric_limits<long>::max()),
            -0.49, 0.0, 0.0
        };
        CHECK(bl::lround(long_max_dd) == std::numeric_limits<long>::max());
        CHECK(bl::lround(long_max_qd) == std::numeric_limits<long>::max());
    }

    const bl::fdd long_long_min_dd{
        static_cast<double>(std::numeric_limits<long long>::min()), 0.5
    };
    const bl::fqd long_long_min_qd{
        static_cast<double>(std::numeric_limits<long long>::min()),
        0.5, 0.0, 0.0
    };
    CHECK(bl::llround(long_long_min_dd) ==
          std::numeric_limits<long long>::min());
    CHECK(bl::llround(long_long_min_qd) ==
          std::numeric_limits<long long>::min());

    CHECK(bl::llround(bl::fdd{ 0x1p52, 0.5 }) == 4503599627370497LL);
    CHECK(bl::llround(bl::fqd{ 0x1p52, 0.5, 0.0, 0.0 }) ==
          4503599627370497LL);
    CHECK(bl::llround(bl::fdd{ -0x1p52, -0.5 }) == -4503599627370497LL);
    CHECK(bl::llround(bl::fqd{ -0x1p52, -0.5, 0.0, 0.0 }) ==
          -4503599627370497LL);
    CHECK(bl::llround(bl::fdd{ 0x1p53, -0.5 }) == 9007199254740992LL);
    CHECK(bl::llround(bl::fqd{ 0x1p53, -0.5, 0.0, 0.0 }) ==
          9007199254740992LL);
}

TEST_CASE("sincos overloads agree and log_as_double is well scaled", "[contracts][math]")
{
    check_sincos<bl::f32>();
    check_sincos<bl::f64>();
    check_sincos<bl::fdd>();
    check_sincos<bl::fqd>();

    CHECK(std::abs(bl::log_as_double(bl::exp(bl::fdd{ 1.0 })) - 1.0) < 1e-14);
    CHECK(std::abs(bl::log_as_double(bl::exp(bl::fqd{ 1.0 })) - 1.0) < 1e-14);
}

TEST_CASE("public expansion math results keep nonoverlapping limbs",
          "[contracts][math][canonical]")
{
    const auto check = []<class T>() {
        const T a = expansion<T>(1.0, 0x1p-60);
        const T b = expansion<T>(-0.375, -0x1p-64);
        const T domain = expansion<T>(0.625, 0x1p-62);
        const T positive = expansion<T>(1.125, 0x1p-60);
        const T gamma_argument = expansion<T>(1.75, 0x1p-62);

        check_canonical("add", T{ a + b });
        check_canonical("subtract", T{ a - b });
        check_canonical("multiply", T{ a * domain });
        check_canonical("divide", T{ a / positive });
        check_canonical("negate", T{ -a });
        check_canonical("abs", bl::abs(b));
        check_canonical("fabs", bl::fabs(b));
        check_canonical("sqr", bl::sqr(a));
        check_canonical("recip", bl::recip(positive));
        check_canonical("clamp", bl::clamp(a, domain, positive));
        check_canonical("fma", bl::fma(a, positive, b));
        check_canonical("fmin", bl::fmin(a, b));
        check_canonical("fmax", bl::fmax(a, b));
        check_canonical("fdim", bl::fdim(a, domain));
        check_canonical("copysign", bl::copysign(a, b));
        check_canonical("floor", bl::floor(b));
        check_canonical("ceil", bl::ceil(b));
        check_canonical("trunc", bl::trunc(b));
        check_canonical("round", bl::round(b));
        check_canonical("roundeven", bl::roundeven(b));
        check_canonical("fmod", bl::fmod(a, domain));
        check_canonical("remainder", bl::remainder(a, domain));

        int quotient = 0;
        check_canonical("remquo", bl::remquo(a, domain, &quotient));
        check_canonical("sqrt", bl::sqrt(positive));
        check_canonical("cbrt", bl::cbrt(b));
        check_canonical("hypot", bl::hypot(a, b));
        check_canonical("pow", bl::pow(positive, domain));
        check_canonical("ipow", bl::ipow(positive, -3));
        check_canonical("exp", bl::exp(domain));
        check_canonical("exp2", bl::exp2(domain));
        check_canonical("expm1", bl::expm1(domain));
        check_canonical("log", bl::log(positive));
        check_canonical("log2", bl::log2(positive));
        check_canonical("log10", bl::log10(positive));
        check_canonical("log1p", bl::log1p(domain));

        const auto trig = bl::sincos<T>(domain);
        check_canonical("sincos.sin", trig.s);
        check_canonical("sincos.cos", trig.c);
        check_canonical("sin", bl::sin(domain));
        check_canonical("cos", bl::cos(domain));
        check_canonical("tan", bl::tan(domain));
        check_canonical("atan", bl::atan(domain));
        check_canonical("atan2", bl::atan2(b, a));
        check_canonical("asin", bl::asin(domain));
        check_canonical("acos", bl::acos(domain));
        check_canonical("sinh", bl::sinh(domain));
        check_canonical("cosh", bl::cosh(domain));
        check_canonical("tanh", bl::tanh(domain));
        check_canonical("asinh", bl::asinh(b));
        check_canonical("acosh", bl::acosh(positive));
        check_canonical("atanh", bl::atanh(domain));
        check_canonical("erf", bl::erf(domain));
        check_canonical("erfc", bl::erfc(domain));
        check_canonical("lgamma", bl::lgamma(gamma_argument));
        check_canonical("tgamma", bl::tgamma(gamma_argument));

        int exponent = 0;
        T integral{};
        check_canonical("frexp", bl::frexp(a, &exponent));
        check_canonical("modf.fraction", bl::modf(a, &integral));
        check_canonical("modf.integral", integral);
        check_canonical("ldexp", bl::ldexp(a, 7));
        check_canonical("scalbn", bl::scalbn(a, -7));
        check_canonical("scalbln", bl::scalbln(a, 7L));
        check_canonical("logb", bl::logb(a));
        check_canonical("nextafter", bl::nextafter(a, positive));
        check_canonical("nexttoward", bl::nexttoward(a, positive));
        check_canonical(
            "round_decimals",
            bl::round_decimals(T{ 1.23456789 }, 5));
        check_canonical(
            "round_significant",
            bl::round_significant(T{ 12345.6789 }, 5));
    };

    check.template operator()<bl::fdd>();
    check.template operator()<bl::fqd>();
}

TEST_CASE("arithmetic special values follow IEEE conventions", "[contracts][math][special]")
{
    // Native runtime wrappers inherit the active std/compiler floating-point
    // mode. Fast-math special values remain observational metrics rather than
    // a stronger FLTX contract; strict runtime and constexpr paths are checked.
#if !defined(FLTX_FAST_MATH)
    check_arithmetic_special_values<bl::f32>("f32");
    check_arithmetic_special_values<bl::f64>("f64");
#endif
    check_arithmetic_special_values<bl::fdd>("dd");
    check_arithmetic_special_values<bl::fqd>("qd");
}

TEST_CASE("roots and powers handle poles and invalid domains", "[contracts][math][special]")
{
#if !defined(FLTX_FAST_MATH)
    check_root_and_power_special_values<bl::f32>("f32");
    check_root_and_power_special_values<bl::f64>("f64");
#endif
    check_root_and_power_special_values<bl::fdd>("dd");
    check_root_and_power_special_values<bl::fqd>("qd");
}

TEST_CASE("transcendentals preserve signed outputs and domain semantics", "[contracts][math][special]")
{
#if !defined(FLTX_FAST_MATH)
    check_transcendental_special_values<bl::f32>("f32");
    check_transcendental_special_values<bl::f64>("f64");
#endif
    check_transcendental_special_values<bl::fdd>("dd");
    check_transcendental_special_values<bl::fqd>("qd");
}

TEST_CASE("rounding and remainder special values preserve signs", "[contracts][math][special]")
{
#if !defined(FLTX_FAST_MATH)
    check_rounding_and_remainder_special_values<bl::f32>("f32");
    check_rounding_and_remainder_special_values<bl::f64>("f64");
#endif
    check_rounding_and_remainder_special_values<bl::fdd>("dd");
    check_rounding_and_remainder_special_values<bl::fqd>("qd");
}

TEST_CASE("fixed rounding ignores the process rounding mode", "[contracts][math][rounding]")
{
#if defined(__EMSCRIPTEN__)
    // WebAssembly has a fixed nearest-even environment and does not expose
    // mutable hardware rounding modes.
    CHECK(bl::roundeven(2.5) == 2.0);
    CHECK(bl::round(bl::fdd{ 2.5 }) == bl::fdd{ 3.0 });
    CHECK(bl::floor(bl::fqd{ -2.25 }) == bl::fqd{ -3.0 });
#else
    struct guard
    {
        int previous = std::fegetround();
        ~guard() { std::fesetround(previous); }
    } restore;

    constexpr std::array modes{ FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO };
    for (const int mode : modes)
    {
        REQUIRE(std::fesetround(mode) == 0);
        CHECK(bl::roundeven(2.5) == 2.0);
        CHECK(bl::round(bl::fdd{ 2.5 }) == bl::fdd{ 3.0 });
        CHECK(bl::floor(bl::fqd{ -2.25 }) == bl::fqd{ -3.0 });
    }
#endif
}
