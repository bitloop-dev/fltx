#pragma once

#include "mpfr.hpp"
#include "samples.hpp"

#ifndef FLTX_METRICS_HAS_QDPP
#define FLTX_METRICS_HAS_QDPP 0
#endif

#if FLTX_METRICS_HAS_QDPP
#include <qd/dd.h>
#include <qd/qd_real.h>
#endif

#include <boost/multiprecision/cpp_double_fp.hpp>
#include <boost/multiprecision/mpfr.hpp>

#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include <fltx/f128.h>
#include <fltx/f256.h>

namespace fltx::tests::implementations
{
    using cppdd = boost::multiprecision::cpp_double_double;
    using mpfr64 = boost::multiprecision::number<
        boost::multiprecision::mpfr_float_backend<64>,
        boost::multiprecision::et_off>;

#if FLTX_METRICS_HAS_QDPP
    using qd_f128 = dd_real;
    using qd_f256 = qd_real;
#else
    using qd_f128 = double;
    using qd_f256 = double;
#endif

    struct identity
    {
        std::string_view id;
        std::string_view short_label;
        std::string_view label;
        std::string_view api;
        bool enabled = true;
    };

    inline constexpr identity fltx_f128{
        "fltx", "fltx", "fltx bl::f128", "fltx"
    };
    inline constexpr identity fltx_f256{
        "fltx", "fltx", "fltx bl::f256", "fltx"
    };
    inline constexpr identity native_f32{
        "native", "float", "native float", "bl:: native overloads"
    };
    inline constexpr identity native_f64{
        "native", "double", "native double", "bl:: native overloads"
    };
    inline constexpr identity qdpp_f128{
        "qdpp", "ddreal", "qdpp dd_real", "qdpp", FLTX_METRICS_HAS_QDPP != 0
    };
    inline constexpr identity qdpp_f256{
        "qdpp", "qdreal", "qdpp qd_real", "qdpp", FLTX_METRICS_HAS_QDPP != 0
    };
    inline constexpr identity boost_f128{
        "cppdd",
        "cppdd",
        "boost::multiprecision::cpp_double_double",
        "Boost.Multiprecision"
    };
    inline constexpr identity boost_f256{
        "mpfr64",
        "mpfr64",
        "boost::multiprecision::mpfr_float_backend<64>",
        "Boost.Multiprecision"
    };
    template<class Float>
    inline constexpr std::string_view precision_name =
        std::is_same_v<Float, float> ? "f32" :
        std::is_same_v<Float, double> ? "f64" :
        std::is_same_v<Float, bl::f128> ? "f128" : "f256";

    template<class Float>
    inline constexpr identity primary_identity =
        std::is_same_v<Float, float> ? native_f32 :
        std::is_same_v<Float, double> ? native_f64 :
        std::is_same_v<Float, bl::f128> ? fltx_f128 : fltx_f256;

    template<class Float>
    inline constexpr identity qdpp_identity =
        std::is_same_v<Float, bl::f128> ? qdpp_f128 : qdpp_f256;

    template<class Float>
    inline constexpr identity boost_identity =
        std::is_same_v<Float, bl::f128> ? boost_f128 : boost_f256;

    using timing_components = std::array<double, 4>;

    [[nodiscard]] inline timing_components observe(float value) noexcept
    {
        return { value, 0.0, 0.0, 0.0 };
    }

    [[nodiscard]] inline timing_components observe(double value) noexcept
    {
        return { value, 0.0, 0.0, 0.0 };
    }

    [[nodiscard]] inline timing_components observe(
        const bl::f128_s& value) noexcept
    {
        return { value.hi, value.lo, 0.0, 0.0 };
    }

    [[nodiscard]] inline timing_components observe(
        const bl::f128& value) noexcept
    {
        return observe(static_cast<const bl::f128_s&>(value));
    }

    [[nodiscard]] inline timing_components observe(
        const bl::f256_s& value) noexcept
    {
        return { value.x0, value.x1, value.x2, value.x3 };
    }

    [[nodiscard]] inline timing_components observe(
        const bl::f256& value) noexcept
    {
        return observe(static_cast<const bl::f256_s&>(value));
    }

    [[nodiscard]] inline timing_components observe(const cppdd& value) noexcept
    {
        const auto parts = value.backend().crep();
        return { parts.first, parts.second, 0.0, 0.0 };
    }

