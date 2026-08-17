#ifndef FLTX_TESTS_ACCURACY_SIGNED_ZERO_INCLUDED
#define FLTX_TESTS_ACCURACY_SIGNED_ZERO_INCLUDED

#include "../support/implementations.hpp"
#include "../support/mpfr.hpp"
#include "../support/native_fp.hpp"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

namespace fltx::tests::accuracy::signed_zero
{
    struct support
    {
        bool applicable = false;
        bool preserved = true;

        [[nodiscard]] std::string category() const
        {
            if (!applicable)
                return "-";
            return preserved ? "yes" : "no";
        }

        void record(bool matches) noexcept
        {
            applicable = true;
            preserved = preserved && matches;
        }
    };

    template<class Value>
    [[nodiscard]] Value input_zero(bool negative)
    {
        return implementations::value_traits<Value>::from_sample(
            make_exact_sample(native_fp::signed_zero<double>(negative)));
    }

    template<class Value>
    [[nodiscard]] Value input(double value)
    {
        return implementations::value_traits<Value>::from_sample(
            make_exact_sample(value));
    }

    template<class Value>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION bool matches_zero(
        const Value& value,
        bool negative)
    {
        constexpr std::uint64_t sign_mask = UINT64_C(0x8000000000000000);
        constexpr std::uint64_t magnitude_mask = ~sign_mask;
        const auto components = implementations::observe(value);
        for (const double component : components)
        {
            if ((native_fp::bits(component) & magnitude_mask) != 0)
                return false;
        }
        return ((native_fp::bits(components[0]) & sign_mask) != 0) == negative;
    }

    template<class Value, class Evaluate>
    FLTX_VALIDATION_PRECISE_FUNCTION void record(
        support& result,
        Evaluate evaluate,
        bool negative)
    {
        try
        {
            result.record(matches_zero(Value(evaluate()), negative));
        }
        catch (...)
        {
            result.record(false);
        }
    }

    template<class Value, class Eval>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support unary(
        std::string_view operation,
        Eval evaluate)
    {
        support result{};
        const auto same_sign = [&] {
            record<Value>(result, [&] { return evaluate(input_zero<Value>(false)); }, false);
            record<Value>(result, [&] { return evaluate(input_zero<Value>(true)); }, true);
        };
        const auto positive = [&] {
            record<Value>(result, [&] { return evaluate(input_zero<Value>(false)); }, false);
            record<Value>(result, [&] { return evaluate(input_zero<Value>(true)); }, false);
        };

        if (operation == "abs" || operation == "fabs" || operation == "sqr")
            positive();
        else if (
            operation == "floor" || operation == "ceil" ||
            operation == "trunc" || operation == "round" ||
            operation == "roundeven" || operation == "round_decimals" ||
            operation == "round_significant" || operation == "ldexp" ||
            operation == "scalbn" || operation == "scalbln" ||
            operation == "sqrt" || operation == "cbrt" ||
            operation == "sin" || operation == "tan" ||
            operation == "asin" || operation == "atan" ||
            operation == "expm1" || operation == "log1p" ||
            operation == "sinh" || operation == "tanh" ||
            operation == "asinh" || operation == "atanh" ||
            operation == "erf" || operation == "ipow")
        {
            same_sign();
        }
        else if (operation == "recip")
        {
            record<Value>(result, [&] {
                return evaluate(input<Value>(native_fp::positive_infinity<double>()));
            }, false);
            record<Value>(result, [&] {
                return evaluate(input<Value>(native_fp::negative_infinity<double>()));
            }, true);
        }
        return result;
    }

