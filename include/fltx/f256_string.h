/**
 * fltx/f256_string.h - Constexpr string formatting, parsing, and literals for f256.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef F256_STRING_INCLUDED
#define F256_STRING_INCLUDED
#include <cstddef>
#include <ios>
#include <string>

#include "fltx/f256_limits.h"
#include "fltx/detail/f256_math_basic.h"
#include "fltx/detail/common_io.h"

namespace bl {

namespace detail::_f256 // primitives and kernels
{
    struct f256_io_traits;

    struct exact_traits
    {
        using value_type = f256_s;
        static constexpr int limb_count       = 4;
        static constexpr int significand_bits = 212;
        static constexpr int conversion_significand_bits = 53 * 5;
        static constexpr int max_binary_exponent = 1023;
        static constexpr int min_binary_exponent = -1074;

        static constexpr double limb(const value_type& x, int index) noexcept
        {
            switch (index)
            {
            case 0: return x.x0;
            case 1: return x.x1;
            case 2: return x.x2;
            default: return x.x3;
            }
        }

        static constexpr value_type zero(bool neg = false) noexcept
        {
            return neg ? value_type{ -0.0, 0.0, 0.0, 0.0 } : value_type{ 0.0, 0.0, 0.0, 0.0 };
        }

        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

        static constexpr value_type pack_from_significand(const detail::exact_decimal::biguint& q, int e2, bool neg) noexcept
        {
            if (q.bit_length() > significand_bits)
            {
                const std::uint64_t c4 = q.get_bits(0, 53);
                const std::uint64_t c3 = q.get_bits(53, 53);
                const std::uint64_t c2 = q.get_bits(106, 53);
                const std::uint64_t c1 = q.get_bits(159, 53);
                const std::uint64_t c0 = q.get_bits(212, 53);

                const double x0 = c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - 52) : 0.0;
                const double x1 = c1 ? detail::fp::ldexp(static_cast<double>(c1), e2 - 105) : 0.0;
                const double x2 = c2 ? detail::fp::ldexp(static_cast<double>(c2), e2 - 158) : 0.0;
                const double x3 = c3 ? detail::fp::ldexp(static_cast<double>(c3), e2 - 211) : 0.0;
                const double x4 = c4 ? detail::fp::ldexp(static_cast<double>(c4), e2 - 264) : 0.0;

                f256_s out = renorm5(x0, x1, x2, x3, x4);
                if (neg)
                    out = -out;
                return out;
            }

            const std::uint64_t c3 = q.get_bits(0, 53);
            const std::uint64_t c2 = q.get_bits(53, 53);
            const std::uint64_t c1 = q.get_bits(106, 53);
            const std::uint64_t c0 = q.get_bits(159, 53);

            const double x0 = c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - 52) : 0.0;
            const double x1 = c1 ? detail::fp::ldexp(static_cast<double>(c1), e2 - 105) : 0.0;
            const double x2 = c2 ? detail::fp::ldexp(static_cast<double>(c2), e2 - 158) : 0.0;
            const double x3 = c3 ? detail::fp::ldexp(static_cast<double>(c3), e2 - 211) : 0.0;

            f256_s out = renorm(x0, x1, x2, x3);
            if (neg)
                out = -out;
            return out;
        }
    };

    template<typename String>
    constexpr inline bool exact_scientific_digits(const f256_s& x, int sig, String& digits, int& exp10)
    {
        return detail::exact_decimal::exact_scientific_digits<exact_traits>(x, sig, digits, exp10);
    }

    inline constexpr detail::fltx_char_result emit_scientific_sig_to_chars(char* first, char* last, const f256_s& x, int sig_digits, bool strip_trailing_zeros) noexcept
    {
        if (iszero(x))
            return detail::emit_single_zero_to_chars(first, last);

        if (sig_digits < 1) sig_digits = 1;

        const bool neg = (x.x0 < 0.0);
        const f256_s v = neg ? -x : x;
        const int sig = static_cast<int>(sig_digits);

        bl::f256_io_string digits;
        int e = 0;
        if (!detail::_f256::exact_scientific_digits(v, sig, digits, e)) {
            if (first >= last) return { first, false };
            *first = '0';
            return { first + 1, true };
        }

        return detail::emit_scientific_digits_to_chars(
            first,
            last,
            neg,
            digits.data(),
            sig,
            e,
            strip_trailing_zeros);
    }

    struct f256_io_traits
    {
        using value_type = f256_s;
        using string_type = bl::f256_io_string;
        using exact_decimal_traits = detail::_f256::exact_traits;

        static constexpr int max_parse_order = detail::fltx_max_parse_order;
        static constexpr int min_parse_order = detail::fltx_min_parse_order;
        static constexpr int limb_count = detail::_f256::exact_traits::limb_count;
        static constexpr int significand_bits = detail::_f256::exact_traits::significand_bits;
        static constexpr int conversion_significand_bits = detail::_f256::exact_traits::conversion_significand_bits;
        static constexpr int max_binary_exponent = 1023;
        static constexpr int min_normal_binary_exponent = -1022;
        static constexpr int min_binary_exponent = -1074;
        static constexpr int max_fixed_integer_digits = 309;

        static constexpr double limb(const value_type& x, int index) noexcept { return detail::_f256::exact_traits::limb(x, index); }
        static constexpr bool isnan(const value_type& x)       noexcept { return bl::isnan(x); }
        static constexpr bool isinf(const value_type& x)       noexcept { return bl::isinf(x); }
        static constexpr bool iszero(const value_type& x)      noexcept { return bl::iszero(x); }
        static constexpr bool is_negative(const value_type& x) noexcept { return detail::_f256::signbit(x.x0); }
        static constexpr value_type abs(const value_type& x)   noexcept { return (x.x0 < 0.0) ? -x : x; }
        static constexpr value_type zero(bool neg = false) noexcept { return neg ? value_type{ -0.0, 0.0, 0.0, 0.0 } : value_type{ 0.0, 0.0, 0.0, 0.0 }; }
        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

        static constexpr value_type quiet_nan() noexcept { return std::numeric_limits<value_type>::quiet_NaN(); }
        static constexpr detail::fltx_char_result to_chars_general(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return detail::emit_general_decimal_for_traits<f256_io_traits>(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr detail::fltx_char_result to_chars_fixed(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return detail::emit_exact_fixed_decimal_to_chars<exact_decimal_traits, string_type>(
                first,
                last,
                x,
                precision,
                strip_trailing_zeros,
                iszero(x),
                is_negative(x));
        }

        static constexpr detail::fltx_char_result to_chars_default_fixed(
            char* first,
            char* last,
            const value_type& x,
            int precision,
            int,
            int,
            bool strip_trailing_zeros)
        {
            return detail::emit_exact_fixed_decimal_to_chars<exact_decimal_traits, string_type>(
                first,
                last,
                x,
                precision,
                strip_trailing_zeros,
                iszero(x),
                is_negative(x));
        }

        static constexpr detail::fltx_char_result to_chars_scientific_frac(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return detail::emit_scientific_frac_for_traits<f256_io_traits>(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr detail::fltx_char_result to_chars_scientific_sig(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return emit_scientific_sig_to_chars(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr value_type exact_uint64_to_value(std::uint64_t value, bool neg)
        {
            value_type out = bl::to_f256(value);
            return neg ? -out : out;
        }

        static constexpr bool compact_decimal_to_value(std::uint64_t coeff, int dec_exp, bool neg, value_type& out)
        {
            return detail::exact_decimal::compact_decimal_to_value<detail::_f256::exact_traits>(coeff, dec_exp, neg, out);
        }

        static constexpr value_type exact_decimal_to_value(const detail::exact_decimal::biguint& coeff, int dec_exp, bool neg)
        {
            return detail::exact_decimal::exact_decimal_to_value<detail::_f256::exact_traits>(coeff, dec_exp, neg);
        }

        static constexpr value_type pack_from_significand(const detail::exact_decimal::biguint& q, int e2, bool neg) noexcept
        {
            return detail::_f256::exact_traits::pack_from_significand(q, e2, neg);
        }
    };

    [[nodiscard]] BL_MSVC_NOINLINE constexpr bool parse(const char* s, f256_s& out, const char** endptr = nullptr) noexcept
    {
        return detail::parse_flt<f256_io_traits>(s, out, endptr);
    }

} // namespace detail::_f256

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s to_f256(const char* s) noexcept
{
    f256_s ret;
    if (detail::_f256::parse(s, ret))
        return ret;
    return f256_s{ 0.0 };
}

[[nodiscard]] BL_FORCE_INLINE constexpr f256_s to_f256(const std::string& s) noexcept
{
    return to_f256(s.c_str());
}

[[nodiscard]] BL_MSVC_NOINLINE constexpr bl::f256_io_string to_static_string(
    const f256_s& value,
    int precision = std::numeric_limits<f256_s>::digits10,
    std::ios_base::fmtflags flags = std::ios_base::fmtflags{})
{
    return detail::to_static_string_impl<detail::_f256::f256_io_traits>(value, precision, flags);
}

[[nodiscard]] BL_NO_INLINE std::string to_string(
    const f256_s& value,
    precision_info precision = std::numeric_limits<f256_s>::digits10,
    std::ios_base::fmtflags flags = std::ios_base::fmtflags{});

namespace detail::_f256 // primitives and kernels
{
    [[nodiscard]] consteval f256_s parse_qd_literal(const char* text, const char* expected_end)
    {
        f256_s out{};

        if (!bl::detail::parse_literal_float_text<f256_io_traits>(text, expected_end, out))
            throw "invalid _qd literal";

        return out;
    }

} // namespace detail::_f256

namespace literals
{
    [[nodiscard]] consteval f256 operator""_qd(const char* text, std::size_t length)
    {
        return detail::_f256::parse_qd_literal(text, text + length);
    }

    template<char... Chars>
    [[nodiscard]] consteval f256 operator""_qd()
    {
        constexpr char text[] = { Chars..., '\0' };
        return detail::_f256::parse_qd_literal(text, text + sizeof...(Chars));
    }

} // namespace literals

constexpr f256::f256(const char* text)
{
    const char* end = text;
    if (!detail::_f256::parse(text, *this, &end))
        throw "invalid f256";
}

} // namespace bl

#endif
