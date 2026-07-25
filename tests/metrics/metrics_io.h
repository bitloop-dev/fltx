#ifndef FLTX_TESTS_METRICS_IO_INCLUDED
#define FLTX_TESTS_METRICS_IO_INCLUDED

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <ios>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>

#include <fltx/io.h>

#include "metrics_case_output.h"
#include "metrics_config.h"
#include "metrics_benchmark.h"
#include "metrics_f128_primary.h"
#include "metrics_f256_primary.h"
#include "metrics_reference.h"

namespace bl::test::metrics::io_metrics
{
    inline constexpr domain_id io_domain{ "io", domain_role::primary };
    inline constexpr std::size_t samples_per_kind         = config::scale_accuracy_sample_count(20000);
    inline constexpr std::size_t benchmark_min_iterations = config::scale_mixed_iterations(400000);

    struct io_accuracy_measurement
    {
        accuracy_result accuracy;
        std::string failure;

        [[nodiscard]] bool succeeded() const noexcept
        {
            return failure.empty();
        }
    };

    [[nodiscard]] inline io_accuracy_measurement make_io_accuracy_failure(
        std::string_view reason,
        std::size_t sample_index)
    {
        io_accuracy_measurement result;
        result.failure.reserve(reason.size() + 32);
        result.failure += reason;
        result.failure += " at sample ";
        result.failure += std::to_string(sample_index);
        return result;
    }

    [[nodiscard]] inline std::string exception_failure_reason(
        std::string_view prefix,
        const std::exception& exception)
    {
        std::string reason;
        reason.reserve(prefix.size() + 2 + std::string_view(exception.what()).size());
        reason += prefix;
        reason += ": ";
        reason += exception.what();
        return reason;
    }

    [[nodiscard]] inline std::vector<std::string>& pending_io_backend_warnings()
    {
        static std::vector<std::string> warnings;
        return warnings;
    }

    inline void clear_io_backend_warnings()
    {
        pending_io_backend_warnings().clear();
    }

    inline void report_io_backend_unsupported(
        std::string_view operation,
        std::string_view backend,
        std::string_view phase,
        std::string_view reason)
    {
        std::string warning;
        warning.reserve(
            operation.size() + backend.size() + phase.size() + reason.size() + 40);
        warning += "[metrics warning] ";
        warning += operation;
        warning += ' ';
        warning += backend;
        warning += ' ';
        warning += phase;
        warning += " unsupported: ";
        warning += reason;
        pending_io_backend_warnings().push_back(std::move(warning));
    }

    inline void flush_io_backend_warnings(std::ostream& out = std::cerr)
    {
        auto& warnings = pending_io_backend_warnings();
        if (warnings.empty())
            return;

        out << "\n[metrics warnings]\n";
        for (const std::string& warning : warnings)
            out << warning << '\n';
        warnings.clear();
    }

    template<class Float>
    struct io_profile;

    template<>
    struct io_profile<bl::f128>
    {
        using references = reference_types<bl::f128>;
        using fltx_type = references::fltx_type;
        using perfect_ref = references::perfect_ref;
        using competitor_ref = references::competitor_ref;
        using extra_competitor_ref = references::extra_competitor_ref;

        static constexpr precision_type precision = precision_type::f128;
        static constexpr std::string_view title = "f128 IO metrics results";
        static constexpr double ideal_bits = f128_primary::domain_ideal_bits;

        [[nodiscard]] static perfect_ref to_perfect(const fltx_type& value) { return f128_primary::to_perfect(value); }
        [[nodiscard]] static perfect_ref to_perfect(const competitor_ref& value) { return f128_primary::to_perfect(value); }
        [[nodiscard]] static perfect_ref to_perfect(const extra_competitor_ref& value) { return f128_primary::to_perfect(value); }
        [[nodiscard]] static double matching_bits(const perfect_ref& actual, const perfect_ref& expected)
        {
            return reference_matching_bits_with_scale(
                actual,
                expected,
                reference_relative_error_scale(expected));
        }
        [[nodiscard]] static double finite_for_mean(double bits) noexcept { return f128_primary::finite_for_mean(bits); }
        [[nodiscard]] static double cap_accuracy_bits(double bits) noexcept { return f128_primary::cap_accuracy_bits(bits); }
        template<class Value>
        [[nodiscard]] static double domain_ideal_bits_for(const perfect_ref& expected)
        {
            (void)sizeof(Value);
            return target_domain_ideal_bits<fltx_type>(expected);
        }

        static void consume(const fltx_type& value) { f128_primary::consume_benchmark_value(value); }
        static void consume(const competitor_ref& value) { f128_primary::consume_benchmark_value(value); }
        static void consume(const extra_competitor_ref& value) { f128_primary::consume_benchmark_value(value); }
        static void consume_text(std::string_view text)
        {
            f128_primary::consume_benchmark_limb(static_cast<double>(text.size()));
            if (!text.empty())
                f128_primary::consume_benchmark_limb(static_cast<unsigned char>(text.front()));
        }
    };

    template<>
    struct io_profile<bl::f256>
    {
        using references = reference_types<bl::f256>;
        using fltx_type = references::fltx_type;
        using perfect_ref = references::perfect_ref;
        using competitor_ref = references::competitor_ref;
        using extra_competitor_ref = references::extra_competitor_ref;

        static constexpr precision_type precision = precision_type::f256;
        static constexpr std::string_view title = "f256 IO metrics results";
        static constexpr double ideal_bits = f256_primary::domain_ideal_bits;

        [[nodiscard]] static perfect_ref to_perfect(const fltx_type& value) { return f256_primary::to_perfect(value); }
        [[nodiscard]] static perfect_ref to_perfect(const competitor_ref& value) { return f256_primary::to_perfect(value); }
        [[nodiscard]] static perfect_ref to_perfect(const extra_competitor_ref& value) { return f256_primary::to_perfect(value); }
        [[nodiscard]] static double matching_bits(const perfect_ref& actual, const perfect_ref& expected)
        {
            return reference_matching_bits_with_scale(
                actual,
                expected,
                reference_relative_error_scale(expected));
        }
        [[nodiscard]] static double finite_for_mean(double bits) noexcept { return f256_primary::finite_for_mean(bits); }
        [[nodiscard]] static double cap_accuracy_bits(double bits) noexcept { return f256_primary::cap_accuracy_bits(bits); }
        template<class Value>
        [[nodiscard]] static double domain_ideal_bits_for(const perfect_ref& expected)
        {
            (void)sizeof(Value);
            return target_domain_ideal_bits<fltx_type>(expected);
        }

        static void consume(const fltx_type& value) { f256_primary::consume_benchmark_value(value); }
        static void consume(const competitor_ref& value) { f256_primary::consume_benchmark_value(value); }
        static void consume(const extra_competitor_ref& value) { f256_primary::consume_benchmark_value(value); }
        static void consume_text(std::string_view text)
        {
            f256_primary::consume_benchmark_limb(static_cast<double>(text.size()));
            if (!text.empty())
                f256_primary::consume_benchmark_limb(static_cast<unsigned char>(text.front()));
        }
    };

    template<class Profile>
    struct io_sample
    {
        std::string text;
        typename Profile::perfect_ref oracle;
        int precision = -1;
    };

    template<class Profile>
    struct io_sample_group
    {
        std::string_view label;
        std::vector<io_sample<Profile>> defaultfloat;
        std::vector<io_sample<Profile>> fixed;
        std::vector<io_sample<Profile>> scientific;
        std::vector<io_sample<Profile>> hexfloat;
        bool random_precision = false;
        bool average_candidate = false;
    };

    struct io_format_case
    {
        std::string_view label;
        std::ios_base::fmtflags flags;
        bool hexfloat = false;
    };

    template<class Profile>
    [[nodiscard]] io_sample<Profile> make_sample(std::string text, bool hexfloat, int precision = -1);

    [[nodiscard]] inline std::string_view intern_operation_name(std::string text)
    {
        static std::deque<std::string> storage;
        storage.push_back(std::move(text));
        return storage.back();
    }

    [[nodiscard]] inline std::array<io_format_case, 4> format_cases() noexcept
    {
        return {
            io_format_case{ "DEF", std::ios_base::fmtflags{}, false },
            io_format_case{ "FIX", std::ios_base::fixed, false },
            io_format_case{ "SCI", std::ios_base::scientific, false },
            io_format_case{ "HEX", std::ios_base::fixed | std::ios_base::scientific, true }
        };
    }

    template<class Profile>
    [[nodiscard]] constexpr std::array<int, 3> precision_cases() noexcept
    {
        constexpr int digits = std::numeric_limits<typename Profile::fltx_type>::digits10;
        if constexpr (digits > 40)
            return { digits, digits / 2, 15 };
        else
            return { digits, digits / 2, 7 };
    }

    class io_text_rng
    {
    public:
        explicit io_text_rng(std::uint64_t seed) noexcept
            : engine(seed)
        {
        }

        [[nodiscard]] int integer(int min, int max)
        {
            std::uniform_int_distribution<int> distribution(min, max);
            return distribution(engine);
        }

