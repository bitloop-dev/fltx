#ifndef FLTX_TESTS_SUPPORT_MPFR_INCLUDED
#define FLTX_TESTS_SUPPORT_MPFR_INCLUDED

#include "samples.hpp"
#include "thresholds.hpp"

#include <boost/multiprecision/mpfr.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <fltx/f128.h>
#include <fltx/f256.h>

namespace fltx::tests::mpfr
{
    using real = boost::multiprecision::number<
        boost::multiprecision::mpfr_float_backend<400>,
        boost::multiprecision::et_off>;

    template<class Float>
    struct traits;

    template<>
    struct traits<float>
    {
        static constexpr thresholds::precision precision = thresholds::precision::f32;
        static constexpr std::string_view name = "f32";

        [[nodiscard]] static float from_sample(const sample& value)
        {
            return static_cast<float>(value.limb[0]);
        }

        [[nodiscard]] static real to_real(float value)
        {
            return real{ value };
        }

        [[nodiscard]] static bool is_finite(float value)
        {
            return std::isfinite(value);
        }

        [[nodiscard]] static bool sign_bit(float value)
        {
            return std::signbit(value);
        }
    };

    template<>
    struct traits<double>
    {
        static constexpr thresholds::precision precision = thresholds::precision::f64;
        static constexpr std::string_view name = "f64";

        [[nodiscard]] static double from_sample(const sample& value)
        {
            return value.limb[0];
        }

        [[nodiscard]] static real to_real(double value)
        {
            return real{ value };
        }

        [[nodiscard]] static bool is_finite(double value)
        {
            return std::isfinite(value);
        }

        [[nodiscard]] static bool sign_bit(double value)
        {
            return std::signbit(value);
        }
    };

    template<>
    struct traits<bl::f128>
    {
        static constexpr thresholds::precision precision = thresholds::precision::f128;
        static constexpr std::string_view name = "f128";

        [[nodiscard]] static bl::f128 from_sample(const sample& value)
        {
            return { value.limb[0], value.limb[1] };
        }

        [[nodiscard]] static real to_real(const bl::f128_s& value)
        {
            const real out = real{ value.hi } + real{ value.lo };
            return out == 0 && std::signbit(value.hi)
                ? real{ -0.0 }
                : out;
        }

        [[nodiscard]] static bool is_finite(const bl::f128_s& value)
        {
            return std::isfinite(value.hi) && std::isfinite(value.lo);
        }

        [[nodiscard]] static bool sign_bit(const bl::f128_s& value)
        {
            return std::signbit(value.hi);
        }
    };

    template<>
    struct traits<bl::f256>
    {
        static constexpr thresholds::precision precision = thresholds::precision::f256;
        static constexpr std::string_view name = "f256";

        [[nodiscard]] static bl::f256 from_sample(const sample& value)
        {
            return { value.limb[0], value.limb[1], value.limb[2], value.limb[3] };
        }

        [[nodiscard]] static real to_real(const bl::f256_s& value)
        {
            const real out = real{ value.x0 } + real{ value.x1 } +
                             real{ value.x2 } + real{ value.x3 };
            return out == 0 && std::signbit(value.x0)
                ? real{ -0.0 }
                : out;
        }

        [[nodiscard]] static bool is_finite(const bl::f256_s& value)
        {
            return std::isfinite(value.x0) && std::isfinite(value.x1) &&
                   std::isfinite(value.x2) && std::isfinite(value.x3);
        }

        [[nodiscard]] static bool sign_bit(const bl::f256_s& value)
        {
            return std::signbit(value.x0);
        }
    };

    [[nodiscard]] inline real to_real(const sample& value)
    {
        return real{ value.limb[0] } + real{ value.limb[1] } +
               real{ value.limb[2] } + real{ value.limb[3] };
    }

    template<class Float>
    [[nodiscard]] real input_real(const sample& value)
    {
        real out;
        if constexpr (std::is_same_v<Float, float>)
            out = real{ static_cast<float>(value.limb[0]) };
        else if constexpr (std::is_same_v<Float, double>)
            out = real{ value.limb[0] };
        else if constexpr (std::is_same_v<Float, bl::f128>)
            out = real{ value.limb[0] } + real{ value.limb[1] };
        else
            out = to_real(value);

        if (out == 0 && std::signbit(value.limb[0]))
            return real{ -0.0 };
        return out;
    }

