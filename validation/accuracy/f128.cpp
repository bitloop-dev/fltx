#include "comparison_ops.hpp"
#include "trig_domains.hpp"

#include <boost/math/special_functions/erf.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fltx/io.h>
#include <fltx/math.h>

namespace fltx::tests::accuracy
{
    namespace
    {
        using domains::domain;
        using real = mpfr::real;
        using namespace comparison_ops;

        [[nodiscard]] std::vector<domain> ordinary(std::size_t n)
        {
            std::vector<domain> out;
            out.push_back(domains::near_one(n));
            out.push_back(domains::moderate(n));
            return out;
        }

        [[nodiscard]] std::vector<domain> positive(std::size_t n)
        {
            std::vector<domain> out;
            out.push_back(domains::positive(domains::near_one(n), 0x1p-40));
            out.push_back(domains::positive(domains::wide_exponent(n, true), 0x1p-900));
            out.push_back(domains::positive(domains::extreme_finite(n, true), 0x1p-1021));
            return out;
        }

        [[nodiscard]] std::vector<domain> trig(std::size_t n)
        {
            std::vector<domain> out;
            out.push_back(domains::moderate(n));
            out.push_back(domains::argument_reduction_f128(n));
            out.push_back(
                trig_domains::quadrant_boundaries<bl::f128>(n));
            return out;
        }

        [[nodiscard]] std::vector<domain> unit(std::size_t n)
        {
            std::vector<domain> out;
            out.push_back(domains::bounded(domains::moderate(n), -0.999, 0.999));
            out.push_back(domains::boundary(n, -1.0, 1.0));
            return out;
        }

        [[nodiscard]] std::vector<domain> exponential(std::size_t n)
        {
            std::vector<domain> out;
            out.push_back(domains::moderate(n, -8.0, 8.0));
            out.push_back(domains::interval("boundary", -700.0, 700.0, n, default_seed ^ 0x15u));
            return out;
        }

        [[nodiscard]] domain nonzero_moderate(std::size_t n)
        {
            domain out = domains::moderate(n);
            for (sample& value : out.values)
            {
                if (std::abs(value.limb[0]) < 0.25)
                    value = make_exact_sample(value.limb[0] < 0.0 ? -0.25 : 0.25);
            }
            return out;
        }

        [[nodiscard]] sample as_sample(
            const bl::f128& value,
            std::string label)
        {
            return {{value.hi, value.lo, 0.0, 0.0}, std::move(label)};
        }

        [[nodiscard]] domains::binary_domain fmod_quotient_reduction_inputs()
        {
            domains::binary_domain out{
                "quotient_reduction", default_seed ^ 0x19u, {}
            };
            const bl::f128 decimal_lhs = bl::parse<bl::f128>(
                "4.6958550912494028428400315673292717414e+19");
            const bl::f128 decimal_rhs = bl::parse<bl::f128>(
                "2.9410562077176174010123838366180003482e+02");
            const double huge =
                std::nextafter(std::ldexp(1.0, 900), 0.0);

            out.values.push_back({
                as_sample(decimal_lhs, "decimal huge-quotient lhs"),
                as_sample(decimal_rhs, "decimal huge-quotient rhs")
            });
            out.values.push_back({
                make_exact_sample(huge, "maximum binary64 below 2^900"),
                make_exact_sample(-7.5, "negative double divisor")
            });
            return out;
        }

        [[nodiscard]] domain rounding_inputs(std::size_t n)
        {
            domain out{"boundary", default_seed ^ 0x18u, {}};
            out.values = {make_exact_sample(0.0, "+0"),
                          make_exact_sample(-0.0, "-0"),
                          make_exact_sample(-3.5, "-3.5 tie"),
                          make_exact_sample(-2.5, "-2.5 tie"),
                          make_exact_sample(-0.5, "-0.5 tie"),
                          make_exact_sample(0.5, "0.5 tie"),
                          make_exact_sample(2.5, "2.5 tie"),
                          make_exact_sample(3.5, "3.5 tie"),
                          {{2.5, -0x1p-54, 0.0, 0.0}, "just below 2.5"},
                          {{2.5, 0x1p-54, 0.0, 0.0}, "just above 2.5"},
                          {{-2.5, -0x1p-54, 0.0, 0.0}, "just below -2.5"},
                          {{-2.5, 0x1p-54, 0.0, 0.0}, "just above -2.5"}};

            random_bits rng(out.seed);
            for (std::size_t i = 0; i < n; ++i)
            {
                const double integer = static_cast<double>(static_cast<int>(rng.next() % 33u) - 16);
                const double fraction = static_cast<double>(rng.next() % 9u) * 0.125;
                out.values.push_back(
                    make_sample((rng.next() & 1u) != 0 ? integer + fraction : integer - fraction));
            }
            return out;
        }

        [[nodiscard]] domain long_rounding_inputs(std::size_t n)
        {
            domain out = rounding_inputs(n);
            out.values.push_back(
                {{static_cast<double>(std::numeric_limits<long>::min()), 0.5, 0.0, 0.0},
                 "long min above half"});
            if constexpr (std::numeric_limits<long>::digits <= 53)
            {
                out.values.push_back(
                    {{static_cast<double>(std::numeric_limits<long>::max()), -0.49, 0.0, 0.0},
                     "long max below half"});
            }
            else
            {
                out.values.push_back({{0x1p52, 0.5, 0.0, 0.0}, "long two52 half"});
                out.values.push_back({{0x1p53, 1.0, 0.0, 0.0}, "long two53 plus one"});
                out.values.push_back({{-0x1p53, 0.5, 0.0, 0.0}, "negative long two53 below half"});
            }
            return out;
        }

