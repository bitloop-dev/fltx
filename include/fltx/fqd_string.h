/**
 * fltx/fqd_string.h - Constexpr string formatting, parsing, and literals for qd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_STRING_INCLUDED
#define FQD_STRING_INCLUDED
#include <cstddef>
#include <ios>
#include <string>

#include "fltx/fqd_limits.h"
#include "fltx/detail/fqd_math_basic.h"
#include "fltx/detail/common_io.h"

namespace bl {

namespace detail::_qd // primitives and kernels
{
    struct qd_io_traits;

    struct exact_traits
    {
        using value_type = fqd_s;
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
            return detail::_qd::signed_zero(neg);
        }

        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

#if BL_FP_BARRIER_ACTIVE
        static constexpr std::uint64_t scaled_significand_limb_bits(
            std::uint64_t coefficient,
            int exponent) noexcept
        {
            if (coefficient == 0)
                return 0;

            constexpr std::uint64_t hidden_bit = 0x0010000000000000ull;
            constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
            constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;

            const int leading_bit = detail::fp::highest_bit_index(coefficient);
            const std::uint64_t significand = coefficient << (52 - leading_bit);
            const int unbiased_exponent = leading_bit + exponent;

            if (unbiased_exponent > 1023)
                return exponent_mask;
            if (unbiased_exponent >= -1022)
            {
                const std::uint64_t exponent_bits =
                    static_cast<std::uint64_t>(unbiased_exponent + 1023) << 52;
                return exponent_bits | (significand & fraction_mask);
            }

            const int shift = -1022 - unbiased_exponent;
            if (shift >= 64)
                return 0;

            const std::uint64_t truncated = significand >> shift;
            const std::uint64_t remainder_mask =
                (std::uint64_t{ 1 } << shift) - 1;
            const std::uint64_t remainder = significand & remainder_mask;
            const std::uint64_t halfway = std::uint64_t{ 1 } << (shift - 1);
            const bool round_up = remainder > halfway ||
                (remainder == halfway && (truncated & 1u) != 0);
            const std::uint64_t rounded = truncated + static_cast<std::uint64_t>(round_up);
            return rounded >= hidden_bit ? hidden_bit : rounded;
        }

        // Extracts each limb from the exact signed remainder, avoiding the
        // floating-point error-free transforms that fast-math can reassociate.
        static constexpr value_type pack_guarded_significand(
            const detail::exact_decimal::biguint& q,
            int e2,
            bool neg) noexcept
        {
            constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
            constexpr std::uint64_t limb_unit = std::uint64_t{ 1 } << 53;
            constexpr std::uint64_t halfway = std::uint64_t{ 1 } << 52;

            const int bits = q.bit_length();
            if (bits <= 0)
                return zero(neg);

            const int unit_exponent = e2 - (bits - 1);
            detail::exact_decimal::signed_biguint remainder{ q, neg };
            std::uint64_t limb_bits[4]{};
            for (int i = 0; i < 4 && !remainder.mag.is_zero(); ++i)
            {
                const int remainder_bits = remainder.mag.bit_length();
                const int highest_exponent = unit_exponent + remainder_bits - 1;
                const int shift = highest_exponent >= -1022
                    ? (remainder_bits > 53 ? remainder_bits - 53 : 0)
                    : (-1074 > unit_exponent ? -1074 - unit_exponent : 0);

                detail::exact_decimal::biguint rounded =
                    detail::exact_decimal::shr_bits_copy(remainder.mag, shift);
                if (shift > 0)
                {
                    const bool round_bit = remainder.mag.get_bit(shift - 1);
                    const bool sticky = shift > 1 &&
                        detail::exact_decimal::any_low_bits_set(remainder.mag, shift - 1);
                    if (round_bit && (sticky || rounded.is_odd()))
                        rounded.add_small(1);
                }

                const std::uint64_t coefficient = rounded.get_bits(0, 54);
                if (coefficient == 0)
                    break;

                std::uint64_t bits = coefficient == limb_unit
                    ? scaled_significand_limb_bits(halfway, unit_exponent + shift + 1)
                    : scaled_significand_limb_bits(coefficient, unit_exponent + shift);
                if (remainder.neg)
                    bits |= sign_mask;
                limb_bits[i] = bits;

                if (i != 3)
                {
                    detail::exact_decimal::biguint term{ coefficient };
                    term.shl_bits(shift);
                    detail::exact_decimal::add_signed(remainder, term, !remainder.neg);
                }
            }
            return {
                std::bit_cast<double>(limb_bits[0]),
                std::bit_cast<double>(limb_bits[1]),
                std::bit_cast<double>(limb_bits[2]),
                std::bit_cast<double>(limb_bits[3])
            };
        }
#endif

        static constexpr value_type pack_from_significand(const detail::exact_decimal::biguint& q, int e2, bool neg) noexcept
        {
#if BL_FP_BARRIER_ACTIVE
            return pack_guarded_significand(q, e2, neg);
#else
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

                fqd_s out;
                if (x0 == std::numeric_limits<double>::max())
                {
                    // Keep a maximum-binary64 head below the overflow boundary
                    // during constant evaluation. Power-of-two scaling is exact
                    // for every limb here.
                    out = renorm5(
                        x0 * 0.5,
                        x1 * 0.5,
                        x2 * 0.5,
                        x3 * 0.5,
                        x4 * 0.5);
                    out.x0 = detail::fp::ldexp(out.x0, 1);
                    out.x1 = detail::fp::ldexp(out.x1, 1);
                    out.x2 = detail::fp::ldexp(out.x2, 1);
                    out.x3 = detail::fp::ldexp(out.x3, 1);
                }
                else
                {
                    out = renorm5(x0, x1, x2, x3, x4);
                }
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

            fqd_s out = renorm(x0, x1, x2, x3);
            if (neg)
                out = -out;
            return out;
#endif
        }
    };

    template<typename String>
    constexpr inline bool exact_scientific_digits(const fqd_s& x, int sig, String& digits, int& exp10)
    {
        return detail::exact_decimal::exact_scientific_digits<exact_traits>(x, sig, digits, exp10);
    }

    inline constexpr detail::fltx_char_result emit_scientific_sig_to_chars(char* first, char* last, const fqd_s& x, int sig_digits, bool strip_trailing_zeros) noexcept
    {
        if (iszero(x))
            return detail::emit_single_zero_to_chars(first, last);

        if (sig_digits < 1) sig_digits = 1;

        const bool neg = (x.x0 < 0.0);
        const fqd_s v = neg ? -x : x;
        const int sig = static_cast<int>(sig_digits);

        bl::fqd_io_string digits;
        int e = 0;
        if (!detail::_qd::exact_scientific_digits(v, sig, digits, e)) {
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

    struct qd_io_traits
    {
        using value_type = fqd_s;
        using string_type = bl::fqd_io_string;
        using exact_decimal_traits = detail::_qd::exact_traits;

        static constexpr int max_parse_order = detail::fltx_max_parse_order;
        static constexpr int min_parse_order = detail::fltx_min_parse_order;
        static constexpr int limb_count = detail::_qd::exact_traits::limb_count;
        static constexpr int significand_bits = detail::_qd::exact_traits::significand_bits;
        static constexpr int conversion_significand_bits = detail::_qd::exact_traits::conversion_significand_bits;
        static constexpr int max_binary_exponent = 1023;
        static constexpr int min_normal_binary_exponent = -1022;
        static constexpr int min_binary_exponent = -1074;
        static constexpr int max_fixed_integer_digits = 309;

        static constexpr double limb(const value_type& x, int index) noexcept { return detail::_qd::exact_traits::limb(x, index); }
        static constexpr bool isnan(const value_type& x)       noexcept { return bl::isnan(x); }
        static constexpr bool isinf(const value_type& x)       noexcept { return bl::isinf(x); }
        static constexpr bool iszero(const value_type& x)      noexcept { return bl::iszero(x); }
        static constexpr bool is_negative(const value_type& x) noexcept { return detail::_qd::signbit(x.x0); }
        static constexpr value_type abs(const value_type& x)   noexcept { return (x.x0 < 0.0) ? -x : x; }
        static constexpr value_type zero(bool neg = false) noexcept { return detail::_qd::signed_zero(neg); }
        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

        static constexpr value_type quiet_nan() noexcept { return std::numeric_limits<value_type>::quiet_NaN(); }
        static constexpr value_type max_finite() noexcept { return std::numeric_limits<value_type>::max(); }
        static constexpr bool needs_nominal_refinement(const value_type& value) noexcept
        {
#if BL_FP_BARRIER_ACTIVE
            (void)value;
            return false;
#else
            constexpr int guard_underflow_exponent =
                min_binary_exponent + conversion_significand_bits - 1;
            return iszero(value) ||
                detail::fp::absd(value.x0) <
                    detail::fp::positive_power_of_two(guard_underflow_exponent);
#endif
        }
        static constexpr detail::fltx_char_result to_chars_general(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return detail::emit_general_decimal_for_traits<qd_io_traits>(first, last, x, precision, strip_trailing_zeros);
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
            return detail::emit_scientific_frac_for_traits<qd_io_traits>(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr detail::fltx_char_result to_chars_scientific_sig(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return emit_scientific_sig_to_chars(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr value_type exact_uint64_to_value(std::uint64_t value, bool neg)
        {
            value_type out = detail::_qd::integer_to_qd(value);
            return neg ? -out : out;
        }

        static constexpr bool compact_decimal_to_value(std::uint64_t coeff, int dec_exp, bool neg, value_type& out)
        {
            return detail::exact_decimal::compact_decimal_to_value<detail::_qd::exact_traits>(coeff, dec_exp, neg, out);
        }

        static constexpr value_type exact_decimal_to_value(const detail::exact_decimal::biguint& coeff, int dec_exp, bool neg)
        {
            return detail::exact_decimal::exact_decimal_to_value<detail::_qd::exact_traits>(coeff, dec_exp, neg);
        }

        static constexpr value_type nextafter(const value_type& from, const value_type& to) noexcept
        {
            return detail::_qd_impl::nextafter(from, to);
        }

        static constexpr value_type pack_from_significand(const detail::exact_decimal::biguint& q, int e2, bool neg) noexcept
        {
            return detail::_qd::exact_traits::pack_from_significand(q, e2, neg);
        }
    };

    [[nodiscard]] BL_MSVC_NOINLINE constexpr bool parse(const char* s, fqd_s& out, const char** endptr = nullptr) noexcept
    {
        return detail::parse_flt<qd_io_traits>(s, out, endptr);
    }

} // namespace detail::_qd

[[nodiscard]] BL_MSVC_NOINLINE constexpr bl::fqd_io_string to_static_string(
    const fqd_s& value,
    int precision = std::numeric_limits<fqd_s>::max_digits10,
    std::ios_base::fmtflags flags = std::ios_base::fmtflags{})
{
    return detail::to_static_string_impl<detail::_qd::qd_io_traits>(value, precision, flags);
}

[[nodiscard]] BL_NO_INLINE std::string to_string(
    const fqd_s& value,
    precision_info precision = std::numeric_limits<fqd_s>::max_digits10,
    std::ios_base::fmtflags flags = std::ios_base::fmtflags{});

namespace detail::_qd // primitives and kernels
{
    [[nodiscard]] consteval fqd_s parse_qd_literal(const char* text, const char* expected_end)
    {
        fqd_s out{};

        if (!bl::detail::parse_literal_float_text<qd_io_traits>(text, expected_end, out))
            throw "invalid _fqd literal";

        return out;
    }

} // namespace detail::_qd

namespace literals
{
    [[nodiscard]] consteval fqd operator""_qd(const char* text, std::size_t length)
    {
        return detail::_qd::parse_qd_literal(text, text + length);
    }

    template<char... Chars>
    [[nodiscard]] consteval fqd operator""_qd()
    {
        constexpr char text[] = { Chars..., '\0' };
        return detail::_qd::parse_qd_literal(text, text + sizeof...(Chars));
    }

} // namespace literals

} // namespace bl

#endif
