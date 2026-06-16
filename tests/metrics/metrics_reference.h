#ifndef FLTX_TESTS_METRICS_REFERENCE_INCLUDED
#define FLTX_TESTS_METRICS_REFERENCE_INCLUDED

#include <algorithm>
#include <cmath>
#include <limits>
#include <string_view>
#include <type_traits>

#include <boost/multiprecision/cpp_double_fp.hpp>
#include <boost/multiprecision/mpfr.hpp>

#include <qd/dd_real.h>
#include <qd/qd_real.h>

#include <fltx/f128_math.h>
#include <fltx/f256_math.h>

#include "metrics_types.h"

namespace bl::test::metrics
{
    template<class Float>
    struct reference_types;

    template<>
    struct reference_types<bl::f128>
    {
        static constexpr int oracle_digits10 = 200;
        static constexpr int qdpp_competitor_digits10 = 31;
        static constexpr int extra_competitor_digits10 = qdpp_competitor_digits10;
        static constexpr double target_precision_bits = 106.0;

        using fltx_type = bl::f128;
        using perfect_ref = boost::multiprecision::number<
            boost::multiprecision::mpfr_float_backend<oracle_digits10>,
            boost::multiprecision::et_off>;

        using competitor_ref       = boost::multiprecision::cpp_double_double;
        using extra_competitor_ref = dd_real;

        static constexpr precision_type precision = precision_type::f128;
        static constexpr std::string_view precision_name = "f128";
        static constexpr std::string_view competitor_name = "boost::multiprecision::cpp_double_double";
        static constexpr std::string_view extra_competitor_name = "qdpp (dd_real)";
    };

    template<>
    struct reference_types<bl::f256>
    {
        static constexpr int oracle_digits10 = 1000;
        static constexpr int competitor_digits10 = 64;
        static constexpr int qdpp_competitor_digits10 = 62;
        static constexpr int extra_competitor_digits10 = qdpp_competitor_digits10;
        static constexpr double target_precision_bits = 212.0;

        using fltx_type = bl::f256;
        using perfect_ref = boost::multiprecision::number<
            boost::multiprecision::mpfr_float_backend<oracle_digits10>,
            boost::multiprecision::et_off>;

        using competitor_ref       = boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<competitor_digits10>, boost::multiprecision::et_off>;
        using extra_competitor_ref = qd_real;

        static constexpr precision_type precision = precision_type::f256;
        static constexpr std::string_view precision_name = "f256";
        static constexpr std::string_view competitor_name = "boost::multiprecision::mpfr_float_backend<64>";
        static constexpr std::string_view extra_competitor_name = "qdpp (qd_real)";
    };