    [[nodiscard]] inline timing_components observe(const mpfr64& value)
    {
        return { static_cast<double>(value), 0.0, 0.0, 0.0 };
    }

#if FLTX_METRICS_HAS_QDPP
    [[nodiscard]] inline timing_components observe(const dd_real& value) noexcept
    {
        return { value.x[0], value.x[1], 0.0, 0.0 };
    }

    [[nodiscard]] inline timing_components observe(const qd_real& value) noexcept
    {
        return { value.x[0], value.x[1], value.x[2], value.x[3] };
    }
#endif

    template<class Value>
    [[nodiscard]] timing_components observe(const Value& value) noexcept
    {
        return { static_cast<double>(value), 0.0, 0.0, 0.0 };
    }

    template<class Value>
    struct value_traits;

    template<>
    struct value_traits<float>
    {
        static constexpr double nominal_bits =
            std::numeric_limits<float>::digits;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<float>();
            return value;
        }

        [[nodiscard]] static float from_sample(const sample& value)
        {
            return static_cast<float>(value.limb[0]);
        }

        [[nodiscard]] static mpfr::real to_real(float value)
        {
            return mpfr::native_float_to_real(value);
        }

        [[nodiscard]] static bool is_finite(float value)
        {
            return native_fp::is_finite(value);
        }

        [[nodiscard]] static bool sign_bit(float value)
        {
            return native_fp::sign_bit(value);
        }
    };

    template<>
    struct value_traits<double>
    {
        static constexpr double nominal_bits =
            std::numeric_limits<double>::digits;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<double>();
            return value;
        }

        [[nodiscard]] static double from_sample(const sample& value)
        {
            return value.limb[0];
        }

        [[nodiscard]] static mpfr::real to_real(double value)
        {
            return mpfr::native_float_to_real(value);
        }

        [[nodiscard]] static bool is_finite(double value)
        {
            return native_fp::is_finite(value);
        }

        [[nodiscard]] static bool sign_bit(double value)
        {
            return native_fp::sign_bit(value);
        }
    };

    template<>
    struct value_traits<bl::f128>
    {
        static constexpr double nominal_bits =
            std::numeric_limits<bl::f128>::digits;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<double>();
            return value;
        }

        [[nodiscard]] static bl::f128 from_sample(const sample& value)
        {
            return { value.limb[0], value.limb[1] };
        }

        [[nodiscard]] static mpfr::real to_real(const bl::f128_s& value)
        {
            return mpfr::traits<bl::f128>::to_real(value);
        }

        [[nodiscard]] static bool is_finite(const bl::f128_s& value)
        {
            return mpfr::traits<bl::f128>::is_finite(value);
        }

        [[nodiscard]] static bool sign_bit(const bl::f128_s& value)
        {
            return mpfr::traits<bl::f128>::sign_bit(value);
        }
    };

    template<>
    struct value_traits<bl::f256>
    {
        static constexpr double nominal_bits =
            std::numeric_limits<bl::f256>::digits;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<double>();
            return value;
        }

        [[nodiscard]] static bl::f256 from_sample(const sample& value)
        {
            return { value.limb[0], value.limb[1], value.limb[2], value.limb[3] };
        }

        [[nodiscard]] static mpfr::real to_real(const bl::f256_s& value)
        {
            return mpfr::traits<bl::f256>::to_real(value);
        }

        [[nodiscard]] static bool is_finite(const bl::f256_s& value)
        {
            return mpfr::traits<bl::f256>::is_finite(value);
        }

        [[nodiscard]] static bool sign_bit(const bl::f256_s& value)
        {
            return mpfr::traits<bl::f256>::sign_bit(value);
        }
    };

    template<>
    struct value_traits<cppdd>
    {
        static constexpr double nominal_bits = 106.0;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<double>();
            return value;
        }

        [[nodiscard]] static cppdd from_sample(const sample& value)
        {
            if (!native_fp::is_finite(value.limb[0]))
                return cppdd{ value.limb[0] };
            if (value.limb[0] == 0.0 && value.limb[1] == 0.0)
                return cppdd{ native_fp::sign_bit(value.limb[0]) ? "-0" : "0" };
            return cppdd{ value.limb[0] } + cppdd{ value.limb[1] };
        }

        [[nodiscard]] static mpfr::real to_real(const cppdd& value)
        {
            const auto parts = value.backend().crep();
            const mpfr::real out =
                mpfr::real{ parts.first } + mpfr::real{ parts.second };
            using boost::multiprecision::signbit;
            return out == 0 && signbit(value) ? mpfr::signed_zero(true) : out;
        }

        [[nodiscard]] static bool is_finite(const cppdd& value)
        {
            using boost::multiprecision::isfinite;
            return isfinite(value);
        }

        [[nodiscard]] static bool sign_bit(const cppdd& value)
        {
            using boost::multiprecision::signbit;
            return signbit(value);
        }
    };

    template<>
    struct value_traits<mpfr64>
    {
        static constexpr double nominal_bits =
            std::numeric_limits<mpfr64>::digits;

        // MPFR's exponent range is not tied to a fixed storage format. A
        // zero resolution disables fixed-format underflow adjustment while
        // retaining the ordinary relative-error metric.
        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value{0};
            return value;
        }

        [[nodiscard]] static mpfr64 from_sample(const sample& value)
        {
            if (!native_fp::is_finite(value.limb[0]))
                return mpfr64{ value.limb[0] };
            if (value.limb[0] == 0.0 && value.limb[1] == 0.0 &&
                value.limb[2] == 0.0 && value.limb[3] == 0.0)
            {
                return mpfr64{ native_fp::sign_bit(value.limb[0]) ? "-0" : "0" };
            }
            return mpfr64{ value.limb[0] } + mpfr64{ value.limb[1] } +
                   mpfr64{ value.limb[2] } + mpfr64{ value.limb[3] };
        }

        [[nodiscard]] static mpfr::real to_real(const mpfr64& value)
        {
            return mpfr::real{ value };
        }

        [[nodiscard]] static bool is_finite(const mpfr64& value)
        {
            using boost::multiprecision::isfinite;
            return isfinite(value);
        }

        [[nodiscard]] static bool sign_bit(const mpfr64& value)
        {
            using boost::multiprecision::signbit;
            return signbit(value);
        }
    };

