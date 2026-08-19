#ifndef FLTX_TESTS_SUPPORT_THRESHOLDS_INCLUDED
#define FLTX_TESTS_SUPPORT_THRESHOLDS_INCLUDED

#include <string_view>

namespace fltx::tests::thresholds
{
    enum class precision
    {
        f32,
        f64,
        f128,
        f256
    };

    [[nodiscard]] constexpr double target_bits(precision value) noexcept
    {
        if (value == precision::f32)
            return 24.0;
        if (value == precision::f64)
            return 53.0;
        return value == precision::f128 ? 106.0 : 212.0;
    }

    [[nodiscard]] constexpr double required(
        precision value,
        std::string_view operation,
        std::string_view domain) noexcept
    {
        if (value == precision::f32)
        {
            if (domain == "argument_reduction" && operation == "tan")
                return 18.0;
            return 20.0;
        }
        if (value == precision::f64)
        {
            if (domain == "argument_reduction" && operation == "tan")
                return 45.0;
            return 48.0;
        }

        const bool f128 = value == precision::f128;
        const double arithmetic = f128 ? 90.0 : 190.0;
        const double transcendental = f128 ? 80.0 : 180.0;

        if (operation == "parse")
            return target_bits(value);

        if (domain == "argument_reduction")
        {
            if (operation == "tan")
                return f128 ? 75.0 : 170.0;
            return transcendental;
        }
        if (domain == "cancellation")
        {
            if (operation == "expm1" || operation == "log1p")
                return transcendental;
            return arithmetic;
        }
        if (domain == "boundary")
            return transcendental;
        if (operation == "tan")
            return f128 ? 75.0 : 170.0;
        if (operation == "sin" || operation == "cos" ||
            operation == "atan" || operation == "atan2" ||
            operation == "asin" || operation == "acos" ||
            operation == "exp" || operation == "exp2" ||
            operation == "expm1" || operation == "log" ||
            operation == "log2" || operation == "log10" ||
            operation == "log1p" || operation == "pow" ||
            operation == "sinh" || operation == "cosh" ||
            operation == "tanh" || operation == "asinh" ||
            operation == "acosh" || operation == "atanh" ||
            operation == "erf" || operation == "erfc" ||
            operation == "lgamma" || operation == "tgamma")
        {
            return transcendental;
        }
        return arithmetic;
    }
}

#endif
