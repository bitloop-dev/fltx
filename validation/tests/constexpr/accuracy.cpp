#include "mpfr.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <limits>
#include <string_view>
#include <type_traits>

#include <fltx/math.h>

namespace
{
    namespace oracle = fltx::tests::mpfr;
    namespace thresholds = fltx::tests::thresholds;

    enum class unary_op
    {
        sqrt,
        exp,
        expm1,
        log,
        log1p,
        sin,
        cos,
        tan,
        atan,
        asin,
        acos,
        sinh,
        cosh,
        tanh,
        asinh,
        acosh,
        atanh,
        cbrt,
        erf,
        erfc,
        lgamma,
        tgamma
    };

    enum class binary_op
    {
        add,
        subtract,
        multiply,
        divide,
        fmod,
        remainder,
        pow,
        atan2,
        hypot
    };

    template<class Float>
    struct unary_case
    {
        std::string_view label;
        unary_op operation;
        Float input;
        Float result;
    };

    template<class Float>
    struct binary_case
    {
        std::string_view label;
        binary_op operation;
        Float lhs;
        Float rhs;
        Float result;
    };

    [[nodiscard]] constexpr std::string_view name(unary_op operation) noexcept
    {
        switch (operation)
        {
        case unary_op::sqrt:   return "sqrt";
        case unary_op::exp:    return "exp";
        case unary_op::expm1:  return "expm1";
        case unary_op::log:    return "log";
        case unary_op::log1p:  return "log1p";
        case unary_op::sin:    return "sin";
        case unary_op::cos:    return "cos";
        case unary_op::tan:    return "tan";
        case unary_op::atan:   return "atan";
        case unary_op::asin:   return "asin";
        case unary_op::acos:   return "acos";
        case unary_op::sinh:   return "sinh";
        case unary_op::cosh:   return "cosh";
        case unary_op::tanh:   return "tanh";
        case unary_op::asinh:  return "asinh";
        case unary_op::acosh:  return "acosh";
        case unary_op::atanh:  return "atanh";
        case unary_op::cbrt:   return "cbrt";
        case unary_op::erf:    return "erf";
        case unary_op::erfc:   return "erfc";
        case unary_op::lgamma: return "lgamma";
        case unary_op::tgamma: return "tgamma";
        }
        return "unknown";
    }

    [[nodiscard]] constexpr std::string_view name(binary_op operation) noexcept
    {
        switch (operation)
        {
        case binary_op::add:       return "add";
        case binary_op::subtract:  return "subtract";
        case binary_op::multiply:  return "multiply";
        case binary_op::divide:    return "divide";
        case binary_op::fmod:      return "fmod";
        case binary_op::remainder: return "remainder";
        case binary_op::pow:       return "pow";
        case binary_op::atan2:     return "atan2";
        case binary_op::hypot:     return "hypot";
        }
        return "unknown";
    }

    template<class Float>
    [[nodiscard]] constexpr Float fp(double value)
    {
        return Float{ value };
    }

    template<class Float>
    [[nodiscard]] constexpr Float q(double numerator, double denominator)
    {
        return fp<Float>(numerator) / fp<Float>(denominator);
    }

    template<class Float>
    [[nodiscard]] constexpr Float mixed(
        double whole,
        double numerator,
        double denominator)
    {
        return fp<Float>(whole) + q<Float>(numerator, denominator);
    }

    template<class Float>
    [[nodiscard]] constexpr Float evaluate(unary_op operation, Float input)
    {
        switch (operation)
        {
        case unary_op::sqrt:   return bl::sqrt(input);
        case unary_op::exp:    return bl::exp(input);
        case unary_op::expm1:  return bl::expm1(input);
        case unary_op::log:    return bl::log(input);
        case unary_op::log1p:  return bl::log1p(input);
        case unary_op::sin:    return bl::sin(input);
        case unary_op::cos:    return bl::cos(input);
        case unary_op::tan:    return bl::tan(input);
        case unary_op::atan:   return bl::atan(input);
        case unary_op::asin:   return bl::asin(input);
        case unary_op::acos:   return bl::acos(input);
        case unary_op::sinh:   return bl::sinh(input);
        case unary_op::cosh:   return bl::cosh(input);
        case unary_op::tanh:   return bl::tanh(input);
        case unary_op::asinh:  return bl::asinh(input);
        case unary_op::acosh:  return bl::acosh(input);
        case unary_op::atanh:  return bl::atanh(input);
        case unary_op::cbrt:   return bl::cbrt(input);
        case unary_op::erf:    return bl::erf(input);
        case unary_op::erfc:   return bl::erfc(input);
        case unary_op::lgamma: return bl::lgamma(input);
        case unary_op::tgamma: return bl::tgamma(input);
        }
        return std::numeric_limits<Float>::quiet_NaN();
    }

