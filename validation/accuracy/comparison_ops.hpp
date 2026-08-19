#pragma once

#include "../support/tlfloat_ops.hpp"
#include "runner.hpp"

#include <boost/math/special_functions/next.hpp>

#include <array>
#include <cstddef>
#include <ios>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace fltx::tests::accuracy::comparison_ops
{
    template<class Float>
    using qd_value = std::conditional_t<std::is_same_v<Float, bl::f128>, implementations::qd_f128,
                                        implementations::qd_f256>;

    template<class Float>
    using boost_value = std::conditional_t<std::is_same_v<Float, bl::f128>, implementations::cppdd,
                                           implementations::mpfr64>;

    template<class Float>
    using tlfloat_value = std::conditional_t<std::is_same_v<Float, bl::f128>,
                                             implementations::tl_f128, implementations::tl_f256>;

    template<class Float, class Eval> [[nodiscard]] auto qdpp(std::string_view api, Eval evaluate)
    {
        return comparison<qd_value<Float>>(implementations::qdpp_identity<Float>, api,
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
        return comparison<boost_value<Float>>(implementations::boost_identity<Float>, api,
                                              std::move(evaluate));
    }

    template<class Float, class Eval>
    [[nodiscard]] auto tlfloat_impl(std::string_view api, Eval evaluate)
    {
        return comparison<tlfloat_value<Float>>(implementations::tlfloat_identity<Float>, api,
                                                std::move(evaluate));
    }

    template<class Float>
    void register_comparison_accuracy(
        runner<Float>& run,
        std::size_t samples)
    {
        using real = mpfr::real;
        const std::vector<domains::domain> test_domains{
            domains::moderate(samples)
        };
        run.predicate(
            "comparisons", "equal", test_domains,
            [](const auto& x, const auto& y) { return x == y; },
            [](const real& x, const real& y) { return x == y; },
            qdpp<Float>("qdpp operator==",
                        [](const auto& x, const auto& y) { return x == y; }),
            boost_impl<Float>("Boost operator==",
                              [](const auto& x, const auto& y) { return x == y; }),
            tlfloat_impl<Float>("TLFloat operator==",
                                [](const auto& x, const auto& y) { return x == y; }));
        run.predicate(
            "comparisons", "not_equal", test_domains,
            [](const auto& x, const auto& y) { return x != y; },
            [](const real& x, const real& y) { return x != y; },
            qdpp<Float>("qdpp operator!=",
                        [](const auto& x, const auto& y) { return x != y; }),
            boost_impl<Float>("Boost operator!=",
                              [](const auto& x, const auto& y) { return x != y; }),
            tlfloat_impl<Float>("TLFloat operator!=",
                                [](const auto& x, const auto& y) { return x != y; }));
        run.predicate(
            "comparisons", "less", test_domains,
            [](const auto& x, const auto& y) { return x < y; },
            [](const real& x, const real& y) { return x < y; },
            qdpp<Float>("qdpp operator<",
                        [](const auto& x, const auto& y) { return x < y; }),
            boost_impl<Float>("Boost operator<",
                              [](const auto& x, const auto& y) { return x < y; }),
            tlfloat_impl<Float>("TLFloat operator<",
                                [](const auto& x, const auto& y) { return x < y; }));
        run.predicate(
            "comparisons", "less_equal", test_domains,
            [](const auto& x, const auto& y) { return x <= y; },
            [](const real& x, const real& y) { return x <= y; },
            qdpp<Float>("qdpp operator<=",
                        [](const auto& x, const auto& y) { return x <= y; }),
            boost_impl<Float>("Boost operator<=",
                              [](const auto& x, const auto& y) { return x <= y; }),
            tlfloat_impl<Float>("TLFloat operator<=",
                                [](const auto& x, const auto& y) { return x <= y; }));
        run.predicate(
            "comparisons", "greater", test_domains,
            [](const auto& x, const auto& y) { return x > y; },
            [](const real& x, const real& y) { return x > y; },
            qdpp<Float>("qdpp operator>",
                        [](const auto& x, const auto& y) { return x > y; }),
            boost_impl<Float>("Boost operator>",
                              [](const auto& x, const auto& y) { return x > y; }),
            tlfloat_impl<Float>("TLFloat operator>",
                                [](const auto& x, const auto& y) { return x > y; }));
        run.predicate(
            "comparisons", "greater_equal", test_domains,
            [](const auto& x, const auto& y) { return x >= y; },
            [](const real& x, const real& y) { return x >= y; },
            qdpp<Float>("qdpp operator>=",
                        [](const auto& x, const auto& y) { return x >= y; }),
            boost_impl<Float>("Boost operator>=",
                              [](const auto& x, const auto& y) { return x >= y; }),
            tlfloat_impl<Float>("TLFloat operator>=",
                                [](const auto& x, const auto& y) { return x >= y; }));
    }

    template<class Value> [[nodiscard]] auto qd_cbrt(const Value& value)
    {
#if FLTX_METRICS_HAS_QDPP
        const Value root = ::nroot(value < 0.0 ? -value : value, 3);
        return value < 0.0 ? -root : root;
#else
        return value;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_sqr(const Value& value)
    {
#if FLTX_METRICS_HAS_QDPP
        return ::sqr(value);
#else
        return value * value;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_inv(const Value& value)
    {
#if FLTX_METRICS_HAS_QDPP
        return ::inv(value);
#else
        return Value{1} / value;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_min(const Value& x, const Value& y)
    {
#if FLTX_METRICS_HAS_QDPP
        return (::min)(x, y);
#else
        return x < y ? x : y;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_max(const Value& x, const Value& y)
    {
#if FLTX_METRICS_HAS_QDPP
        return (::max)(x, y);
#else
        return x < y ? y : x;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_trunc(const Value& value)
    {
#if FLTX_METRICS_HAS_QDPP
        return ::aint(value);
#else
        return value;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_remainder(const Value& x, const Value& y)
    {
#if FLTX_METRICS_HAS_QDPP
        return ::drem(x, y);
#else
        return x - y;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_ipow(const Value& value)
    {
#if FLTX_METRICS_HAS_QDPP
        return ::npwr(value, 7);
#else
        return value;
#endif
    }

    template<class Value> [[nodiscard]] auto qd_remquo(const Value& x, const Value& y)
    {
#if FLTX_METRICS_HAS_QDPP
        Value remainder{};
        const Value quotient = ::divrem(x, y, remainder);
        const double raw = ::to_double(quotient);
        const int low = static_cast<int>(std::fmod(std::abs(raw), 8.0));
        return std::pair{remainder, Value{raw < 0.0 ? -low : low}};
#else
        return std::pair{x, y};
#endif
    }

    template<class Value>
    [[nodiscard]] Value qd_parse(const std::string& text)
    {
#if FLTX_METRICS_HAS_QDPP
        return Value::read(text.c_str());
#else
        return Value{};
#endif
    }

    template<class Value>
    [[nodiscard]] std::string qd_to_string(const Value& value, int digits)
    {
#if FLTX_METRICS_HAS_QDPP
        return value.to_string(
            digits, 0, std::ios_base::scientific, false, false);
#else
        (void)digits;
        return std::to_string(value);
#endif
    }

    template<class Value>
    [[nodiscard]] std::string qd_write_string(const Value& value, int digits)
    {
#if FLTX_METRICS_HAS_QDPP
        std::array<char, 512> buffer{};
        value.write(
            buffer.data(),
            static_cast<int>(buffer.size()),
            digits,
            false,
            false);
        return std::string{buffer.data()};
#else
        (void)digits;
        return std::to_string(value);
#endif
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

    template<class Value> [[nodiscard]] auto boost_lround_value(const Value& value)
    {
        using boost::multiprecision::lround;
        return Value{lround(value)};
    }

    template<class Value> [[nodiscard]] auto boost_llround_value(const Value& value)
    {
        using boost::multiprecision::llround;
        return Value{llround(value)};
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

    template<class Value> [[nodiscard]] auto boost_remquo(const Value& x, const Value& y)
    {
        int quotient = 0;
        using boost::multiprecision::remquo;
        const Value remainder = remquo(x, y, &quotient);
        const int low = quotient < 0 ? -((-quotient) & 7) : (quotient & 7);
        return std::pair{remainder, Value{low}};
    }

    template<class Value> [[nodiscard]] auto boost_modf(const Value& value)
    {
        Value integral{};
        using boost::multiprecision::modf;
        const Value fraction = modf(value, &integral);
        return std::pair{fraction, integral};
    }

    template<class Value> [[nodiscard]] auto boost_frexp(const Value& value)
    {
        int exponent = 0;
        using boost::multiprecision::frexp;
        const Value fraction = frexp(value, &exponent);
        return std::pair{fraction, Value{exponent}};
    }

    template<class Value> [[nodiscard]] auto boost_ldexp(const Value& value, int exponent)
    {
        using boost::multiprecision::ldexp;
        return ldexp(value, exponent);
    }

    template<class Value> [[nodiscard]] auto boost_scalbn(const Value& value, int exponent)
    {
        using boost::multiprecision::scalbn;
        return scalbn(value, exponent);
    }

    template<class Value> [[nodiscard]] auto boost_scalbln(const Value& value, long exponent)
    {
        using boost::multiprecision::scalbln;
        return scalbln(value, exponent);
    }

    template<class Value> [[nodiscard]] auto boost_logb(const Value& value)
    {
        using boost::multiprecision::logb;
        return logb(value);
    }

    template<class Value>
    [[nodiscard]] auto boost_fma(const Value& x, const Value& y, const Value& z)
    {
        using boost::multiprecision::fma;
        return fma(x, y, z);
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

    template<class Value> [[nodiscard]] auto boost_ilogb_value(const Value& value)
    {
        using boost::multiprecision::ilogb;
        return Value{ilogb(value)};
    }

    template<class Value> [[nodiscard]] auto boost_nextafter(const Value& x, const Value& y)
    {
        return boost::math::nextafter(x, y);
    }

    template<class Value> [[nodiscard]] auto boost_ipow(const Value& value)
    {
        using boost::multiprecision::pow;
        return pow(value, 7);
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

    template<class Value> [[nodiscard]] auto boost_hypot(const Value& x, const Value& y)
    {
        using boost::multiprecision::hypot;
        return hypot(x, y);
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

    template<class Value> [[nodiscard]] auto boost_exp2(const Value& value)
    {
        using boost::multiprecision::exp2;
        return exp2(value);
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

    template<class Value> [[nodiscard]] auto boost_log2(const Value& value)
    {
        using boost::multiprecision::log2;
        return log2(value);
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
} // namespace fltx::tests::accuracy::comparison_ops