    template<class Float>
    [[nodiscard]] sample sample_from_real(real value, std::string label = {})
    {
        sample out{};
        out.label = std::move(label);
        constexpr std::size_t limb_count = std::is_same_v<Float, bl::f128> ? 2 : 4;
        for (std::size_t i = 0; i < limb_count; ++i)
        {
            out.limb[i] = static_cast<double>(value);
            value -= real{ out.limb[i] };
        }
        return out;
    }

    [[nodiscard]] inline real round_even(const real& value)
    {
        if (value == 0)
            return value;

        using boost::multiprecision::floor;
        const real lower = floor(value);
        const real fraction = value - lower;
        real rounded;
        if (fraction < real{ 0.5 })
            rounded = lower;
        else if (fraction > real{ 0.5 })
            rounded = lower + 1;
        else
            rounded = floor(lower / 2) * 2 == lower ? lower : lower + 1;
        return rounded == 0 && value < 0 ? real{ -0.0 } : rounded;
    }

    [[nodiscard]] inline real round_away_from_zero(const real& value)
    {
        if (value == 0)
            return value;

        using boost::multiprecision::ceil;
        using boost::multiprecision::floor;
        const real rounded = value < 0
            ? ceil(value - real{ 0.5 })
            : floor(value + real{ 0.5 });
        return rounded == 0 && value < 0 ? real{ -0.0 } : rounded;
    }

    [[nodiscard]] inline real trunc(const real& value)
    {
        if (value == 0)
            return value;

        using boost::multiprecision::ceil;
        using boost::multiprecision::floor;
        const real rounded = value < 0 ? ceil(value) : floor(value);
        return rounded == 0 && value < 0 ? real{ -0.0 } : rounded;
    }

    [[nodiscard]] inline real round_decimals(const real& value, int digits)
    {
        if (digits <= 0 || value == 0)
            return value;

        using boost::multiprecision::pow;
        const real scale = pow(real{ 10 }, digits);
        return round_even(value * scale) / scale;
    }

    [[nodiscard]] inline real round_significant(
        const real& value,
        int figures)
    {
        if (figures <= 0 || value == 0)
            return value;

        using boost::multiprecision::abs;
        using boost::multiprecision::floor;
        using boost::multiprecision::log10;
        using boost::multiprecision::pow;
        const int exponent =
            static_cast<int>(floor(log10(abs(value))));
        const real scale = pow(real{ 10 }, figures - 1 - exponent);
        return round_even(value * scale) / scale;
    }

    [[nodiscard]] inline real remainder(const real& x, const real& y)
    {
        const real result = x - round_even(x / y) * y;
        if (result != 0)
            return result;

        using boost::multiprecision::signbit;
        return signbit(x) ? real{ -0.0 } : real{ 0.0 };
    }

    [[nodiscard]] inline int remquo_bits(const real& x, const real& y)
    {
        const real quotient = round_even(x / y);
        using boost::multiprecision::abs;
        using boost::multiprecision::fmod;
        const int low_bits = static_cast<int>(fmod(abs(quotient), real{ 8 }));
        return quotient < 0 ? -low_bits : low_bits;
    }

    [[nodiscard]] inline int ilogb(const real& value)
    {
        using boost::multiprecision::abs;
        using boost::multiprecision::frexp;
        int exponent = 0;
        (void)frexp(abs(value), &exponent);
        return exponent - 1;
    }

    [[nodiscard]] inline bool is_nan(const real& value)
    {
        using boost::multiprecision::isnan;
        return isnan(value);
    }

    [[nodiscard]] inline bool is_inf(const real& value)
    {
        using boost::multiprecision::isinf;
        return isinf(value);
    }

    [[nodiscard]] inline bool sign_bit(const real& value)
    {
        using boost::multiprecision::signbit;
        return signbit(value);
    }