    template<class Float>
    [[nodiscard]] constexpr Float evaluate(
        binary_op operation,
        Float lhs,
        Float rhs)
    {
        switch (operation)
        {
        case binary_op::add:       return lhs + rhs;
        case binary_op::subtract:  return lhs - rhs;
        case binary_op::multiply:  return lhs * rhs;
        case binary_op::divide:    return lhs / rhs;
        case binary_op::fmod:      return bl::fmod(lhs, rhs);
        case binary_op::remainder: return bl::remainder(lhs, rhs);
        case binary_op::pow:       return bl::pow(lhs, rhs);
        case binary_op::atan2:     return bl::atan2(lhs, rhs);
        case binary_op::hypot:     return bl::hypot(lhs, rhs);
        }
        return std::numeric_limits<Float>::quiet_NaN();
    }

    template<class Float>
    [[nodiscard]] constexpr unary_case<Float> make_case(
        std::string_view label,
        unary_op operation,
        Float input)
    {
        return { label, operation, input, evaluate(operation, input) };
    }

    template<class Float>
    [[nodiscard]] constexpr binary_case<Float> make_case(
        std::string_view label,
        binary_op operation,
        Float lhs,
        Float rhs)
    {
        return { label, operation, lhs, rhs, evaluate(operation, lhs, rhs) };
    }

    template<class Float>
    [[nodiscard]] constexpr auto core_unary_cases()
    {
        return std::array{
            make_case<Float>("sqrt tiny scaled", unary_op::sqrt,
                bl::ldexp(mixed<Float>(1.0, 1.0, 3.0), -80)),
            make_case<Float>("sqrt two", unary_op::sqrt, fp<Float>(2.0)),
            make_case<Float>("sqrt large scaled", unary_op::sqrt,
                bl::ldexp(mixed<Float>(1.0, 1.0, 5.0), 180)),
            make_case<Float>("exp negative", unary_op::exp, q<Float>(-5.0, 4.0)),
            make_case<Float>("exp positive", unary_op::exp, q<Float>(3.0, 2.0)),
            make_case<Float>("expm1 moderate negative", unary_op::expm1, q<Float>(-1.0, 2.0)),
            make_case<Float>("expm1 moderate positive", unary_op::expm1, q<Float>(1.0, 4.0)),
            make_case<Float>("log subunit", unary_op::log, q<Float>(1.0, 8.0)),
            make_case<Float>("log mixed", unary_op::log, mixed<Float>(1.0, 1.0, 7.0)),
            make_case<Float>("log1p negative", unary_op::log1p, q<Float>(-3.0, 4.0)),
            make_case<Float>("log1p positive", unary_op::log1p, q<Float>(5.0, 4.0)),
            make_case<Float>("sin reduced negative", unary_op::sin, q<Float>(-3.0, 4.0)),
            make_case<Float>("sin reduced positive", unary_op::sin, q<Float>(1.0, 2.0)),
            make_case<Float>("cos reduced negative", unary_op::cos, q<Float>(-3.0, 4.0)),
            make_case<Float>("cos reduced positive", unary_op::cos, q<Float>(1.0, 2.0)),
            make_case<Float>("tan reduced", unary_op::tan, q<Float>(1.0, 4.0)),
            make_case<Float>("atan negative", unary_op::atan, q<Float>(-5.0, 8.0)),
            make_case<Float>("atan positive", unary_op::atan, q<Float>(3.0, 4.0))
        };
    }

    template<class Float>
    [[nodiscard]] constexpr auto inverse_unary_cases()
    {
        return std::array{
            make_case<Float>("asin negative", unary_op::asin, q<Float>(-1.0, 4.0)),
            make_case<Float>("asin positive", unary_op::asin, q<Float>(1.0, 2.0)),
            make_case<Float>("acos negative", unary_op::acos, q<Float>(-1.0, 4.0)),
            make_case<Float>("acos positive", unary_op::acos, q<Float>(1.0, 2.0)),
            make_case<Float>("asinh negative", unary_op::asinh, q<Float>(-5.0, 4.0)),
            make_case<Float>("asinh positive", unary_op::asinh, q<Float>(3.0, 4.0)),
            make_case<Float>("acosh near one", unary_op::acosh, mixed<Float>(1.0, 1.0, 16.0)),
            make_case<Float>("acosh moderate", unary_op::acosh, q<Float>(5.0, 2.0)),
            make_case<Float>("atanh negative", unary_op::atanh, q<Float>(-1.0, 4.0)),
            make_case<Float>("atanh positive", unary_op::atanh, q<Float>(1.0, 2.0))
        };
    }

