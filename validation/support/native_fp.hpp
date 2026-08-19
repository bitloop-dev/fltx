#ifndef FLTX_TESTS_SUPPORT_NATIVE_FP_INCLUDED
#define FLTX_TESTS_SUPPORT_NATIVE_FP_INCLUDED

#include <bit>
#include <cstdint>
#include <type_traits>

#if defined(__clang__) && defined(FLTX_FAST_MATH)
  #define FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION \
      inline __attribute__((noinline, optnone))
  #define FLTX_VALIDATION_PRECISE_FUNCTION \
      inline __attribute__((noinline, optnone))
#elif defined(__GNUC__) && defined(FLTX_FAST_MATH)
  #define FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION \
      inline __attribute__((noinline, optimize("no-fast-math")))
  #define FLTX_VALIDATION_PRECISE_FUNCTION \
      inline __attribute__((noinline, optimize("no-fast-math")))
#elif defined(_MSC_VER)
  #define FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION __forceinline
  #define FLTX_VALIDATION_PRECISE_FUNCTION inline __declspec(noinline)
#else
  #define FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION inline
  #define FLTX_VALIDATION_PRECISE_FUNCTION inline
#endif

namespace fltx::tests::native_fp
{
    template<class Float>
    struct binary_format;

    template<>
    struct binary_format<float>
    {
        using bits_type = std::uint32_t;
        static constexpr int fraction_bits = 23;
        static constexpr int exponent_bits = 8;
        static constexpr int exponent_bias = 127;
        static constexpr bits_type sign_mask = UINT32_C(0x80000000);
        static constexpr bits_type fraction_mask = UINT32_C(0x007fffff);
        static constexpr bits_type exponent_mask = UINT32_C(0x7f800000);
        static constexpr bits_type infinity = exponent_mask;
        static constexpr bits_type quiet_nan = UINT32_C(0x7fc00000);
        static constexpr bits_type denorm_min = UINT32_C(1);
    };

    template<>
    struct binary_format<double>
    {
        using bits_type = std::uint64_t;
        static constexpr int fraction_bits = 52;
        static constexpr int exponent_bits = 11;
        static constexpr int exponent_bias = 1023;
        static constexpr bits_type sign_mask = UINT64_C(0x8000000000000000);
        static constexpr bits_type fraction_mask = UINT64_C(0x000fffffffffffff);
        static constexpr bits_type exponent_mask = UINT64_C(0x7ff0000000000000);
        static constexpr bits_type infinity = exponent_mask;
        static constexpr bits_type quiet_nan = UINT64_C(0x7ff8000000000000);
        static constexpr bits_type denorm_min = UINT64_C(1);
    };

    template<class Float>
    using bits_type = typename binary_format<Float>::bits_type;

    template<class Float>
    [[nodiscard]] constexpr bits_type<Float> bits(Float value) noexcept
    {
        static_assert(std::is_same_v<Float, float> || std::is_same_v<Float, double>);
        return std::bit_cast<bits_type<Float>>(value);
    }

    template<class Float>
    [[nodiscard]] constexpr bits_type<Float> magnitude_bits(Float value) noexcept
    {
        return bits(value) & ~binary_format<Float>::sign_mask;
    }

    template<class Float>
    [[nodiscard]] constexpr bool sign_bit(Float value) noexcept
    {
        return (bits(value) & binary_format<Float>::sign_mask) != 0;
    }

    template<class Float>
    [[nodiscard]] constexpr bool is_nan(Float value) noexcept
    {
        return magnitude_bits(value) > binary_format<Float>::infinity;
    }

    template<class Float>
    [[nodiscard]] constexpr bool is_inf(Float value) noexcept
    {
        return magnitude_bits(value) == binary_format<Float>::infinity;
    }

    template<class Float>
    [[nodiscard]] constexpr bool is_finite(Float value) noexcept
    {
        return magnitude_bits(value) < binary_format<Float>::infinity;
    }

    template<class Float>
    [[nodiscard]] FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION constexpr Float
    positive_infinity() noexcept
    {
        return std::bit_cast<Float>(binary_format<Float>::infinity);
    }

    template<class Float>
    [[nodiscard]] FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION constexpr Float
    negative_infinity() noexcept
    {
        return std::bit_cast<Float>(
            binary_format<Float>::sign_mask | binary_format<Float>::infinity);
    }

    template<class Float>
    [[nodiscard]] FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION constexpr Float
    quiet_nan() noexcept
    {
        return std::bit_cast<Float>(binary_format<Float>::quiet_nan);
    }

    template<class Float>
    [[nodiscard]] FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION constexpr Float
    signed_zero(bool negative) noexcept
    {
        return std::bit_cast<Float>(
            negative ? binary_format<Float>::sign_mask : bits_type<Float>{0});
    }

    template<class Float>
    [[nodiscard]] FLTX_VALIDATION_SPECIAL_VALUE_FUNCTION constexpr Float
    denorm_min() noexcept
    {
        return std::bit_cast<Float>(binary_format<Float>::denorm_min);
    }

    template<class Float>
    [[nodiscard]] constexpr Float next_up(Float value) noexcept
    {
        using format = binary_format<Float>;
        auto raw = bits(value);
        if (is_nan(value) || raw == format::infinity)
            return value;
        if (magnitude_bits(value) == 0)
            return denorm_min<Float>();
        if (sign_bit(value))
            --raw;
        else
            ++raw;
        return std::bit_cast<Float>(raw);
    }

    template<class Float>
    [[nodiscard]] constexpr Float next_down(Float value) noexcept
    {
        using format = binary_format<Float>;
        auto raw = bits(value);
        if (is_nan(value) ||
            raw == (format::sign_mask | format::infinity))
            return value;
        if (magnitude_bits(value) == 0)
        {
            return std::bit_cast<Float>(
                format::sign_mask | format::denorm_min);
        }
        if (sign_bit(value))
            ++raw;
        else
            --raw;
        return std::bit_cast<Float>(raw);
    }
}

#endif