        [[nodiscard]] domain long_long_rounding_inputs(std::size_t n)
        {
            domain out = rounding_inputs(n);
            out.values.push_back(
                {{static_cast<double>(std::numeric_limits<long long>::min()), 0.5, 0.0, 0.0},
                 "long long min above half"});
            out.values.push_back({{0x1p52, 0.5, 0.0, 0.0}, "long long two52 half"});
            out.values.push_back({{-0x1p52, -0.5, 0.0, 0.0}, "negative long long two52 half"});
            out.values.push_back({{0x1p53, -0.5, 0.0, 0.0}, "long long two53 below half"});
            out.values.push_back({{0x1p53, 1.0, 0.0, 0.0}, "long long two53 plus one"});
            out.values.push_back({{-0x1p53, 0.5, 0.0, 0.0}, "negative long long two53 below half"});
            return out;
        }
    } // namespace

    int run_f128(csv_writer& output, const options& settings)
    {
        using Float = bl::f128;
        runner<bl::f128> run(output, settings);
        const std::size_t n = settings.samples;

        constexpr int parse_digits =
            std::numeric_limits<Float>::max_digits10 + 16;
        auto parse_moderate = domains::decimal_text(
            "moderate",
            n,
            parse_digits,
            -20,
            20,
            default_seed ^ 0x71u);
        parse_moderate.values.push_back(
            "1.000000000000000000000000000000012325951644078309459558258"
            "8325435348386438505485784844495356082916259765625");
        auto parse_wide = domains::decimal_text(
            "wide_exponent",
            n,
            parse_digits,
            -300,
            300,
            default_seed ^ 0x72u);
        parse_wide.values.push_back(
            "-4.215228812298638183537364477017984307424253045187e-293");
        parse_wide.values.push_back(
            "1.108604482660954417384267803154443052106302458983e-292");
        run.parse(
            "io",
            "parse",
            {
                std::move(parse_moderate),
                std::move(parse_wide)
            },
            [](const std::string& text) { return bl::parse<Float>(text); },
            qdpp<Float>(
                "qdpp read",
                [](const std::string& text) {
                    return qd_parse<qd_value<Float>>(text);
                }),
            boost_impl<Float>(
                "Boost string constructor",
                [](const std::string& text) {
                    return boost_value<Float>{text};
                }),
            tlfloat_impl<Float>(
                "TLFloat string constructor",
                [](const std::string& text) {
                    return tlfloat_ops::parse<tlfloat_value<Float>>(text);
                }));

        constexpr int format_digits =
            std::numeric_limits<Float>::max_digits10;
        run.format(
            "io",
            "to_string",
            {domains::moderate(n), domains::wide_exponent(n)},
            [=](const auto& value) {
                return bl::to_string(
                    value, format_digits, std::ios_base::scientific);
            },
            qdpp<Float>(
                "qdpp to_string",
                [=](const auto& value) {
                    return qd_to_string(value, format_digits);
                }),
            boost_impl<Float>(
                "Boost number::str",
                [=](const auto& value) {
                    return value.str(
                        format_digits, std::ios_base::scientific);
                }),
            tlfloat_impl<Float>(
                "tlfloat::to_string",
                [=](const auto& value) {
                    return tlfloat_ops::to_string(value, format_digits);
                }));
        run.format(
            "io",
            "to_chars",
            {domains::moderate(n), domains::wide_exponent(n)},
            [=](const auto& value) {
                std::array<char, 512> buffer{};
                const auto written = bl::to_chars(
                    buffer.data(),
                    buffer.data() + buffer.size(),
                    value,
                    std::chars_format::scientific,
                    format_digits);
                if (written.ec != std::errc{})
                    throw std::runtime_error("bl::to_chars failed");
                return std::string{buffer.data(), written.ptr};
            },
            qdpp<Float>(
                "qdpp write",
                [=](const auto& value) {
                    return qd_write_string(value, format_digits);
                }));

        register_comparison_accuracy<Float>(run, n);

        run.binary_pairs(
            "arithmetic", "add",
            {domains::arithmetic_general(
                domains::arithmetic_operation::add,
                n,
                std::numeric_limits<Float>::digits)},
            [](auto x, auto y) { return x + y; },
            [](const real& x, const real& y) { return x + y; },
            qdpp<Float>("qdpp operator+", [](auto x, auto y) { return x + y; }),
            boost_impl<Float>("Boost operator+", [](auto x, auto y) { return x + y; }),
            tlfloat_impl<Float>("TLFloat operator+", [](auto x, auto y) { return x + y; }));
        run.binary_pairs(
            "arithmetic", "subtract",
            {domains::arithmetic_general(
                domains::arithmetic_operation::subtract,
                n,
                std::numeric_limits<Float>::digits)},
            [](auto x, auto y) { return x - y; },
            [](const real& x, const real& y) { return x - y; },
            qdpp<Float>("qdpp operator-", [](auto x, auto y) { return x - y; }),
            boost_impl<Float>("Boost operator-", [](auto x, auto y) { return x - y; }),
            tlfloat_impl<Float>("TLFloat operator-", [](auto x, auto y) { return x - y; }));
        run.binary_pairs(
            "arithmetic", "multiply",
            {domains::arithmetic_general(
                domains::arithmetic_operation::multiply,
                n,
                std::numeric_limits<Float>::digits)},
            [](auto x, auto y) { return x * y; },
            [](const real& x, const real& y) { return x * y; },
            qdpp<Float>("qdpp operator*", [](auto x, auto y) { return x * y; }),
            boost_impl<Float>("Boost operator*", [](auto x, auto y) { return x * y; }),
            tlfloat_impl<Float>("TLFloat operator*", [](auto x, auto y) { return x * y; }));
        run.binary_pairs(
            "arithmetic", "divide",
            {domains::arithmetic_general(
                domains::arithmetic_operation::divide,
                n,
                std::numeric_limits<Float>::digits)},
            [](auto x, auto y) { return x / y; },
            [](const real& x, const real& y) { return x / y; },
            qdpp<Float>("qdpp operator/", [](auto x, auto y) { return x / y; }),
            boost_impl<Float>("Boost operator/", [](auto x, auto y) { return x / y; }),
            tlfloat_impl<Float>("TLFloat operator/", [](auto x, auto y) { return x / y; }));
        run.ternary(
            "floating_point_utilities", "fma",
            {domains::moderate(n), domains::near_equal_cancellation(n),
             domains::wide_exponent(n, false, -300, 300)},
            [](auto x, auto y, auto z) { return bl::fma(x, y, z); },
            [](const real& x, const real& y, const real& z) { return mpfr::fma(x, y, z); },
            boost_impl<Float>("boost::multiprecision::fma",
                              [](auto x, auto y, auto z) { return boost_fma(x, y, z); }),
            tlfloat_impl<Float>("tlfloat::fma",
                                [](auto x, auto y, auto z) { return tlfloat_ops::fma(x, y, z); }));
        run.unary_exact(
            "floating_point_utilities", "abs", {domains::wide_exponent(n), domains::subnormal(n)},
            [](auto x) { return bl::abs(x); },
            [](const real& x) {
                using boost::multiprecision::abs;
                return abs(x);
            },
            qdpp<Float>("qdpp abs", [](auto x) { return ::abs(x); }),
            boost_impl<Float>("boost::multiprecision::abs", [](auto x) { return boost_abs(x); }),
            tlfloat_impl<Float>("tlfloat::abs", [](auto x) { return tlfloat_ops::abs(x); }));
        run.unary_exact(
            "floating_point_utilities", "fabs", {domains::moderate(n)}, [](auto x) { return bl::fabs(x); },
            [](const real& x) {
                using boost::multiprecision::abs;
                return abs(x);
            },
            qdpp<Float>("qdpp fabs", [](auto x) { return ::fabs(x); }),
            boost_impl<Float>("boost::multiprecision::fabs", [](auto x) { return boost_fabs(x); }),
            tlfloat_impl<Float>("tlfloat::abs", [](auto x) { return tlfloat_ops::abs(x); }));
        run.unary(
            "roots_and_powers", "sqr",
            {domains::moderate(n), domains::wide_exponent(n, false, -400, 400)},
            [](auto x) { return bl::sqr(x); }, [](const real& x) { return x * x; },
            qdpp<Float>("qdpp sqr", [](auto x) { return qd_sqr(x); }));
        run.unary(
            "floating_point_utilities", "recip",
            {domains::positive(domains::near_one(n), 0.125),
             domains::positive(domains::wide_exponent(n, true, -400, 400), 0x1p-400)},
            [](auto x) { return bl::recip(x); }, [](const real& x) { return real{1} / x; },
            qdpp<Float>("qdpp inv", [](auto x) { return qd_inv(x); }));
        run.binary_exact(
            "floating_point_utilities", "fmin", {domains::moderate(n), domains::wide_exponent(n)},
            [](auto x, auto y) { return bl::fmin(x, y); },
            [](const real& x, const real& y) { return x < y ? x : y; },
            boost_impl<Float>("boost::multiprecision::fmin",
                              [](auto x, auto y) { return boost_fmin(x, y); }),
            tlfloat_impl<Float>("tlfloat::fmin",
                                [](auto x, auto y) { return tlfloat_ops::fmin(x, y); }));
        run.binary_exact(
            "floating_point_utilities", "fmax", {domains::moderate(n), domains::wide_exponent(n)},
            [](auto x, auto y) { return bl::fmax(x, y); },
            [](const real& x, const real& y) { return x > y ? x : y; },
            boost_impl<Float>("boost::multiprecision::fmax",
                              [](auto x, auto y) { return boost_fmax(x, y); }),
            tlfloat_impl<Float>("tlfloat::fmax",
                                [](auto x, auto y) { return tlfloat_ops::fmax(x, y); }));
        run.binary(
            "floating_point_utilities", "fdim", {domains::moderate(n), domains::near_equal_cancellation(n)},
            [](auto x, auto y) { return bl::fdim(x, y); },
            [](const real& x, const real& y) { return x > y ? x - y : real{0}; },
            boost_impl<Float>("boost::multiprecision::fdim",
                              [](auto x, auto y) { return boost_fdim(x, y); }),
            tlfloat_impl<Float>("tlfloat::fdim",
                                [](auto x, auto y) { return tlfloat_ops::fdim(x, y); }));
        run.binary_exact(
            "floating_point_utilities", "copysign", {domains::moderate(n)},
            [](auto x, auto y) { return bl::copysign(x, y); },
            [](const real& x, const real& y) {
                using boost::multiprecision::abs;
                return y < 0 ? -abs(x) : abs(x);
            },
            boost_impl<Float>("boost::multiprecision::copysign",
                              [](auto x, auto y) { return boost_copysign(x, y); }),
            tlfloat_impl<Float>("tlfloat::copysign",
                                [](auto x, auto y) { return tlfloat_ops::copysign(x, y); }));

        run.unary_exact(
            "rounding", "floor", {rounding_inputs(n)}, [](auto x) { return bl::floor(x); },
            [](const real& x) {
                using boost::multiprecision::floor;
                return floor(x);
            },
            qdpp<Float>("qdpp floor", [](auto x) { return ::floor(x); }),
            boost_impl<Float>("boost::multiprecision::floor",
                              [](auto x) { return boost_floor(x); }),
            tlfloat_impl<Float>("tlfloat::floor", [](auto x) { return tlfloat_ops::floor(x); }));
        run.unary_exact(
            "rounding", "ceil", {rounding_inputs(n)}, [](auto x) { return bl::ceil(x); },
            [](const real& x) {
                using boost::multiprecision::ceil;
                return ceil(x);
            },
            qdpp<Float>("qdpp ceil", [](auto x) { return ::ceil(x); }),
            boost_impl<Float>("boost::multiprecision::ceil", [](auto x) { return boost_ceil(x); }),
            tlfloat_impl<Float>("tlfloat::ceil", [](auto x) { return tlfloat_ops::ceil(x); }));
        run.unary_exact(
            "rounding", "trunc", {rounding_inputs(n)}, [](auto x) { return bl::trunc(x); },
            [](const real& x) { return mpfr::trunc(x); },
            qdpp<Float>("qdpp aint", [](auto x) { return qd_trunc(x); }),
            boost_impl<Float>("boost::multiprecision::trunc",
                              [](auto x) { return boost_trunc(x); }),
            tlfloat_impl<Float>("tlfloat::trunc", [](auto x) { return tlfloat_ops::trunc(x); }));
        run.unary_exact(
            "rounding", "round", {rounding_inputs(n)}, [](auto x) { return bl::round(x); },
            [](const real& x) { return mpfr::round_away_from_zero(x); },
            qdpp<Float>("qdpp round", [](auto x) { return ::round(x); }),
            boost_impl<Float>("boost::multiprecision::round",
                              [](auto x) { return boost_round(x); }),
            tlfloat_impl<Float>("tlfloat::round", [](auto x) { return tlfloat_ops::round(x); }));
        run.unary_exact(
            "rounding", "roundeven", {rounding_inputs(n)}, [](auto x) { return bl::roundeven(x); },
            [](const real& x) { return mpfr::round_even(x); },
            tlfloat_impl<Float>("tlfloat::rint", [](auto x) { return tlfloat_ops::roundeven(x); }));
        run.unary(
            "rounding", "round_decimals", {domains::moderate(n)},
            [](auto x) { return bl::round_decimals(x, 3); },
            [](const real& x) { return mpfr::round_decimals(x, 3); });
        run.unary(
            "rounding", "round_significant",
            {domains::wide_exponent(n, false, -100, 100)},
            [](auto x) { return bl::round_significant(x, 7); },
            [](const real& x) { return mpfr::round_significant(x, 7); });
        run.unary_exact(
            "rounding", "lround", {long_rounding_inputs(n)},
            [](auto x) { return bl::f128{static_cast<std::int64_t>(bl::lround(x))}; },
            [](const real& x) {
                const real rounded = mpfr::round_away_from_zero(x);
                return rounded == 0 ? real{0} : rounded;
            },
            boost_impl<Float>("boost::multiprecision::lround",
                              [](auto x) { return boost_lround_value(x); }));
        run.unary_exact(
            "rounding", "llround", {long_long_rounding_inputs(n)},
            [](auto x) { return bl::f128{static_cast<std::int64_t>(bl::llround(x))}; },
            [](const real& x) {
                const real rounded = mpfr::round_away_from_zero(x);
                return rounded == 0 ? real{0} : rounded;
            },
            boost_impl<Float>("boost::multiprecision::llround",
                              [](auto x) { return boost_llround_value(x); }));

        run.binary_pair(
            "remainders", "remquo", {nonzero_moderate(n)},
            [](auto x, auto y) {
                int quotient = 0;
                const bl::f128 remainder = bl::remquo(x, y, &quotient);
                const int low_bits = quotient < 0 ? -((-quotient) & 7) : (quotient & 7);
                return std::pair{remainder, bl::f128{static_cast<double>(low_bits)}};
            },
            [](const real& x, const real& y) {
                return std::pair{mpfr::remainder(x, y), real{mpfr::remquo_bits(x, y)}};
            },
            qdpp<Float>("qdpp divrem", [](auto x, auto y) { return qd_remquo(x, y); }),
            boost_impl<Float>("boost::multiprecision::remquo",
                              [](auto x, auto y) { return boost_remquo(x, y); }),
            tlfloat_impl<Float>("tlfloat::remquo",
                                [](auto x, auto y) { return tlfloat_ops::remquo(x, y); }));
        run.unary_pair_exact(
            "floating_point_utilities", "modf", {domains::moderate(n), domains::wide_exponent(n)},
            [](auto x) {
                bl::f128 integral{};
                const bl::f128 fractional = bl::modf(x, &integral);
                return std::pair{fractional, integral};
            },
            [](const real& x) {
                const real integral = mpfr::trunc(x);
                real fractional = x == 0 ? x : x - integral;
                if (fractional == 0 && x < 0)
                    fractional = real{-0.0};
                return std::pair{fractional, integral};
            },
            boost_impl<Float>("boost::multiprecision::modf", [](auto x) { return boost_modf(x); }),
            tlfloat_impl<Float>("tlfloat::modf", [](auto x) { return tlfloat_ops::modf(x); }));
        run.unary_exact(
            "floating_point_utilities", "ldexp", {domains::wide_exponent(n, false, -800, 800)},
            [](auto x) { return bl::ldexp(x, 17); }, [](const real& x) { return x * real{131072}; },
            qdpp<Float>("qdpp ldexp", [](auto x) { return ::ldexp(x, 17); }),
            boost_impl<Float>("boost::multiprecision::ldexp",
                              [](auto x) { return boost_ldexp(x, 17); }),
            tlfloat_impl<Float>("tlfloat::ldexp",
                                [](auto x) { return tlfloat_ops::ldexp(x, 17); }));
        run.unary_exact(
            "floating_point_utilities", "scalbn", {domains::wide_exponent(n, false, -800, 800)},
            [](auto x) { return bl::scalbn(x, 17); },
            [](const real& x) { return x * real{131072}; },
            qdpp<Float>("qdpp ldexp", [](auto x) { return ::ldexp(x, 17); }),
            boost_impl<Float>("boost::multiprecision::scalbn",
                              [](auto x) { return boost_scalbn(x, 17); }));
        run.unary_exact(
            "floating_point_utilities", "scalbln", {domains::wide_exponent(n, false, -800, 800)},
            [](auto x) { return bl::scalbln(x, 17L); },
            [](const real& x) { return x * real{131072}; },
            qdpp<Float>("qdpp ldexp", [](auto x) { return ::ldexp(x, 17); }),
            boost_impl<Float>("boost::multiprecision::scalbln",
                              [](auto x) { return boost_scalbln(x, 17L); }));
        run.unary_pair_exact(
            "floating_point_utilities", "frexp", {domains::wide_exponent(n), domains::subnormal(n)},
            [](auto x) {
                int exponent = 0;
                const bl::f128 fraction = bl::frexp(x, &exponent);
                return std::pair{fraction, bl::f128{static_cast<double>(exponent)}};
            },
            [](const real& x) {
                using boost::multiprecision::frexp;
                int exponent = 0;
                const real fraction = frexp(x, &exponent);
                return std::pair{fraction, real{exponent}};
            },
            boost_impl<Float>("boost::multiprecision::frexp",
                              [](auto x) { return boost_frexp(x); }),
            tlfloat_impl<Float>("tlfloat::frexp", [](auto x) { return tlfloat_ops::frexp(x); }));
        run.unary_exact(
            "floating_point_utilities", "ilogb", {nonzero_moderate(n), domains::wide_exponent(n)},
            [](auto x) { return bl::f128{static_cast<double>(bl::ilogb(x))}; },
            [](const real& x) { return real{mpfr::ilogb(x)}; },
            boost_impl<Float>("boost::multiprecision::ilogb",
                              [](auto x) { return boost_ilogb_value(x); }),
            tlfloat_impl<Float>("tlfloat::ilogb", [](auto x) { return tlfloat_ops::ilogb(x); }));
        run.unary_exact(
            "floating_point_utilities", "logb", {nonzero_moderate(n), domains::wide_exponent(n)},
            [](auto x) { return bl::logb(x); }, [](const real& x) { return real{mpfr::ilogb(x)}; },
            boost_impl<Float>("boost::multiprecision::logb", [](auto x) { return boost_logb(x); }));

        auto sqrt_domains = positive(n);
        sqrt_domains.push_back(domains::positive(domains::subnormal(n)));
        run.unary(
            "roots_and_powers", "sqrt", std::move(sqrt_domains), [](auto x) { return bl::sqrt(x); },
            [](const real& x) {
                using boost::multiprecision::sqrt;
                return sqrt(x);
            },
            qdpp<Float>("qdpp sqrt", [](auto x) { return ::sqrt(x); }),
            boost_impl<Float>("boost::multiprecision::sqrt", [](auto x) { return boost_sqrt(x); }),
            tlfloat_impl<Float>("tlfloat::sqrt", [](auto x) { return tlfloat_ops::sqrt(x); }));
        run.unary(
            "roots_and_powers", "cbrt", {domains::moderate(n), domains::wide_exponent(n)},
            [](auto x) { return bl::cbrt(x); }, [](const real& x) { return mpfr::cbrt(x); },
            qdpp<Float>("qdpp nroot", [](auto x) { return qd_cbrt(x); }),
            boost_impl<Float>("boost::multiprecision::cbrt", [](auto x) { return boost_cbrt(x); }),
            tlfloat_impl<Float>("tlfloat::cbrt", [](auto x) { return tlfloat_ops::cbrt(x); }));
        run.binary(
            "roots_and_powers", "hypot", {domains::moderate(n), domains::wide_exponent(n)},
            [](auto x, auto y) { return bl::hypot(x, y); },
            [](const real& x, const real& y) { return mpfr::hypot(x, y); },
            boost_impl<Float>("boost::multiprecision::hypot",
                              [](auto x, auto y) { return boost_hypot(x, y); }),
            tlfloat_impl<Float>("tlfloat::hypot",
                                [](auto x, auto y) { return tlfloat_ops::hypot(x, y); }));

        run.unary(
            "trigonometric", "sin", trig(n), [](auto x) { return bl::sin(x); },
            [](const real& x) {
                using boost::multiprecision::sin;
                return sin(x);
            },
            qdpp<Float>("qdpp sin", [](auto x) { return ::sin(x); }),
            boost_impl<Float>("boost::multiprecision::sin", [](auto x) { return boost_sin(x); }),
            tlfloat_impl<Float>("tlfloat::sin", [](auto x) { return tlfloat_ops::sin(x); }));
        run.unary(
            "trigonometric", "cos", trig(n), [](auto x) { return bl::cos(x); },
            [](const real& x) {
                using boost::multiprecision::cos;
                return cos(x);
            },
            qdpp<Float>("qdpp cos", [](auto x) { return ::cos(x); }),
            boost_impl<Float>("boost::multiprecision::cos", [](auto x) { return boost_cos(x); }),
            tlfloat_impl<Float>("tlfloat::cos", [](auto x) { return tlfloat_ops::cos(x); }));
        run.unary(
            "trigonometric", "tan", trig(n), [](auto x) { return bl::tan(x); },
            [](const real& x) {
                using boost::multiprecision::tan;
                return tan(x);
            },
            qdpp<Float>("qdpp tan", [](auto x) { return ::tan(x); }),
            boost_impl<Float>("boost::multiprecision::tan", [](auto x) { return boost_tan(x); }),
            tlfloat_impl<Float>("tlfloat::tan", [](auto x) { return tlfloat_ops::tan(x); }));
        run.unary(
            "trigonometric", "atan", ordinary(n), [](auto x) { return bl::atan(x); },
            [](const real& x) {
                using boost::multiprecision::atan;
                return atan(x);
            },
            qdpp<Float>("qdpp atan", [](auto x) { return ::atan(x); }),
            boost_impl<Float>("boost::multiprecision::atan", [](auto x) { return boost_atan(x); }),
            tlfloat_impl<Float>("tlfloat::atan", [](auto x) { return tlfloat_ops::atan(x); }));
        run.binary(
            "trigonometric", "atan2", {domains::near_one(n), domains::moderate(n), domains::atan2_extreme()},
            [](auto y, auto x) { return bl::atan2(y, x); },
            [](const real& y, const real& x) {
                using boost::multiprecision::atan2;
                return atan2(y, x);
            },
            qdpp<Float>("qdpp atan2", [](auto y, auto x) { return ::atan2(y, x); }),
            boost_impl<Float>("boost::multiprecision::atan2",
                              [](auto y, auto x) { return boost_atan2(y, x); }),
            tlfloat_impl<Float>("tlfloat::atan2",
                                [](auto x, auto y) { return tlfloat_ops::atan2(x, y); }));
        run.unary(
            "trigonometric", "asin", unit(n), [](auto x) { return bl::asin(x); },
            [](const real& x) {
                using boost::multiprecision::asin;
                return asin(x);
            },
            qdpp<Float>("qdpp asin", [](auto x) { return ::asin(x); }),
            boost_impl<Float>("boost::multiprecision::asin", [](auto x) { return boost_asin(x); }),
            tlfloat_impl<Float>("tlfloat::asin", [](auto x) { return tlfloat_ops::asin(x); }));
        run.unary(
            "trigonometric", "acos", unit(n), [](auto x) { return bl::acos(x); },
            [](const real& x) {
                using boost::multiprecision::acos;
                return acos(x);
            },
            qdpp<Float>("qdpp acos", [](auto x) { return ::acos(x); }),
            boost_impl<Float>("boost::multiprecision::acos", [](auto x) { return boost_acos(x); }),
            tlfloat_impl<Float>("tlfloat::acos", [](auto x) { return tlfloat_ops::acos(x); }));

        run.unary(
            "exponentials", "exp", exponential(n), [](auto x) { return bl::exp(x); },
            [](const real& x) {
                using boost::multiprecision::exp;
                return exp(x);
            },
            qdpp<Float>("qdpp exp", [](auto x) { return ::exp(x); }),
            boost_impl<Float>("boost::multiprecision::exp", [](auto x) { return boost_exp(x); }),
            tlfloat_impl<Float>("tlfloat::exp", [](auto x) { return tlfloat_ops::exp(x); }));
        run.unary(
            "exponentials", "exp2", exponential(n), [](auto x) { return bl::exp2(x); },
            [](const real& x) { return mpfr::exp2(x); },
            boost_impl<Float>("boost::multiprecision::exp2", [](auto x) { return boost_exp2(x); }),
            tlfloat_impl<Float>("tlfloat::exp2", [](auto x) { return tlfloat_ops::exp2(x); }));
        run.unary(
            "exponentials", "expm1",
            {domains::moderate(n, -8.0, 8.0), domains::near_zero_cancellation(n)},
            [](auto x) { return bl::expm1(x); },
            [](const real& x) {
                using boost::multiprecision::expm1;
                return expm1(x);
            },
            qdpp<Float>("qdpp expm1", [](auto x) { return ::expm1(x); }),
            boost_impl<Float>("boost::multiprecision::expm1",
                              [](auto x) { return boost_expm1(x); }),
            tlfloat_impl<Float>("tlfloat::expm1", [](auto x) { return tlfloat_ops::expm1(x); }));
        run.unary(
            "logarithms", "log", positive(n), [](auto x) { return bl::log(x); },
            [](const real& x) {
                using boost::multiprecision::log;
                return log(x);
            },
            qdpp<Float>("qdpp log", [](auto x) { return ::log(x); }),
            boost_impl<Float>("boost::multiprecision::log", [](auto x) { return boost_log(x); }),
            tlfloat_impl<Float>("tlfloat::log", [](auto x) { return tlfloat_ops::log(x); }));
        run.unary(
            "logarithms", "log2", positive(n), [](auto x) { return bl::log2(x); },
            [](const real& x) { return mpfr::log2(x); },
            boost_impl<Float>("boost::multiprecision::log2", [](auto x) { return boost_log2(x); }),
            tlfloat_impl<Float>("tlfloat::log2", [](auto x) { return tlfloat_ops::log2(x); }));
        run.unary(
            "logarithms", "log10", positive(n), [](auto x) { return bl::log10(x); },
            [](const real& x) { return mpfr::log10(x); },
            qdpp<Float>("qdpp log10", [](auto x) { return ::log10(x); }),
            boost_impl<Float>("boost::multiprecision::log10",
                              [](auto x) { return boost_log10(x); }),
            tlfloat_impl<Float>("tlfloat::log10", [](auto x) { return tlfloat_ops::log10(x); }));
        run.unary(
            "logarithms", "log1p",
            {domains::bounded(domains::moderate(n), -0.99, 8.0),
             domains::near_zero_cancellation(n)},
            [](auto x) { return bl::log1p(x); },
            [](const real& x) {
                using boost::multiprecision::log1p;
                return log1p(x);
            },
            qdpp<Float>("qdpp log1p", [](auto x) { return ::log1p(x); }),
            boost_impl<Float>("boost::multiprecision::log1p",
                              [](auto x) { return boost_log1p(x); }),
            tlfloat_impl<Float>("tlfloat::log1p", [](auto x) { return tlfloat_ops::log1p(x); }));
        run.binary(
            "roots_and_powers", "pow",
            {domains::positive(domains::near_one(n), 0.125),
             domains::positive(domains::moderate(n, 0.125, 8.0), 0.125)},
            [](auto x, auto y) { return bl::pow(x, y); },
            [](const real& x, const real& y) {
                using boost::multiprecision::pow;
                return pow(x, y);
            },
            qdpp<Float>("qdpp pow", [](auto x, auto y) { return ::pow(x, y); }),
            boost_impl<Float>("boost::multiprecision::pow",
                              [](auto x, auto y) { return boost_pow(x, y); }),
            tlfloat_impl<Float>("tlfloat::pow",
                                [](auto x, auto y) { return tlfloat_ops::pow(x, y); }));
        run.unary(
            "roots_and_powers", "ipow", {domains::moderate(n), domains::wide_exponent(n, false, -100, 100)},
            [](auto x) { return bl::ipow(x, 7); },
            [](const real& x) { return x * x * x * x * x * x * x; },
            qdpp<Float>("qdpp npwr", [](auto x) { return qd_ipow(x); }),
            boost_impl<Float>("boost::multiprecision::pow(value, int)",
                              [](auto x) { return boost_ipow(x); }));

        run.binary(
            "remainders", "fmod",
            {domains::positive(domains::moderate(n, 0.125, 8.0), 0.125),
             domains::positive(domains::wide_exponent(n, true, -300, 300), 0x1p-300)},
            [](auto x, auto y) { return bl::fmod(x, y); },
            [](const real& x, const real& y) { return mpfr::fmod(x, y); },
            qdpp<Float>("qdpp fmod", [](auto x, auto y) { return ::fmod(x, y); }),
            boost_impl<Float>("boost::multiprecision::fmod",
                              [](auto x, auto y) { return boost_fmod(x, y); }),
            tlfloat_impl<Float>("tlfloat::fmod",
                                [](auto x, auto y) { return tlfloat_ops::fmod(x, y); }));
        run.binary_pairs(
            "remainders", "fmod", {fmod_quotient_reduction_inputs()},
            [](auto x, auto y) { return bl::fmod(x, y); },
            [](const real& x, const real& y) { return mpfr::fmod(x, y); },
            qdpp<Float>("qdpp fmod", [](auto x, auto y) { return ::fmod(x, y); }),
            boost_impl<Float>("boost::multiprecision::fmod",
                              [](auto x, auto y) { return boost_fmod(x, y); }),
            tlfloat_impl<Float>("tlfloat::fmod",
                                [](auto x, auto y) { return tlfloat_ops::fmod(x, y); }));
        run.binary(
            "remainders", "remainder", {domains::positive(domains::moderate(n, 0.125, 8.0), 0.125)},
            [](auto x, auto y) { return bl::remainder(x, y); },
            [](const real& x, const real& y) { return mpfr::remainder(x, y); },
            qdpp<Float>("qdpp drem", [](auto x, auto y) { return qd_remainder(x, y); }),
            boost_impl<Float>("boost::multiprecision::remainder",
                              [](auto x, auto y) { return boost_remainder(x, y); }),
            tlfloat_impl<Float>("tlfloat::remainder",
                                [](auto x, auto y) { return tlfloat_ops::remainder(x, y); }));

        run.unary(
            "hyperbolic", "sinh", ordinary(n), [](auto x) { return bl::sinh(x); },
            [](const real& x) {
                using boost::multiprecision::sinh;
                return sinh(x);
            },
            qdpp<Float>("qdpp sinh", [](auto x) { return ::sinh(x); }),
            boost_impl<Float>("boost::multiprecision::sinh", [](auto x) { return boost_sinh(x); }),
            tlfloat_impl<Float>("tlfloat::sinh", [](auto x) { return tlfloat_ops::sinh(x); }));
        run.unary(
            "hyperbolic", "cosh", ordinary(n), [](auto x) { return bl::cosh(x); },
            [](const real& x) {
                using boost::multiprecision::cosh;
                return cosh(x);
            },
            qdpp<Float>("qdpp cosh", [](auto x) { return ::cosh(x); }),
            boost_impl<Float>("boost::multiprecision::cosh", [](auto x) { return boost_cosh(x); }),
            tlfloat_impl<Float>("tlfloat::cosh", [](auto x) { return tlfloat_ops::cosh(x); }));
        run.unary(
            "hyperbolic", "tanh", ordinary(n), [](auto x) { return bl::tanh(x); },
            [](const real& x) {
                using boost::multiprecision::tanh;
                return tanh(x);
            },
            qdpp<Float>("qdpp tanh", [](auto x) { return ::tanh(x); }),
            boost_impl<Float>("boost::multiprecision::tanh", [](auto x) { return boost_tanh(x); }),
            tlfloat_impl<Float>("tlfloat::tanh", [](auto x) { return tlfloat_ops::tanh(x); }));
        run.unary(
            "inverse_hyperbolic", "asinh", ordinary(n), [](auto x) { return bl::asinh(x); },
            [](const real& x) {
                using boost::multiprecision::asinh;
                return asinh(x);
            },
            qdpp<Float>("qdpp asinh", [](auto x) { return ::asinh(x); }),
            boost_impl<Float>("boost::multiprecision::asinh",
                              [](auto x) { return boost_asinh(x); }),
            tlfloat_impl<Float>("tlfloat::asinh", [](auto x) { return tlfloat_ops::asinh(x); }));
        run.unary(
            "inverse_hyperbolic", "acosh",
            {domains::interval("moderate", 1.0, 20.0, n, default_seed ^ 0x25u),
             domains::boundary(n, 1.0, 2.0)},
            [](auto x) { return bl::acosh(x); },
            [](const real& x) {
                using boost::multiprecision::acosh;
                return acosh(x);
            },
            qdpp<Float>("qdpp acosh", [](auto x) { return ::acosh(x); }),
            boost_impl<Float>("boost::multiprecision::acosh",
                              [](auto x) { return boost_acosh(x); }),
            tlfloat_impl<Float>("tlfloat::acosh", [](auto x) { return tlfloat_ops::acosh(x); }));
        run.unary(
            "inverse_hyperbolic", "atanh",
            {domains::bounded(domains::moderate(n), -0.99, 0.99),
             domains::boundary(n, -0.999, 0.999)},
            [](auto x) { return bl::atanh(x); },
            [](const real& x) {
                using boost::multiprecision::atanh;
                return atanh(x);
            },
            qdpp<Float>("qdpp atanh", [](auto x) { return ::atanh(x); }),
            boost_impl<Float>("boost::multiprecision::atanh",
                              [](auto x) { return boost_atanh(x); }),
            tlfloat_impl<Float>("tlfloat::atanh", [](auto x) { return tlfloat_ops::atanh(x); }));

        run.unary(
            "special_functions", "erf", ordinary(n), [](auto x) { return bl::erf(x); },
            [](const real& x) { return boost::math::erf(x); },
            boost_impl<Float>("boost::multiprecision::erf", [](auto x) { return boost_erf(x); }),
            tlfloat_impl<Float>("tlfloat::erf", [](auto x) { return tlfloat_ops::erf(x); }));
        run.unary(
            "special_functions", "erfc", ordinary(n), [](auto x) { return bl::erfc(x); },
            [](const real& x) { return boost::math::erfc(x); },
            boost_impl<Float>("boost::multiprecision::erfc", [](auto x) { return boost_erfc(x); }),
            tlfloat_impl<Float>("tlfloat::erfc", [](auto x) { return tlfloat_ops::erfc(x); }));
        run.unary(
            "special_functions", "lgamma",
            {domains::interval("moderate", 0.125, 20.0, n, default_seed ^ 0x35u)},
            [](auto x) { return bl::lgamma(x); },
            [](const real& x) { return boost::math::lgamma(x); },
            boost_impl<Float>("boost::multiprecision::lgamma",
                              [](auto x) { return boost_lgamma(x); }),
            tlfloat_impl<Float>("tlfloat::lgamma", [](auto x) { return tlfloat_ops::lgamma(x); }));
        run.unary(
            "special_functions", "tgamma",
            {domains::interval("moderate", 0.125, 20.0, n, default_seed ^ 0x36u)},
            [](auto x) { return bl::tgamma(x); },
            [](const real& x) { return boost::math::tgamma(x); },
            boost_impl<Float>("boost::multiprecision::tgamma",
                              [](auto x) { return boost_tgamma(x); }),
            tlfloat_impl<Float>("tlfloat::tgamma", [](auto x) { return tlfloat_ops::tgamma(x); }));

        run.require_complete(123);
        return run.failures();
    }
} // namespace fltx::tests::accuracy