    template<class Float>
    [[nodiscard]] constexpr auto hyperbolic_unary_cases()
    {
        return std::array{
            make_case<Float>("sinh negative", unary_op::sinh, q<Float>(-3.0, 4.0)),
            make_case<Float>("sinh positive", unary_op::sinh, q<Float>(5.0, 4.0)),
            make_case<Float>("cosh small", unary_op::cosh, q<Float>(1.0, 2.0)),
            make_case<Float>("cosh moderate", unary_op::cosh, q<Float>(5.0, 4.0)),
            make_case<Float>("tanh negative", unary_op::tanh, q<Float>(-3.0, 4.0)),
            make_case<Float>("tanh positive", unary_op::tanh, q<Float>(5.0, 4.0))
        };
    }

    template<class Float>
    [[nodiscard]] constexpr auto special_unary_cases()
    {
        return std::array{
            make_case<Float>("cbrt negative", unary_op::cbrt, fp<Float>(-27.0)),
            make_case<Float>("cbrt positive", unary_op::cbrt, q<Float>(5.0, 8.0)),
            make_case<Float>("erf negative", unary_op::erf, q<Float>(-5.0, 4.0)),
            make_case<Float>("erf positive", unary_op::erf, q<Float>(1.0, 2.0)),
            make_case<Float>("erfc negative", unary_op::erfc, q<Float>(-5.0, 4.0)),
            make_case<Float>("erfc positive", unary_op::erfc, q<Float>(1.0, 2.0)),
            make_case<Float>("lgamma small", unary_op::lgamma, q<Float>(1.0, 8.0)),
            make_case<Float>("lgamma moderate", unary_op::lgamma, q<Float>(15.0, 4.0)),
            make_case<Float>("tgamma half", unary_op::tgamma, q<Float>(1.0, 2.0)),
            make_case<Float>("tgamma moderate", unary_op::tgamma, q<Float>(9.0, 4.0))
        };
    }

    template<class Float>
    [[nodiscard]] constexpr auto arithmetic_binary_cases()
    {
        return std::array{
            make_case<Float>("add cancellation", binary_op::add,
                mixed<Float>(1.0, 1.0, 7.0), q<Float>(-8.0, 7.0)),
            make_case<Float>("subtract close", binary_op::subtract,
                mixed<Float>(1.0, 1.0, 4096.0), fp<Float>(1.0)),
            make_case<Float>("multiply mixed", binary_op::multiply,
                mixed<Float>(3.0, 1.0, 7.0), q<Float>(-11.0, 13.0)),
            make_case<Float>("divide mixed", binary_op::divide,
                mixed<Float>(7.0, 2.0, 11.0), mixed<Float>(3.0, 1.0, 5.0))
        };
    }

    template<class Float>
    [[nodiscard]] constexpr auto reduction_binary_cases()
    {
        return std::array{
            make_case<Float>("fmod simple", binary_op::fmod,
                mixed<Float>(5.0, 1.0, 4.0), fp<Float>(2.0)),
            make_case<Float>("fmod fractional divisor", binary_op::fmod,
                mixed<Float>(17.0, 1.0, 7.0), mixed<Float>(3.0, 1.0, 11.0)),
            make_case<Float>("fmod scaled quotient", binary_op::fmod,
                bl::ldexp(mixed<Float>(1.0, 3.0, 17.0), 48),
                mixed<Float>(5.0, 1.0, 8.0)),
            make_case<Float>("remainder simple", binary_op::remainder,
                mixed<Float>(17.0, 1.0, 7.0), mixed<Float>(3.0, 1.0, 11.0))
        };
    }

