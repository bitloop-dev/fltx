#include "runner.hpp"

#include <boost/math/special_functions/erf.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include <fltx/math.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace fltx::tests::accuracy
{
    namespace
    {
        using domains::domain;
        using real = mpfr::real;

        [[nodiscard]] std::vector<domain> ordinary(std::size_t n)
        {
            return { domains::near_one(n), domains::moderate(n) };
        }

        [[nodiscard]] domain native_extreme(
            std::size_t n,
            bool positive,
            int exponent_limit,
            std::uint64_t seed = default_seed ^ 0x41u)
        {
            domain out = domains::wide_exponent(
                n, positive, -exponent_limit, exponent_limit, seed);
            out.name = "extreme_finite";
            return out;
        }

        template<class Float>
        [[nodiscard]] domain product_extreme(
            int exponent_limit,
            std::uint64_t seed = default_seed ^ 0x42u)
        {
            domain out{ "extreme_finite", seed, {} };
            const Float large = std::ldexp(Float{ 0.75 }, exponent_limit);
            const Float small = std::ldexp(Float{ 0.625 }, -exponent_limit);
            const Float large_neighbour = std::nextafter(large, Float{ 0 });
            const Float small_neighbour = std::nextafter(small, Float{ 1 });
            out.values = {
                make_sample(static_cast<double>(large), "large positive"),
                make_sample(static_cast<double>(-large), "large negative"),
                make_sample(static_cast<double>(small), "small positive"),
                make_sample(static_cast<double>(-small), "small negative"),
                make_sample(static_cast<double>(large_neighbour), "large neighbour"),
                make_sample(static_cast<double>(-large_neighbour), "negative large neighbour"),
                make_sample(static_cast<double>(small_neighbour), "small neighbour"),
                make_sample(static_cast<double>(-small_neighbour), "negative small neighbour")
            };
            return out;
        }

        template<class Float>
        [[nodiscard]] domain atan2_extreme(
            int exponent_limit,
            std::uint64_t seed = default_seed ^ 0x43u)
        {
            domain out{ "extreme_finite", seed, {} };
            const Float huge = std::ldexp(Float{ 0.75 }, exponent_limit);
            const Float tiny = std::ldexp(Float{ 0.625 }, -exponent_limit);
            const Float huge_neighbour = std::nextafter(huge, Float{ 0 });
            const Float tiny_neighbour = std::nextafter(tiny, Float{ 1 });
            out.values = {
                make_sample(static_cast<double>(huge), "huge positive"),
                make_sample(static_cast<double>(-huge), "huge negative"),
                make_sample(static_cast<double>(tiny), "tiny positive"),
                make_sample(static_cast<double>(-tiny), "tiny negative"),
                make_sample(static_cast<double>(huge_neighbour), "huge neighbour"),
                make_sample(static_cast<double>(-huge_neighbour), "negative huge neighbour"),
                make_sample(static_cast<double>(tiny_neighbour), "tiny neighbour"),
                make_sample(static_cast<double>(-tiny_neighbour), "negative tiny neighbour")
            };
            return out;
        }

        template<class Float>
        [[nodiscard]] domain native_subnormal(
            std::size_t n,
            std::uint64_t seed = default_seed ^ 0x44u)
        {
            domain out{ "subnormal", seed, {} };
            out.values = {
                make_exact_sample(0.0, "+0"),
                make_exact_sample(-0.0, "-0"),
                make_exact_sample(
                    static_cast<double>(std::numeric_limits<Float>::denorm_min()),
                    "denorm_min"),
                make_exact_sample(
                    static_cast<double>(std::numeric_limits<Float>::min()),
                    "min normal")
            };

            constexpr int first_exponent =
                std::is_same_v<Float, float> ? -149 : -1074;
            constexpr int exponent_count =
                std::is_same_v<Float, float> ? 23 : 52;
            random_bits rng(seed);
            for (std::size_t i = 0; i < n; ++i)
            {
                const int exponent =
                    first_exponent + static_cast<int>(rng.next() % exponent_count);
                Float value = static_cast<Float>(
                    std::ldexp(rng.between(0.5, 1.0), exponent));
                if ((rng.next() & 1u) == 0)
                    value = -value;
                out.values.push_back(make_exact_sample(static_cast<double>(value)));
            }
            return out;
        }

        template<class Float>
        [[nodiscard]] domain near_equal_cancellation(
            std::size_t n,
            std::uint64_t seed = default_seed ^ 0x45u)
        {
            domain out{ "cancellation", seed, {} };
            out.values.push_back(make_exact_sample(1.0, "one"));
            out.values.push_back(make_exact_sample(
                static_cast<double>(std::nextafter(Float{ 1 }, Float{ 2 })),
                "next value after one"));

            constexpr int first_exponent =
                std::is_same_v<Float, float> ? 8 : 20;
            constexpr int exponent_count =
                std::is_same_v<Float, float> ? 16 : 33;
            random_bits rng(seed);
            for (std::size_t i = 0; i < n; ++i)
            {
                const int exponent =
                    first_exponent + static_cast<int>(rng.next() % exponent_count);
                const Float value = static_cast<Float>(
                    1.0 + std::ldexp(rng.between(0.5, 1.0), -exponent));
                out.values.push_back(make_exact_sample(static_cast<double>(value)));
            }
            return out;
        }

        template<class Float>
        [[nodiscard]] domains::ternary_domain fma_cancellation(
            std::size_t n,
            std::uint64_t seed = default_seed ^ 0x45u)
        {
            return domains::zip_ternary(
                near_equal_cancellation<Float>(n, seed),
                near_equal_cancellation<Float>(
                    n, seed ^ domains::ternary_y_seed_mask),
                near_equal_cancellation<Float>(
                    n, seed ^ domains::ternary_z_seed_mask));
        }

        template<class Float>
        [[nodiscard]] domain near_zero_cancellation(
            std::size_t n,
            std::uint64_t seed = default_seed ^ 0x46u)
        {
            domain out{ "cancellation", seed, {} };
            out.values.push_back(make_exact_sample(0x1p-20, "small positive"));
            out.values.push_back(make_exact_sample(-0x1p-20, "small negative"));

            constexpr int first_exponent =
                std::is_same_v<Float, float> ? 5 : 20;
            constexpr int exponent_count =
                std::is_same_v<Float, float> ? 116 : 881;
            random_bits rng(seed);
            for (std::size_t i = 0; i < n; ++i)
            {
                const int exponent =
                    first_exponent + static_cast<int>(rng.next() % exponent_count);
                const double sign = (rng.next() & 1u) != 0 ? 1.0 : -1.0;
                const Float value = static_cast<Float>(
                    sign * std::ldexp(rng.between(0.5, 1.0), -exponent));
                out.values.push_back(make_exact_sample(static_cast<double>(value)));
            }
            return out;
        }

        [[nodiscard]] std::vector<domain> positive(
            std::size_t n,
            int wide_limit,
            int extreme_limit)
        {
            return {
                domains::positive(domains::near_one(n), std::ldexp(1.0, -wide_limit)),
                domains::positive(
                    domains::wide_exponent(n, true, -wide_limit, wide_limit),
                    std::ldexp(0.5, -wide_limit)),
                domains::positive(
                    native_extreme(n, true, extreme_limit),
                    std::ldexp(0.5, -extreme_limit))
            };
        }

        template<class Float>
        [[nodiscard]] domain argument_reduction(
            std::size_t n,
            int max_exponent,
            std::uint64_t seed = default_seed ^ 0x4cu)
        {
            domain out{ "argument_reduction", seed, {} };
            out.values.reserve(n + 18);

            constexpr Float half_pi =
                static_cast<Float>(1.57079632679489661923132169163975144L);
            const auto append_neighborhood = [&](int exponent, bool negative) {
                Float anchor = std::ldexp(half_pi, exponent);
                if (negative)
                    anchor = -anchor;

                const Float below = native_fp::next_down(anchor);
                const Float above = native_fp::next_up(anchor);
                if (!(below < anchor && anchor < above))
                    throw std::logic_error("native argument-reduction neighborhood collapsed");

                const std::string suffix =
                    " 2^" + std::to_string(exponent) + " * pi/2 anchor";
                out.values.push_back(make_exact_sample(
                    static_cast<double>(below), "below" + suffix));
                out.values.push_back(make_exact_sample(
                    static_cast<double>(anchor), "nearest" + suffix));
                out.values.push_back(make_exact_sample(
                    static_cast<double>(above), "above" + suffix));
            };

            static constexpr std::array<int, 6> anchor_exponents{
                0, 20, 53, 56, 100, 159
            };
            for (std::size_t i = 0; i < anchor_exponents.size(); ++i)
            {
                if (anchor_exponents[i] <= max_exponent)
                    append_neighborhood(anchor_exponents[i], (i & 1u) != 0);
            }

            random_bits rng(seed);
            for (std::size_t i = 0; i < n; ++i)
            {
                const int exponent = static_cast<int>(
                    rng.next() % static_cast<std::uint64_t>(max_exponent + 1));
                Float anchor = std::ldexp(half_pi, exponent);
                if ((rng.next() & 1u) != 0)
                    anchor = -anchor;

                const bool upward = (rng.next() & 1u) != 0;
                const int steps = 1 + static_cast<int>(rng.next() % 16u);
                Float value = anchor;
                for (int step = 0; step < steps; ++step)
                {
                    value = upward
                        ? native_fp::next_up(value)
                        : native_fp::next_down(value);
                }

                out.values.push_back(make_exact_sample(
                    static_cast<double>(value),
                    "near 2^" + std::to_string(exponent) + " * pi/2"));
            }
            return out;
        }

        template<class Float>
        [[nodiscard]] std::vector<domain> trig(
            std::size_t n,
            int reduction_limit)
        {
            return {
                domains::moderate(n),
                argument_reduction<Float>(n, reduction_limit)
            };
        }

        [[nodiscard]] std::vector<domain> unit(std::size_t n)
        {
            return {
                domains::bounded(domains::moderate(n), -0.999, 0.999),
                domains::boundary(n, -1.0, 1.0)
            };
        }

        [[nodiscard]] std::vector<domain> exponential(
            std::size_t n,
            double limit)
        {
            return {
                domains::moderate(n, -8.0, 8.0),
                domains::interval(
                    "boundary", -limit, limit, n, default_seed ^ 0x47u)
            };
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

        template<class Float>
        [[nodiscard]] domain rounding_inputs(std::size_t n)
        {
            domain out{ "boundary", default_seed ^ 0x4bu, {} };
            out.values.push_back(make_exact_sample(0.0, "+0"));
            out.values.push_back(make_exact_sample(-0.0, "-0"));
            const auto append_neighbours = [&](Float tie, std::string label) {
                out.values.push_back(make_exact_sample(
                    static_cast<double>(std::nextafter(
                        tie, -std::numeric_limits<Float>::infinity())),
                    label + " below"));
                out.values.push_back(make_exact_sample(
                    static_cast<double>(tie),
                    label + " tie"));
                out.values.push_back(make_exact_sample(
                    static_cast<double>(std::nextafter(
                        tie, std::numeric_limits<Float>::infinity())),
                    label + " above"));
            };
            append_neighbours(Float{ -2.5 }, "-2.5");
            append_neighbours(Float{ -0.5 }, "-0.5");
            append_neighbours(Float{ 0.5 }, "0.5");
            append_neighbours(Float{ 2.5 }, "2.5");

            random_bits rng(out.seed);
            for (std::size_t i = 0; i < n; ++i)
            {
                const Float integer = static_cast<Float>(
                    static_cast<int>(rng.next() % 33u) - 16);
                const Float fraction =
                    static_cast<Float>(rng.next() % 9u) * Float{ 0.125 };
                out.values.push_back(make_exact_sample(static_cast<double>(
                    (rng.next() & 1u) != 0
                        ? integer + fraction
                        : integer - fraction)));
            }
            return out;
        }

        template<class Float>
        int run_native(csv_writer& output, const options& settings)
        {
            runner<Float> run(output, settings);
            const std::size_t n = settings.samples;
            constexpr bool single_precision = std::is_same_v<Float, float>;
            const int wide_limit = single_precision ? 120 : 900;
            const int extreme_limit = single_precision ? 125 : 1021;
            const int product_limit = single_precision ? 50 : 400;
            const int reduction_limit = single_precision ? 100 : 159;
            const int integer_power_limit = single_precision ? 15 : 100;
            const double exponential_limit = single_precision ? 80.0 : 700.0;

            run.binary("arithmetic", "add", ordinary(n),
                [](auto x, auto y) { return x + y; },
                [](const real& x, const real& y) { return x + y; });
            run.binary("arithmetic", "add",
                { domains::wide_exponent(n, false, -wide_limit, wide_limit),
                  native_extreme(n, false, extreme_limit),
                  native_subnormal<Float>(n) },
                [](auto x, auto y) { return x + y; },
                [](const real& x, const real& y) { return x + y; });
            run.binary("arithmetic", "subtract",
                { domains::near_one(n),
                  near_equal_cancellation<Float>(n),
                  native_subnormal<Float>(n) },
                [](auto x, auto y) { return x - y; },
                [](const real& x, const real& y) { return x - y; });
            run.binary("arithmetic", "multiply", ordinary(n),
                [](auto x, auto y) { return x * y; },
                [](const real& x, const real& y) { return x * y; });
            run.binary("arithmetic", "multiply",
                { domains::wide_exponent(
                      n, false, -product_limit, product_limit),
                  product_extreme<Float>(product_limit) },
                [](auto x, auto y) { return x * y; },
                [](const real& x, const real& y) { return x * y; });
            run.binary("arithmetic", "divide",
                { domains::positive(domains::near_one(n), 0.125),
                  domains::positive(domains::moderate(n), 0.125),
                  domains::positive(
                      domains::wide_exponent(
                          n, true, -product_limit, product_limit),
                      std::ldexp(0.5, -product_limit)),
                  domains::positive(product_extreme<Float>(product_limit),
                                    std::ldexp(0.5, -product_limit)) },
                [](auto x, auto y) { return x / y; },
                [](const real& x, const real& y) { return x / y; });
            run.ternary("floating_point_utilities", "fma",
                { domains::moderate_ternary(n),
                  fma_cancellation<Float>(n),
                  domains::wide_exponent_ternary(
                      n, false, -product_limit, product_limit) },
                [](auto x, auto y, auto z) { return bl::fma(x, y, z); },
                [](const real& x, const real& y, const real& z) {
                    return mpfr::fma(x, y, z);
                });
            run.unary_exact("floating_point_utilities", "abs",
                { domains::wide_exponent(
                      n, false, -wide_limit, wide_limit),
                  native_subnormal<Float>(n) },
                [](auto x) { return bl::abs(x); },
                [](const real& x) {
                    using boost::multiprecision::abs;
                    return abs(x);
                });
            run.unary_exact("floating_point_utilities", "fabs", { domains::moderate(n) },
                [](auto x) { return bl::fabs(x); },
                [](const real& x) {
                    using boost::multiprecision::abs;
                    return abs(x);
                });
            run.unary("roots_and_powers", "sqr",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -product_limit, product_limit) },
                [](auto x) { return bl::sqr(x); },
                [](const real& x) { return x * x; });
            run.unary("floating_point_utilities", "recip",
                { domains::positive(domains::near_one(n), 0.125),
                  domains::positive(
                      domains::wide_exponent(
                          n, true, -product_limit, product_limit),
                      std::ldexp(0.5, -product_limit)) },
                [](auto x) { return bl::recip(x); },
                [](const real& x) { return real{ 1 } / x; });
            run.binary_exact("floating_point_utilities", "fmin",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x, auto y) { return bl::fmin(x, y); },
                [](const real& x, const real& y) {
                    return x < y ? x : y;
                });
            run.binary_exact("floating_point_utilities", "fmax",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x, auto y) { return bl::fmax(x, y); },
                [](const real& x, const real& y) {
                    return x > y ? x : y;
                });
            run.binary("floating_point_utilities", "fdim",
                { domains::moderate(n),
                  near_equal_cancellation<Float>(n) },
                [](auto x, auto y) { return bl::fdim(x, y); },
                [](const real& x, const real& y) {
                    return x > y ? x - y : real{ 0 };
                });
            run.binary_exact("floating_point_utilities", "copysign", { domains::moderate(n) },
                [](auto x, auto y) { return bl::copysign(x, y); },
                [](const real& x, const real& y) {
                    using boost::multiprecision::abs;
                    return y < 0 ? -abs(x) : abs(x);
                });

            run.unary_exact("rounding", "floor", { rounding_inputs<Float>(n) },
                [](auto x) { return bl::floor(x); },
                [](const real& x) {
                    using boost::multiprecision::floor;
                    return floor(x);
                });
            run.unary_exact("rounding", "ceil", { rounding_inputs<Float>(n) },
                [](auto x) { return bl::ceil(x); },
                [](const real& x) {
                    using boost::multiprecision::ceil;
                    return ceil(x);
                });
            run.unary_exact("rounding", "trunc", { rounding_inputs<Float>(n) },
                [](auto x) { return bl::trunc(x); },
                [](const real& x) { return mpfr::trunc(x); });
            run.unary_exact("rounding", "round", { rounding_inputs<Float>(n) },
                [](auto x) { return bl::round(x); },
                [](const real& x) {
                    return mpfr::round_away_from_zero(x);
                });
            run.unary_exact("rounding", "roundeven", { rounding_inputs<Float>(n) },
                [](auto x) { return bl::roundeven(x); },
                [](const real& x) { return mpfr::round_even(x); });
            run.unary(
                "rounding", "round_decimals", { domains::moderate(n) },
                [](auto x) { return bl::round_decimals(x, 3); },
                [](const real& x) {
                    return mpfr::round_decimals(x, 3);
                });
            run.unary(
                "rounding", "round_significant",
                { domains::wide_exponent(
                    n, false, -integer_power_limit, integer_power_limit) },
                [](auto x) {
                    return bl::round_significant(x, 7);
                },
                [](const real& x) {
                    return mpfr::round_significant(x, 7);
                });
            run.unary_exact("rounding", "lround", { rounding_inputs<Float>(n) },
                [](auto x) {
                    return static_cast<Float>(bl::lround(x));
                },
                [](const real& x) {
                    const real rounded = mpfr::round_away_from_zero(x);
                    return rounded == 0 ? real{ 0 } : rounded;
                });
            run.unary_exact("rounding", "llround", { rounding_inputs<Float>(n) },
                [](auto x) {
                    return static_cast<Float>(bl::llround(x));
                },
                [](const real& x) {
                    const real rounded = mpfr::round_away_from_zero(x);
                    return rounded == 0 ? real{ 0 } : rounded;
                });

            run.binary_pair(
                "remainders", "remquo", { nonzero_moderate(n) },
                [](auto x, auto y) {
                    int quotient = 0;
                    const Float remainder = bl::remquo(x, y, &quotient);
                    const int low_bits =
                        quotient < 0 ? -((-quotient) & 7) : (quotient & 7);
                    return std::pair{
                        remainder,
                        static_cast<Float>(low_bits)
                    };
                },
                [](const real& x, const real& y) {
                    return std::pair{
                        mpfr::remainder(x, y),
                        real{ mpfr::remquo_bits(x, y) }
                    };
                });
            run.unary_pair_exact("floating_point_utilities", "modf",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x) {
                    Float integral{};
                    const Float fractional = bl::modf(x, &integral);
                    return std::pair{ fractional, integral };
                },
                [](const real& x) {
                    const real integral = mpfr::trunc(x);
                    real fractional = x == 0 ? x : x - integral;
                    if (fractional == 0 && x < 0)
                        fractional = mpfr::signed_zero(true);
                    return std::pair{ fractional, integral };
                });
            run.unary_exact("floating_point_utilities", "ldexp",
                { domains::wide_exponent(
                    n, false, -product_limit, product_limit) },
                [](auto x) { return bl::ldexp(x, 17); },
                [](const real& x) { return x * real{ 131072 }; });
            run.unary_exact("floating_point_utilities", "scalbn",
                { domains::wide_exponent(
                    n, false, -product_limit, product_limit) },
                [](auto x) { return bl::scalbn(x, 17); },
                [](const real& x) { return x * real{ 131072 }; });
            run.unary_exact("floating_point_utilities", "scalbln",
                { domains::wide_exponent(
                    n, false, -product_limit, product_limit) },
                [](auto x) { return bl::scalbln(x, 17L); },
                [](const real& x) { return x * real{ 131072 }; });
            run.unary_pair_exact("floating_point_utilities", "frexp",
                { domains::wide_exponent(
                      n, false, -wide_limit, wide_limit),
                  native_subnormal<Float>(n) },
                [](auto x) {
                    int exponent = 0;
                    const Float fraction = bl::frexp(x, &exponent);
                    return std::pair{
                        fraction,
                        static_cast<Float>(exponent)
                    };
                },
                [](const real& x) {
                    using boost::multiprecision::frexp;
                    int exponent = 0;
                    const real fraction = frexp(x, &exponent);
                    return std::pair{ fraction, real{ exponent } };
                });
            run.unary_exact("floating_point_utilities", "ilogb",
                { nonzero_moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x) {
                    return static_cast<Float>(bl::ilogb(x));
                },
                [](const real& x) {
                    return real{ mpfr::ilogb(x) };
                });
            run.unary_exact("floating_point_utilities", "logb",
                { nonzero_moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x) { return bl::logb(x); },
                [](const real& x) {
                    return real{ mpfr::ilogb(x) };
                });

            auto sqrt_domains = positive(n, wide_limit, extreme_limit);
            sqrt_domains.push_back(
                domains::positive(native_subnormal<Float>(n)));
            run.unary("roots_and_powers", "sqrt", std::move(sqrt_domains),
                [](auto x) { return bl::sqrt(x); },
                [](const real& x) {
                    using boost::multiprecision::sqrt;
                    return sqrt(x);
                });
            run.unary("roots_and_powers", "cbrt",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x) { return bl::cbrt(x); },
                [](const real& x) { return mpfr::cbrt(x); });
            run.binary("roots_and_powers", "hypot",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -wide_limit, wide_limit) },
                [](auto x, auto y) { return bl::hypot(x, y); },
                [](const real& x, const real& y) {
                    return mpfr::hypot(x, y);
                });

            run.unary("trigonometric", "sin", trig<Float>(n, reduction_limit),
                [](auto x) { return bl::sin(x); },
                [](const real& x) {
                    using boost::multiprecision::sin;
                    return sin(x);
                });
            run.unary("trigonometric", "cos", trig<Float>(n, reduction_limit),
                [](auto x) { return bl::cos(x); },
                [](const real& x) {
                    using boost::multiprecision::cos;
                    return cos(x);
                });
            run.unary("trigonometric", "tan", trig<Float>(n, reduction_limit),
                [](auto x) { return bl::tan(x); },
                [](const real& x) {
                    using boost::multiprecision::tan;
                    return tan(x);
                });
            run.unary("trigonometric", "atan", ordinary(n),
                [](auto x) { return bl::atan(x); },
                [](const real& x) {
                    using boost::multiprecision::atan;
                    return atan(x);
                });
            run.binary("trigonometric", "atan2",
                { domains::near_one(n), domains::moderate(n),
                  atan2_extreme<Float>(extreme_limit) },
                [](auto y, auto x) { return bl::atan2(y, x); },
                [](const real& y, const real& x) {
                    using boost::multiprecision::atan2;
                    return atan2(y, x);
                });
            run.unary("trigonometric", "asin", unit(n),
                [](auto x) { return bl::asin(x); },
                [](const real& x) {
                    using boost::multiprecision::asin;
                    return asin(x);
                });
            run.unary("trigonometric", "acos", unit(n),
                [](auto x) { return bl::acos(x); },
                [](const real& x) {
                    using boost::multiprecision::acos;
                    return acos(x);
                });

            run.unary("exponentials", "exp", exponential(n, exponential_limit),
                [](auto x) { return bl::exp(x); },
                [](const real& x) {
                    using boost::multiprecision::exp;
                    return exp(x);
                });
            run.unary("exponentials", "exp2", exponential(n, exponential_limit),
                [](auto x) { return bl::exp2(x); },
                [](const real& x) { return mpfr::exp2(x); });
            run.unary("exponentials", "expm1",
                { domains::moderate(n, -8.0, 8.0),
                  near_zero_cancellation<Float>(n) },
                [](auto x) { return bl::expm1(x); },
                [](const real& x) {
                    using boost::multiprecision::expm1;
                    return expm1(x);
                });
            run.unary("logarithms", "log",
                positive(n, wide_limit, extreme_limit),
                [](auto x) { return bl::log(x); },
                [](const real& x) {
                    using boost::multiprecision::log;
                    return log(x);
                });
            run.unary("logarithms", "log2",
                positive(n, wide_limit, extreme_limit),
                [](auto x) { return bl::log2(x); },
                [](const real& x) { return mpfr::log2(x); });
            run.unary("logarithms", "log10",
                positive(n, wide_limit, extreme_limit),
                [](auto x) { return bl::log10(x); },
                [](const real& x) { return mpfr::log10(x); });
            run.unary("logarithms", "log1p",
                { domains::bounded(domains::moderate(n), -0.99, 8.0),
                  near_zero_cancellation<Float>(n) },
                [](auto x) { return bl::log1p(x); },
                [](const real& x) {
                    using boost::multiprecision::log1p;
                    return log1p(x);
                });
            run.binary("roots_and_powers", "pow",
                { domains::positive(domains::near_one(n), 0.125),
                  domains::positive(
                      domains::moderate(n, 0.125, 8.0), 0.125) },
                [](auto x, auto y) { return bl::pow(x, y); },
                [](const real& x, const real& y) {
                    using boost::multiprecision::pow;
                    return pow(x, y);
                });
            run.unary("roots_and_powers", "ipow",
                { domains::moderate(n),
                  domains::wide_exponent(
                      n, false, -integer_power_limit, integer_power_limit) },
                [](auto x) { return bl::ipow(x, 7); },
                [](const real& x) {
                    return x * x * x * x * x * x * x;
                });

            run.binary("remainders", "fmod",
                { domains::positive(
                      domains::moderate(n, 0.125, 8.0), 0.125),
                  domains::positive(
                      domains::wide_exponent(
                          n, true, -product_limit, product_limit),
                      std::ldexp(0.5, -product_limit)) },
                [](auto x, auto y) { return bl::fmod(x, y); },
                [](const real& x, const real& y) {
                    return mpfr::fmod(x, y);
                });
            run.binary("remainders", "remainder",
                { domains::positive(
                    domains::moderate(n, 0.125, 8.0), 0.125) },
                [](auto x, auto y) { return bl::remainder(x, y); },
                [](const real& x, const real& y) {
                    return mpfr::remainder(x, y);
                });

            run.unary("hyperbolic", "sinh", ordinary(n),
                [](auto x) { return bl::sinh(x); },
                [](const real& x) {
                    using boost::multiprecision::sinh;
                    return sinh(x);
                });
            run.unary("hyperbolic", "cosh", ordinary(n),
                [](auto x) { return bl::cosh(x); },
                [](const real& x) {
                    using boost::multiprecision::cosh;
                    return cosh(x);
                });
            run.unary("hyperbolic", "tanh", ordinary(n),
                [](auto x) { return bl::tanh(x); },
                [](const real& x) {
                    using boost::multiprecision::tanh;
                    return tanh(x);
                });
            run.unary("inverse_hyperbolic", "asinh", ordinary(n),
                [](auto x) { return bl::asinh(x); },
                [](const real& x) {
                    using boost::multiprecision::asinh;
                    return asinh(x);
                });
            run.unary("inverse_hyperbolic", "acosh",
                { domains::interval(
                      "moderate", 1.0, 20.0, n, default_seed ^ 0x48u),
                  domains::boundary(n, 1.0, 2.0) },
                [](auto x) { return bl::acosh(x); },
                [](const real& x) {
                    using boost::multiprecision::acosh;
                    return acosh(x);
                });
            run.unary("inverse_hyperbolic", "atanh",
                { domains::bounded(domains::moderate(n), -0.99, 0.99),
                  domains::boundary(n, -0.999, 0.999) },
                [](auto x) { return bl::atanh(x); },
                [](const real& x) {
                    using boost::multiprecision::atanh;
                    return atanh(x);
                });

            run.unary("special_functions", "erf", ordinary(n),
                [](auto x) { return bl::erf(x); },
                [](const real& x) { return boost::math::erf(x); });
            run.unary("special_functions", "erfc", ordinary(n),
                [](auto x) { return bl::erfc(x); },
                [](const real& x) { return boost::math::erfc(x); });
            run.unary("special_functions", "lgamma",
                { domains::interval(
                    "moderate", 0.125, 20.0, n, default_seed ^ 0x49u) },
                [](auto x) { return bl::lgamma(x); },
                [](const real& x) { return boost::math::lgamma(x); });
            run.unary("special_functions", "tgamma",
                { domains::interval(
                    "moderate", 0.125, 20.0, n, default_seed ^ 0x4au) },
                [](auto x) { return bl::tgamma(x); },
                [](const real& x) { return boost::math::tgamma(x); });

            run.require_complete(119);
            return run.failures();
        }
    }

    int run_f32(csv_writer& output, const options& settings)
    {
        return run_native<float>(output, settings);
    }

    int run_f64(csv_writer& output, const options& settings)
    {
        return run_native<double>(output, settings);
    }
}
