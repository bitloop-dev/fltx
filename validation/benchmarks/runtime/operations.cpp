#include "../../support/tlfloat_ops.hpp"
#include "runner.hpp"

#include <fltx/io.h>
#include <fltx/math.h>
#include <fltx/random.h>

#include <array>
#include <boost/math/special_functions/next.hpp>
#include <charconv>
#include <cstddef>
#include <ios>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace fltx::tests::benchmark
{
    namespace
    {
        template<class Float>
        using qd_value = std::conditional_t<std::is_same_v<Float, bl::f128>,
                                            implementations::qd_f128, implementations::qd_f256>;

        template<class Float>
        using boost_value = std::conditional_t<std::is_same_v<Float, bl::f128>,
                                               implementations::cppdd, implementations::mpfr64>;

        template<class Float>
        using tlfloat_value =
            std::conditional_t<std::is_same_v<Float, bl::f128>, implementations::tl_f128,
                               implementations::tl_f256>;

        template<class Float, class Eval>
        [[nodiscard]] auto qdpp(std::string_view api, Eval evaluate)
        {
            return implementation<qd_value<Float>>(implementations::qdpp_identity<Float>, api,
                                                   std::move(evaluate));
        }

        template<class Float, class Eval>
        [[nodiscard]] auto qdpp_if(bool supported, std::string_view api, Eval evaluate)
        {
            auto out = qdpp<Float>(api, std::move(evaluate));
            out.info.enabled = out.info.enabled && supported;
            return out;
        }

        template<class Float, class Eval>
        [[nodiscard]] auto boost_impl(std::string_view api, Eval evaluate)
        {
            return implementation<boost_value<Float>>(implementations::boost_identity<Float>, api,
                                                      std::move(evaluate));
        }

        template<class Float, class Eval>
        [[nodiscard]] auto tlfloat_impl(std::string_view api, Eval evaluate)
        {
            return implementation<tlfloat_value<Float>>(implementations::tlfloat_identity<Float>,
                                                        api, std::move(evaluate));
        }

        template<class Value> [[nodiscard]] auto boost_abs(const Value& value)
        {
            using boost::multiprecision::abs;
            return abs(value);
        }

        template<class Value> [[nodiscard]] auto boost_fabs(const Value& value)
        {
            using boost::multiprecision::fabs;
            return fabs(value);
        }

        template<class Value> [[nodiscard]] auto boost_floor(const Value& value)
        {
            using boost::multiprecision::floor;
            return floor(value);
        }

        template<class Value> [[nodiscard]] auto boost_ceil(const Value& value)
        {
            using boost::multiprecision::ceil;
            return ceil(value);
        }

        template<class Value> [[nodiscard]] auto boost_trunc(const Value& value)
        {
            using boost::multiprecision::trunc;
            return trunc(value);
        }

        template<class Value> [[nodiscard]] auto boost_round(const Value& value)
        {
            using boost::multiprecision::round;
            return round(value);
        }

        template<class Value> [[nodiscard]] auto boost_fmod(const Value& x, const Value& y)
        {
            using boost::multiprecision::fmod;
            return fmod(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_remainder(const Value& x, const Value& y)
        {
            using boost::multiprecision::remainder;
            return remainder(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_fmin(const Value& x, const Value& y)
        {
            using boost::multiprecision::fmin;
            return fmin(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_fmax(const Value& x, const Value& y)
        {
            using boost::multiprecision::fmax;
            return fmax(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_fdim(const Value& x, const Value& y)
        {
            using boost::multiprecision::fdim;
            return fdim(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_copysign(const Value& x, const Value& y)
        {
            using boost::multiprecision::copysign;
            return copysign(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_remquo(const Value& x, const Value& y)
        {
            int quotient = 0;
            using boost::multiprecision::remquo;
            const Value remainder = remquo(x, y, &quotient);
            return std::pair{remainder, quotient};
        }

        template<class Value> [[nodiscard]] auto boost_modf(const Value& value)
        {
            Value integral{};
            using boost::multiprecision::modf;
            const Value fractional = modf(value, &integral);
            return std::pair{fractional, integral};
        }

        template<class Value> [[nodiscard]] auto boost_frexp(const Value& value)
        {
            int exponent = 0;
            using boost::multiprecision::frexp;
            const Value fraction = frexp(value, &exponent);
            return std::pair{fraction, exponent};
        }

        template<class Value> [[nodiscard]] auto boost_ilogb(const Value& value)
        {
            using boost::multiprecision::ilogb;
            return ilogb(value);
        }

        template<class Value> [[nodiscard]] auto boost_logb(const Value& value)
        {
            using boost::multiprecision::logb;
            return logb(value);
        }

        template<class Value> [[nodiscard]] auto boost_nextafter(const Value& x, const Value& y)
        {
            return boost::math::nextafter(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_lround(const Value& value)
        {
            using boost::multiprecision::lround;
            return lround(value);
        }

        template<class Value> [[nodiscard]] auto boost_llround(const Value& value)
        {
            using boost::multiprecision::llround;
            return llround(value);
        }

        template<class Value>
        [[nodiscard]] auto boost_fma(const Value& x, const Value& y, const Value& z)
        {
            using boost::multiprecision::fma;
            return fma(x, y, z);
        }

        template<class Value> [[nodiscard]] auto boost_hypot(const Value& x, const Value& y)
        {
            using boost::multiprecision::hypot;
            return hypot(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_exp2(const Value& value)
        {
            using boost::multiprecision::exp2;
            return exp2(value);
        }

        template<class Value> [[nodiscard]] auto boost_log2(const Value& value)
        {
            using boost::multiprecision::log2;
            return log2(value);
        }

        template<class Value> [[nodiscard]] auto boost_sqrt(const Value& value)
        {
            using boost::multiprecision::sqrt;
            return sqrt(value);
        }

        template<class Value> [[nodiscard]] auto boost_cbrt(const Value& value)
        {
            using boost::multiprecision::cbrt;
            return cbrt(value);
        }

        template<class Value> [[nodiscard]] auto boost_sin(const Value& value)
        {
            using boost::multiprecision::sin;
            return sin(value);
        }

        template<class Value> [[nodiscard]] auto boost_cos(const Value& value)
        {
            using boost::multiprecision::cos;
            return cos(value);
        }

        template<class Value> [[nodiscard]] auto boost_tan(const Value& value)
        {
            using boost::multiprecision::tan;
            return tan(value);
        }

        template<class Value> [[nodiscard]] auto boost_atan(const Value& value)
        {
            using boost::multiprecision::atan;
            return atan(value);
        }

        template<class Value> [[nodiscard]] auto boost_atan2(const Value& y, const Value& x)
        {
            using boost::multiprecision::atan2;
            return atan2(y, x);
        }

        template<class Value> [[nodiscard]] auto boost_asin(const Value& value)
        {
            using boost::multiprecision::asin;
            return asin(value);
        }

        template<class Value> [[nodiscard]] auto boost_acos(const Value& value)
        {
            using boost::multiprecision::acos;
            return acos(value);
        }

        template<class Value> [[nodiscard]] auto boost_exp(const Value& value)
        {
            using boost::multiprecision::exp;
            return exp(value);
        }

        template<class Value> [[nodiscard]] auto boost_expm1(const Value& value)
        {
            using boost::multiprecision::expm1;
            return expm1(value);
        }

        template<class Value> [[nodiscard]] auto boost_log(const Value& value)
        {
            using boost::multiprecision::log;
            return log(value);
        }

        template<class Value> [[nodiscard]] auto boost_log10(const Value& value)
        {
            using boost::multiprecision::log10;
            return log10(value);
        }

        template<class Value> [[nodiscard]] auto boost_log1p(const Value& value)
        {
            using boost::multiprecision::log1p;
            return log1p(value);
        }

        template<class Value> [[nodiscard]] auto boost_pow(const Value& x, const Value& y)
        {
            using boost::multiprecision::pow;
            return pow(x, y);
        }

        template<class Value> [[nodiscard]] auto boost_ipow(const Value& value)
        {
            using boost::multiprecision::pow;
            return pow(value, 7);
        }

        template<class Value> [[nodiscard]] auto boost_sinh(const Value& value)
        {
            using boost::multiprecision::sinh;
            return sinh(value);
        }

        template<class Value> [[nodiscard]] auto boost_cosh(const Value& value)
        {
            using boost::multiprecision::cosh;
            return cosh(value);
        }

        template<class Value> [[nodiscard]] auto boost_tanh(const Value& value)
        {
            using boost::multiprecision::tanh;
            return tanh(value);
        }

        template<class Value> [[nodiscard]] auto boost_asinh(const Value& value)
        {
            using boost::multiprecision::asinh;
            return asinh(value);
        }

        template<class Value> [[nodiscard]] auto boost_acosh(const Value& value)
        {
            using boost::multiprecision::acosh;
            return acosh(value);
        }

        template<class Value> [[nodiscard]] auto boost_atanh(const Value& value)
        {
            using boost::multiprecision::atanh;
            return atanh(value);
        }

        template<class Value> [[nodiscard]] auto boost_erf(const Value& value)
        {
            using boost::multiprecision::erf;
            return erf(value);
        }

        template<class Value> [[nodiscard]] auto boost_erfc(const Value& value)
        {
            using boost::multiprecision::erfc;
            return erfc(value);
        }

        template<class Value> [[nodiscard]] auto boost_lgamma(const Value& value)
        {
            using boost::multiprecision::lgamma;
            return lgamma(value);
        }

        template<class Value> [[nodiscard]] auto boost_tgamma(const Value& value)
        {
            using boost::multiprecision::tgamma;
            return tgamma(value);
        }

        template<class Value> [[nodiscard]] auto magnitude(Value value)
        {
            return value < Value{0} ? -value : value;
        }

        template<class Value> [[nodiscard]] auto reference_trunc(const Value& value)
        {
#if FLTX_METRICS_HAS_QDPP
            return ::aint(value);
#else
            return value;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_remainder(const Value& x, const Value& y)
        {
#if FLTX_METRICS_HAS_QDPP
            return ::drem(x, y);
#else
            return x - y;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_cbrt(const Value& value)
        {
#if FLTX_METRICS_HAS_QDPP
            const auto root = ::nroot(magnitude(value), 3);
            return value < 0.0 ? -root : root;
#else
            return value;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_ipow(const Value& value)
        {
#if FLTX_METRICS_HAS_QDPP
            return ::npwr(value, 7);
#else
            return value;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_sincos(const Value& value)
        {
#if FLTX_METRICS_HAS_QDPP
            Value sine{};
            Value cosine{};
            ::sincos(value, sine, cosine);
            return sine + cosine;
#else
            return value;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_sqr(const Value& value)
        {
#if FLTX_METRICS_HAS_QDPP
            return ::sqr(value);
#else
            return value * value;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_recip(const Value& value)
        {
#if FLTX_METRICS_HAS_QDPP
            return ::inv(value);
#else
            return Value{1} / value;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_divrem(const Value& x, const Value& y)
        {
#if FLTX_METRICS_HAS_QDPP
            Value remainder{};
            const Value quotient = ::divrem(x, y, remainder);
            return std::pair{remainder, quotient};
#else
            return std::pair{x, y};
#endif
        }

        template<class Value>
        [[nodiscard]] std::string reference_to_string(const Value& value, int digits)
        {
#if FLTX_METRICS_HAS_QDPP
            return value.to_string(digits, 0, std::ios_base::scientific);
#else
            (void)value;
            (void)digits;
            return {};
#endif
        }

        template<class Value> [[nodiscard]] auto reference_parse(const std::string& text)
        {
#if FLTX_METRICS_HAS_QDPP
            return Value::read(text.c_str());
#else
            (void)text;
            return Value{};
#endif
        }

        template<class Value, class Engine> [[nodiscard]] auto reference_uniform(Engine& engine)
        {
#if FLTX_METRICS_HAS_QDPP
            if constexpr (std::is_same_v<Value, dd_real>)
                return ::ddrand(engine);
            else
                return ::qdrand(engine);
#else
            (void)engine;
            return Value{};
#endif
        }

        template<class Value> [[nodiscard]] auto reference_min(const Value& x, const Value& y)
        {
#if FLTX_METRICS_HAS_QDPP
            return (::min)(x, y);
#else
            return x < y ? x : y;
#endif
        }

        template<class Value> [[nodiscard]] auto reference_max(const Value& x, const Value& y)
        {
#if FLTX_METRICS_HAS_QDPP
            return (::max)(x, y);
#else
            return x < y ? y : x;
#endif
        }

        template<class Float> void run_operations(csv_writer& output, const options& settings)
        {
            runner<Float> run(output, settings);

            run.binary(
                "arithmetic", "add", [](const auto& x, const auto& y) { return x + y; },
                qdpp<Float>("qdpp operator+", [](const auto& x, const auto& y) { return x + y; }),
                boost_impl<Float>("Boost operator+",
                                  [](const auto& x, const auto& y) { return x + y; }),
                tlfloat_impl<Float>("TLFloat operator+",
                                    [](const auto& x, const auto& y) { return x + y; }));
            run.binary(
                "arithmetic", "subtract", [](const auto& x, const auto& y) { return x - y; },
                qdpp<Float>("qdpp operator-", [](const auto& x, const auto& y) { return x - y; }),
                boost_impl<Float>("Boost operator-",
                                  [](const auto& x, const auto& y) { return x - y; }),
                tlfloat_impl<Float>("TLFloat operator-",
                                    [](const auto& x, const auto& y) { return x - y; }));
            run.binary(
                "arithmetic", "multiply", [](const auto& x, const auto& y) { return x * y; },
                qdpp<Float>("qdpp operator*", [](const auto& x, const auto& y) { return x * y; }),
                boost_impl<Float>("Boost operator*",
                                  [](const auto& x, const auto& y) { return x * y; }),
                tlfloat_impl<Float>("TLFloat operator*",
                                    [](const auto& x, const auto& y) { return x * y; }));
            run.binary(
                "arithmetic", "divide", [](const auto& x, const auto& y) { return x / y; },
                qdpp<Float>("qdpp operator/", [](const auto& x, const auto& y) { return x / y; }),
                boost_impl<Float>("boost::multiprecision::operator/",
                                  [](const auto& x, const auto& y) { return x / y; }),
                tlfloat_impl<Float>("TLFloat operator/",
                                    [](const auto& x, const auto& y) { return x / y; }));
            run.ternary(
                "floating_point_utilities", "fma",
                [](const auto& x, const auto& y, const auto& z) { return bl::fma(x, y, z); },
                boost_impl<Float>(
                    "boost::multiprecision::fma",
                    [](const auto& x, const auto& y, const auto& z) { return boost_fma(x, y, z); }),
                tlfloat_impl<Float>("tlfloat::fma",
                                    [](const auto& x, const auto& y, const auto& z) {
                                        return tlfloat_ops::fma(x, y, z);
                                    }));
            run.unary(
                "floating_point_utilities", "abs", [](const auto& x) { return bl::abs(x); },
                qdpp<Float>("qdpp abs", [](const auto& x) { return ::abs(x); }),
                boost_impl<Float>("boost::multiprecision::abs",
                                  [](const auto& x) { return boost_abs(x); }),
                tlfloat_impl<Float>("tlfloat::abs",
                                    [](const auto& x) { return tlfloat_ops::abs(x); }));
            run.unary(
                "floating_point_utilities", "fabs", [](const auto& x) { return bl::fabs(x); },
                qdpp<Float>("qdpp fabs", [](const auto& x) { return ::fabs(x); }),
                boost_impl<Float>("boost::multiprecision::fabs",
                                  [](const auto& x) { return boost_fabs(x); }),
                tlfloat_impl<Float>("tlfloat::abs",
                                    [](const auto& x) { return tlfloat_ops::abs(x); }));
            run.unary(
                "roots_and_powers", "sqr", [](const auto& x) { return bl::sqr(x); },
                qdpp<Float>("qdpp sqr", [](const auto& x) { return reference_sqr(x); }));
            run.unary(
                "floating_point_utilities", "recip", [](const auto& x) { return bl::recip(x); },
                qdpp<Float>("qdpp inv", [](const auto& x) { return reference_recip(x); }));
            run.binary(
                "floating_point_utilities", "fmin", [](const auto& x, const auto& y) { return bl::fmin(x, y); },
                qdpp_if<Float>(std::is_same_v<Float, bl::f256>, "qdpp min",
                               [](const auto& x, const auto& y) { return reference_min(x, y); }),
                boost_impl<Float>("boost::multiprecision::fmin",
                                  [](const auto& x, const auto& y) { return boost_fmin(x, y); }),
                tlfloat_impl<Float>("tlfloat::fmin", [](const auto& x, const auto& y) {
                    return tlfloat_ops::fmin(x, y);
                }));
            run.binary(
                "floating_point_utilities", "fmax", [](const auto& x, const auto& y) { return bl::fmax(x, y); },
                qdpp_if<Float>(std::is_same_v<Float, bl::f256>, "qdpp max",
                               [](const auto& x, const auto& y) { return reference_max(x, y); }),
                boost_impl<Float>("boost::multiprecision::fmax",
                                  [](const auto& x, const auto& y) { return boost_fmax(x, y); }),
                tlfloat_impl<Float>("tlfloat::fmax", [](const auto& x, const auto& y) {
                    return tlfloat_ops::fmax(x, y);
                }));
            run.binary(
                "floating_point_utilities", "fdim", [](const auto& x, const auto& y) { return bl::fdim(x, y); },
                boost_impl<Float>("boost::multiprecision::fdim",
                                  [](const auto& x, const auto& y) { return boost_fdim(x, y); }),
                tlfloat_impl<Float>("tlfloat::fdim", [](const auto& x, const auto& y) {
                    return tlfloat_ops::fdim(x, y);
                }));
            run.binary(
                "floating_point_utilities", "copysign",
                [](const auto& x, const auto& y) { return bl::copysign(x, y); },
                boost_impl<Float>(
                    "boost::multiprecision::copysign",
                    [](const auto& x, const auto& y) { return boost_copysign(x, y); }),
                tlfloat_impl<Float>("tlfloat::copysign", [](const auto& x, const auto& y) {
                    return tlfloat_ops::copysign(x, y);
                }));

            run.unary(
                "rounding", "floor", [](const auto& x) { return bl::floor(x); },
                qdpp<Float>("qdpp floor", [](const auto& x) { return ::floor(x); }),
                boost_impl<Float>("boost::multiprecision::floor",
                                  [](const auto& x) { return boost_floor(x); }),
                tlfloat_impl<Float>("tlfloat::floor",
                                    [](const auto& x) { return tlfloat_ops::floor(x); }));
            run.unary(
                "rounding", "ceil", [](const auto& x) { return bl::ceil(x); },
                qdpp<Float>("qdpp ceil", [](const auto& x) { return ::ceil(x); }),
                boost_impl<Float>("boost::multiprecision::ceil",
                                  [](const auto& x) { return boost_ceil(x); }),
                tlfloat_impl<Float>("tlfloat::ceil",
                                    [](const auto& x) { return tlfloat_ops::ceil(x); }));
            run.unary(
                "rounding", "trunc", [](const auto& x) { return bl::trunc(x); },
                qdpp<Float>("qdpp aint", [](const auto& x) { return reference_trunc(x); }),
                boost_impl<Float>("boost::multiprecision::trunc",
                                  [](const auto& x) { return boost_trunc(x); }),
                tlfloat_impl<Float>("tlfloat::trunc",
                                    [](const auto& x) { return tlfloat_ops::trunc(x); }));
            run.unary(
                "rounding", "round", [](const auto& x) { return bl::round(x); },
                qdpp<Float>("qdpp round", [](const auto& x) { return ::round(x); }),
                boost_impl<Float>("boost::multiprecision::round",
                                  [](const auto& x) { return boost_round(x); }),
                tlfloat_impl<Float>("tlfloat::round",
                                    [](const auto& x) { return tlfloat_ops::round(x); }));
            run.unary(
                "rounding", "roundeven", [](const auto& x) { return bl::roundeven(x); },
                tlfloat_impl<Float>("tlfloat::rint",
                                    [](const auto& x) { return tlfloat_ops::roundeven(x); }));
            run.unary_result(
                "rounding", "lround", [](const auto& x) { return bl::lround(x); },
                boost_impl<Float>("boost::multiprecision::lround",
                                  [](const auto& x) { return boost_lround(x); }));
            run.unary_result(
                "rounding", "llround", [](const auto& x) { return bl::llround(x); },
                boost_impl<Float>("boost::multiprecision::llround",
                                  [](const auto& x) { return boost_llround(x); }));
            run.unary("rounding", "round_decimals",
                      [](const auto& x) { return bl::round_decimals(x, 3); });
            run.unary("rounding", "round_significant",
                      [](const auto& x) { return bl::round_significant(x, 4); });

            run.binary(
                "remainders", "fmod", [](const auto& x, const auto& y) { return bl::fmod(x, y); },
                qdpp<Float>("qdpp fmod", [](const auto& x, const auto& y) { return ::fmod(x, y); }),
                boost_impl<Float>("boost::multiprecision::fmod",
                                  [](const auto& x, const auto& y) { return boost_fmod(x, y); }),
                tlfloat_impl<Float>("tlfloat::fmod", [](const auto& x, const auto& y) {
                    return tlfloat_ops::fmod(x, y);
                }));
            run.binary(
                "remainders", "remainder",
                [](const auto& x, const auto& y) { return bl::remainder(x, y); },
                qdpp<Float>("qdpp drem",
                            [](const auto& x, const auto& y) { return reference_remainder(x, y); }),
                boost_impl<Float>(
                    "boost::multiprecision::remainder",
                    [](const auto& x, const auto& y) { return boost_remainder(x, y); }),
                tlfloat_impl<Float>("tlfloat::remainder", [](const auto& x, const auto& y) {
                    return tlfloat_ops::remainder(x, y);
                }));
            run.binary_result(
                "remainders", "remquo",
                [](const auto& x, const auto& y) {
                    int quotient = 0;
                    const auto result = bl::remquo(x, y, &quotient);
                    return std::pair{result, quotient};
                },
                qdpp<Float>("qdpp divrem",
                            [](const auto& x, const auto& y) { return reference_divrem(x, y); }),
                boost_impl<Float>("boost::multiprecision::remquo",
                                  [](const auto& x, const auto& y) { return boost_remquo(x, y); }),
                tlfloat_impl<Float>("tlfloat::remquo", [](const auto& x, const auto& y) {
                    return tlfloat_ops::remquo(x, y);
                }));

            run.unary_result(
                "floating_point_utilities", "modf",
                [](const auto& x) {
                    std::remove_cvref_t<decltype(x)> integral{};
                    const auto fraction = bl::modf(x, &integral);
                    return std::pair{fraction, integral};
                },
                boost_impl<Float>("boost::multiprecision::modf",
                                  [](const auto& x) { return boost_modf(x); }),
                tlfloat_impl<Float>("tlfloat::modf",
                                    [](const auto& x) { return tlfloat_ops::modf(x); }));
            run.unary(
                "floating_point_utilities", "ldexp", [](const auto& x) { return bl::ldexp(x, 7); },
                qdpp<Float>("qdpp ldexp", [](const auto& x) { return ::ldexp(x, 7); }),
                boost_impl<Float>("boost::multiprecision::ldexp",
                                  [](const auto& x) {
                                      using boost::multiprecision::ldexp;
                                      return ldexp(x, 7);
                                  }),
                tlfloat_impl<Float>("tlfloat::ldexp",
                                    [](const auto& x) { return tlfloat_ops::ldexp(x, 7); }));
            run.unary(
                "floating_point_utilities", "scalbn", [](const auto& x) { return bl::scalbn(x, -7); },
                qdpp<Float>("qdpp ldexp", [](const auto& x) { return ::ldexp(x, -7); }),
                boost_impl<Float>("boost::multiprecision::scalbn", [](const auto& x) {
                    using boost::multiprecision::scalbn;
                    return scalbn(x, -7);
                }));
            run.unary(
                "floating_point_utilities", "scalbln", [](const auto& x) { return bl::scalbln(x, -7L); },
                qdpp<Float>("qdpp ldexp", [](const auto& x) { return ::ldexp(x, -7); }),
                boost_impl<Float>("boost::multiprecision::scalbln", [](const auto& x) {
                    using boost::multiprecision::scalbln;
                    return scalbln(x, -7L);
                }));
            run.unary_result(
                "floating_point_utilities", "frexp",
                [](const auto& x) {
                    int exponent = 0;
                    const auto fraction = bl::frexp(x, &exponent);
                    return std::pair{fraction, exponent};
                },
                boost_impl<Float>("boost::multiprecision::frexp",
                                  [](const auto& x) { return boost_frexp(x); }),
                tlfloat_impl<Float>("tlfloat::frexp",
                                    [](const auto& x) { return tlfloat_ops::frexp(x); }));
            run.unary_result(
                "floating_point_utilities", "ilogb", [](const auto& x) { return bl::ilogb(x); },
                boost_impl<Float>("boost::multiprecision::ilogb",
                                  [](const auto& x) { return boost_ilogb(x); }),
                tlfloat_impl<Float>("tlfloat::ilogb",
                                    [](const auto& x) { return tlfloat_ops::ilogb(x); }));
            run.unary(
                "floating_point_utilities", "logb", [](const auto& x) { return bl::logb(x); },
                boost_impl<Float>("boost::multiprecision::logb",
                                  [](const auto& x) { return boost_logb(x); }));

            run.binary(
                "floating_point_utilities", "nextafter",
                [](const auto& x, const auto& y) { return bl::nextafter(x, y); },
                boost_impl<Float>(
                    "boost::math::nextafter",
                    [](const auto& x, const auto& y) { return boost_nextafter(x, y); }),
                tlfloat_impl<Float>("tlfloat::nextafter", [](const auto& x, const auto& y) {
                    return tlfloat_ops::nextafter(x, y);
                }));
            run.binary(
                "floating_point_utilities", "nexttoward",
                [](const auto& x, const auto& y) { return bl::nexttoward(x, y); },
                boost_impl<Float>(
                    "boost::math::nextafter (same type)",
                    [](const auto& x, const auto& y) { return boost_nextafter(x, y); }),
                tlfloat_impl<Float>(
                    "tlfloat::nextafter (same type)",
                    [](const auto& x, const auto& y) { return tlfloat_ops::nextafter(x, y); }));

            run.binary_result(
                "comparisons", "equal", [](const auto& x, const auto& y) { return x == y; },
                qdpp<Float>("qdpp operator==", [](const auto& x, const auto& y) { return x == y; }),
                boost_impl<Float>("Boost operator==",
                                  [](const auto& x, const auto& y) { return x == y; }),
                tlfloat_impl<Float>("TLFloat operator==",
                                    [](const auto& x, const auto& y) { return x == y; }));
            run.binary_result(
                "comparisons", "not_equal", [](const auto& x, const auto& y) { return x != y; },
                qdpp<Float>("qdpp operator!=", [](const auto& x, const auto& y) { return x != y; }),
                boost_impl<Float>("Boost operator!=",
                                  [](const auto& x, const auto& y) { return x != y; }),
                tlfloat_impl<Float>("TLFloat operator!=",
                                    [](const auto& x, const auto& y) { return x != y; }));
            run.binary_result(
                "comparisons", "less", [](const auto& x, const auto& y) { return x < y; },
                qdpp<Float>("qdpp operator<", [](const auto& x, const auto& y) { return x < y; }),
                boost_impl<Float>("Boost operator<",
                                  [](const auto& x, const auto& y) { return x < y; }),
                tlfloat_impl<Float>("TLFloat operator<",
                                    [](const auto& x, const auto& y) { return x < y; }));
            run.binary_result(
                "comparisons", "less_equal", [](const auto& x, const auto& y) { return x <= y; },
                qdpp<Float>("qdpp operator<=", [](const auto& x, const auto& y) { return x <= y; }),
                boost_impl<Float>("Boost operator<=",
                                  [](const auto& x, const auto& y) { return x <= y; }),
                tlfloat_impl<Float>("TLFloat operator<=",
                                    [](const auto& x, const auto& y) { return x <= y; }));
            run.binary_result(
                "comparisons", "greater", [](const auto& x, const auto& y) { return x > y; },
                qdpp<Float>("qdpp operator>", [](const auto& x, const auto& y) { return x > y; }),
                boost_impl<Float>("Boost operator>",
                                  [](const auto& x, const auto& y) { return x > y; }),
                tlfloat_impl<Float>("TLFloat operator>",
                                    [](const auto& x, const auto& y) { return x > y; }));
            run.binary_result(
                "comparisons", "greater_equal", [](const auto& x, const auto& y) { return x >= y; },
                qdpp<Float>("qdpp operator>=", [](const auto& x, const auto& y) { return x >= y; }),
                boost_impl<Float>("Boost operator>=",
                                  [](const auto& x, const auto& y) { return x >= y; }),
                tlfloat_impl<Float>("TLFloat operator>=",
                                    [](const auto& x, const auto& y) { return x >= y; }));

            run.unary(
                "roots_and_powers", "sqrt", [](const auto& x) { return bl::sqrt(x); },
                qdpp<Float>("qdpp sqrt", [](const auto& x) { return ::sqrt(x); }),
                boost_impl<Float>("boost::multiprecision::sqrt",
                                  [](const auto& x) { return boost_sqrt(x); }),
                tlfloat_impl<Float>("tlfloat::sqrt",
                                    [](const auto& x) { return tlfloat_ops::sqrt(x); }));
            run.unary(
                "roots_and_powers", "cbrt", [](const auto& x) { return bl::cbrt(x); },
                qdpp<Float>("qdpp nroot", [](const auto& x) { return reference_cbrt(x); }),
                boost_impl<Float>("boost::multiprecision::cbrt",
                                  [](const auto& x) { return boost_cbrt(x); }),
                tlfloat_impl<Float>("tlfloat::cbrt",
                                    [](const auto& x) { return tlfloat_ops::cbrt(x); }));
            run.binary(
                "roots_and_powers", "hypot", [](const auto& x, const auto& y) { return bl::hypot(x, y); },
                boost_impl<Float>("boost::multiprecision::hypot",
                                  [](const auto& x, const auto& y) { return boost_hypot(x, y); }),
                tlfloat_impl<Float>("tlfloat::hypot", [](const auto& x, const auto& y) {
                    return tlfloat_ops::hypot(x, y);
                }));

            run.unary(
                "trigonometric", "sin", [](const auto& x) { return bl::sin(x); },
                qdpp<Float>("qdpp sin", [](const auto& x) { return ::sin(x); }),
                boost_impl<Float>("boost::multiprecision::sin",
                                  [](const auto& x) { return boost_sin(x); }),
                tlfloat_impl<Float>("tlfloat::sin",
                                    [](const auto& x) { return tlfloat_ops::sin(x); }));
            run.unary(
                "trigonometric", "cos", [](const auto& x) { return bl::cos(x); },
                qdpp<Float>("qdpp cos", [](const auto& x) { return ::cos(x); }),
                boost_impl<Float>("boost::multiprecision::cos",
                                  [](const auto& x) { return boost_cos(x); }),
                tlfloat_impl<Float>("tlfloat::cos",
                                    [](const auto& x) { return tlfloat_ops::cos(x); }));
            run.unary(
                "trigonometric", "sincos",
                [](const auto& x) {
                    std::remove_cvref_t<decltype(x)> sine{};
                    std::remove_cvref_t<decltype(x)> cosine{};
                    (void)bl::sincos(x, sine, cosine);
                    return sine + cosine;
                },
                qdpp<Float>("qdpp sincos", [](const auto& x) { return reference_sincos(x); }),
                tlfloat_impl<Float>("tlfloat::sincos",
                                    [](const auto& x) { return tlfloat_ops::sincos_sum(x); }));
            run.unary(
                "trigonometric", "tan", [](const auto& x) { return bl::tan(x); },
                qdpp<Float>("qdpp tan", [](const auto& x) { return ::tan(x); }),
                boost_impl<Float>("boost::multiprecision::tan",
                                  [](const auto& x) { return boost_tan(x); }),
                tlfloat_impl<Float>("tlfloat::tan",
                                    [](const auto& x) { return tlfloat_ops::tan(x); }));
            run.unary(
                "trigonometric", "atan", [](const auto& x) { return bl::atan(x); },
                qdpp<Float>("qdpp atan", [](const auto& x) { return ::atan(x); }),
                boost_impl<Float>("boost::multiprecision::atan",
                                  [](const auto& x) { return boost_atan(x); }),
                tlfloat_impl<Float>("tlfloat::atan",
                                    [](const auto& x) { return tlfloat_ops::atan(x); }));
            run.binary(
                "trigonometric", "atan2", [](const auto& y, const auto& x) { return bl::atan2(y, x); },
                qdpp<Float>("qdpp atan2",
                            [](const auto& y, const auto& x) { return ::atan2(y, x); }),
                boost_impl<Float>("boost::multiprecision::atan2",
                                  [](const auto& y, const auto& x) { return boost_atan2(y, x); }),
                tlfloat_impl<Float>("tlfloat::atan2", [](const auto& x, const auto& y) {
                    return tlfloat_ops::atan2(x, y);
                }));
            run.unary(
                "trigonometric", "asin", [](const auto& x) { return bl::asin(x); },
                qdpp<Float>("qdpp asin", [](const auto& x) { return ::asin(x); }),
                boost_impl<Float>("boost::multiprecision::asin",
                                  [](const auto& x) { return boost_asin(x); }),
                tlfloat_impl<Float>("tlfloat::asin",
                                    [](const auto& x) { return tlfloat_ops::asin(x); }));
            run.unary(
                "trigonometric", "acos", [](const auto& x) { return bl::acos(x); },
                qdpp<Float>("qdpp acos", [](const auto& x) { return ::acos(x); }),
                boost_impl<Float>("boost::multiprecision::acos",
                                  [](const auto& x) { return boost_acos(x); }),
                tlfloat_impl<Float>("tlfloat::acos",
                                    [](const auto& x) { return tlfloat_ops::acos(x); }));

            run.unary(
                "exponentials", "exp", [](const auto& x) { return bl::exp(x); },
                qdpp<Float>("qdpp exp", [](const auto& x) { return ::exp(x); }),
                boost_impl<Float>("boost::multiprecision::exp",
                                  [](const auto& x) { return boost_exp(x); }),
                tlfloat_impl<Float>("tlfloat::exp",
                                    [](const auto& x) { return tlfloat_ops::exp(x); }));
            run.unary(
                "exponentials", "exp2", [](const auto& x) { return bl::exp2(x); },
                boost_impl<Float>("boost::multiprecision::exp2",
                                  [](const auto& x) { return boost_exp2(x); }),
                tlfloat_impl<Float>("tlfloat::exp2",
                                    [](const auto& x) { return tlfloat_ops::exp2(x); }));
            run.unary(
                "exponentials", "expm1", [](const auto& x) { return bl::expm1(x); },
                qdpp<Float>("qdpp expm1", [](const auto& x) { return ::expm1(x); }),
                boost_impl<Float>("boost::multiprecision::expm1",
                                  [](const auto& x) { return boost_expm1(x); }),
                tlfloat_impl<Float>("tlfloat::expm1",
                                    [](const auto& x) { return tlfloat_ops::expm1(x); }));
            run.unary(
                "logarithms", "log", [](const auto& x) { return bl::log(x); },
                qdpp<Float>("qdpp log", [](const auto& x) { return ::log(x); }),
                boost_impl<Float>("boost::multiprecision::log",
                                  [](const auto& x) { return boost_log(x); }),
                tlfloat_impl<Float>("tlfloat::log",
                                    [](const auto& x) { return tlfloat_ops::log(x); }));
            run.unary(
                "logarithms", "log2", [](const auto& x) { return bl::log2(x); },
                boost_impl<Float>("boost::multiprecision::log2",
                                  [](const auto& x) { return boost_log2(x); }),
                tlfloat_impl<Float>("tlfloat::log2",
                                    [](const auto& x) { return tlfloat_ops::log2(x); }));
            run.unary(
                "logarithms", "log10", [](const auto& x) { return bl::log10(x); },
                qdpp<Float>("qdpp log10", [](const auto& x) { return ::log10(x); }),
                boost_impl<Float>("boost::multiprecision::log10",
                                  [](const auto& x) { return boost_log10(x); }),
                tlfloat_impl<Float>("tlfloat::log10",
                                    [](const auto& x) { return tlfloat_ops::log10(x); }));
            run.unary(
                "logarithms", "log1p", [](const auto& x) { return bl::log1p(x); },
                qdpp<Float>("qdpp log1p", [](const auto& x) { return ::log1p(x); }),
                boost_impl<Float>("boost::multiprecision::log1p",
                                  [](const auto& x) { return boost_log1p(x); }),
                tlfloat_impl<Float>("tlfloat::log1p",
                                    [](const auto& x) { return tlfloat_ops::log1p(x); }));
            run.binary(
                "roots_and_powers", "pow", [](const auto& x, const auto& y) { return bl::pow(x, y); },
                qdpp<Float>("qdpp pow", [](const auto& x, const auto& y) { return ::pow(x, y); }),
                boost_impl<Float>("boost::multiprecision::pow",
                                  [](const auto& x, const auto& y) { return boost_pow(x, y); }),
                tlfloat_impl<Float>("tlfloat::pow", [](const auto& x, const auto& y) {
                    return tlfloat_ops::pow(x, y);
                }));
            run.unary(
                "roots_and_powers", "ipow", [](const auto& x) { return bl::ipow(x, 7); },
                qdpp<Float>("qdpp npwr", [](const auto& x) { return reference_ipow(x); }),
                boost_impl<Float>("boost::multiprecision::pow(value, int)",
                                  [](const auto& x) { return boost_ipow(x); }));

            run.ternary(
                "mixed_workloads", "product_sum",
                [](const auto& x, const auto& y, const auto& z) { return x * y + y * z; },
                qdpp<Float>("same expression", [](const auto& x, const auto& y,
                                                  const auto& z) { return x * y + y * z; }),
                boost_impl<Float>("same expression", [](const auto& x, const auto& y,
                                                        const auto& z) { return x * y + y * z; }),
                tlfloat_impl<Float>(
                    "same expression",
                    [](const auto& x, const auto& y, const auto& z) { return x * y + y * z; }));

            run.unary(
                "hyperbolic", "sinh", [](const auto& x) { return bl::sinh(x); },
                qdpp<Float>("qdpp sinh", [](const auto& x) { return ::sinh(x); }),
                boost_impl<Float>("boost::multiprecision::sinh",
                                  [](const auto& x) { return boost_sinh(x); }),
                tlfloat_impl<Float>("tlfloat::sinh",
                                    [](const auto& x) { return tlfloat_ops::sinh(x); }));
            run.unary(
                "hyperbolic", "cosh", [](const auto& x) { return bl::cosh(x); },
                qdpp<Float>("qdpp cosh", [](const auto& x) { return ::cosh(x); }),
                boost_impl<Float>("boost::multiprecision::cosh",
                                  [](const auto& x) { return boost_cosh(x); }),
                tlfloat_impl<Float>("tlfloat::cosh",
                                    [](const auto& x) { return tlfloat_ops::cosh(x); }));
            run.unary(
                "hyperbolic", "tanh", [](const auto& x) { return bl::tanh(x); },
                qdpp<Float>("qdpp tanh", [](const auto& x) { return ::tanh(x); }),
                boost_impl<Float>("boost::multiprecision::tanh",
                                  [](const auto& x) { return boost_tanh(x); }),
                tlfloat_impl<Float>("tlfloat::tanh",
                                    [](const auto& x) { return tlfloat_ops::tanh(x); }));
            run.unary(
                "inverse_hyperbolic", "asinh", [](const auto& x) { return bl::asinh(x); },
                qdpp<Float>("qdpp asinh", [](const auto& x) { return ::asinh(x); }),
                boost_impl<Float>("boost::multiprecision::asinh",
                                  [](const auto& x) { return boost_asinh(x); }),
                tlfloat_impl<Float>("tlfloat::asinh",
                                    [](const auto& x) { return tlfloat_ops::asinh(x); }));
            run.unary(
                "inverse_hyperbolic", "acosh", [](const auto& x) { return bl::acosh(x); },
                qdpp<Float>("qdpp acosh", [](const auto& x) { return ::acosh(x); }),
                boost_impl<Float>("boost::multiprecision::acosh",
                                  [](const auto& x) { return boost_acosh(x); }),
                tlfloat_impl<Float>("tlfloat::acosh",
                                    [](const auto& x) { return tlfloat_ops::acosh(x); }));
            run.unary(
                "inverse_hyperbolic", "atanh", [](const auto& x) { return bl::atanh(x); },
                qdpp<Float>("qdpp atanh", [](const auto& x) { return ::atanh(x); }),
                boost_impl<Float>("boost::multiprecision::atanh",
                                  [](const auto& x) { return boost_atanh(x); }),
                tlfloat_impl<Float>("tlfloat::atanh",
                                    [](const auto& x) { return tlfloat_ops::atanh(x); }));

            run.unary(
                "special_functions", "erf", [](const auto& x) { return bl::erf(x); },
                boost_impl<Float>("boost::multiprecision::erf",
                                  [](const auto& x) { return boost_erf(x); }),
                tlfloat_impl<Float>("tlfloat::erf",
                                    [](const auto& x) { return tlfloat_ops::erf(x); }));
            run.unary(
                "special_functions", "erfc", [](const auto& x) { return bl::erfc(x); },
                boost_impl<Float>("boost::multiprecision::erfc",
                                  [](const auto& x) { return boost_erfc(x); }),
                tlfloat_impl<Float>("tlfloat::erfc",
                                    [](const auto& x) { return tlfloat_ops::erfc(x); }));
            run.unary(
                "special_functions", "lgamma", [](const auto& x) { return bl::lgamma(x); },
                boost_impl<Float>("boost::multiprecision::lgamma",
                                  [](const auto& x) { return boost_lgamma(x); }),
                tlfloat_impl<Float>("tlfloat::lgamma",
                                    [](const auto& x) { return tlfloat_ops::lgamma(x); }));
            run.unary(
                "special_functions", "tgamma", [](const auto& x) { return bl::tgamma(x); },
                boost_impl<Float>("boost::multiprecision::tgamma",
                                  [](const auto& x) { return boost_tgamma(x); }),
                tlfloat_impl<Float>("tlfloat::tgamma",
                                    [](const auto& x) { return tlfloat_ops::tgamma(x); }));

            const auto values = run.template values<Float>();
            const auto qd_values = run.template values<qd_value<Float>>();
            const auto boost_values = run.template values<boost_value<Float>>();
            const auto tlfloat_values = run.template values<tlfloat_value<Float>>();
            constexpr int digits = std::numeric_limits<Float>::max_digits10;
            run.measured_task(
                "io", "to_string", values.size(),
                [&](std::size_t batches) {
                    double checksum = 0.0;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        for (const Float& value : values)
                        {
                            checksum += static_cast<double>(
                                bl::to_string(value, digits, std::ios_base::scientific).size());
                        }
                    }
                    return checksum;
                },
                task(implementations::qdpp_identity<Float>, "qdpp to_string",
                     [&](std::size_t batches) {
                         double checksum = 0.0;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (const auto& value : qd_values)
                                 checksum +=
                                     static_cast<double>(reference_to_string(value, digits).size());
                         }
                         return checksum;
                     }),
                task(implementations::boost_identity<Float>, "Boost number::str",
                     [&](std::size_t batches) {
                         double checksum = 0.0;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (const auto& value : boost_values)
                             {
                                 checksum += static_cast<double>(
                                     value.str(digits, std::ios_base::scientific).size());
                             }
                         }
                         return checksum;
                     }),
                task(implementations::tlfloat_identity<Float>, "tlfloat::to_string",
                     [&](std::size_t batches) {
                         double checksum = 0.0;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (const auto& value : tlfloat_values)
                             {
                                 checksum += static_cast<double>(
                                     tlfloat_ops::to_string(value, digits).size());
                             }
                         }
                         return checksum;
                     }));
            run.measured_task(
                "io", "to_chars", values.size(),
                [&](std::size_t batches) {
                    std::array<char, 512> buffer{};
                    double checksum = 0.0;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        for (const Float& value : values)
                        {
                            const auto result =
                                bl::to_chars(buffer.data(), buffer.data() + buffer.size(), value,
                                             std::chars_format::scientific, digits);
                            checksum += static_cast<double>(result.ptr - buffer.data());
                        }
                    }
                    return checksum;
                },
                task(implementations::qdpp_identity<Float>, "qdpp write", [&](std::size_t batches) {
                    std::array<char, 512> buffer{};
                    double checksum = 0.0;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        for (const auto& value : qd_values)
                        {
#if FLTX_METRICS_HAS_QDPP
                            value.write(buffer.data(), static_cast<int>(buffer.size()), digits,
                                        false, false);
                            checksum += static_cast<unsigned char>(buffer[0]);
                            checksum += static_cast<unsigned char>(buffer[digits]);
#else
                                (void)value;
#endif
                        }
                    }
                    return checksum;
                }));

            const std::vector<std::string> texts = samples::decimal_strings(values.size(), digits);
            if (digits <= std::numeric_limits<double>::max_digits10 ||
                texts != samples::decimal_strings(values.size(), digits))
            {
                throw std::logic_error(
                    "parse corpus must be deterministic and exceed double precision");
            }

            run.measured_task(
                "io", "parse", texts.size(),
                [&](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                        for (const std::string& text : texts)
                            checksum.add(bl::parse<Float>(text));
                    return checksum.value();
                },
                task(implementations::qdpp_identity<Float>, "qdpp read",
                     [&](std::size_t batches) {
                         checksum_accumulator checksum;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (const std::string& text : texts)
                                 checksum.add(reference_parse<qd_value<Float>>(text));
                         }
                         return checksum.value();
                     }),
                task(implementations::boost_identity<Float>, "Boost string constructor",
                     [&](std::size_t batches) {
                         checksum_accumulator checksum;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (const std::string& text : texts)
                                 checksum.add(boost_value<Float>{text});
                         }
                         return checksum.value();
                     }),
                task(implementations::tlfloat_identity<Float>, "TLFloat string constructor",
                     [&](std::size_t batches) {
                         checksum_accumulator checksum;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (const std::string& text : texts)
                             {
                                 checksum.add(tlfloat_ops::parse<tlfloat_value<Float>>(text));
                             }
                         }
                         return checksum.value();
                     }));

            constexpr std::size_t random_draws = 64;
            run.measured_task("random", "mt19937_64", random_draws, [](std::size_t batches) {
                bl::mt19937_64 engine{0x1020304050607080ull};
                double checksum = 0.0;
                for (std::size_t batch = 0; batch < batches; ++batch)
                    for (std::size_t i = 0; i < random_draws; ++i)
                        checksum += static_cast<double>(engine());
                return checksum;
            });
            run.measured_task(
                "random", "uniform_real", random_draws,
                [](std::size_t batches) {
                    bl::mt19937_64 engine{0x1020304050607080ull};
                    bl::uniform_real_distribution<Float> distribution{Float{0.0}, Float{1.0}};
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                        for (std::size_t i = 0; i < random_draws; ++i)
                            checksum.add(distribution(engine));
                    return checksum.value();
                },
                task(implementations::qdpp_identity<Float>,
                     std::is_same_v<Float, bl::f128> ? "qdpp ddrand" : "qdpp qdrand",
                     [](std::size_t batches) {
                         bl::mt19937_64 engine{0x1020304050607080ull};
                         checksum_accumulator checksum;
                         for (std::size_t batch = 0; batch < batches; ++batch)
                         {
                             for (std::size_t i = 0; i < random_draws; ++i)
                             {
                                 checksum.add(reference_uniform<qd_value<Float>>(engine));
                             }
                         }
                         return checksum.value();
                     }));
            run.measured_task("random", "normal", random_draws, [](std::size_t batches) {
                bl::mt19937_64 engine{0x1020304050607080ull};
                bl::normal_distribution<Float> distribution{Float{1.0}, Float{0.5}};
                checksum_accumulator checksum;
                for (std::size_t batch = 0; batch < batches; ++batch)
                    for (std::size_t i = 0; i < random_draws; ++i)
                        checksum.add(distribution(engine));
                return checksum.value();
            });
        }
    } // namespace

    void run_operations_f128(csv_writer& output, const options& settings)
    {
        run_operations<bl::f128>(output, settings);
    }

    void run_operations_f256(csv_writer& output, const options& settings)
    {
        run_operations<bl::f256>(output, settings);
    }
} // namespace fltx::tests::benchmark