    template<class Float>
    [[nodiscard]] constexpr auto transcendental_binary_cases()
    {
        return std::array{
            make_case<Float>("pow fractional", binary_op::pow,
                mixed<Float>(1.0, 1.0, 4.0), mixed<Float>(1.0, 1.0, 8.0)),
            make_case<Float>("pow reciprocal", binary_op::pow,
                q<Float>(5.0, 2.0), q<Float>(-3.0, 2.0)),
            make_case<Float>("atan2 quadrant one", binary_op::atan2,
                q<Float>(1.0, 2.0), mixed<Float>(1.0, 1.0, 4.0)),
            make_case<Float>("atan2 quadrant three", binary_op::atan2,
                q<Float>(-3.0, 4.0), q<Float>(-5.0, 4.0)),
            make_case<Float>("hypot 3 4", binary_op::hypot,
                fp<Float>(3.0), fp<Float>(4.0)),
            make_case<Float>("hypot fractional", binary_op::hypot,
                q<Float>(5.0, 8.0), q<Float>(7.0, 9.0))
        };
    }

    template<class Float>
    struct corpus_traits;

    template<>
    struct corpus_traits<bl::f128_s>
    {
        using public_type = bl::f128;
        static constexpr thresholds::precision precision = thresholds::precision::f128;
        static constexpr std::string_view label = "f128";
    };

    template<>
    struct corpus_traits<bl::f256_s>
    {
        using public_type = bl::f256;
        static constexpr thresholds::precision precision = thresholds::precision::f256;
        static constexpr std::string_view label = "f256";
    };

    template<class Float>
    [[nodiscard]] oracle::real to_real(const Float& value)
    {
        using public_type = typename corpus_traits<Float>::public_type;
        return oracle::traits<public_type>::to_real(value);
    }

    template<class Float>
    [[nodiscard]] double matched_bits(
        const Float& observed,
        const oracle::real& expected)
    {
        using traits = corpus_traits<Float>;
        using public_type = typename traits::public_type;
        return oracle::resolution_adjusted_bits(
            to_real(observed),
            expected,
            oracle::absolute_resolution<public_type>(),
            thresholds::target_bits(traits::precision));
    }

    [[nodiscard]] oracle::real evaluate(unary_op operation, const oracle::real& input)
    {
        using boost::multiprecision::abs;
        using boost::multiprecision::acos;
        using boost::multiprecision::acosh;
        using boost::multiprecision::asin;
        using boost::multiprecision::asinh;
        using boost::multiprecision::atan;
        using boost::multiprecision::atanh;
        using boost::multiprecision::cos;
        using boost::multiprecision::cosh;
        using boost::multiprecision::erf;
        using boost::multiprecision::erfc;
        using boost::multiprecision::exp;
        using boost::multiprecision::expm1;
        using boost::multiprecision::lgamma;
        using boost::multiprecision::log;
        using boost::multiprecision::log1p;
        using boost::multiprecision::pow;
        using boost::multiprecision::sin;
        using boost::multiprecision::sinh;
        using boost::multiprecision::sqrt;
        using boost::multiprecision::tan;
        using boost::multiprecision::tanh;
        using boost::multiprecision::tgamma;

        switch (operation)
        {
        case unary_op::sqrt:   return sqrt(input);
        case unary_op::exp:    return exp(input);
        case unary_op::expm1:  return expm1(input);
        case unary_op::log:    return log(input);
        case unary_op::log1p:  return log1p(input);
        case unary_op::sin:    return sin(input);
        case unary_op::cos:    return cos(input);
        case unary_op::tan:    return tan(input);
        case unary_op::atan:   return atan(input);
        case unary_op::asin:   return asin(input);
        case unary_op::acos:   return acos(input);
        case unary_op::sinh:   return sinh(input);
        case unary_op::cosh:   return cosh(input);
        case unary_op::tanh:   return tanh(input);
        case unary_op::asinh:  return asinh(input);
        case unary_op::acosh:  return acosh(input);
        case unary_op::atanh:  return atanh(input);
        case unary_op::cbrt:   return oracle::cbrt(input);
        case unary_op::erf:    return erf(input);
        case unary_op::erfc:   return erfc(input);
        case unary_op::lgamma: return lgamma(input);
        case unary_op::tgamma: return tgamma(input);
        }
        return std::numeric_limits<oracle::real>::quiet_NaN();
    }