#if FLTX_METRICS_HAS_QDPP
    template<>
    struct value_traits<dd_real>
    {
        static constexpr double nominal_bits = 106.0;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<double>();
            return value;
        }

        [[nodiscard]] static dd_real from_sample(const sample& value)
        {
            if (!native_fp::is_finite(value.limb[0]))
                return dd_real{ value.limb[0] };
            return dd_real{ value.limb[0] } + dd_real{ value.limb[1] };
        }

        [[nodiscard]] static mpfr::real to_real(const dd_real& value)
        {
            const mpfr::real out =
                mpfr::real{ value.x[0] } + mpfr::real{ value.x[1] };
            return out == 0 && native_fp::sign_bit(value.x[0])
                ? mpfr::signed_zero(true)
                : out;
        }

        [[nodiscard]] static bool is_finite(const dd_real& value)
        {
            return native_fp::is_finite(value.x[0]) &&
                   native_fp::is_finite(value.x[1]);
        }

        [[nodiscard]] static bool sign_bit(const dd_real& value)
        {
            return native_fp::sign_bit(value.x[0]);
        }
    };

    template<>
    struct value_traits<qd_real>
    {
        static constexpr double nominal_bits = 212.0;

        [[nodiscard]] static const mpfr::real& absolute_resolution()
        {
            static const mpfr::real value =
                mpfr::absolute_resolution<double>();
            return value;
        }

        [[nodiscard]] static qd_real from_sample(const sample& value)
        {
            if (!native_fp::is_finite(value.limb[0]))
                return qd_real{ value.limb[0] };
            return qd_real{ value.limb[0] } + qd_real{ value.limb[1] } +
                   qd_real{ value.limb[2] } + qd_real{ value.limb[3] };
        }

        [[nodiscard]] static mpfr::real to_real(const qd_real& value)
        {
            const mpfr::real out =
                mpfr::real{ value.x[0] } + mpfr::real{ value.x[1] } +
                mpfr::real{ value.x[2] } + mpfr::real{ value.x[3] };
            return out == 0 && native_fp::sign_bit(value.x[0])
                ? mpfr::signed_zero(true)
                : out;
        }

        [[nodiscard]] static bool is_finite(const qd_real& value)
        {
            return native_fp::is_finite(value.x[0]) &&
                   native_fp::is_finite(value.x[1]) &&
                   native_fp::is_finite(value.x[2]) &&
                   native_fp::is_finite(value.x[3]);
        }

        [[nodiscard]] static bool sign_bit(const qd_real& value)
        {
            return native_fp::sign_bit(value.x[0]);
        }
    };
#endif

}
