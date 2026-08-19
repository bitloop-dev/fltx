#pragma once

#include "tlfloat_implementation.hpp"

#if FLTX_METRICS_HAS_TLFLOAT
#include <tlfloat/tlmath.hpp>
#endif

#include <cmath>
#include <string>
#include <utility>

namespace fltx::tests::tlfloat_ops
{
    namespace adl_calls
    {
        template<class Value> [[nodiscard]] auto fabs_(const Value& x)
        {
            return fabs(x);
        }
        template<class Value> [[nodiscard]] auto floor_(const Value& x)
        {
            return floor(x);
        }
        template<class Value> [[nodiscard]] auto ceil_(const Value& x)
        {
            return ceil(x);
        }
        template<class Value> [[nodiscard]] auto trunc_(const Value& x)
        {
            return trunc(x);
        }
        template<class Value> [[nodiscard]] auto round_(const Value& x)
        {
            return round(x);
        }
        template<class Value> [[nodiscard]] auto rint_(const Value& x)
        {
            return rint(x);
        }
        template<class Value> [[nodiscard]] auto sqrt_(const Value& x)
        {
            return sqrt(x);
        }
        template<class Value> [[nodiscard]] auto fmin_(const Value& x, const Value& y)
        {
            return fmin(x, y);
        }
        template<class Value> [[nodiscard]] auto fmax_(const Value& x, const Value& y)
        {
            return fmax(x, y);
        }
        template<class Value> [[nodiscard]] auto fdim_(const Value& x, const Value& y)
        {
            return fdim(x, y);
        }
        template<class Value> [[nodiscard]] auto copysign_(const Value& x, const Value& y)
        {
            return copysign(x, y);
        }
        template<class Value> [[nodiscard]] auto nextafter_(const Value& x, const Value& y)
        {
            return nextafter(x, y);
        }
        template<class Value> [[nodiscard]] auto hypot_(const Value& x, const Value& y)
        {
            return hypot(x, y);
        }
        template<class Value>
        [[nodiscard]] auto fma_(const Value& x, const Value& y, const Value& z)
        {
            return fma(x, y, z);
        }
        template<class Value> [[nodiscard]] auto modf_(const Value& x, Value* integral)
        {
            return modf(x, integral);
        }
        template<class Value> [[nodiscard]] auto frexp_(const Value& x, int* exponent)
        {
            return frexp(x, exponent);
        }
        template<class Value> [[nodiscard]] auto ldexp_(const Value& x, int exponent)
        {
            return ldexp(x, exponent);
        }
        template<class Value> [[nodiscard]] auto ilogb_(const Value& x)
        {
            return ilogb(x);
        }
    } // namespace adl_calls

    template<class Value> [[nodiscard]] auto abs(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return adl_calls::fabs_(x);
#else
        return std::fabs(x);
#endif
    }

#if FLTX_METRICS_HAS_TLFLOAT
#define FLTX_METRICS_TLFLOAT_UNARY(name)                                                                  \
    template<class Value> [[nodiscard]] auto name(const Value& x)                                  \
    {                                                                                              \
        return tlfloat::name(x);                                                                   \
    }
#else
#define FLTX_METRICS_TLFLOAT_UNARY(name)                                                                  \
    template<class Value> [[nodiscard]] auto name(const Value& x)                                  \
    {                                                                                              \
        return std::name(x);                                                                       \
    }
#endif

    FLTX_METRICS_TLFLOAT_UNARY(sin)
    FLTX_METRICS_TLFLOAT_UNARY(cos)
    FLTX_METRICS_TLFLOAT_UNARY(tan)
    FLTX_METRICS_TLFLOAT_UNARY(atan)
    FLTX_METRICS_TLFLOAT_UNARY(asin)
    FLTX_METRICS_TLFLOAT_UNARY(acos)
    FLTX_METRICS_TLFLOAT_UNARY(exp)
    FLTX_METRICS_TLFLOAT_UNARY(exp2)
    FLTX_METRICS_TLFLOAT_UNARY(expm1)
    FLTX_METRICS_TLFLOAT_UNARY(log)
    FLTX_METRICS_TLFLOAT_UNARY(log2)
    FLTX_METRICS_TLFLOAT_UNARY(log10)
    FLTX_METRICS_TLFLOAT_UNARY(log1p)
    FLTX_METRICS_TLFLOAT_UNARY(sinh)
    FLTX_METRICS_TLFLOAT_UNARY(cosh)
    FLTX_METRICS_TLFLOAT_UNARY(tanh)
    FLTX_METRICS_TLFLOAT_UNARY(asinh)
    FLTX_METRICS_TLFLOAT_UNARY(acosh)
    FLTX_METRICS_TLFLOAT_UNARY(atanh)
    FLTX_METRICS_TLFLOAT_UNARY(erf)
    FLTX_METRICS_TLFLOAT_UNARY(erfc)
    FLTX_METRICS_TLFLOAT_UNARY(lgamma)
    FLTX_METRICS_TLFLOAT_UNARY(tgamma)

#undef FLTX_METRICS_TLFLOAT_UNARY

#define FLTX_METRICS_TLFLOAT_ADL_UNARY(name)                                                              \
    template<class Value> [[nodiscard]] auto name(const Value& x)                                  \
    {                                                                                              \
        return adl_calls::name##_(x);                                                              \
    }

