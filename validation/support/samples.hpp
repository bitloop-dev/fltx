#ifndef FLTX_TESTS_SUPPORT_SAMPLES_INCLUDED
#define FLTX_TESTS_SUPPORT_SAMPLES_INCLUDED

#include "native_fp.hpp"

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fltx::tests
{
    inline constexpr std::uint64_t default_seed = 0x8f3f73b5cf1c9adeull;

    class random_bits
    {
    public:
        explicit constexpr random_bits(std::uint64_t seed) noexcept
            : state_(seed != 0 ? seed : default_seed)
        {
        }

        [[nodiscard]] constexpr std::uint64_t next() noexcept
        {
            std::uint64_t x = state_;
            x ^= x >> 12;
            x ^= x << 25;
            x ^= x >> 27;
            state_ = x;
            return x * 0x2545f4914f6cdd1dull;
        }

        [[nodiscard]] double unit() noexcept
        {
            return static_cast<double>(next() >> 11) * 0x1.0p-53;
        }

        [[nodiscard]] double between(double low, double high) noexcept
        {
            return low + (high - low) * unit();
        }

    private:
        std::uint64_t state_;
    };

    struct sample
    {
        std::array<double, 4> limb{};
        std::string label;
    };

    [[nodiscard]] inline sample make_sample(double value, std::string label = {})
    {
        const double scale = std::abs(value);
        if (scale == 0.0 || !native_fp::is_finite(scale))
            return { { value, 0.0, 0.0, 0.0 }, std::move(label) };

        const double tail1 = std::copysign(std::ldexp(scale, -55), value == 0.0 ? 1.0 : value);
        const double tail2 = -std::copysign(std::ldexp(scale, -109), tail1);
        const double tail3 = std::copysign(std::ldexp(scale, -163), tail1);
        return { { value, tail1, tail2, tail3 }, std::move(label) };
    }

    [[nodiscard]] inline sample make_exact_sample(double value, std::string label = {})
    {
        return { { value, 0.0, 0.0, 0.0 }, std::move(label) };
    }

    [[nodiscard]] inline std::string describe(const sample& value)
    {
        std::ostringstream out;
        if (!value.label.empty())
            out << value.label << ' ';
        out << std::setprecision(17)
            << '[' << value.limb[0] << ',' << value.limb[1]
            << ',' << value.limb[2] << ',' << value.limb[3] << ']';
        return out.str();
    }

    [[nodiscard]] inline std::vector<std::string> decimal_strings(
        std::size_t count,
        int significant_digits,
        int minimum_exponent,
        int maximum_exponent,
        std::uint64_t seed)
    {
        const int digits = std::max(significant_digits, 18);
        const int exponent_span = maximum_exponent - minimum_exponent + 1;
        std::vector<std::string> out;
        out.reserve(count);
        random_bits rng(seed);

        for (std::size_t index = 0; index < count; ++index)
        {
            std::string text;
            text.reserve(static_cast<std::size_t>(digits) + 10);
            if ((index & 1u) != 0)
                text.push_back('-');
            text.push_back(static_cast<char>('1' + rng.next() % 9));
            text.push_back('.');
            for (int digit = 1; digit < digits; ++digit)
                text.push_back(static_cast<char>('0' + rng.next() % 10));

            const int exponent = minimum_exponent +
                static_cast<int>(
                    rng.next() % static_cast<std::uint64_t>(exponent_span));
            text.push_back('e');
            text.push_back(exponent < 0 ? '-' : '+');
            const int magnitude = exponent < 0 ? -exponent : exponent;
            if (magnitude < 10)
                text.push_back('0');
            text += std::to_string(magnitude);
            out.push_back(std::move(text));
        }
        return out;
    }
}

#endif
