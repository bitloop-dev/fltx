#ifndef FLTX_TESTS_ACCURACY_SPECIAL_VALUES_INCLUDED
#define FLTX_TESTS_ACCURACY_SPECIAL_VALUES_INCLUDED

#include "../support/implementations.hpp"

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace fltx::tests::accuracy::special_values
{
    struct support
    {
        bool applicable = false;
        bool infinity = false;
        bool nan = false;

        [[nodiscard]] std::string category() const
        {
            if (!applicable)
                return "-";
            if (infinity && nan)
                return "Both";
            if (infinity)
                return "Inf";
            if (nan)
                return "NaN";
            return "No";
        }
    };

    [[nodiscard]] inline bool same_value(
        const mpfr::real& observed,
        double expected)
    {
        if (std::isnan(expected))
            return mpfr::is_nan(observed);
        if (std::isinf(expected))
        {
            return mpfr::is_inf(observed) &&
                   mpfr::sign_bit(observed) == std::signbit(expected);
        }
        if (expected == 0.0)
        {
            return observed == 0 &&
                   mpfr::sign_bit(observed) == std::signbit(expected);
        }
        return static_cast<double>(observed) == expected;
    }

    template<class Value>
    [[nodiscard]] Value input(double value)
    {
        return implementations::value_traits<Value>::from_sample(
            make_exact_sample(value));
    }

    template<class Value>
    [[nodiscard]] bool same_value(const Value& observed, double expected)
    {
        return same_value(
            implementations::value_traits<Value>::to_real(observed),
            expected);
    }

    [[nodiscard]] inline std::optional<double> unary_expected(
        std::string_view operation,
        double x)
    {
        // These operations return integers. Infinity and NaN therefore have
        // no representable result category to propagate; any errno, exception,
        // or sentinel contract belongs in API tests rather than this column.
        if (operation == "lround" || operation == "llround" ||
            operation == "ilogb")
            return std::nullopt;
        if (operation == "abs" || operation == "fabs")
            return std::fabs(x);
        if (operation == "sqr")
            return x * x;
        if (operation == "recip")
            return 1.0 / x;
        if (operation == "floor")
            return std::floor(x);
        if (operation == "ceil")
            return std::ceil(x);
        if (operation == "trunc")
            return std::trunc(x);
        if (operation == "round")
            return std::round(x);
        if (operation == "roundeven")
            return std::nearbyint(x);
        if (operation == "round_decimals" ||
            operation == "round_significant")
            return x;
        if (operation == "ldexp" || operation == "scalbn" ||
            operation == "scalbln")
            return std::ldexp(x, 17);
        if (operation == "logb")
            return std::logb(x);
        if (operation == "sqrt")
            return std::sqrt(x);
        if (operation == "cbrt")
            return std::cbrt(x);
        if (operation == "sin")
            return std::sin(x);
        if (operation == "cos")
            return std::cos(x);
        if (operation == "tan")
            return std::tan(x);
        if (operation == "atan")
            return std::atan(x);
        if (operation == "asin")
            return std::asin(x);
        if (operation == "acos")
            return std::acos(x);
        if (operation == "exp")
            return std::exp(x);
        if (operation == "exp2")
            return std::exp2(x);
        if (operation == "expm1")
            return std::expm1(x);
        if (operation == "log")
            return std::log(x);
        if (operation == "log2")
            return std::log2(x);
        if (operation == "log10")
            return std::log10(x);
        if (operation == "log1p")
            return std::log1p(x);
        if (operation == "ipow")
            return std::pow(x, 7);
        if (operation == "sinh")
            return std::sinh(x);
        if (operation == "cosh")
            return std::cosh(x);
        if (operation == "tanh")
            return std::tanh(x);
        if (operation == "asinh")
            return std::asinh(x);
        if (operation == "acosh")
            return std::acosh(x);
        if (operation == "atanh")
            return std::atanh(x);
        if (operation == "erf")
            return std::erf(x);
        if (operation == "erfc")
            return std::erfc(x);
        if (operation == "lgamma")
            return std::lgamma(x);
        if (operation == "tgamma")
            return std::tgamma(x);
        return std::nullopt;
    }

    [[nodiscard]] inline std::optional<double> binary_expected(
        std::string_view operation,
        double x,
        double y)
    {
        if (operation == "add")
            return x + y;
        if (operation == "subtract")
            return x - y;
        if (operation == "multiply")
            return x * y;
        if (operation == "divide")
            return x / y;
        if (operation == "fmin")
            return std::fmin(x, y);
        if (operation == "fmax")
            return std::fmax(x, y);
        if (operation == "fdim")
            return std::fdim(x, y);
        if (operation == "copysign")
            return std::copysign(x, y);
        if (operation == "hypot")
            return std::hypot(x, y);
        if (operation == "atan2")
            return std::atan2(x, y);
        if (operation == "pow")
            return std::pow(x, y);
        if (operation == "fmod")
            return std::fmod(x, y);
        if (operation == "remainder")
            return std::remainder(x, y);
        return std::nullopt;
    }

    [[nodiscard]] inline std::optional<bool> predicate_expected(
        std::string_view operation,
        double x,
        double y)
    {
        const auto bits = [](double value) {
            return std::bit_cast<std::uint64_t>(value);
        };
        const auto magnitude = [&](double value) {
            return bits(value) & 0x7fffffffffffffffULL;
        };
        const auto is_nan = [&](double value) {
            return magnitude(value) > 0x7ff0000000000000ULL;
        };
        const auto equal = [&](double lhs, double rhs) {
            if (is_nan(lhs) || is_nan(rhs))
                return false;
            return (magnitude(lhs) == 0 && magnitude(rhs) == 0) ||
                   bits(lhs) == bits(rhs);
        };
        const auto less = [&](double lhs, double rhs) {
            if (is_nan(lhs) || is_nan(rhs) || equal(lhs, rhs))
                return false;
            const bool lhs_negative = (bits(lhs) >> 63) != 0;
            const bool rhs_negative = (bits(rhs) >> 63) != 0;
            if (lhs_negative != rhs_negative)
                return lhs_negative;
            return lhs_negative
                ? magnitude(lhs) > magnitude(rhs)
                : magnitude(lhs) < magnitude(rhs);
        };

        // Derive IEEE comparison semantics from encodings so the oracle itself
        // remains valid when this consumer translation unit uses fast-math.
        if (operation == "equal")
            return equal(x, y);
        if (operation == "not_equal")
            return !equal(x, y);
        if (operation == "less")
            return less(x, y);
        if (operation == "less_equal")
            return less(x, y) || equal(x, y);
        if (operation == "greater")
            return less(y, x);
        if (operation == "greater_equal")
            return less(y, x) || equal(x, y);
        return std::nullopt;
    }

    template<class Value>
    [[nodiscard]] std::optional<double> at_value_precision(
        std::optional<double> expected)
    {
        if constexpr (implementations::value_traits<Value>::nominal_bits <= 24.0)
        {
            if (expected && std::isfinite(*expected))
                *expected = static_cast<double>(static_cast<float>(*expected));
        }
        return expected;
    }

    template<class Value, class Eval>
    [[nodiscard]] support unary(std::string_view operation, Eval evaluate)
    {
        const auto expected = [&](double value) {
            return at_value_precision<Value>(unary_expected(operation, value));
        };
        if (!expected(1.0))
            return {};

        bool infinity = true;
        for (const double value : {
                 std::numeric_limits<double>::infinity(),
                 -std::numeric_limits<double>::infinity() })
        {
            try
            {
                infinity = infinity &&
                    same_value(Value(evaluate(input<Value>(value))),
                               *expected(value));
            }
            catch (...)
            {
                infinity = false;
            }
        }

        bool nan = true;
        try
        {
            const double value = std::numeric_limits<double>::quiet_NaN();
            nan = same_value(
                Value(evaluate(input<Value>(value))),
                *expected(value));
        }
        catch (...)
        {
            nan = false;
        }
        return {true, infinity, nan};
    }

    template<class Value, class Eval>
    [[nodiscard]] support binary(std::string_view operation, Eval evaluate)
    {
        const auto expected = [&](double x, double y) {
            return at_value_precision<Value>(binary_expected(operation, x, y));
        };
        if (!expected(1.0, 2.0))
            return {};

        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double nan_value = std::numeric_limits<double>::quiet_NaN();
        const std::pair<double, double> infinity_cases[] = {
            {inf, 1.0}, {-inf, 1.0}, {1.0, inf},
            {1.0, -inf}, {inf, inf}, {inf, -inf}
        };
        const std::pair<double, double> nan_cases[] = {
            {nan_value, 1.0}, {1.0, nan_value}, {nan_value, nan_value}
        };

        const auto passes = [&](const auto& cases) {
            for (const auto [x, y] : cases)
            {
                try
                {
                    if (!same_value(
                            Value(evaluate(input<Value>(x), input<Value>(y))),
                            *expected(x, y)))
                        return false;
                }
                catch (...)
                {
                    return false;
                }
            }
            return true;
        };
        return {true, passes(infinity_cases), passes(nan_cases)};
    }

    template<class Value, class Eval>
    [[nodiscard]] support predicate(std::string_view operation, Eval evaluate)
    {
        if (!predicate_expected(operation, 1.0, 2.0))
            return {};

        const auto matches = [&](double x, double y) {
            try
            {
                return static_cast<bool>(
                           evaluate(input<Value>(x), input<Value>(y))) ==
                       *predicate_expected(operation, x, y);
            }
            catch (...)
            {
                return false;
            }
        };

        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double nan_value = std::numeric_limits<double>::quiet_NaN();
        const std::pair<double, double> infinity_cases[] = {
            {inf, inf}, {-inf, -inf}, {inf, -inf}, {-inf, inf},
            {inf, 1.0}, {1.0, inf}, {-inf, 1.0}, {1.0, -inf}
        };
        const std::pair<double, double> nan_cases[] = {
            {nan_value, nan_value}, {nan_value, 1.0}, {1.0, nan_value},
            {nan_value, inf}, {inf, nan_value}
        };
        const auto passes = [&](const auto& cases) {
            for (const auto [x, y] : cases)
            {
                if (!matches(x, y))
                    return false;
            }
            return true;
        };
        return {true, passes(infinity_cases), passes(nan_cases)};
    }

    template<class Value, class Eval>
    [[nodiscard]] support ternary(std::string_view operation, Eval evaluate)
    {
        if (operation != "fma")
            return {};

        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double nan_value = std::numeric_limits<double>::quiet_NaN();
        const auto matches = [&](double x, double y, double z) {
            try
            {
                return same_value(
                    Value(evaluate(
                        input<Value>(x), input<Value>(y), input<Value>(z))),
                    std::fma(x, y, z));
            }
            catch (...)
            {
                return false;
            }
        };
        return {
            true,
            matches(inf, 1.0, 0.0) &&
                matches(-inf, 1.0, 0.0) &&
                matches(inf, 0.0, 1.0) &&
                matches(1.0, 1.0, inf),
            matches(nan_value, 1.0, 0.0) &&
                matches(1.0, nan_value, 0.0) &&
                matches(1.0, 1.0, nan_value)
        };
    }

    template<class Value, class Parse>
    [[nodiscard]] support parse(Parse evaluate)
    {
        const auto matches = [&](const char* text, double expected) {
            try
            {
                return same_value(Value(evaluate(std::string{text})), expected);
            }
            catch (...)
            {
                return false;
            }
        };

        constexpr double inf = std::numeric_limits<double>::infinity();
        return {
            true,
            matches("inf", inf) && matches("-inf", -inf),
            matches("nan", std::numeric_limits<double>::quiet_NaN())
        };
    }

    template<class Value, class Format>
    [[nodiscard]] support format(Format evaluate)
    {
        const auto matches = [&](double input_value) {
            try
            {
                const std::string text =
                    evaluate(input<Value>(input_value));
                return same_value(mpfr::real{text}, input_value);
            }
            catch (...)
            {
                return false;
            }
        };

        constexpr double inf = std::numeric_limits<double>::infinity();
        return {
            true,
            matches(inf) && matches(-inf),
            matches(std::numeric_limits<double>::quiet_NaN())
        };
    }

    template<class Value, class Eval>
    [[nodiscard]] support unary_pair(std::string_view operation, Eval evaluate)
    {
        if (operation != "modf" && operation != "frexp")
            return {};

        const auto matches = [&](double input_value) {
            try
            {
                const auto [first, second] = evaluate(input<Value>(input_value));
                if (operation == "modf")
                {
                    if (std::isinf(input_value))
                    {
                        return same_value(
                                   Value(first),
                                   std::copysign(0.0, input_value)) &&
                               same_value(Value(second), input_value);
                    }
                    return same_value(Value(first), input_value) &&
                           same_value(Value(second), input_value);
                }

                int exponent = 0;
                const double fraction = std::frexp(input_value, &exponent);
                return same_value(Value(first), fraction) &&
                       (!std::isfinite(input_value) ||
                        same_value(Value(second), static_cast<double>(exponent)));
            }
            catch (...)
            {
                return false;
            }
        };
        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double nan_value = std::numeric_limits<double>::quiet_NaN();
        return {
            true,
            matches(inf) && matches(-inf),
            matches(nan_value)
        };
    }

    template<class Value, class Eval>
    [[nodiscard]] support binary_pair(std::string_view operation, Eval evaluate)
    {
        if (operation != "remquo")
            return {};

        const auto matches = [&](double x, double y) {
            try
            {
                const auto [remainder, quotient] =
                    evaluate(input<Value>(x), input<Value>(y));
                int expected_quotient = 0;
                const double expected_remainder =
                    std::remquo(x, y, &expected_quotient);
                return same_value(Value(remainder), expected_remainder) &&
                       (std::isnan(expected_remainder) ||
                         same_value(
                             Value(quotient),
                             static_cast<double>(expected_quotient)));
            }
            catch (...)
            {
                return false;
            }
        };
        constexpr double inf = std::numeric_limits<double>::infinity();
        constexpr double nan_value = std::numeric_limits<double>::quiet_NaN();
        return {
            true,
            matches(inf, 1.0) && matches(1.0, inf) && matches(inf, inf),
            matches(nan_value, 1.0) && matches(1.0, nan_value)
        };
    }
}

#endif