    FLTX_METRICS_TLFLOAT_ADL_UNARY(floor)
    FLTX_METRICS_TLFLOAT_ADL_UNARY(ceil)
    FLTX_METRICS_TLFLOAT_ADL_UNARY(trunc)
    FLTX_METRICS_TLFLOAT_ADL_UNARY(round)
    FLTX_METRICS_TLFLOAT_ADL_UNARY(sqrt)

#undef FLTX_METRICS_TLFLOAT_ADL_UNARY

    template<class Value> [[nodiscard]] auto cbrt(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return tlfloat::cbrt(x);
#else
        return std::cbrt(x);
#endif
    }

    template<class Value> [[nodiscard]] auto roundeven(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return adl_calls::rint_(x);
#else
        return std::nearbyint(x);
#endif
    }

#if FLTX_METRICS_HAS_TLFLOAT
#define FLTX_METRICS_TLFLOAT_BINARY(name)                                                                 \
    template<class Value> [[nodiscard]] auto name(const Value& x, const Value& y)                  \
    {                                                                                              \
        return tlfloat::name(x, y);                                                                \
    }
#else
#define FLTX_METRICS_TLFLOAT_BINARY(name)                                                                 \
    template<class Value> [[nodiscard]] auto name(const Value& x, const Value& y)                  \
    {                                                                                              \
        return std::name(x, y);                                                                    \
    }
#endif

    FLTX_METRICS_TLFLOAT_BINARY(fmod)
    FLTX_METRICS_TLFLOAT_BINARY(remainder)
    FLTX_METRICS_TLFLOAT_BINARY(atan2)
    FLTX_METRICS_TLFLOAT_BINARY(pow)

#undef FLTX_METRICS_TLFLOAT_BINARY

#define FLTX_METRICS_TLFLOAT_ADL_BINARY(name)                                                             \
    template<class Value> [[nodiscard]] auto name(const Value& x, const Value& y)                  \
    {                                                                                              \
        return adl_calls::name##_(x, y);                                                           \
    }

    FLTX_METRICS_TLFLOAT_ADL_BINARY(fmin)
    FLTX_METRICS_TLFLOAT_ADL_BINARY(fmax)
    FLTX_METRICS_TLFLOAT_ADL_BINARY(fdim)
    FLTX_METRICS_TLFLOAT_ADL_BINARY(copysign)
    FLTX_METRICS_TLFLOAT_ADL_BINARY(nextafter)
    FLTX_METRICS_TLFLOAT_ADL_BINARY(hypot)

#undef FLTX_METRICS_TLFLOAT_ADL_BINARY

    template<class Value> [[nodiscard]] auto fma(const Value& x, const Value& y, const Value& z)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return adl_calls::fma_(x, y, z);
#else
        return std::fma(x, y, z);
#endif
    }

    template<class Value> [[nodiscard]] auto remquo(const Value& x, const Value& y)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        const auto result = tlfloat::remquo(x, y);
        // TLFloat returns the full quotient; C remquo exposes only signed low bits.
        return std::pair{result.first, result.second % 8};
#else
        int quotient = 0;
        return std::pair{std::remquo(x, y, &quotient), quotient};
#endif
    }

    template<class Value> [[nodiscard]] auto modf(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        Value integral{};
        return std::pair{adl_calls::modf_(x, &integral), integral};
#else
        Value integral{};
        return std::pair{std::modf(x, &integral), integral};
#endif
    }

    template<class Value> [[nodiscard]] auto frexp(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        int exponent = 0;
        return std::pair{adl_calls::frexp_(x, &exponent), exponent};
#else
        int exponent = 0;
        return std::pair{std::frexp(x, &exponent), exponent};
#endif
    }

    template<class Value> [[nodiscard]] auto ldexp(const Value& x, int exponent)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return adl_calls::ldexp_(x, exponent);
#else
        return std::ldexp(x, exponent);
#endif
    }

    template<class Value> [[nodiscard]] auto ilogb(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return adl_calls::ilogb_(x);
#else
        return std::ilogb(x);
#endif
    }

    template<class Value> [[nodiscard]] auto sincos_sum(const Value& x)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        const auto result = tlfloat::sincos(x);
        return result.first + result.second;
#else
        return std::sin(x) + std::cos(x);
#endif
    }

    template<class Value> [[nodiscard]] std::string to_string(const Value& x, int digits)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return tlfloat::to_string(x, digits);
#else
        (void)digits;
        return std::to_string(x);
#endif
    }

    template<class Value> [[nodiscard]] Value parse(const std::string& text)
    {
#if FLTX_METRICS_HAS_TLFLOAT
        return Value{text.c_str()};
#else
        return std::stod(text);
#endif
    }
} // namespace fltx::tests::tlfloat_ops