        [[nodiscard]] int normal_integer(int min, int max)
        {
            if (min >= max)
                return min;

            const double center = 0.5 * static_cast<double>(min + max);
            const double sigma = std::max(1.0, static_cast<double>(max - min) / 6.0);
            std::normal_distribution<double> distribution(center, sigma);

            for (int attempt = 0; attempt < 16; ++attempt)
            {
                const int value = static_cast<int>(std::lround(distribution(engine)));
                if (value >= min && value <= max)
                    return value;
            }

            return std::clamp(static_cast<int>(std::lround(distribution(engine))), min, max);
        }

        [[nodiscard]] bool negative()
        {
            return integer(0, 1) != 0;
        }

        [[nodiscard]] char digit(bool nonzero = false)
        {
            return static_cast<char>('0' + integer(nonzero ? 1 : 0, 9));
        }

        [[nodiscard]] char hex_digit(bool nonzero = false)
        {
            constexpr std::string_view digits = "0123456789abcdef";
            return digits[static_cast<std::size_t>(integer(nonzero ? 1 : 0, 15))];
        }

        [[nodiscard]] std::string digits(std::size_t count, bool first_nonzero = true)
        {
            std::string out;
            out.reserve(count);
            for (std::size_t index = 0; index < count; ++index)
                out.push_back(digit(first_nonzero && index == 0));
            return out;
        }

        [[nodiscard]] std::string hex_digits(std::size_t count)
        {
            std::string out;
            out.reserve(count);
            for (std::size_t index = 0; index < count; ++index)
                out.push_back(hex_digit());
            return out;
        }

    private:
        std::mt19937_64 engine;
    };

    template<class Profile>
    [[nodiscard]] typename Profile::perfect_ref parse_decimal_oracle(std::string_view text)
    {
        return typename Profile::perfect_ref{ std::string(text) };
    }

    [[nodiscard]] inline int hex_value(char ch)
    {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        if (ch >= 'a' && ch <= 'f')
            return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F')
            return ch - 'A' + 10;
        return -1;
    }

    template<class Profile>
    [[nodiscard]] typename Profile::perfect_ref parse_hex_oracle(std::string_view text)
    {
        using boost::multiprecision::cpp_int;
        using boost::multiprecision::ldexp;
        using perfect_ref = typename Profile::perfect_ref;

        std::size_t index = 0;
        bool negative = false;
        if (index < text.size() && (text[index] == '+' || text[index] == '-'))
            negative = text[index++] == '-';

        if (index + 2 <= text.size() && text[index] == '0' && (text[index + 1] == 'x' || text[index + 1] == 'X'))
            index += 2;

        cpp_int significand = 0;
        int fractional_hex_digits = 0;
        bool after_point = false;
        bool consumed_digit = false;

        while (index < text.size())
        {
            const char ch = text[index];
            if (ch == '.')
            {
                after_point = true;
                ++index;
                continue;
            }
            if (ch == 'p' || ch == 'P')
                break;

            const int digit = hex_value(ch);
            if (digit < 0)
                throw std::invalid_argument("invalid hexfloat oracle");

            significand <<= 4;
            significand += digit;
            fractional_hex_digits += after_point ? 1 : 0;
            consumed_digit = true;
            ++index;
        }

        if (!consumed_digit || index >= text.size() || (text[index] != 'p' && text[index] != 'P'))
            throw std::invalid_argument("invalid hexfloat oracle");

        ++index;
        bool negative_exponent = false;
        if (index < text.size() && (text[index] == '+' || text[index] == '-'))
            negative_exponent = text[index++] == '-';

        int exponent = 0;
        bool consumed_exponent = false;
        while (index < text.size())
        {
            const char ch = text[index++];
            if (ch < '0' || ch > '9')
                throw std::invalid_argument("invalid hexfloat exponent");
            exponent = exponent * 10 + (ch - '0');
            consumed_exponent = true;
        }

        if (!consumed_exponent)
            throw std::invalid_argument("invalid hexfloat exponent");

        if (negative_exponent)
            exponent = -exponent;

        perfect_ref value{ significand };
        value = ldexp(value, exponent - 4 * fractional_hex_digits);
        return negative ? -value : value;
    }

    template<class Profile>
    [[nodiscard]] typename Profile::perfect_ref parse_oracle(std::string_view text, bool hexfloat)
    {
        return hexfloat
            ? parse_hex_oracle<Profile>(text)
            : parse_decimal_oracle<Profile>(text);
    }

