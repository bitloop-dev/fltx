/**
 * fltx/fdd_string.h - Constexpr string formatting, parsing, and literals for dd values.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_STRING_INCLUDED
#define FDD_STRING_INCLUDED
#include <charconv>
#include <cstddef>
#include <ios>
#include <string>
#include <type_traits>

#include "fltx/fdd_limits.h"
#include "fltx/detail/fdd_math_basic.h"
#include "fltx/detail/common_io.h"

namespace bl {

namespace detail::_dd // primitives and kernels
{
    struct dd_io_traits;

    [[nodiscard]] BL_FORCE_INLINE constexpr bool runtime_native_double_subnormal(const fdd_s& x) noexcept
    {
        return !std::is_constant_evaluated() &&
            x.lo == 0.0 &&
            detail::fp::isfinite(x.hi) &&
            detail::fp::absd(x.hi) < std::numeric_limits<double>::min();
    }

    BL_FORCE_INLINE detail::fltx_char_result strip_scientific_trailing_zeros(char* first, char* ptr) noexcept
    {
        char* exp = first;
        while (exp < ptr && *exp != 'e' && *exp != 'E')
            ++exp;
        if (exp == ptr)
            return { ptr, true };

        char* frac_end = exp;
        while (frac_end > first && frac_end[-1] == '0')
            --frac_end;
        if (frac_end > first && frac_end[-1] == '.')
            --frac_end;
        if (frac_end == exp)
            return { ptr, true };

        char* out = frac_end;
        for (char* in = exp; in < ptr; ++in)
            *out++ = *in;
        return { out, true };
    }

    BL_FORCE_INLINE detail::fltx_char_result emit_native_double_to_chars(
        char* first,
        char* last,
        double x,
        std::chars_format format,
        int precision,
        bool strip_trailing_zeros = false) noexcept
    {
        auto result = std::to_chars(first, last, x, format, precision);
        if (result.ec != std::errc{})
            return { first, false };
        if (strip_trailing_zeros && format == std::chars_format::scientific)
            return strip_scientific_trailing_zeros(first, result.ptr);
        return { result.ptr, true };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool normalize10(const fdd_s& x, fdd_s& m, int& exp10)
    {
        if (x.hi == 0.0 && x.lo == 0.0) { m = fdd_s{ 0.0 }; exp10 = 0; return true; }

        fdd_s ax = abs(x);

        int e2 = detail::fp::frexp_exponent(ax.hi); // ax.hi = f * 2^(e2-1)
        int e10 = (int)detail::fp::floor((e2 - 1) * 0.30102999566398114); // approx log10(2)

        m = ax * bl::detail::_dd_impl::pow10_fdd(-e10);
        if (!detail::fp::isfinite(m.hi))
            return false;

        int guard = 0;
        while (m >= fdd_s{ 10.0 }) {
            m = m / fdd_s{ 10.0 };
            ++e10;
            if (!detail::fp::isfinite(m.hi) || ++guard > 16)
                return false;
        }

        guard = 0;
        while (m < fdd_s{ 1.0 }) {
            m = m * fdd_s{ 10.0 };
            --e10;
            if (!detail::fp::isfinite(m.hi) || ++guard > 16)
                return false;
        }
        exp10 = e10;
        return true;
    }

    BL_PUSH_PRECISE;
    BL_FORCE_INLINE constexpr fdd_s mul_by_double_print(fdd_s a, double b) noexcept
    {
        double p, err;
        detail::_dd::two_prod_precise(a.hi, b, p, err);
        err += a.lo * b;

        double s, e;
        detail::_dd::two_sum_precise(p, err, s, e);
        return fdd_s{ s, e };
    }

    BL_FORCE_INLINE constexpr fdd_s sub_by_double_print(fdd_s a, double b) noexcept
    {
        double s, e;
        detail::_dd::two_sum_precise(a.hi, -b, s, e);
        e += a.lo;

        double ss, ee;
        detail::_dd::two_sum_precise(s, e, ss, ee);
        return fdd_s{ ss, ee };
    }
    BL_POP_PRECISE;

    constexpr inline bool approximate_scientific_digits(const fdd_s& x, int sig, char* digits, int capacity, int& exp10) noexcept
    {
        if (sig < 1)
            sig = 1;

        fdd_s scaled{};
        if (!normalize10(x, scaled, exp10))
            return false;
        if (!(scaled >= fdd_s{ 1.0 }) || !(scaled < fdd_s{ 10.0 }))
            return false;

        const int digit_count = sig + 2;
        if (digit_count > capacity)
            return false;

        for (int i = 0; i < digit_count; ++i)
        {
            const int digit = static_cast<int>(scaled.hi);
            if (digit < -9 || digit > 19)
                return false;

            digits[i] = static_cast<char>('0' + digit);
            scaled = mul_by_double_print(sub_by_double_print(scaled, static_cast<double>(digit)), 10.0);
        }

        for (int i = digit_count - 1; i > 0; --i)
        {
            if (digits[i] < '0')
            {
                --digits[i - 1];
                digits[i] = static_cast<char>(digits[i] + 10);
            }
            else if (digits[i] > '9')
            {
                ++digits[i - 1];
                digits[i] = static_cast<char>(digits[i] - 10);
            }
        }

        if (digits[0] <= '0' || digits[0] > '9')
            return false;

        const int round_digit_index = sig;
        const int sticky_digit_index = sig + 1;
        const int round_digit = digits[round_digit_index] - '0';

        // The approximate path is fast, but decimal cases close to a rounding
        // boundary are exactly where the stream-compatible policy matters.
        if (round_digit == 4 || round_digit == 5)
            return false;

        const bool sticky =
            digits[sticky_digit_index] != '0' ||
            scaled.hi > 0.0 ||
            scaled.lo > 0.0;
        const int last_kept_digit = digits[sig - 1] - '0';
        const bool round_up =
            round_digit > 5 ||
            (round_digit == 5 && (sticky || ((last_kept_digit & 1) != 0)));

        if (round_up)
        {
            ++digits[sig - 1];
            int i = sig - 1;
            while (i > 0 && digits[i] > '9')
            {
                digits[i] = static_cast<char>(digits[i] - 10);
                ++digits[--i];
            }
        }

        if (digits[0] > '9')
        {
            ++exp10;
            for (int i = sig - 1; i >= 2; --i)
                digits[i] = digits[i - 1];
            digits[0] = '1';
            if (sig > 1)
                digits[1] = '0';
        }

        return true;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool should_try_approximate_scientific_digits(const fdd_s& x, int sig) noexcept
    {
        if (sig <= 16)
            return true;

        constexpr int max_meaningful_sig = std::numeric_limits<fdd_s>::digits10 + 1;
        return sig < max_meaningful_sig && x.hi >= 1.0e-4 && x.hi < 1.0e5;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool should_accept_approximate_scientific_digits(int sig, int exp10) noexcept
    {
        if (sig <= 16)
            return true;

        constexpr int max_meaningful_sig = std::numeric_limits<fdd_s>::digits10 + 1;
        return sig < max_meaningful_sig && exp10 >= -4 && exp10 <= 4;
    }

    BL_FORCE_INLINE constexpr int emit_uint_rev_buf(char* dst, fdd_s n)
    {
        // n is a non-negative integer in dd
        const fdd_s base = fdd_s{ 1000000000.0 }; // 1e9

        int len = 0;

        if (n < fdd_s{ 10.0 }) {
            const std::uint32_t d = detail::bounded_floor_to_u32(detail::fp::floor(n.hi), 9u);
            dst[len++] = static_cast<char>('0' + d);
            return len;
        }

        while (n >= base) {
            fdd_s q = detail::_dd_impl::floor(n / base);
            fdd_s r = n - q * base;

            std::uint32_t chunk = detail::bounded_floor_to_u32(detail::fp::floor(r.hi), 1000000000u);
            if (chunk >= 1000000000u) { chunk = 0; q = q + fdd_s{ 1.0 }; }

            for (int i = 0; i < 9; ++i) {
                const std::uint32_t next = chunk / 10u;
                const std::uint32_t d = chunk - next * 10u;
                dst[len++] = static_cast<char>('0' + d);
                chunk = next;
            }

            n = q;
        }

        len += detail::append_uint32_rev(
            dst + len,
            detail::bounded_floor_to_u32(detail::fp::floor(n.hi), 999999999u));

        return len;
    }

    struct exact_traits
    {
        using value_type = fdd_s;
        static constexpr int limb_count       = 2;
        static constexpr int significand_bits = 106;
        static constexpr int max_binary_exponent = 1023;
        static constexpr int min_binary_exponent = -1074;

        static constexpr double scaled_significand_limb(
            std::uint64_t coefficient,
            int exponent) noexcept
        {
            if (coefficient == 0)
                return 0.0;

#if BL_FP_BARRIER_ACTIVE
            constexpr std::uint64_t hidden_bit = 0x0010000000000000ull;
            constexpr std::uint64_t fraction_mask = 0x000fffffffffffffull;
            constexpr std::uint64_t exponent_mask = 0x7ff0000000000000ull;

            const int leading_bit = detail::fp::highest_bit_index(coefficient);
            const std::uint64_t significand = coefficient << (52 - leading_bit);
            const int unbiased_exponent = leading_bit + exponent;

            if (unbiased_exponent > 1023)
                return std::bit_cast<double>(exponent_mask);
            if (unbiased_exponent >= -1022)
            {
                const std::uint64_t exponent_bits =
                    static_cast<std::uint64_t>(unbiased_exponent + 1023) << 52;
                return std::bit_cast<double>(
                    exponent_bits | (significand & fraction_mask));
            }

            const int shift = -1022 - unbiased_exponent;
            if (shift >= 64)
                return 0.0;

            const std::uint64_t truncated = significand >> shift;
            const std::uint64_t remainder_mask =
                (std::uint64_t{ 1 } << shift) - 1;
            const std::uint64_t remainder = significand & remainder_mask;
            const std::uint64_t halfway = std::uint64_t{ 1 } << (shift - 1);
            const bool round_up = remainder > halfway ||
                (remainder == halfway && (truncated & 1u) != 0);
            const std::uint64_t rounded = truncated + static_cast<std::uint64_t>(round_up);
            return std::bit_cast<double>(
                rounded >= hidden_bit ? hidden_bit : rounded);
#else
            return detail::fp::ldexp(static_cast<double>(coefficient), exponent);
#endif
        }

#if BL_FP_BARRIER_ACTIVE
        // Canonicalizes the guarded integer significand without passing a
        // subnormal residual through floating-point arithmetic.
        static constexpr value_type pack_guarded_significand(
            const detail::exact_decimal::biguint& q,
            int e2,
            bool neg) noexcept
        {
            constexpr std::uint64_t sign_mask = 0x8000000000000000ull;
            constexpr std::uint64_t limb_unit = std::uint64_t{ 1 } << 53;
            const int discarded_bits = q.bit_length() - significand_bits;

            std::uint64_t lo_coefficient = q.get_bits(discarded_bits, 53);
            std::uint64_t hi_coefficient = q.get_bits(discarded_bits + 53, 53);
            if (discarded_bits > 0)
            {
                const bool round_bit = q.get_bit(discarded_bits - 1);
                const bool sticky = discarded_bits > 1 &&
                    detail::exact_decimal::any_low_bits_set(q, discarded_bits - 1);
                if (round_bit && (sticky || (lo_coefficient & 1u) != 0))
                {
                    ++lo_coefficient;
                    if (lo_coefficient == limb_unit)
                    {
                        lo_coefficient = 0;
                        ++hi_coefficient;
                    }
                }
            }

            if (hi_coefficient == limb_unit)
            {
                hi_coefficient = std::uint64_t{ 1 } << 52;
                ++e2;
            }

            constexpr std::uint64_t halfway = std::uint64_t{ 1 } << 52;
            const bool round_hi = lo_coefficient > halfway ||
                (lo_coefficient == halfway && (hi_coefficient & 1u) != 0);
            if (round_hi)
                ++hi_coefficient;

            double hi{};
            if (hi_coefficient == limb_unit)
                hi = scaled_significand_limb(std::uint64_t{ 1 } << 52, e2 - 51);
            else
                hi = scaled_significand_limb(hi_coefficient, e2 - 52);

            const std::int64_t residual = static_cast<std::int64_t>(lo_coefficient) -
                (round_hi ? static_cast<std::int64_t>(limb_unit) : 0);
            const std::uint64_t residual_magnitude = residual < 0
                ? static_cast<std::uint64_t>(-residual)
                : static_cast<std::uint64_t>(residual);
            double lo = scaled_significand_limb(residual_magnitude, e2 - 105);
            if (residual < 0)
                lo = std::bit_cast<double>(std::bit_cast<std::uint64_t>(lo) | sign_mask);

            if (neg)
            {
                hi = std::bit_cast<double>(std::bit_cast<std::uint64_t>(hi) ^ sign_mask);
                lo = std::bit_cast<double>(std::bit_cast<std::uint64_t>(lo) ^ sign_mask);
            }
            return { hi, lo };
        }
#endif

        static constexpr double limb(const value_type& x, int index) noexcept
        {
            return index == 0 ? x.hi : x.lo;
        }

        static constexpr value_type zero(bool neg = false) noexcept
        {
            return detail::_dd::signed_zero(neg);
        }

        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

        static constexpr value_type pack_from_significand(const detail::exact_decimal::biguint& q, int e2, bool neg) noexcept
        {
            const int bits = q.bit_length();
            if (bits <= 0)
                return zero(neg);

            if (bits <= 53)
            {
                const std::uint64_t c0 = q.get_bits(0, bits);
                value_type out{ c0 ? detail::fp::ldexp(static_cast<double>(c0), e2 - (bits - 1)) : 0.0, 0.0 };
                return neg ? -out : out;
            }

            if (bits <= 106)
            {
                const int lo_width = bits - 53;
                const std::uint64_t c1 = q.get_bits(0, lo_width);
                const std::uint64_t c0 = q.get_bits(lo_width, 53);

                const double hi = scaled_significand_limb(c0, e2 - 52);
                const double lo = scaled_significand_limb(c1, e2 - (bits - 1));
#if BL_FP_BARRIER_ACTIVE
                double guarded_hi = hi;  BL_FP_BARRIER(guarded_hi);
                double guarded_lo = lo;  BL_FP_BARRIER(guarded_lo);
                value_type out = detail::_dd::is_subnormal_limb(guarded_lo)
                    ? value_type{ guarded_hi, guarded_lo }
                    : detail::_dd::renorm(guarded_hi, guarded_lo);
#else
                value_type out = detail::_dd::renorm(hi, lo);
#endif
                return neg ? -out : out;
            }

            const int lo_width = bits - 106;
            const std::uint64_t c2 = q.get_bits(0, lo_width);
            const std::uint64_t c1 = q.get_bits(lo_width, 53);
            const std::uint64_t c0 = q.get_bits(bits - 53, 53);

#if BL_FP_BARRIER_ACTIVE
            constexpr int guarded_significand_bits = significand_bits + 53;
            constexpr int guarded_underflow_exponent =
                min_binary_exponent + guarded_significand_bits - 1;
            if (e2 < guarded_underflow_exponent)
                return pack_guarded_significand(q, e2, neg);
#endif

            const double hi = scaled_significand_limb(c0, e2 - 52);
            const double mid = scaled_significand_limb(c1, e2 - 105);
            const double lo = scaled_significand_limb(c2, e2 - (bits - 1));

#if BL_FP_BARRIER_ACTIVE
            double guarded_hi = hi;    BL_FP_BARRIER(guarded_hi);
            double guarded_mid = mid;  BL_FP_BARRIER(guarded_mid);
            double guarded_lo = lo;    BL_FP_BARRIER(guarded_lo);
            if (detail::_dd::is_subnormal_limb(guarded_mid))
            {
                constexpr std::uint64_t magnitude_mask = 0x7fffffffffffffffull;
                const std::uint64_t tail_bits =
                    (std::bit_cast<std::uint64_t>(guarded_mid) & magnitude_mask) +
                    (std::bit_cast<std::uint64_t>(guarded_lo) & magnitude_mask);
                value_type out{
                    guarded_hi,
                    std::bit_cast<double>(tail_bits)
                };
                return neg ? -out : out;
            }

            const fdd_s tail = detail::_dd::renorm(guarded_mid, guarded_lo);
#else
            const fdd_s tail = detail::_dd::renorm(mid, lo);
#endif
            fdd_s out = detail::_dd::renorm(hi, tail.hi);
            out = detail::_dd::renorm(out.hi, out.lo + tail.lo);
            if (neg)
                out = -out;
            return out;
        }
    };

    template<typename String>
    constexpr inline bool exact_scientific_digits(const fdd_s& x, int sig, String& digits, int& exp10)
    {
        if (sig < 1)
            sig = 1;

        detail::exact_decimal::biguint magnitude;
        int common_exp = 0;
        bool exact_neg = false;
        if (sig <= 32 &&
            detail::exact_decimal::exact_binary_components<exact_traits>(x, magnitude, common_exp, exact_neg) &&
            !exact_neg)
        {
            exp10 = detail::exact_decimal::decimal_exponent_from_components(magnitude, common_exp);

            detail::exact_decimal::biguint coefficient;
            int exact_exp10 = exp10;
            if (detail::exact_decimal::exact_significant_decimal<exact_traits>(
                    magnitude,
                    common_exp,
                    sig,
                    coefficient,
                    exact_exp10))
            {
                exp10 = exact_exp10;
                digits = detail::exact_decimal::to_decimal_string<String>(coefficient);
                if (static_cast<int>(digits.size()) < sig)
                {
                    const std::size_t zero_pad_count = static_cast<std::size_t>(sig - static_cast<int>(digits.size()));
                    digits.insert(0, zero_pad_count, '0');
                }
                return true;
            }

            fdd_s m = x * bl::detail::_dd_impl::pow10_fdd(-exp10);
            if (detail::fp::isfinite(m.hi))
            {
                detail::exact_decimal::biguint candidate;
                for (int i = 0; i < sig; ++i)
                {
                    int digit = static_cast<int>(detail::fp::floor(m.hi));
                    if (digit < 0) digit = 0;
                    else if (digit > 9) digit = 9;

                    candidate.mul_small(10);
                    candidate.add_small(static_cast<std::uint32_t>(digit));

                    m = mul_by_double_print(sub_by_double_print(m, static_cast<double>(digit)), 10.0);
                }

                detail::exact_decimal::biguint coefficient;
                int corrected_exp10 = exp10;
                if (detail::exact_decimal::exact_significant_decimal_from_floor_candidate<exact_traits>(
                        magnitude,
                        common_exp,
                        sig,
                        candidate,
                        corrected_exp10,
                        coefficient))
                {
                    exp10 = corrected_exp10;
                    digits = detail::exact_decimal::to_decimal_string<String>(coefficient);
                    if (static_cast<int>(digits.size()) < sig)
                    {
                        const std::size_t zero_pad_count = static_cast<std::size_t>(sig - static_cast<int>(digits.size()));
                        digits.insert(0, zero_pad_count, '0');
                    }
                    return true;
                }
            }
        }

        return detail::exact_decimal::exact_scientific_digits<exact_traits>(x, sig, digits, exp10);
    }

    constexpr detail::fltx_char_result emit_fixed_dec_to_chars(char* first, char* last, fdd_s x, int prec, bool strip_trailing_zeros) noexcept
    {
        if (prec < 0) prec = 0;

        if (x.hi == 0.0 && x.lo == 0.0)
            return detail::emit_fixed_zero_to_chars(first, last, detail::_dd::signbit(x.hi), prec, strip_trailing_zeros);

        const bool neg = (x.hi < 0.0);
        if (neg) x = fdd_s{ -x.hi, -x.lo };
        x = detail::_dd::renorm(x.hi, x.lo);

        fdd_s ip = detail::_dd_impl::floor(x);
        fdd_s fp = x - ip;

        if (fp >= fdd_s{ 1.0 }) { fp = fp - fdd_s{ 1.0 }; ip = ip + fdd_s{ 1.0 }; }
        else if (fp < fdd_s{ 0.0 }) { fp = fdd_s{ 0.0 }; }

        constexpr int kFracStack = 2048;
        char frac_stack[kFracStack];
        char* frac = frac_stack;

        std::string frac_dyn;
        if (prec > kFracStack) {
            frac_dyn.resize((size_t)prec);
            frac = (char*)frac_dyn.data();
        }

        int frac_len = (prec > 0) ? prec : 0;

        if (prec > 0) {
            constexpr double kPow10[10] = {
                1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0,
                1000000.0, 10000000.0, 100000000.0, 1000000000.0
            };
            constexpr uint32_t kPow10u32[10] = {
                1u, 10u, 100u, 1000u, 10000u, 100000u,
                1000000u, 10000000u, 100000000u, 1000000000u
            };

            int written = 0;
            const int full = prec / 9;
            const int rem  = prec - full * 9;

            for (int c = 0; c < full; ++c) {
                fp = mul_by_double_print(fp, kPow10[9]);

                uint32_t chunk = 0;
                if (fp.hi > 0.0) {
                    const double hi_floor = detail::fp::floor(fp.hi);
                    if (hi_floor >= (double)kPow10u32[9])
                        chunk = kPow10u32[9] - 1u;
                    else
                        chunk = (uint32_t)hi_floor;
                }

                fp = sub_by_double_print(fp, (double)chunk);

                if (fp < fdd_s{ 0.0 }) {
                    if (chunk > 0u) {
                        --chunk;
                        fp = sub_by_double_print(fp, -1.0);
                    }
                    else {
                        fp = fdd_s{ 0.0 };
                    }
                }

                for (int i = 8; i >= 0; --i) {
                    frac[written + i] = char('0' + (chunk % 10u));
                    chunk /= 10u;
                }
                written += 9;
            }

            if (rem > 0) {
                fp = mul_by_double_print(fp, kPow10[rem]);

                uint32_t chunk = 0;
                const uint32_t chunk_limit = kPow10u32[rem] - 1u;
                if (fp.hi > 0.0) {
                    const double hi_floor = detail::fp::floor(fp.hi);
                    if (hi_floor >= (double)kPow10u32[rem])
                        chunk = chunk_limit;
                    else
                        chunk = (uint32_t)hi_floor;
                }

                fp = sub_by_double_print(fp, (double)chunk);

                if (fp < fdd_s{ 0.0 }) {
                    if (chunk > 0u) {
                        --chunk;
                        fp = sub_by_double_print(fp, -1.0);
                    }
                    else {
                        fp = fdd_s{ 0.0 };
                    }
                }

                for (int i = rem - 1; i >= 0; --i) {
                    frac[written + i] = char('0' + (chunk % 10u));
                    chunk /= 10u;
                }
                written += rem;
            }

            fdd_s la = mul_by_double_print(fp, 10.0);
            int next = (int)la.hi;
            if (next < 0) next = 0; else if (next > 9) next = 9;
            fdd_s remv = sub_by_double_print(la, (double)next);

            const int last_digit = frac[prec - 1] - '0';
            bool round_up = false;
            if (next > 5) round_up = true;
            else if (next < 5) round_up = false;
            else {
                const bool gt_half = (remv.hi > 0.0) || (remv.lo > 0.0);
                round_up = gt_half || ((last_digit & 1) != 0);
            }

            if (round_up) {
                int i = prec - 1;
                for (; i >= 0; --i) {
                    char& c = frac[i];
                    if (c == '9') c = '0';
                    else { c = char(c + 1); break; }
                }
                if (i < 0) {
                    ip = ip + fdd_s{ 1.0 };
                    for (int j = 0; j < prec; ++j) frac[j] = '0';
                }
            }

            if (strip_trailing_zeros) {
                while (frac_len > 0 && frac[frac_len - 1] == '0') --frac_len;
            }
        }

        char int_rev[320];
        int int_len = emit_uint_rev_buf(int_rev, ip);

        if (neg && int_len == 1 && int_rev[0] == '0' && frac_len == 0) {
            if (first >= last) return { first, false };
            *first = '0';
            return { first + 1, true };
        }

        const size_t needed = (size_t)(neg ? 1 : 0) + (size_t)int_len + (frac_len ? (size_t)(1 + frac_len) : 0u);
        if ((size_t)(last - first) < needed) return { first, false };

        char* p = first;
        if (neg) *p++ = '-';

        for (int i = int_len - 1; i >= 0; --i) *p++ = int_rev[i];

        if (frac_len > 0) {
            *p++ = '.';
            detail::copy_chars(p, frac, static_cast<std::size_t>(frac_len));
            p += frac_len;
        }

        return { p, true };
    }

    BL_FORCE_INLINE constexpr detail::fltx_char_result emit_scientific_sig_to_chars_dd(char* first, char* last, const fdd_s& x, int sig_digits, bool strip_trailing_zeros) noexcept
    {
        if (iszero(x))
            return detail::emit_single_zero_to_chars(first, last);

        if (sig_digits < 1) sig_digits = 1;
        const bool neg = (x.hi < 0.0);
        const fdd_s v = neg ? -x : x;
        const int sig = static_cast<int>(sig_digits);
        char approximate_digits[40]{};
        bl::fdd_io_string exact_digits;
        const char* digit_data = approximate_digits;
        int e = 0;
        bool have_digits = false;
        if (detail::_dd::should_try_approximate_scientific_digits(v, sig) &&
            detail::_dd::approximate_scientific_digits(
                v,
                sig,
                approximate_digits,
                static_cast<int>(sizeof(approximate_digits)),
                e) &&
            detail::_dd::should_accept_approximate_scientific_digits(sig, e))
        {
            have_digits = true;
        }

        if (!have_digits) {
            if (!detail::_dd::exact_scientific_digits(v, sig, exact_digits, e))
            {
                if (first >= last) return { first, false };
                *first = '0';
                return { first + 1, true };
            }
            digit_data = exact_digits.data();
        }

        return detail::emit_scientific_digits_to_chars(
            first,
            last,
            neg,
            digit_data,
            sig,
            e,
            strip_trailing_zeros);
    }

    struct dd_io_traits
    {
        using value_type = fdd_s;
        using string_type = bl::fdd_io_string;
        using exact_decimal_traits = detail::_dd::exact_traits;

        static constexpr int max_parse_order = detail::fltx_max_parse_order;
        static constexpr int min_parse_order = detail::fltx_min_parse_order;
        static constexpr int limb_count = detail::_dd::exact_traits::limb_count;
        static constexpr int significand_bits = detail::_dd::exact_traits::significand_bits;
        static constexpr int conversion_significand_bits = 53 * (limb_count + 1);
        static constexpr int decimal_conversion_guard_bits = conversion_significand_bits - significand_bits;
        static constexpr int decimal_conversion_significand_bits =
            significand_bits + decimal_conversion_guard_bits;
        static constexpr int max_binary_exponent = 1023;
        static constexpr int min_normal_binary_exponent = -1022;
        static constexpr int min_binary_exponent = -1074;
        static constexpr int max_fixed_integer_digits = 309;

        static constexpr double limb(const value_type& x, int index) noexcept { return detail::_dd::exact_traits::limb(x, index); }
        static constexpr bool isnan(const value_type& x)       noexcept { return bl::isnan(x); }
        static constexpr bool isinf(const value_type& x)       noexcept { return bl::isinf(x); }
        static constexpr bool iszero(const value_type& x)      noexcept { return bl::iszero(x); }
        static constexpr bool is_negative(const value_type& x) noexcept { return detail::_dd::signbit(x.hi); }
        static constexpr value_type abs(const value_type& x)   noexcept { return (x.hi < 0.0) ? -x : x; }
        static constexpr value_type zero(bool neg = false) noexcept { return detail::_dd::signed_zero(neg); }
        static constexpr value_type infinity(bool neg = false) noexcept
        {
            const value_type inf = std::numeric_limits<value_type>::infinity();
            return neg ? -inf : inf;
        }

        static constexpr value_type quiet_nan() noexcept { return std::numeric_limits<value_type>::quiet_NaN(); }
        static constexpr value_type max_finite() noexcept { return std::numeric_limits<value_type>::max(); }
        static constexpr bool needs_nominal_refinement(const value_type& value) noexcept
        {
            constexpr int guard_underflow_exponent =
                min_binary_exponent + conversion_significand_bits - 1;
            return iszero(value) ||
                detail::fp::absd(value.hi) <
                    detail::fp::positive_power_of_two(guard_underflow_exponent);
        }
        static constexpr detail::fltx_char_result to_chars_general(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            if (detail::_dd::runtime_native_double_subnormal(x))
                return detail::_dd::emit_native_double_to_chars(first, last, x.hi, std::chars_format::general, precision, strip_trailing_zeros);

            return detail::emit_general_decimal_for_traits<dd_io_traits>(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr detail::fltx_char_result to_chars_fixed(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            if (detail::_dd::runtime_native_double_subnormal(x))
                return detail::_dd::emit_native_double_to_chars(first, last, x.hi, std::chars_format::fixed, precision);

            if (precision <= std::numeric_limits<value_type>::digits10)
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

            return detail::emit_fixed_decimal_for_traits<dd_io_traits>(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr detail::fltx_char_result to_chars_fixed_fast(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            return emit_fixed_dec_to_chars(first, last, x, precision, strip_trailing_zeros);
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
            if (detail::_dd::runtime_native_double_subnormal(x))
                return detail::_dd::emit_native_double_to_chars(first, last, x.hi, std::chars_format::scientific, precision);

            return detail::emit_scientific_frac_for_traits<dd_io_traits>(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr detail::fltx_char_result to_chars_scientific_sig(char* first, char* last, const value_type& x, int precision, bool strip_trailing_zeros)
        {
            if (detail::_dd::runtime_native_double_subnormal(x))
            {
                const int frac_digits = precision > 1 ? precision - 1 : 0;
                return detail::_dd::emit_native_double_to_chars(
                    first,
                    last,
                    x.hi,
                    std::chars_format::scientific,
                    frac_digits,
                    strip_trailing_zeros);
            }

            return emit_scientific_sig_to_chars_dd(first, last, x, precision, strip_trailing_zeros);
        }

        static constexpr value_type exact_uint64_to_value(std::uint64_t value, bool neg)
        {
            value_type out = detail::_dd::uint64_to_dd(value);
            return neg ? -out : out;
        }

        static constexpr bool compact_decimal_to_value(std::uint64_t coeff, int dec_exp, bool neg, value_type& out)
        {
            return detail::exact_decimal::compact_decimal_to_value<dd_io_traits>(coeff, dec_exp, neg, out);
        }

        static constexpr value_type exact_decimal_to_value(const detail::exact_decimal::biguint& coeff, int dec_exp, bool neg)
        {
            return detail::exact_decimal::exact_decimal_to_value<dd_io_traits>(coeff, dec_exp, neg);
        }

        static constexpr value_type nextafter(const value_type& from, const value_type& to) noexcept
        {
            return detail::_dd_impl::nextafter(from, to);
        }

        static constexpr value_type pack_from_significand(const detail::exact_decimal::biguint& q, int e2, bool neg) noexcept
        {
            return detail::_dd::exact_traits::pack_from_significand(q, e2, neg);
        }
    };

    [[nodiscard]] BL_MSVC_NOINLINE constexpr bool parse(const char* s, fdd_s& out, const char** endptr = nullptr) noexcept
    {
        return detail::parse_flt<dd_io_traits>(s, out, endptr);
    }

} // namespace detail::_dd

[[nodiscard]] constexpr bl::fdd_io_string to_static_string(
    const fdd_s& value,
    int precision = std::numeric_limits<fdd_s>::max_digits10,
    std::ios_base::fmtflags flags = std::ios_base::fmtflags{})
{
    return detail::to_static_string_impl<detail::_dd::dd_io_traits>(value, precision, flags);
}

[[nodiscard]] BL_NO_INLINE std::string to_string(
    const fdd_s& value,
    precision_info precision = std::numeric_limits<fdd_s>::max_digits10,
    std::ios_base::fmtflags flags = std::ios_base::fmtflags{});

namespace detail::_dd // primitives and kernels
{
    [[nodiscard]] consteval fdd_s parse_dd_literal(const char* text, const char* expected_end)
    {
        fdd_s out{};

        if (!bl::detail::parse_literal_float_text<dd_io_traits>(text, expected_end, out))
            throw "invalid _fdd literal";

        return out;
    }

} // namespace detail::_dd

namespace literals
{
    [[nodiscard]] consteval fdd operator""_dd(const char* text, std::size_t length)
    {
        return detail::_dd::parse_dd_literal(text, text + length);
    }

    template<char... Chars>
    [[nodiscard]] consteval fdd operator""_dd()
    {
        constexpr char text[] = { Chars..., '\0' };
        return detail::_dd::parse_dd_literal(text, text + sizeof...(Chars));
    }

} // namespace literals

} // namespace bl

#endif