    template<class Float>
    [[nodiscard]] real absolute_resolution()
    {
        if constexpr (std::is_same_v<Float, float>)
            return real{ std::numeric_limits<float>::denorm_min() };
        else
            return real{ std::numeric_limits<double>::denorm_min() };
    }

    [[nodiscard]] inline double resolution_adjusted_bits(
        const real& observed,
        const real& reference,
        const real& absolute_resolution,
        double nominal_bits)
    {
        if (is_nan(observed) || is_nan(reference))
            return is_nan(observed) && is_nan(reference)
                ? std::numeric_limits<double>::infinity()
                : 0.0;

        if (is_inf(observed) || is_inf(reference))
            return is_inf(observed) && is_inf(reference) && sign_bit(observed) == sign_bit(reference)
                ? std::numeric_limits<double>::infinity()
                : 0.0;

        if (reference == 0)
        {
            return observed == 0 && sign_bit(observed) == sign_bit(reference)
                ? std::numeric_limits<double>::infinity()
                : 0.0;
        }

        real reference_magnitude = reference;
        if (reference_magnitude < 0)
            reference_magnitude = -reference_magnitude;

        // Below half of the component type's smallest value, zero is the
        // correctly rounded result. At and above that boundary, returning
        // zero for a representable nonzero value is a categorical failure,
        // not a high nominal-precision score.
        if (absolute_resolution > 0 &&
            reference_magnitude <= absolute_resolution / 2)
        {
            return observed == 0 && sign_bit(observed) == sign_bit(reference)
                ? std::numeric_limits<double>::infinity()
                : 0.0;
        }
        if (observed == 0 || sign_bit(observed) != sign_bit(reference))
            return 0.0;

        real error = observed - reference;
        if (error < 0)
            error = -error;
        if (error == 0)
            return std::numeric_limits<double>::infinity();

        real scale = reference_magnitude;
        // Expansions retain their nominal precision only while another
        // representable component exists below the result. Once the absolute
        // double/float spacing dominates, score error in ULPs and express it
        // on the same nominal-bit scale as normal values.
        using boost::multiprecision::ldexp;
        if (absolute_resolution > 0)
        {
            const real resolution_limited_scale = ldexp(
                absolute_resolution,
                static_cast<int>(nominal_bits) - 1);
            if (scale < resolution_limited_scale)
                scale = resolution_limited_scale;
        }

        using boost::multiprecision::log2;
        return static_cast<double>(-log2(error / scale));
    }

    [[nodiscard]] inline std::string text(const real& value)
    {
        std::ostringstream out;
        out << std::setprecision(80) << value;
        return out.str();
    }

    [[nodiscard]] inline real abs(real value)
    {
        return value < 0 ? -value : value;
    }

    [[nodiscard]] inline real cbrt(const real& value)
    {
        using boost::multiprecision::pow;
        const real magnitude = pow(abs(value), real{ 1 } / real{ 3 });
        return value < 0 ? -magnitude : magnitude;
    }

    [[nodiscard]] inline real exp2(const real& value)
    {
        using boost::multiprecision::exp;
        using boost::multiprecision::log;
        static const real ln2 = log(real{ 2 });
        return exp(value * ln2);
    }

    [[nodiscard]] inline real log2(const real& value)
    {
        using boost::multiprecision::log;
        static const real ln2 = log(real{ 2 });
        return log(value) / ln2;
    }

    [[nodiscard]] inline real log10(const real& value)
    {
        using boost::multiprecision::log;
        static const real ln10 = log(real{ 10 });
        return log(value) / ln10;
    }

    [[nodiscard]] inline real hypot(const real& x, const real& y)
    {
        using boost::multiprecision::sqrt;
        return sqrt(x * x + y * y);
    }

    [[nodiscard]] inline real fma(const real& x, const real& y, const real& z)
    {
        return x * y + z;
    }

    [[nodiscard]] inline real fmod(const real& x, const real& y)
    {
        using boost::multiprecision::trunc;
        const real result = x - trunc(x / y) * y;
        if (result != 0)
            return result;

        using boost::multiprecision::signbit;
        return signbit(x) ? real{ -0.0 } : real{ 0.0 };
    }
}

#endif