    template<class T>
    [[nodiscard]] constexpr double expansion_precision_bits() noexcept
    {
        using value_type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<value_type, bl::f128> ||
                      std::is_same_v<value_type, bl::f128_s> ||
                      std::is_same_v<value_type, boost::multiprecision::cpp_double_double> ||
                      std::is_same_v<value_type, dd_real>)
        {
            return reference_types<bl::f128>::target_precision_bits;
        }
        else if constexpr (std::is_same_v<value_type, bl::f256> ||
                           std::is_same_v<value_type, bl::f256_s> ||
                           std::is_same_v<value_type, qd_real>)
        {
            return reference_types<bl::f256>::target_precision_bits;
        }
        else
        {
            return 0.0;
        }
    }

    template<class PerfectRef>
    [[nodiscard]] bool ref_signbit(const PerfectRef& value)
    {
        using boost::multiprecision::signbit;
        return signbit(value);
    }

    template<class PerfectRef>
    [[nodiscard]] PerfectRef signed_zero_ref(bool negative)
    {
        return PerfectRef{ negative ? "-0" : "0" };
    }

    template<class PerfectRef>
    [[nodiscard]] bool ref_is_nonfinite(const PerfectRef& value)
    {
        using std::isinf;
        using std::isnan;
        return isnan(value) || isinf(value);
    }

    template<class PerfectRef>
    [[nodiscard]] int floor_log2_abs_ref(const PerfectRef& value)
    {
        const PerfectRef magnitude = value < 0 ? -value : value;
        if (magnitude == 0)
            return 0;

        using std::floor;
        using std::log2;
        return floor(log2(magnitude)).template convert_to<int>();
    }

    template<class Value, class PerfectRef>
    [[nodiscard]] double effective_domain_ideal_bits(const PerfectRef& expected)
    {
        constexpr double precision_bits = expansion_precision_bits<Value>();
        if constexpr (precision_bits <= 0.0)
        {
            return 0.0;
        }
        else
        {
            if (ref_is_nonfinite(expected))
                return precision_bits;

            if (expected == 0)
                return precision_bits;

            constexpr int min_double_bit_exponent =
                std::numeric_limits<double>::min_exponent - std::numeric_limits<double>::digits;
            const double available_bits =
                static_cast<double>(floor_log2_abs_ref(expected) - min_double_bit_exponent + 1);

            return std::clamp(available_bits, 1.0, precision_bits);
        }
    }

    template<class Float>
    struct target_reference_traits;

    template<>
    struct target_reference_traits<bl::f128>
    {
        using references = reference_types<bl::f128>;
        using perfect_ref = references::perfect_ref;
        using value_type = bl::f128_s;

        [[nodiscard]] static perfect_ref to_ref(const value_type& value)
        {
            if (bl::iszero(value))
                return signed_zero_ref<perfect_ref>(bl::signbit(value));
            return perfect_ref{ value.hi } + perfect_ref{ value.lo };
        }

        [[nodiscard]] static value_type quantize_value(const perfect_ref& exact)
        {
            const double hi = static_cast<double>(exact);
            if (std::isnan(hi) || std::isinf(hi))
                return value_type{ hi, 0.0 };
            if (hi == 0.0)
                return value_type{ ref_signbit(exact) ? -0.0 : 0.0, 0.0 };

            perfect_ref residual = exact - perfect_ref{ hi };
            const double lo = static_cast<double>(residual);
            residual -= perfect_ref{ lo };
            const double guard = static_cast<double>(residual);
            residual -= perfect_ref{ guard };
            const double sticky = static_cast<double>(residual);

            return bl::detail::_f128::renorm(hi, lo + (guard + sticky));
        }

        [[nodiscard]] static perfect_ref quantize(const perfect_ref& exact)
        {
            return to_ref(quantize_value(exact));
        }

        [[nodiscard]] static value_type nextafter_value(const value_type& from, const value_type& to)
        {
            if (std::isnan(from.hi) || std::isnan(to.hi))
                return std::numeric_limits<value_type>::quiet_NaN();
            if (from == to)
                return to;
            if (bl::iszero(from))
            {
                return bl::signbit(to)
                    ? value_type{ -std::numeric_limits<double>::denorm_min(), 0.0 }
                    : value_type{  std::numeric_limits<double>::denorm_min(), 0.0 };
            }
            if (bl::isinf(from))
            {
                return bl::signbit(from)
                    ? -std::numeric_limits<value_type>::max()
                    :  std::numeric_limits<value_type>::max();
            }

            const double toward = (from < to)
                ? std::numeric_limits<double>::infinity()
                : -std::numeric_limits<double>::infinity();
            return bl::detail::_f128::renorm(from.hi, detail::fp::nextafter(from.lo, toward));
        }
    };

    template<>
    struct target_reference_traits<bl::f256>
    {
        using references = reference_types<bl::f256>;
        using perfect_ref = references::perfect_ref;
        using value_type = bl::f256_s;

        [[nodiscard]] static perfect_ref to_ref(const value_type& value)
        {
            if (bl::iszero(value))
                return signed_zero_ref<perfect_ref>(bl::signbit(value));
            return perfect_ref{ value.x0 } + perfect_ref{ value.x1 } +
                   perfect_ref{ value.x2 } + perfect_ref{ value.x3 };
        }

        [[nodiscard]] static value_type quantize_value(const perfect_ref& exact)
        {
            const double x0 = static_cast<double>(exact);
            if (std::isnan(x0) || std::isinf(x0))
                return value_type{ x0, 0.0, 0.0, 0.0 };
            if (x0 == 0.0)
                return value_type{ ref_signbit(exact) ? -0.0 : 0.0, 0.0, 0.0, 0.0 };

            perfect_ref residual = exact - perfect_ref{ x0 };
            const double x1 = static_cast<double>(residual);
            residual -= perfect_ref{ x1 };
            const double x2 = static_cast<double>(residual);
            residual -= perfect_ref{ x2 };
            const double x3 = static_cast<double>(residual);
            residual -= perfect_ref{ x3 };
            const double guard = static_cast<double>(residual);
            residual -= perfect_ref{ guard };
            const double sticky = static_cast<double>(residual);

            return bl::detail::_f256::renorm5(x0, x1, x2, x3, guard + sticky);
        }

        [[nodiscard]] static perfect_ref quantize(const perfect_ref& exact)
        {
            return to_ref(quantize_value(exact));
        }

        [[nodiscard]] static value_type normalize_adjacent_tail(const value_type& from, double stepped_x3)
        {
            if (from.x1 == 0.0)
                return value_type{ from.x0, stepped_x3, 0.0, 0.0 };

            if (from.x2 == 0.0 && (from.x1 + stepped_x3) == from.x1)
                return value_type{ from.x0, from.x1, stepped_x3, 0.0 };

            if (from.x1 != 0.0 && from.x2 != 0.0 && (from.x2 + stepped_x3) == from.x2)
                return value_type{ from.x0, from.x1, from.x2, stepped_x3 };

            return bl::detail::_f256::renorm4(from.x0, from.x1, from.x2, stepped_x3);
        }

        [[nodiscard]] static value_type nextafter_value(const value_type& from, const value_type& to)
        {
            if (std::isnan(from.x0) || std::isnan(to.x0))
                return std::numeric_limits<value_type>::quiet_NaN();
            if (from == to)
                return to;
            if (bl::iszero(from))
            {
                return bl::signbit(to)
                    ? value_type{ -std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 }
                    : value_type{  std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 };
            }
            if (bl::isinf(from))
            {
                return bl::signbit(from)
                    ? -std::numeric_limits<value_type>::max()
                    :  std::numeric_limits<value_type>::max();
            }

            const double toward = (from < to)
                ? std::numeric_limits<double>::infinity()
                : -std::numeric_limits<double>::infinity();
            return normalize_adjacent_tail(from, detail::fp::nextafter(from.x3, toward));
        }
    };

    template<class Float>
    [[nodiscard]] typename reference_types<Float>::perfect_ref target_reference_value(
        const typename reference_types<Float>::perfect_ref& exact)
    {
        return target_reference_traits<Float>::quantize(exact);
    }

    template<class Float>
    [[nodiscard]] typename reference_types<Float>::perfect_ref target_nextafter_reference_value(
        const typename reference_types<Float>::perfect_ref& exact_from,
        const typename reference_types<Float>::perfect_ref& exact_to)
    {
        using traits = target_reference_traits<Float>;
        const auto from = traits::quantize_value(exact_from);
        const auto to = traits::quantize_value(exact_to);
        return traits::to_ref(traits::nextafter_value(from, to));
    }

    template<class Float>
    [[nodiscard]] double target_domain_ideal_bits(
        const typename reference_types<Float>::perfect_ref& expected)
    {
        const double effective_bits = effective_domain_ideal_bits<Float>(expected);
        return effective_bits > 0.0 ? effective_bits : reference_types<Float>::target_precision_bits;
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE T round_nearest_even_integer_reference(const T& x)
    {
        using boost::multiprecision::floor;
        using std::floor;

        const T lower = floor(x);
        const T fraction = x - lower;
        if (fraction < T{ 0.5 })
            return lower;

        const T upper = lower + T{ 1 };
        if (fraction > T{ 0.5 })
            return upper;

        const T half_lower = floor(lower / T{ 2 });
        return lower == half_lower * T{ 2 } ? lower : upper;
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE T round_away_from_zero_integer_reference(const T& x)
    {
        using boost::multiprecision::ceil;
        using boost::multiprecision::floor;
        using std::ceil;
        using std::floor;

        return x < T{ 0 } ? ceil(x - T{ 0.5 }) : floor(x + T{ 0.5 });
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE long call_lround_reference(const T& x)
    {
        return static_cast<long>(round_away_from_zero_integer_reference(x));
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE long long call_llround_reference(const T& x)
    {
        return static_cast<long long>(round_away_from_zero_integer_reference(x));
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE long call_lrint_reference(const T& x)
    {
        return static_cast<long>(round_nearest_even_integer_reference(x));
    }

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE long long call_llrint_reference(const T& x)
    {
        return static_cast<long long>(round_nearest_even_integer_reference(x));
    }

    template<class T, class FallbackFn>
    [[nodiscard]] BL_FORCE_INLINE T call_unary_reference(
        std::string_view operation,
        const T& x,
        FallbackFn fallback)
    {
        if (operation == "round")
            return round_away_from_zero_integer_reference(x);
        if (operation == "nearbyint" || operation == "rint")
            return round_nearest_even_integer_reference(x);
        return fallback(x);
    }
}

#endif