    template<class Value, class Eval>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support binary(
        std::string_view operation,
        Eval evaluate)
    {
        support result{};
        const auto zero = [&](bool lhs_negative, bool rhs_negative,
                              bool expected_negative) {
            record<Value>(result, [&] {
                return evaluate(
                    input_zero<Value>(lhs_negative),
                    input_zero<Value>(rhs_negative));
            }, expected_negative);
        };
        const auto zero_value = [&](bool lhs_negative, double rhs,
                                    bool expected_negative) {
            record<Value>(result, [&] {
                return evaluate(input_zero<Value>(lhs_negative), input<Value>(rhs));
            }, expected_negative);
        };

        if (operation == "add")
        {
            zero(false, false, false);
            zero(true, true, true);
            zero(false, true, false);
            zero(true, false, false);
        }
        else if (operation == "subtract")
        {
            zero(false, false, false);
            zero(true, false, true);
            zero(false, true, false);
            zero(true, true, false);
        }
        else if (operation == "multiply" || operation == "divide")
        {
            zero_value(false, 1.0, false);
            zero_value(true, 1.0, true);
            zero_value(false, -1.0, true);
            zero_value(true, -1.0, false);
        }
        else if (operation == "fmin")
        {
            zero(false, true, true);
            zero(true, false, true);
        }
        else if (operation == "fmax")
        {
            zero(false, true, false);
            zero(true, false, false);
        }
        else if (operation == "fdim")
        {
            record<Value>(result, [&] {
                return evaluate(input<Value>(1.0), input<Value>(2.0));
            }, false);
        }
        else if (operation == "copysign")
        {
            zero(false, false, false);
            zero(false, true, true);
        }
        else if (operation == "hypot")
        {
            zero(false, true, false);
            zero(true, true, false);
        }
        else if (operation == "atan2")
        {
            zero_value(false, 1.0, false);
            zero_value(true, 1.0, true);
        }
        else if (operation == "pow")
        {
            zero_value(false, 3.0, false);
            zero_value(true, 3.0, true);
        }
        else if (operation == "fmod" || operation == "remainder")
        {
            zero_value(false, 1.0, false);
            zero_value(true, 1.0, true);
        }
        return result;
    }

    template<class Value, class Eval>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support ternary(
        std::string_view operation,
        Eval evaluate)
    {
        support result{};
        if (operation != "fma")
            return result;
        record<Value>(result, [&] {
            return evaluate(
                input_zero<Value>(false), input<Value>(1.0),
                input_zero<Value>(false));
        }, false);
        record<Value>(result, [&] {
            return evaluate(
                input_zero<Value>(true), input<Value>(1.0),
                input_zero<Value>(true));
        }, true);
        return result;
    }

    template<class Value, class Parse>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support parse(Parse evaluate)
    {
        support result{};
        record<Value>(result, [&] { return evaluate(std::string{"0"}); }, false);
        record<Value>(result, [&] { return evaluate(std::string{"-0"}); }, true);
        return result;
    }

    template<class Value, class Format>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support format(Format evaluate)
    {
        support result{};
        const auto check = [&](bool negative) {
            try
            {
                const mpfr::real parsed{evaluate(input_zero<Value>(negative))};
                result.record(
                    parsed == 0 && mpfr::sign_bit(parsed) == negative);
            }
            catch (...)
            {
                result.record(false);
            }
        };
        check(false);
        check(true);
        return result;
    }

    template<class Value, class Eval>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support unary_pair(
        std::string_view operation,
        Eval evaluate)
    {
        support result{};
        if (operation != "modf" && operation != "frexp")
            return result;
        for (const bool negative : {false, true})
        {
            try
            {
                const auto [first, second] = evaluate(input_zero<Value>(negative));
                bool matches = matches_zero(Value(first), negative);
                if (operation == "modf")
                    matches = matches && matches_zero(Value(second), negative);
                result.record(matches);
            }
            catch (...)
            {
                result.record(false);
            }
        }
        return result;
    }

    template<class Value, class Eval>
    [[nodiscard]] FLTX_VALIDATION_PRECISE_FUNCTION support binary_pair(
        std::string_view operation,
        Eval evaluate)
    {
        support result{};
        if (operation != "remquo")
            return result;
        for (const bool negative : {false, true})
        {
            try
            {
                const auto [remainder, quotient] = evaluate(
                    input_zero<Value>(negative), input<Value>(1.0));
                (void)quotient;
                result.record(matches_zero(Value(remainder), negative));
            }
            catch (...)
            {
                result.record(false);
            }
        }
        return result;
    }
}

#endif