    template<class Profile>
    [[nodiscard]] std::optional<typename Profile::perfect_ref> try_parse_oracle(
        std::string_view text,
        bool hexfloat)
    {
        try
        {
            return parse_oracle<Profile>(text, hexfloat);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    [[nodiscard]] inline std::string with_sign(io_text_rng& rng, std::string text)
    {
        if (rng.negative())
            text.insert(text.begin(), '-');
        return text;
    }

    template<class Profile>
    [[nodiscard]] constexpr int max_decimal_exponent_limit() noexcept
    {
        return std::numeric_limits<typename Profile::fltx_type>::max_exponent10 - 2;
    }

    [[nodiscard]] constexpr int min_decimal_exponent_limit() noexcept
    {
        return -323;
    }

    struct exponent_range
    {
        int min;
        int max;
    };

    template<class Profile>
    [[nodiscard]] constexpr exponent_range magnitude_decimal_exponent_range(std::string_view label) noexcept
    {
        constexpr int max_exp = max_decimal_exponent_limit<Profile>();

        if (label == "subnormal")
            return { min_decimal_exponent_limit(), -308 };
        if (label == "near-zero")
            return { -40, -5 };
        if (label == "unit")
            return { -3, 3 };
        if (label == "medium")
            return { 4, 18 };
        if (label == "large-int")
            return { 19, 99 };
        if (label == "max-exp")
            return { std::max(100, max_exp - 24), max_exp };
        return { -3, 3 };
    }

    template<class Profile>
    [[nodiscard]] constexpr exponent_range magnitude_binary_exponent_range(std::string_view label) noexcept
    {
        (void)sizeof(Profile);

        if (label == "subnormal")
            return { -1074, -1022 };
        if (label == "near-zero")
            return { -132, -17 };
        if (label == "unit")
            return { -12, 12 };
        if (label == "medium")
            return { 13, 60 };
        if (label == "large-int")
            return { 61, 330 };
        if (label == "max-exp")
            return { 900, 1023 };
        return { -12, 12 };
    }

    template<class Profile>
    [[nodiscard]] int random_precision(io_text_rng& rng)
    {
        return rng.integer(0, std::numeric_limits<typename Profile::fltx_type>::digits10);
    }

    [[nodiscard]] inline int random_exponent(exponent_range range, io_text_rng& rng)
    {
        return rng.integer(range.min, range.max);
    }

    template<class Profile>
    [[nodiscard]] int random_digit_count(io_text_rng& rng, int extra_digits = 8)
    {
        return rng.integer(1, std::numeric_limits<typename Profile::fltx_type>::digits10 + extra_digits);
    }

    [[nodiscard]] inline std::string make_decimal_scientific_text(
        io_text_rng& rng,
        int exponent,
        int digit_count)
    {
        std::string text;
        if (rng.negative())
            text.push_back('-');
        text.push_back(rng.digit(true));
        if (digit_count > 1)
        {
            text.push_back('.');
            text += rng.digits(static_cast<std::size_t>(digit_count - 1), false);
        }
        text.push_back('e');
        text.push_back(exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(exponent));
        return text;
    }

    [[nodiscard]] inline std::string make_decimal_fixed_text(
        io_text_rng& rng,
        int exponent,
        int digit_count)
    {
        std::string digits = rng.digits(static_cast<std::size_t>(digit_count));
        std::string text;

        if (exponent >= 0)
        {
            const int integer_digits = exponent + 1;
            if (integer_digits <= digit_count)
            {
                text = digits.substr(0, static_cast<std::size_t>(integer_digits));
                if (integer_digits < digit_count)
                {
                    text.push_back('.');
                    text += digits.substr(static_cast<std::size_t>(integer_digits));
                }
            }
            else
            {
                text = std::move(digits);
                text.append(static_cast<std::size_t>(integer_digits - digit_count), '0');
            }
        }
        else
        {
            text = "0.";
            text.append(static_cast<std::size_t>(-exponent - 1), '0');
            text += digits;
        }

        return with_sign(rng, std::move(text));
    }

    template<class Profile>
    [[nodiscard]] std::string make_magnitude_fixed_text(std::string_view label, io_text_rng& rng)
    {
        const int exponent = random_exponent(magnitude_decimal_exponent_range<Profile>(label), rng);
        const int digit_count = random_digit_count<Profile>(rng);
        return make_decimal_fixed_text(rng, exponent, digit_count);
    }

    template<class Profile>
    [[nodiscard]] std::string make_magnitude_scientific_text(std::string_view label, io_text_rng& rng)
    {
        const int exponent = random_exponent(magnitude_decimal_exponent_range<Profile>(label), rng);
        const int digit_count = random_digit_count<Profile>(rng);
        return make_decimal_scientific_text(rng, exponent, digit_count);
    }

    template<class Profile>
    [[nodiscard]] std::string make_magnitude_default_text(std::string_view label, io_text_rng& rng)
    {
        constexpr int digits = std::numeric_limits<typename Profile::fltx_type>::digits10;
        const int exponent = random_exponent(magnitude_decimal_exponent_range<Profile>(label), rng);
        const int digit_count = random_digit_count<Profile>(rng);
        if (exponent < -4 || exponent >= digits)
            return make_decimal_scientific_text(rng, exponent, digit_count);
        return make_decimal_fixed_text(rng, exponent, digit_count);
    }

    template<class Profile>
    [[nodiscard]] std::string make_magnitude_hex_text(std::string_view label, io_text_rng& rng)
    {
        const int binary_exponent = random_exponent(magnitude_binary_exponent_range<Profile>(label), rng);

        std::string text;
        if (rng.negative())
            text.push_back('-');
        text += "0x";
        text.push_back(rng.hex_digit(true));
        text.push_back('.');
        const int hex_digits = rng.integer(
            1,
            std::max(8, std::numeric_limits<typename Profile::fltx_type>::digits10 / 2));
        text += rng.hex_digits(static_cast<std::size_t>(hex_digits));
        text.push_back('p');
        text.push_back(binary_exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(binary_exponent));
        return text;
    }

    template<class Profile>
    [[nodiscard]] int brute_log_decimal_exponent(io_text_rng& rng)
    {
        return rng.integer(min_decimal_exponent_limit(), max_decimal_exponent_limit<Profile>());
    }

    template<class Profile>
    [[nodiscard]] std::string make_brute_log_fixed_text(io_text_rng& rng)
    {
        return make_decimal_fixed_text(
            rng,
            brute_log_decimal_exponent<Profile>(rng),
            random_digit_count<Profile>(rng));
    }

    template<class Profile>
    [[nodiscard]] std::string make_brute_log_default_text(io_text_rng& rng, int precision)
    {
        const int exponent = brute_log_decimal_exponent<Profile>(rng);
        const int digit_count = std::max(1, precision);
        if (exponent < -4 || exponent >= digit_count)
            return make_decimal_scientific_text(rng, exponent, digit_count);
        return make_decimal_fixed_text(rng, exponent, digit_count);
    }

    template<class Profile>
    [[nodiscard]] std::string make_brute_log_scientific_text(io_text_rng& rng)
    {
        return make_decimal_scientific_text(
            rng,
            brute_log_decimal_exponent<Profile>(rng),
            random_digit_count<Profile>(rng));
    }

    template<class Profile>
    [[nodiscard]] std::string make_brute_log_hex_text(io_text_rng& rng)
    {
        const int binary_exponent = rng.integer(-1074, 1023);

        std::string text;
        if (rng.negative())
            text.push_back('-');
        text += "0x";
        text.push_back(rng.hex_digit(true));
        text.push_back('.');
        const int hex_digits = rng.normal_integer(
            1,
            std::max(8, std::numeric_limits<typename Profile::fltx_type>::digits10 / 2));
        text += rng.hex_digits(static_cast<std::size_t>(hex_digits));
        text.push_back('p');
        text.push_back(binary_exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(binary_exponent));
        return text;
    }

    template<class Profile>
    [[nodiscard]] std::string make_default_boundary_text(io_text_rng& rng, int precision)
    {
        const int digit_count = std::max(1, precision);
        const std::array<int, 4> exponents{ -5, -4, digit_count - 1, digit_count };
        const int exponent = exponents[static_cast<std::size_t>(rng.integer(0, static_cast<int>(exponents.size() - 1)))];
        if (exponent < -4 || exponent >= digit_count)
            return make_decimal_scientific_text(rng, exponent, digit_count);
        return make_decimal_fixed_text(rng, exponent, digit_count);
    }

    [[nodiscard]] inline std::string round_boundary_tail(io_text_rng& rng)
    {
        const char pivot = static_cast<char>('4' + rng.integer(0, 2));
        std::string tail;
        tail.push_back(pivot);
        tail.append(static_cast<std::size_t>(rng.integer(0, 3)), '0');
        if (pivot == '5' && rng.integer(0, 1) != 0)
            tail.push_back(rng.digit(true));
        return tail;
    }

    [[nodiscard]] inline std::string make_fixed_round_boundary_text(io_text_rng& rng, int precision)
    {
        std::string text = with_sign(rng, rng.digits(static_cast<std::size_t>(rng.integer(1, 5))));
        text.push_back('.');
        if (precision > 0)
            text += rng.digits(static_cast<std::size_t>(precision), false);
        text += round_boundary_tail(rng);
        return text;
    }

    [[nodiscard]] inline std::string make_scientific_round_boundary_text(io_text_rng& rng, int precision)
    {
        const int kept_digits = std::max(1, precision);
        std::string text;
        if (rng.negative())
            text.push_back('-');
        text.push_back(rng.digit(true));
        if (kept_digits > 1)
        {
            text.push_back('.');
            text += rng.digits(static_cast<std::size_t>(kept_digits - 1), false);
        }
        text += round_boundary_tail(rng);
        const int exponent = rng.integer(-8, 8);
        text.push_back('e');
        text.push_back(exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(exponent));
        return text;
    }

    [[nodiscard]] inline std::string make_default_round_boundary_text(io_text_rng& rng, int precision)
    {
        return rng.negative()
            ? make_fixed_round_boundary_text(rng, precision)
            : make_scientific_round_boundary_text(rng, precision);
    }

    template<class Profile>
    [[nodiscard]] std::string make_fixed_fraction_text(io_text_rng& rng)
    {
        const int exponent = rng.integer(-40, -1);
        const int digit_count = random_digit_count<Profile>(rng);
        return make_decimal_fixed_text(rng, exponent, digit_count);
    }

    template<class Profile>
    [[nodiscard]] std::string make_fixed_integer_text(io_text_rng& rng)
    {
        const int exponent = rng.integer(20, std::min(120, max_decimal_exponent_limit<Profile>()));
        const int digit_count = random_digit_count<Profile>(rng);
        return make_decimal_fixed_text(rng, exponent, digit_count);
    }

    [[nodiscard]] inline std::string make_fixed_carry_boundary_text(io_text_rng& rng, int precision)
    {
        std::string text = with_sign(rng, std::string(static_cast<std::size_t>(rng.integer(1, 5)), '9'));
        text.push_back('.');
        if (precision > 0)
            text.append(static_cast<std::size_t>(precision), '9');
        text.push_back('5');
        text.append(static_cast<std::size_t>(rng.integer(0, 3)), '0');
        return text;
    }

    [[nodiscard]] inline std::string make_scientific_carry_boundary_text(io_text_rng& rng, int precision)
    {
        std::string text;
        if (rng.negative())
            text.push_back('-');
        text.push_back('9');
        if (precision > 0)
        {
            text.push_back('.');
            text.append(static_cast<std::size_t>(precision), '9');
        }
        text.push_back('5');
        const int exponent = rng.integer(-8, 8);
        text.push_back('e');
        text.push_back(exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(exponent));
        return text;
    }

    [[nodiscard]] inline std::string make_default_carry_boundary_text(io_text_rng& rng, int precision)
    {
        const int significant_digits = std::max(1, precision);
        std::string text;
        if (rng.negative())
            text.push_back('-');
        text.push_back('9');
        if (significant_digits > 1)
        {
            text.push_back('.');
            text.append(static_cast<std::size_t>(significant_digits - 1), '9');
        }
        text.push_back('5');
        const int exponent = rng.integer(-8, 8);
        text.push_back('e');
        text.push_back(exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(exponent));
        return text;
    }

    [[nodiscard]] inline std::string make_hex_carry_boundary_text(io_text_rng& rng, int precision)
    {
        std::string text;
        if (rng.negative())
            text.push_back('-');
        text += "0x1.";
        if (precision > 0)
            text.append(static_cast<std::size_t>(precision), 'f');
        text.push_back('8');
        const int exponent = rng.integer(-32, 32);
        text.push_back('p');
        text.push_back(exponent < 0 ? '-' : '+');
        text += std::to_string(std::abs(exponent));
        return text;
    }

    template<class Profile>
    [[nodiscard]] io_sample<Profile> make_sample(std::string text, bool hexfloat, int precision)
    {
        typename Profile::perfect_ref oracle = parse_oracle<Profile>(text, hexfloat);
        return { std::move(text), std::move(oracle), precision };
    }

    template<class Profile>
    void fill_magnitude_group(io_sample_group<Profile>& group, io_text_rng& rng)
    {
        group.defaultfloat.reserve(samples_per_kind);
        group.fixed.reserve(samples_per_kind);
        group.scientific.reserve(samples_per_kind);
        group.hexfloat.reserve(samples_per_kind);

        for (std::size_t index = 0; index < samples_per_kind; ++index)
        {
            group.defaultfloat.push_back(make_sample<Profile>(make_magnitude_default_text<Profile>(group.label, rng), false));
            group.fixed.push_back(make_sample<Profile>(make_magnitude_fixed_text<Profile>(group.label, rng), false));
            group.scientific.push_back(make_sample<Profile>(make_magnitude_scientific_text<Profile>(group.label, rng), false));
            group.hexfloat.push_back(make_sample<Profile>(make_magnitude_hex_text<Profile>(group.label, rng), true));
        }
    }

    template<class Profile>
    void fill_policy_group(io_sample_group<Profile>& group, io_text_rng& rng)
    {
        group.random_precision = true;
        group.defaultfloat.reserve(samples_per_kind);
        group.fixed.reserve(samples_per_kind);
        group.scientific.reserve(samples_per_kind);
        group.hexfloat.reserve(samples_per_kind);

        for (std::size_t index = 0; index < samples_per_kind; ++index)
        {
            const int default_precision = random_precision<Profile>(rng);
            const int fixed_precision = random_precision<Profile>(rng);
            const int scientific_precision = random_precision<Profile>(rng);
            const int hex_precision = random_precision<Profile>(rng);

            if (group.label == "def-bdr")
            {
                group.defaultfloat.push_back(make_sample<Profile>(
                    make_default_boundary_text<Profile>(rng, default_precision),
                    false,
                    default_precision));
            }
            else if (group.label == "round-bdr")
            {
                group.defaultfloat.push_back(make_sample<Profile>(
                    make_default_round_boundary_text(rng, default_precision),
                    false,
                    default_precision));
                group.fixed.push_back(make_sample<Profile>(
                    make_fixed_round_boundary_text(rng, fixed_precision),
                    false,
                    fixed_precision));
                group.scientific.push_back(make_sample<Profile>(
                    make_scientific_round_boundary_text(rng, scientific_precision),
                    false,
                    scientific_precision));
            }
            else if (group.label == "fixed-frac")
            {
                group.fixed.push_back(make_sample<Profile>(
                    make_fixed_fraction_text<Profile>(rng),
                    false,
                    fixed_precision));
            }
            else if (group.label == "fixed-int")
            {
                group.fixed.push_back(make_sample<Profile>(
                    make_fixed_integer_text<Profile>(rng),
                    false,
                    fixed_precision));
            }
            else
            {
                group.defaultfloat.push_back(make_sample<Profile>(
                    make_default_carry_boundary_text(rng, default_precision),
                    false,
                    default_precision));
                group.fixed.push_back(make_sample<Profile>(
                    make_fixed_carry_boundary_text(rng, fixed_precision),
                    false,
                    fixed_precision));
                group.scientific.push_back(make_sample<Profile>(
                    make_scientific_carry_boundary_text(rng, scientific_precision),
                    false,
                    scientific_precision));
                group.hexfloat.push_back(make_sample<Profile>(
                    make_hex_carry_boundary_text(rng, hex_precision),
                    true,
                    hex_precision));
            }
        }
    }

    template<class Profile>
    void fill_brute_group(io_sample_group<Profile>& group, io_text_rng& rng)
    {
        group.random_precision = true;
        group.average_candidate = true;
        group.defaultfloat.reserve(samples_per_kind);
        group.fixed.reserve(samples_per_kind);
        group.scientific.reserve(samples_per_kind);
        group.hexfloat.reserve(samples_per_kind);

        for (std::size_t index = 0; index < samples_per_kind; ++index)
        {
            const int default_precision = random_precision<Profile>(rng);
            const int fixed_precision = random_precision<Profile>(rng);
            const int scientific_precision = random_precision<Profile>(rng);
            const int hex_precision = random_precision<Profile>(rng);

            if (group.label == "brute-log")
            {
                group.defaultfloat.push_back(make_sample<Profile>(
                    make_brute_log_default_text<Profile>(rng, default_precision),
                    false,
                    default_precision));
                group.fixed.push_back(make_sample<Profile>(
                    make_brute_log_fixed_text<Profile>(rng),
                    false,
                    fixed_precision));
                group.scientific.push_back(make_sample<Profile>(
                    make_brute_log_scientific_text<Profile>(rng),
                    false,
                    scientific_precision));
                group.hexfloat.push_back(make_sample<Profile>(
                    make_brute_log_hex_text<Profile>(rng),
                    true,
                    hex_precision));
            }
            else
            {
                switch (rng.integer(0, 2))
                {
                case 0:
                    group.defaultfloat.push_back(make_sample<Profile>(
                        make_default_boundary_text<Profile>(rng, default_precision),
                        false,
                        default_precision));
                    break;
                case 1:
                    group.defaultfloat.push_back(make_sample<Profile>(
                        make_default_round_boundary_text(rng, default_precision),
                        false,
                        default_precision));
                    break;
                default:
                    group.defaultfloat.push_back(make_sample<Profile>(
                        make_default_carry_boundary_text(rng, default_precision),
                        false,
                        default_precision));
                    break;
                }

                switch (rng.integer(0, 3))
                {
                case 0:
                    group.fixed.push_back(make_sample<Profile>(
                        make_fixed_round_boundary_text(rng, fixed_precision),
                        false,
                        fixed_precision));
                    break;
                case 1:
                    group.fixed.push_back(make_sample<Profile>(
                        make_fixed_fraction_text<Profile>(rng),
                        false,
                        fixed_precision));
                    break;
                case 2:
                    group.fixed.push_back(make_sample<Profile>(
                        make_fixed_integer_text<Profile>(rng),
                        false,
                        fixed_precision));
                    break;
                default:
                    group.fixed.push_back(make_sample<Profile>(
                        make_fixed_carry_boundary_text(rng, fixed_precision),
                        false,
                        fixed_precision));
                    break;
                }

                group.scientific.push_back(make_sample<Profile>(
                    rng.negative()
                        ? make_scientific_round_boundary_text(rng, scientific_precision)
                        : make_scientific_carry_boundary_text(rng, scientific_precision),
                    false,
                    scientific_precision));
                group.hexfloat.push_back(make_sample<Profile>(
                    make_hex_carry_boundary_text(rng, hex_precision),
                    true,
                    hex_precision));
            }
        }
    }

    template<class Profile>
    [[nodiscard]] std::vector<io_sample_group<Profile>> make_sample_groups()
    {
        static constexpr std::array<std::string_view, 6> magnitude_labels{
            "subnormal", "near-zero", "unit", "medium", "large-int", "max-exp"
        };
        static constexpr std::array<std::string_view, 5> policy_labels{
            "def-bdr", "round-bdr", "fixed-frac", "fixed-int", "carry-bdr"
        };
        static constexpr std::array<std::string_view, 2> brute_labels{
            "brute-log", "brute-bdr"
        };

        io_text_rng rng{ Profile::precision == precision_type::f128 ? 0x12810f00dull : 0x25610f00dull };
        io_text_rng focus_rng{ Profile::precision == precision_type::f128 ? 0x128f0c05ull : 0x256f0c05ull };
        io_text_rng brute_rng{ Profile::precision == precision_type::f128 ? 0x128b0010ull : 0x256b0010ull };
        std::vector<io_sample_group<Profile>> groups;
        groups.reserve(magnitude_labels.size() + policy_labels.size() + brute_labels.size());

        for (std::string_view label : magnitude_labels)
        {
            io_sample_group<Profile> group{ label };
            fill_magnitude_group(group, rng);
            groups.push_back(std::move(group));
        }

        for (std::string_view label : policy_labels)
        {
            io_sample_group<Profile> group{ label };
            fill_policy_group(group, focus_rng);
            groups.push_back(std::move(group));
        }

        for (std::string_view label : brute_labels)
        {
            io_sample_group<Profile> group{ label };
            fill_brute_group(group, brute_rng);
            groups.push_back(std::move(group));
        }

        return groups;
    }

    [[nodiscard]] inline std::size_t benchmark_repetitions(std::size_t sample_count) noexcept
    {
        if (sample_count == 0)
            return 0;
        return std::max<std::size_t>(3, (benchmark_min_iterations + sample_count - 1) / sample_count);
    }

    template<class Profile, class Values, class EvalFn, class ConsumeFn>
    [[nodiscard]] benchmark_result benchmark_optional_values(const Values& values, EvalFn eval, ConsumeFn consume)
    {
        const std::size_t repetitions = benchmark_repetitions(values.size());
        return benchmark_trials(values.size(), repetitions, [&]
        {
            for (std::size_t repeat = 0; repeat < repetitions; ++repeat)
            {
                for (const auto& value : values)
                {
                    try
                    {
                        const auto parsed = eval(value);
                        if (parsed)
                            consume(*parsed);
                    }
                    catch (...)
                    {
                    }
                }
            }
        });
    }

    template<class Profile, class Values, class EvalFn, class ConsumeFn>
    [[nodiscard]] benchmark_result benchmark_indexed_values(const Values& values, EvalFn eval, ConsumeFn consume)
    {
        const std::size_t repetitions = benchmark_repetitions(values.size());
        return benchmark_trials(values.size(), repetitions, [&]
        {
            for (std::size_t repeat = 0; repeat < repetitions; ++repeat)
            {
                for (std::size_t index = 0; index < values.size(); ++index)
                {
                    try
                    {
                        consume(eval(index, values[index]));
                    }
                    catch (...)
                    {
                    }
                }
            }
        });
    }

    [[nodiscard]] inline bool is_decimal_digit(char ch) noexcept
    {
        return ch >= '0' && ch <= '9';
    }

    [[nodiscard]] inline std::size_t exponent_marker_pos(std::string_view text) noexcept
    {
        const std::size_t e = text.find_first_of("eE");
        return e == std::string_view::npos ? text.size() : e;
    }

    [[nodiscard]] inline int fractional_digit_count(std::string_view text) noexcept
    {
        const std::size_t exponent = exponent_marker_pos(text);
        const std::size_t point = text.substr(0, exponent).find('.');
        if (point == std::string_view::npos)
            return 0;
        return static_cast<int>(exponent - point - 1);
    }

    [[nodiscard]] inline int significant_digit_count(std::string_view text) noexcept
    {
        const std::size_t exponent = exponent_marker_pos(text);
        int count = 0;
        bool seen_nonzero = false;
        bool saw_digit = false;

        for (std::size_t index = 0; index < exponent; ++index)
        {
            const char ch = text[index];
            if (!is_decimal_digit(ch))
                continue;

            saw_digit = true;
            if (ch != '0')
                seen_nonzero = true;
            if (seen_nonzero)
                ++count;
        }

        return count == 0 && saw_digit ? 1 : count;
    }

    [[nodiscard]] inline bool formatted_decimal_respects_precision(
        std::string_view text,
        int precision,
        const io_format_case& format) noexcept
    {
        if (format.hexfloat)
            return true;

        const int requested = precision < 0 ? 6 : precision;
        const std::ios_base::fmtflags floatfield = format.flags & std::ios_base::floatfield;
        if (floatfield == std::ios_base::fixed)
            return text.find_first_of("eE") == std::string_view::npos &&
                   fractional_digit_count(text) == requested;

        if (floatfield == std::ios_base::scientific)
            return text.find_first_of("eE") != std::string_view::npos &&
                   fractional_digit_count(text) == requested;

        const int max_significant_digits = requested == 0 ? 1 : requested;
        return significant_digit_count(text) <= max_significant_digits;
    }

    template<class Profile>
    [[nodiscard]] std::string format_decimal_oracle(
        const typename Profile::perfect_ref& value,
        int precision,
        std::ios_base::fmtflags flags)
    {
        std::ostringstream stream;
        stream.precision(precision);
        stream.setf(flags & std::ios_base::floatfield, std::ios_base::floatfield);
        stream.setf(flags & (std::ios_base::showpoint | std::ios_base::showpos | std::ios_base::uppercase));
        stream << value;
        return stream.str();
    }

    template<class Profile>
    [[nodiscard]] std::string make_formatter_seed_text(const io_sample<Profile>& sample)
    {
        return format_decimal_oracle<Profile>(
            sample.oracle,
            std::numeric_limits<typename Profile::fltx_type>::max_digits10,
            std::ios_base::scientific);
    }

    template<class Profile, class Value>
    [[nodiscard]] std::optional<typename Profile::perfect_ref> expected_to_string_value(
        const Value& value,
        int precision,
        const io_format_case& format)
    {
        const typename Profile::perfect_ref exact_value = Profile::to_perfect(value);
        if (format.hexfloat)
            return exact_value;

        const std::string rounded_text = format_decimal_oracle<Profile>(exact_value, precision, format.flags);
        if (!formatted_decimal_respects_precision(rounded_text, precision, format))
            return std::nullopt;
        return try_parse_oracle<Profile>(rounded_text, false);
    }

    template<class Profile, class Samples, class Value, class FormatFn>
    [[nodiscard]] io_accuracy_measurement measure_to_string_accuracy(
        const Samples& samples,
        const std::vector<Value>& values,
        int precision,
        const io_format_case& format,
        FormatFn format_value)
    {
        double total_bits = 0.0;
        double worst_bits = std::numeric_limits<double>::infinity();
        std::vector<double> domain_scores;
        domain_scores.reserve(samples.size());

        for (std::size_t index = 0; index < samples.size(); ++index)
        {
            const int sample_precision = samples[index].precision >= 0 ? samples[index].precision : precision;
            const std::optional<typename Profile::perfect_ref> expected =
                expected_to_string_value<Profile>(values[index], sample_precision, format);
            if (!expected)
                return make_io_accuracy_failure("oracle formatter ignored requested precision", index);

            std::string text;
            try
            {
                text = format_value(values[index], sample_precision, format.flags);
            }
            catch (const std::exception& exception)
            {
                return make_io_accuracy_failure(exception_failure_reason("formatter threw", exception), index);
            }
            catch (...)
            {
                return make_io_accuracy_failure("formatter threw", index);
            }

            if (!formatted_decimal_respects_precision(text, sample_precision, format))
                return make_io_accuracy_failure("formatter ignored requested precision", index);

            const std::optional<typename Profile::perfect_ref> actual =
                try_parse_oracle<Profile>(text, format.hexfloat);
            if (!actual)
                return make_io_accuracy_failure("formatter produced invalid oracle text", index);

            double bits = Profile::matching_bits(*actual, *expected);
            if (std::isnan(bits))
                bits = 0.0;

            const double sample_ideal_bits = Profile::template domain_ideal_bits_for<Value>(*expected);
            worst_bits = std::min(worst_bits, Profile::cap_accuracy_bits(bits));
            total_bits += Profile::finite_for_mean(bits);
            domain_scores.push_back(domain_sample_score(bits, sample_ideal_bits));
        }

        return {
            accuracy_result{
                worst_bits,
                total_bits / static_cast<double>(samples.size()),
                samples.size(),
                domain_score(std::move(domain_scores))
            },
            {}
        };
    }

    template<class Profile, class Value, class Samples, class ParseFn>
    [[nodiscard]] io_accuracy_measurement measure_parse_accuracy(
        const Samples& samples,
        ParseFn parse_value)
    {
        double total_bits = 0.0;
        double worst_bits = std::numeric_limits<double>::infinity();
        std::vector<double> domain_scores;
        domain_scores.reserve(samples.size());

        for (std::size_t index = 0; index < samples.size(); ++index)
        {
            std::optional<Value> value;
            try
            {
                value = parse_value(samples[index].text);
            }
            catch (const std::exception& exception)
            {
                return make_io_accuracy_failure(exception_failure_reason("parse threw", exception), index);
            }
            catch (...)
            {
                return make_io_accuracy_failure("parse threw", index);
            }

            if (!value)
                return make_io_accuracy_failure("parse failed", index);

            const typename Profile::perfect_ref actual = Profile::to_perfect(*value);
            const typename Profile::perfect_ref expected =
                target_reference_value<typename Profile::fltx_type>(samples[index].oracle);
            double bits = Profile::matching_bits(actual, expected);
            if (std::isnan(bits))
                bits = 0.0;

            const double sample_ideal_bits = Profile::template domain_ideal_bits_for<Value>(expected);
            worst_bits = std::min(worst_bits, Profile::cap_accuracy_bits(bits));
            total_bits += Profile::finite_for_mean(bits);
            domain_scores.push_back(domain_sample_score(bits, sample_ideal_bits));
        }

        return {
            accuracy_result{
                worst_bits,
                total_bits / static_cast<double>(samples.size()),
                samples.size(),
                domain_score(std::move(domain_scores))
            },
            {}
        };
    }

    template<class T>
    [[nodiscard]] std::optional<T> try_parse_stream_value(std::string_view text)
    {
        try
        {
            std::istringstream stream{ std::string(text) };
            T value{};
            stream >> value;
            if (!stream || stream.peek() != std::char_traits<char>::eof())
                return std::nullopt;
            return value;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    template<class T>
    [[nodiscard]] std::optional<T> try_parse_qdpp_value(std::string_view text)
    {
        try
        {
            std::string copy(text);
            T value{};
            if (T::read(copy.c_str(), value) < 0)
                return std::nullopt;
            return value;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    template<class T>
    [[nodiscard]] std::optional<T> try_parse_fltx_value(std::string_view text)
    {
        try
        {
            const auto parsed = bl::try_parse<T>(text);
            if (!parsed)
                return std::nullopt;
            return parsed.value;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    template<class T>
    [[nodiscard]] std::string format_stream_value(const T& value, int precision, std::ios_base::fmtflags flags)
    {
        std::ostringstream stream;
        stream.precision(precision);
        stream.setf(flags & std::ios_base::floatfield, std::ios_base::floatfield);
        stream << value;
        return stream.str();
    }

    template<class T>
    [[nodiscard]] std::string format_qdpp_value(const T& value, int precision, std::ios_base::fmtflags flags)
    {
        return value.to_string(
            precision,
            0,
            flags,
            (flags & std::ios_base::showpos) != std::ios_base::fmtflags{},
            (flags & std::ios_base::uppercase) != std::ios_base::fmtflags{});
    }

    template<class T>
    [[nodiscard]] bool io_value_is_inf(const T& value)
    {
        using value_type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<value_type, bl::f128> || std::is_same_v<value_type, bl::f256>)
        {
            return bl::isinf(value);
        }
        else if constexpr (std::is_same_v<value_type, dd_real>)
        {
            return std::isinf(value.x[0]);
        }
        else if constexpr (std::is_same_v<value_type, qd_real>)
        {
            return std::isinf(value[0]);
        }
        else
        {
            using std::isinf;
            return isinf(value);
        }
    }

    template<class T>
    [[nodiscard]] bool io_value_is_nan(const T& value)
    {
        using value_type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<value_type, bl::f128> || std::is_same_v<value_type, bl::f256>)
        {
            return bl::isnan(value);
        }
        else if constexpr (std::is_same_v<value_type, dd_real>)
        {
            return std::isnan(value.x[0]) || (std::isfinite(value.x[0]) && std::isnan(value.x[1]));
        }
        else if constexpr (std::is_same_v<value_type, qd_real>)
        {
            return std::isnan(value[0]) || (std::isfinite(value[0]) &&
                (std::isnan(value[1]) || std::isnan(value[2]) || std::isnan(value[3])));
        }
        else
        {
            using std::isnan;
            return isnan(value);
        }
    }

    template<class T>
    [[nodiscard]] bool io_value_signbit(const T& value)
    {
        using value_type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<value_type, bl::f128> || std::is_same_v<value_type, bl::f256>)
        {
            return bl::signbit(value);
        }
        else if constexpr (std::is_same_v<value_type, dd_real>)
        {
            return std::signbit(value.x[0]);
        }
        else if constexpr (std::is_same_v<value_type, qd_real>)
        {
            return std::signbit(value[0]);
        }
        else
        {
            using boost::multiprecision::signbit;
            return signbit(value);
        }
    }

    template<class T>
    [[nodiscard]] T io_positive_infinity()
    {
        if constexpr (std::is_same_v<T, dd_real>)
            return dd_real::_inf;
        else if constexpr (std::is_same_v<T, qd_real>)
            return qd_real::_inf;
        else
            return std::numeric_limits<T>::infinity();
    }

    template<class T>
    [[nodiscard]] T io_quiet_nan()
    {
        if constexpr (std::is_same_v<T, dd_real>)
            return dd_real::_nan;
        else if constexpr (std::is_same_v<T, qd_real>)
            return qd_real::_nan;
        else
            return std::numeric_limits<T>::quiet_NaN();
    }

    [[nodiscard]] inline bool contains_ascii_token(std::string_view text, std::string_view token) noexcept
    {
        if (token.empty() || text.size() < token.size())
            return false;

        const auto lower = [](char ch) noexcept -> char
        {
            return ch >= 'A' && ch <= 'Z' ? static_cast<char>(ch - 'A' + 'a') : ch;
        };

        for (std::size_t offset = 0; offset + token.size() <= text.size(); ++offset)
        {
            bool matched = true;
            for (std::size_t index = 0; index < token.size(); ++index)
            {
                if (lower(text[offset + index]) != token[index])
                {
                    matched = false;
                    break;
                }
            }
            if (matched)
                return true;
        }
        return false;
    }

    [[nodiscard]] inline bool formatted_inf_token(std::string_view text) noexcept
    {
        return contains_ascii_token(text, "inf");
    }

    [[nodiscard]] inline bool formatted_nan_token(std::string_view text) noexcept
    {
        return contains_ascii_token(text, "nan") || contains_ascii_token(text, "qnan") ||
               contains_ascii_token(text, "ind");
    }

    template<class ParseFn>
    [[nodiscard]] bool optional_parse_inf_is_supported(ParseFn parse_value)
    {
        constexpr std::array<std::string_view, 6> positive_tokens{
            "inf", "+inf", "infinity", "+infinity", "INF", "+INF"
        };
        constexpr std::array<std::string_view, 3> negative_tokens{
            "-inf", "-infinity", "-INF"
        };

        bool positive_ok = false;
        bool negative_ok = false;
        for (std::string_view token : positive_tokens)
        {
            try
            {
                const auto value = parse_value(token);
                positive_ok = positive_ok || (value && io_value_is_inf(*value) && !io_value_signbit(*value));
            }
            catch (...)
            {
            }
        }
        for (std::string_view token : negative_tokens)
        {
            try
            {
                const auto value = parse_value(token);
                negative_ok = negative_ok || (value && io_value_is_inf(*value) && io_value_signbit(*value));
            }
            catch (...)
            {
            }
        }
        return positive_ok && negative_ok;
    }

    template<class ParseFn>
    [[nodiscard]] bool optional_parse_nan_is_supported(ParseFn parse_value)
    {
        constexpr std::array<std::string_view, 5> tokens{ "nan", "+nan", "-nan", "NaN", "NAN" };
        for (std::string_view token : tokens)
        {
            try
            {
                const auto value = parse_value(token);
                if (value && io_value_is_nan(*value))
                    return true;
            }
            catch (...)
            {
            }
        }
        return false;
    }

    template<class ParseFn>
    [[nodiscard]] special_support measure_optional_parse_special_support(ParseFn parse_value)
    {
        return make_special_support(
            optional_parse_inf_is_supported(parse_value),
            optional_parse_nan_is_supported(parse_value));
    }

    template<class T, class FormatFn>
    [[nodiscard]] special_support measure_to_string_special_support(
        FormatFn format_value,
        int precision,
        std::ios_base::fmtflags flags)
    {
        bool inf_ok = false;
        bool nan_ok = false;
        try
        {
            const std::string positive = format_value(io_positive_infinity<T>(), precision, flags);
            const std::string negative = format_value(-io_positive_infinity<T>(), precision, flags);
            inf_ok = formatted_inf_token(positive) && formatted_inf_token(negative);
        }
        catch (...)
        {
        }

        try
        {
            nan_ok = formatted_nan_token(format_value(io_quiet_nan<T>(), precision, flags));
        }
        catch (...)
        {
        }

        return make_special_support(inf_ok, nan_ok);
    }

    template<class Profile, class Value, class ParseFn>
    [[nodiscard]] std::optional<std::vector<Value>> try_make_values(
        const std::vector<io_sample<Profile>>& samples,
        ParseFn parse_value,
        std::string& failure)
    {
        std::vector<Value> values;
        values.reserve(samples.size());
        for (std::size_t index = 0; index < samples.size(); ++index)
        {
            const std::string text = make_formatter_seed_text<Profile>(samples[index]);
            std::optional<Value> value;
            try
            {
                value = parse_value(text);
            }
            catch (const std::exception& exception)
            {
                failure = exception_failure_reason("parse threw", exception) + " at sample " + std::to_string(index);
                return std::nullopt;
            }
            catch (...)
            {
                failure = "parse threw at sample " + std::to_string(index);
                return std::nullopt;
            }
            if (!value)
            {
                failure = "parse failed at sample " + std::to_string(index);
                return std::nullopt;
            }
            if (io_value_is_inf(*value) || io_value_is_nan(*value))
            {
                failure = "finite sample parsed as non-finite at sample " + std::to_string(index);
                return std::nullopt;
            }
            values.push_back(*value);
        }
        return values;
    }

    template<class Profile>
    [[nodiscard]] bool qdpp_fixed_format_is_unsafe(
        const std::vector<io_sample<Profile>>& samples,
        const io_format_case& format)
    {
        if ((format.flags & std::ios_base::floatfield) != std::ios_base::fixed)
            return false;

        // qdpp's fixed formatter can hit a fatal path on long decimal
        // expansions, so treat those rows as unsupported for that backend.
        const typename Profile::perfect_ref limit{ "1e60" };
        for (const io_sample<Profile>& sample : samples)
        {
            const typename Profile::perfect_ref magnitude =
                sample.oracle < 0 ? -sample.oracle : sample.oracle;
            if (magnitude > limit)
                return true;
        }
        return false;
    }

    [[nodiscard]] inline bool qdpp_format_is_unsupported(const io_format_case& format) noexcept
    {
        if (format.hexfloat)
            return true;

        // qdpp treats "no floatfield" like scientific output rather than the
        // standard defaultfloat rules, so it is not a comparable reference.
        return (format.flags & std::ios_base::floatfield) == std::ios_base::fmtflags{};
    }

    [[nodiscard]] inline suite_id make_suite(precision_type precision, std::string_view operation) noexcept
    {
        return {
            precision,
            operation_id{ operation, operation },
            io_domain
        };
    }

    [[nodiscard]] inline std::string make_to_string_operation(
        std::string_view sample_label,
        std::string_view precision_label,
        std::string_view format_label)
    {
        std::string operation = "to_string(";
        operation += sample_label;
        operation += ", ";
        operation += precision_label;
        operation += ", ";
        operation += format_label;
        operation += ")";
        return operation;
    }

    [[nodiscard]] inline std::string make_to_string_operation(
        std::string_view sample_label,
        int precision,
        std::string_view format_label)
    {
        return make_to_string_operation(sample_label, std::to_string(precision), format_label);
    }

    template<class Profile>
    [[nodiscard]] const std::vector<io_sample<Profile>>& to_string_samples_for_format(
        const io_sample_group<Profile>& group,
        const io_format_case& format) noexcept
    {
        if (format.hexfloat)
            return group.hexfloat;

        const std::ios_base::fmtflags floatfield = format.flags & std::ios_base::floatfield;
        if (floatfield == std::ios_base::fixed)
            return group.fixed;
        if (floatfield == std::ios_base::scientific)
            return group.scientific;
        return group.defaultfloat;
    }

    [[nodiscard]] inline std::string make_parse_operation(
        std::string_view sample_label,
        std::string_view style_label)
    {
        std::string operation = "parse(";
        operation += sample_label;
        operation += ", ";
        operation += style_label;
        operation += ")";
        return operation;
    }

    [[nodiscard]] inline std::string make_random_parse_operation(
        std::string_view sample_label,
        std::string_view style_label)
    {
        std::string operation = "parse(rnd_prec, ";
        operation += style_label;
        operation += ", ";
        operation += sample_label;
        operation += ")";
        return operation;
    }

    template<class Profile>
    [[nodiscard]] metrics_record make_record(std::string_view operation)
    {
        metrics_record record;
        record.suite = make_suite(Profile::precision, operation);
        record.competitor_name = Profile::references::competitor_name;
        add_extra_competitor(record, Profile::references::extra_competitor_name);
        return record;
    }

    template<class Profile>
    [[nodiscard]] metrics_record make_to_string_record(
        std::string_view operation,
        const std::vector<io_sample<Profile>>& samples,
        int precision,
        const io_format_case& format)
    {
        metrics_record record = make_record<Profile>(operation);

        std::string fltx_failure;
        const auto fltx_values = try_make_values<Profile, typename Profile::fltx_type>(
            samples,
            [](std::string_view text)
            {
                return try_parse_fltx_value<typename Profile::fltx_type>(text);
            },
            fltx_failure);

        record.fltx_special_values = measure_to_string_special_support<typename Profile::fltx_type>(
            [](const auto& value, int digits, std::ios_base::fmtflags flags)
            {
                return bl::to_string(value, digits, flags);
            },
            precision,
            format.flags);
        if (!fltx_values)
        {
            report_io_backend_unsupported(operation, "fltx", "to_string setup", fltx_failure);
        }
        else
        {
            const io_accuracy_measurement fltx_accuracy = measure_to_string_accuracy<Profile>(
                samples,
                *fltx_values,
                precision,
                format,
                [](const auto& value, int digits, std::ios_base::fmtflags flags)
                {
                    return bl::to_string(value, digits, flags);
                });
            if (fltx_accuracy.succeeded())
                record.fltx_accuracy = fltx_accuracy.accuracy;
            else
                report_io_backend_unsupported(operation, "fltx", "to_string accuracy", fltx_accuracy.failure);

            record.fltx_benchmark = benchmark_indexed_values<Profile>(
                *fltx_values,
                [&samples, precision, format](std::size_t index, const auto& value)
                {
                    const int sample_precision = samples[index].precision >= 0 ? samples[index].precision : precision;
                    return bl::to_string(value, sample_precision, format.flags);
                },
                [](const std::string& text)
                {
                    Profile::consume_text(text);
                });
        }

        if constexpr (!config::benchmark_only_fltx)
        {
            if (format.hexfloat)
            {
                record.competitor_supported = false;
                record.extra_competitors.front().supported = false;
                return record;
            }

            std::string competitor_failure;
            const auto competitor_values = try_make_values<Profile, typename Profile::competitor_ref>(
                samples,
                [](std::string_view text)
                {
                    return try_parse_stream_value<typename Profile::competitor_ref>(text);
                },
                competitor_failure);
            if (!competitor_values)
            {
                record.competitor_supported = false;
                report_io_backend_unsupported(operation, record.competitor_name, "to_string setup", competitor_failure);
            }
            else
            {
                const io_accuracy_measurement competitor_accuracy = measure_to_string_accuracy<Profile>(
                    samples,
                    *competitor_values,
                    precision,
                    format,
                    [](const auto& value, int digits, std::ios_base::fmtflags flags)
                    {
                        return format_stream_value(value, digits, flags);
                    });
                if (!competitor_accuracy.succeeded())
                {
                    record.competitor_supported = false;
                    report_io_backend_unsupported(
                        operation,
                        record.competitor_name,
                        "to_string accuracy",
                        competitor_accuracy.failure);
                }
                else
                {
                    record.competitor_accuracy = competitor_accuracy.accuracy;
                    record.competitor_special_values = measure_to_string_special_support<typename Profile::competitor_ref>(
                        [](const auto& value, int digits, std::ios_base::fmtflags flags)
                        {
                            return format_stream_value(value, digits, flags);
                        },
                        precision,
                        format.flags);
                    record.competitor_benchmark = benchmark_indexed_values<Profile>(
                        *competitor_values,
                        [&samples, precision, format](std::size_t index, const auto& value)
                        {
                            const int sample_precision = samples[index].precision >= 0 ? samples[index].precision : precision;
                            return format_stream_value(value, sample_precision, format.flags);
                        },
                        [](const std::string& text)
                        {
                            Profile::consume_text(text);
                        });
                }
            }

            competitor_result& extra = record.extra_competitors.front();
            if (qdpp_format_is_unsupported(format))
            {
                extra.supported = false;
                return record;
            }

            if (qdpp_fixed_format_is_unsafe(samples, format))
            {
                extra.supported = false;
                return record;
            }

            std::string extra_failure;
            const auto extra_values = try_make_values<Profile, typename Profile::extra_competitor_ref>(
                samples,
                [](std::string_view text)
                {
                    return try_parse_qdpp_value<typename Profile::extra_competitor_ref>(text);
                },
                extra_failure);
            if (!extra_values)
            {
                extra.supported = false;
                report_io_backend_unsupported(operation, extra.name, "to_string setup", extra_failure);
                return record;
            }

            const io_accuracy_measurement extra_accuracy = measure_to_string_accuracy<Profile>(
                samples,
                *extra_values,
                precision,
                format,
                [](const auto& value, int digits, std::ios_base::fmtflags flags)
                {
                    return format_qdpp_value(value, digits, flags);
                });
            if (!extra_accuracy.succeeded())
            {
                extra.supported = false;
                report_io_backend_unsupported(operation, extra.name, "to_string accuracy", extra_accuracy.failure);
                return record;
            }

            extra.accuracy = extra_accuracy.accuracy;
            extra.special_values = measure_to_string_special_support<typename Profile::extra_competitor_ref>(
                [](const auto& value, int digits, std::ios_base::fmtflags flags)
                {
                    return format_qdpp_value(value, digits, flags);
                },
                precision,
                format.flags);
            extra.benchmark = benchmark_indexed_values<Profile>(
                *extra_values,
                [&samples, precision, format](std::size_t index, const auto& value)
                {
                    const int sample_precision = samples[index].precision >= 0 ? samples[index].precision : precision;
                    return format_qdpp_value(value, sample_precision, format.flags);
                },
                [](const std::string& text)
                {
                    Profile::consume_text(text);
                });
        }

        return record;
    }

    template<class Profile>
    [[nodiscard]] metrics_record make_parse_record(
        std::string_view operation,
        const std::vector<io_sample<Profile>>& samples,
        bool hexfloat)
    {
        metrics_record record = make_record<Profile>(operation);

        std::vector<std::string> texts;
        texts.reserve(samples.size());
        for (const auto& sample : samples)
            texts.push_back(sample.text);

        const io_accuracy_measurement fltx_accuracy =
            measure_parse_accuracy<Profile, typename Profile::fltx_type>(
            samples,
            [](std::string_view text)
            {
                return try_parse_fltx_value<typename Profile::fltx_type>(text);
            });
        if (fltx_accuracy.succeeded())
            record.fltx_accuracy = fltx_accuracy.accuracy;
        else
            report_io_backend_unsupported(operation, "fltx", "parse accuracy", fltx_accuracy.failure);

        record.fltx_special_values = measure_optional_parse_special_support(
            [](std::string_view text)
            {
                return try_parse_fltx_value<typename Profile::fltx_type>(text);
            });
        record.fltx_benchmark = benchmark_optional_values<Profile>(
            texts,
            [](const std::string& text)
            {
                return try_parse_fltx_value<typename Profile::fltx_type>(text);
            },
            [](const auto& value)
            {
                Profile::consume(value);
            });

        if constexpr (!config::benchmark_only_fltx)
        {
            if (hexfloat)
            {
                record.competitor_supported = false;
                record.extra_competitors.front().supported = false;
                return record;
            }

            const io_accuracy_measurement competitor_accuracy =
                measure_parse_accuracy<Profile, typename Profile::competitor_ref>(
                samples,
                [](std::string_view text)
                {
                    return try_parse_stream_value<typename Profile::competitor_ref>(text);
                });
            if (!competitor_accuracy.succeeded())
            {
                record.competitor_supported = false;
                report_io_backend_unsupported(
                    operation,
                    record.competitor_name,
                    "parse accuracy",
                    competitor_accuracy.failure);
            }
            else
            {
                record.competitor_accuracy = competitor_accuracy.accuracy;
                record.competitor_special_values = measure_optional_parse_special_support(
                    [](std::string_view text)
                    {
                        return try_parse_stream_value<typename Profile::competitor_ref>(text);
                    });
                record.competitor_benchmark = benchmark_optional_values<Profile>(
                    texts,
                    [](const std::string& text)
                    {
                        return try_parse_stream_value<typename Profile::competitor_ref>(text);
                    },
                    [](const auto& value)
                    {
                        Profile::consume(value);
                    });
            }

            competitor_result& extra = record.extra_competitors.front();
            const io_accuracy_measurement extra_accuracy =
                measure_parse_accuracy<Profile, typename Profile::extra_competitor_ref>(
                samples,
                [](std::string_view text)
                {
                    return try_parse_qdpp_value<typename Profile::extra_competitor_ref>(text);
                });
            if (!extra_accuracy.succeeded())
            {
                extra.supported = false;
                report_io_backend_unsupported(operation, extra.name, "parse accuracy", extra_accuracy.failure);
            }
            else
            {
                extra.accuracy = extra_accuracy.accuracy;
                extra.special_values = measure_optional_parse_special_support(
                    [](std::string_view text)
                    {
                        return try_parse_qdpp_value<typename Profile::extra_competitor_ref>(text);
                    });
                extra.benchmark = benchmark_optional_values<Profile>(
                    texts,
                    [](const std::string& text)
                    {
                        return try_parse_qdpp_value<typename Profile::extra_competitor_ref>(text);
                    },
                    [](const auto& value)
                    {
                        Profile::consume(value);
                    });
            }
        }

        return record;
    }

    [[nodiscard]] inline double finite_average_bits(double bits, double exact_bits) noexcept
    {
        if (!std::isinf(bits) || !std::isfinite(exact_bits))
            return bits;
        return bits > 0.0 ? exact_bits : -exact_bits;
    }

    [[nodiscard]] inline accuracy_result average_accuracy(const std::vector<accuracy_result>& values)
    {
        double total_bits = 0.0;
        double total_domain = 0.0;
        double total_worst_bits = 0.0;
        std::size_t sample_count = 0;
        std::size_t row_count = 0;

        for (const accuracy_result& value : values)
        {
            if (value.sample_count == 0)
                continue;

            total_bits += value.mean_bits * static_cast<double>(value.sample_count);
            total_domain += value.domain_score * static_cast<double>(value.sample_count);
            total_worst_bits += finite_average_bits(value.worst_bits, value.mean_bits);
            sample_count += value.sample_count;
            ++row_count;
        }

        if (sample_count == 0 || row_count == 0)
            return {};

        return {
            total_worst_bits / static_cast<double>(row_count),
            total_bits / static_cast<double>(sample_count),
            sample_count,
            total_domain / static_cast<double>(sample_count)
        };
    }

    [[nodiscard]] inline benchmark_result average_benchmark(const std::vector<benchmark_result>& values)
    {
        double total_ns = 0.0;
        std::size_t count = 0;
        std::size_t iterations = 0;
        for (const benchmark_result& value : values)
        {
            if (value.iteration_count == 0)
                continue;
            total_ns += value.ns_per_iter;
            iterations += value.iteration_count;
            ++count;
        }

        if (count == 0)
            return {};

        return { total_ns / static_cast<double>(count), iterations };
    }

    template<class Profile>
    [[nodiscard]] metrics_record make_average_record(
        std::string_view operation,
        const std::vector<metrics_record>& records)
    {
        metrics_record average = make_record<Profile>(operation);
        std::vector<accuracy_result> fltx_accuracy;
        std::vector<accuracy_result> competitor_accuracy;
        std::vector<accuracy_result> extra_accuracy;
        std::vector<benchmark_result> fltx_benchmark;
        std::vector<benchmark_result> competitor_benchmark;
        std::vector<benchmark_result> extra_benchmark;
        special_support fltx_special = special_support::unavailable;
        special_support competitor_special = special_support::unavailable;
        special_support extra_special = special_support::unavailable;

        fltx_accuracy.reserve(records.size());
        competitor_accuracy.reserve(records.size());
        extra_accuracy.reserve(records.size());
        fltx_benchmark.reserve(records.size());
        competitor_benchmark.reserve(records.size());
        extra_benchmark.reserve(records.size());

        bool competitor_has_any_support = false;
        bool extra_has_any_support = false;

        for (const metrics_record& record : records)
        {
            if (record.competitor_supported)
                competitor_has_any_support = true;

            if (!record.extra_competitors.empty() && record.extra_competitors.front().supported)
                extra_has_any_support = true;
        }

        const auto extra_supported = [](const metrics_record& record) noexcept
        {
            return !record.extra_competitors.empty() && record.extra_competitors.front().supported;
        };

        for (const metrics_record& record : records)
        {
            fltx_accuracy.push_back(record.fltx_accuracy);
            fltx_benchmark.push_back(record.fltx_benchmark);
            fltx_special = merge_special_support(fltx_special, record.fltx_special_values);

            if (record.competitor_supported)
            {
                competitor_accuracy.push_back(record.competitor_accuracy);
                competitor_benchmark.push_back(record.competitor_benchmark);
                competitor_special = merge_special_support(competitor_special, record.competitor_special_values);
            }

            if (extra_supported(record))
            {
                extra_accuracy.push_back(record.extra_competitors.front().accuracy);
                extra_benchmark.push_back(record.extra_competitors.front().benchmark);
                extra_special = merge_special_support(extra_special, record.extra_competitors.front().special_values);
            }
        }

        average.fltx_accuracy = average_accuracy(fltx_accuracy);
        average.fltx_special_values = fltx_special;
        average.fltx_benchmark = average_benchmark(fltx_benchmark);
        average.competitor_supported = competitor_has_any_support && !competitor_accuracy.empty();
        average.competitor_accuracy = average_accuracy(competitor_accuracy);
        average.competitor_special_values = competitor_special;
        average.competitor_benchmark = average_benchmark(competitor_benchmark);
        average.extra_competitors.front().supported = extra_has_any_support && !extra_accuracy.empty();
        average.extra_competitors.front().accuracy = average_accuracy(extra_accuracy);
        average.extra_competitors.front().special_values = extra_special;
        average.extra_competitors.front().benchmark = average_benchmark(extra_benchmark);
        return average;
    }

    template<class Profile, class Sink>
    void emit_to_string_records(const std::vector<io_sample_group<Profile>>& groups, Sink&& sink)
    {
        std::vector<metrics_record> records;
        const auto precisions = precision_cases<Profile>();
        const auto formats = format_cases();
        records.reserve(groups.size() * precisions.size() * formats.size() + 1u);
        std::vector<metrics_record> average_source_records;
        average_source_records.reserve(groups.size() * formats.size());

        for (const io_sample_group<Profile>& group : groups)
        {
            if (group.random_precision)
            {
                for (const io_format_case& format : formats)
                {
                    const auto& samples = to_string_samples_for_format(group, format);
                    if (samples.empty())
                        continue;

                    const std::string_view operation = intern_operation_name(
                        make_to_string_operation(group.label, "rng", format.label));
                    records.push_back(make_to_string_record<Profile>(
                        operation,
                        samples,
                        std::numeric_limits<typename Profile::fltx_type>::digits10,
                        format));
                    if (group.average_candidate)
                        average_source_records.push_back(records.back());
                    sink(records.back());
                }
                continue;
            }

            for (int precision : precisions)
            {
                for (const io_format_case& format : formats)
                {
                    const auto& samples = to_string_samples_for_format(group, format);
                    if (samples.empty())
                        continue;

                    const std::string_view operation = intern_operation_name(
                        make_to_string_operation(group.label, precision, format.label));
                    records.push_back(make_to_string_record<Profile>(
                        operation,
                        samples,
                        precision,
                        format));
                    sink(records.back());
                }
            }
        }

        const std::vector<metrics_record>& average_records =
            average_source_records.empty() ? records : average_source_records;
        records.push_back(make_average_record<Profile>(
            intern_operation_name("to_string (average)"),
            average_records));
        sink(records.back());
    }

    template<class Profile>
    [[nodiscard]] std::vector<metrics_record> make_to_string_records(const std::vector<io_sample_group<Profile>>& groups)
    {
        std::vector<metrics_record> records;
        emit_to_string_records<Profile>(
            groups,
            [&records](const metrics_record& record)
            {
                records.push_back(record);
            });
        return records;
    }

    template<class Profile, class Sink>
    void emit_parse_records(const std::vector<io_sample_group<Profile>>& groups, Sink&& sink)
    {
        struct parse_case
        {
            std::string_view label;
            bool hexfloat;
            std::vector<io_sample<Profile>> io_sample_group<Profile>::* samples;
        };

        const std::array<parse_case, 4> cases{
            parse_case{ "DEF", false, &io_sample_group<Profile>::defaultfloat },
            parse_case{ "FIX", false, &io_sample_group<Profile>::fixed },
            parse_case{ "SCI", false, &io_sample_group<Profile>::scientific },
            parse_case{ "HEX", true, &io_sample_group<Profile>::hexfloat }
        };

        std::vector<metrics_record> records;
        records.reserve(groups.size() * cases.size() + 1u);
        std::vector<metrics_record> average_source_records;
        average_source_records.reserve(groups.size() * cases.size());
        for (const io_sample_group<Profile>& group : groups)
        {
            for (const parse_case& parse : cases)
            {
                const auto& samples = group.*(parse.samples);
                if (samples.empty())
                    continue;

                const std::string_view operation = intern_operation_name(
                    group.random_precision
                        ? make_random_parse_operation(group.label, parse.label)
                        : make_parse_operation(group.label, parse.label));
                records.push_back(make_parse_record<Profile>(operation, samples, parse.hexfloat));
                if (group.average_candidate)
                    average_source_records.push_back(records.back());
                sink(records.back());
            }
        }

        const std::vector<metrics_record>& average_records =
            average_source_records.empty() ? records : average_source_records;
        records.push_back(make_average_record<Profile>(
            intern_operation_name("parse (average)"),
            average_records));
        sink(records.back());
    }

    template<class Profile>
    [[nodiscard]] std::vector<metrics_record> make_parse_records(const std::vector<io_sample_group<Profile>>& groups)
    {
        std::vector<metrics_record> records;
        emit_parse_records<Profile>(
            groups,
            [&records](const metrics_record& record)
            {
                records.push_back(record);
            });
        return records;
    }

    template<class Profile, class Sink>
    void emit_io_records(Sink&& sink)
    {
        clear_io_backend_warnings();
        const std::vector<io_sample_group<Profile>> groups = make_sample_groups<Profile>();
        emit_to_string_records<Profile>(groups, sink);
        emit_parse_records<Profile>(groups, sink);
        flush_io_backend_warnings();
    }

    template<class Profile>
    [[nodiscard]] std::vector<metrics_record> make_io_records()
    {
        std::vector<metrics_record> records;
        emit_io_records<Profile>(
            [&records](const metrics_record& record)
            {
                records.push_back(record);
            });
        return records;
    }
}

#endif