    [[nodiscard]] oracle::real evaluate(
        binary_op operation,
        const oracle::real& lhs,
        const oracle::real& rhs)
    {
        using boost::multiprecision::atan2;
        using boost::multiprecision::pow;

        switch (operation)
        {
        case binary_op::add:       return lhs + rhs;
        case binary_op::subtract:  return lhs - rhs;
        case binary_op::multiply:  return lhs * rhs;
        case binary_op::divide:    return lhs / rhs;
        case binary_op::fmod:      return oracle::fmod(lhs, rhs);
        case binary_op::remainder: return oracle::remainder(lhs, rhs);
        case binary_op::pow:       return pow(lhs, rhs);
        case binary_op::atan2:     return atan2(lhs, rhs);
        case binary_op::hypot:     return oracle::hypot(lhs, rhs);
        }
        return std::numeric_limits<oracle::real>::quiet_NaN();
    }

    template<class Float, std::size_t Size>
    void check(const std::array<unary_case<Float>, Size>& cases)
    {
        for (const auto& test : cases)
        {
            const oracle::real expected = evaluate(test.operation, to_real(test.input));
            const double bits = matched_bits(test.result, expected);
            const double required = thresholds::required(
                corpus_traits<Float>::precision,
                name(test.operation),
                "moderate");

            INFO("type=" << corpus_traits<Float>::label);
            INFO("operation=" << name(test.operation));
            INFO("case=" << test.label);
            INFO("constexpr result=" << oracle::text(to_real(test.result)));
            INFO("expected=" << oracle::text(expected));
            INFO("matched bits=" << bits);
            INFO("required bits=" << required);
            CHECK(bits >= required);
        }
    }

    template<class Float, std::size_t Size>
    void check(const std::array<binary_case<Float>, Size>& cases)
    {
        for (const auto& test : cases)
        {
            const oracle::real expected = evaluate(
                test.operation,
                to_real(test.lhs),
                to_real(test.rhs));
            const double bits = matched_bits(test.result, expected);
            const double required = thresholds::required(
                corpus_traits<Float>::precision,
                name(test.operation),
                test.operation == binary_op::add ? "cancellation" : "moderate");

            INFO("type=" << corpus_traits<Float>::label);
            INFO("operation=" << name(test.operation));
            INFO("case=" << test.label);
            INFO("constexpr result=" << oracle::text(to_real(test.result)));
            INFO("expected=" << oracle::text(expected));
            INFO("matched bits=" << bits);
            INFO("required bits=" << required);
            CHECK(bits >= required);
        }
    }

    template<class Float>
    void check_corpus()
    {
        constexpr auto core = core_unary_cases<Float>();
        constexpr auto inverse = inverse_unary_cases<Float>();
        constexpr auto hyperbolic = hyperbolic_unary_cases<Float>();
        constexpr auto special = special_unary_cases<Float>();
        constexpr auto arithmetic = arithmetic_binary_cases<Float>();
        constexpr auto reduction = reduction_binary_cases<Float>();
        constexpr auto transcendental = transcendental_binary_cases<Float>();

        static_assert(core.size() + inverse.size() + hyperbolic.size() +
                      special.size() + arithmetic.size() + reduction.size() +
                      transcendental.size() == 58);

        check(core);
        check(inverse);
        check(hyperbolic);
        check(special);
        check(arithmetic);
        check(reduction);
        check(transcendental);
    }
}

TEST_CASE("MPFR observation preserves native values by representation",
          "[constexpr][oracle]")
{
    namespace native_fp = fltx::tests::native_fp;
    using boost::multiprecision::ldexp;

    const double negative_zero = native_fp::signed_zero<double>(true);
    const double denorm = native_fp::denorm_min<double>();
    const double infinity = native_fp::positive_infinity<double>();
    const double nan = native_fp::quiet_nan<double>();

    const oracle::real observed_zero = oracle::native_float_to_real(negative_zero);
    CHECK(observed_zero == 0);
    CHECK(oracle::sign_bit(observed_zero));
    CHECK(oracle::native_float_to_real(denorm) == ldexp(oracle::real{1}, -1074));
    CHECK(oracle::is_inf(oracle::native_float_to_real(infinity)));
    CHECK(oracle::is_nan(oracle::native_float_to_real(nan)));
    CHECK(oracle::is_exact_score(oracle::exact_score()));
}

TEST_CASE("genuine constexpr f128 corpus meets MPFR accuracy gates",
          "[constexpr][accuracy][corpus][f128]")
{
    check_corpus<bl::f128_s>();
}

TEST_CASE("genuine constexpr f256 corpus meets MPFR accuracy gates",
          "[constexpr][accuracy][corpus][f256]")
{
    check_corpus<bl::f256_s>();
}
