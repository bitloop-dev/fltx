#include <catch2/catch_test_macros.hpp>

#include <array>
#include <bit>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <fltx/io.h>
#include <fltx/format.h>
#include <fltx/math.h>
#include <fltx/numbers.h>

#include "../../support/samples.hpp"

namespace
{
    template<class T>
    void check_parse_and_format()
    {
        constexpr std::array<std::string_view, 6> texts{
            "0", "-0", "1.25", "-3.75", "1e-20", "3.1415926535897932384626433832795"
        };

        for (const auto text : texts)
        {
            T parsed{};
            const auto result = bl::from_chars(text.data(), text.data() + text.size(), parsed);
            REQUIRE(result.ec == std::errc{});
            REQUIRE(result.ptr == text.data() + text.size());

            const std::string formatted =
                bl::to_string(parsed, std::numeric_limits<T>::max_digits10, std::ios_base::scientific);
            CHECK(bl::parse<T>(formatted) == parsed);
        }
    }

    template<class T>
    void check_invalid_input_is_transactional()
    {
        for (const std::string_view text : { "", " ", ".", "+.", "e10", "--1" })
        {
            T value{ 42.0 };
            const auto result = bl::from_chars(text.data(), text.data() + text.size(), value);
            CAPTURE(text);
            CHECK(result.ec == std::errc::invalid_argument);
            CHECK(result.ptr == text.data());
            CHECK(value == T{ 42.0 });
        }
    }

    enum class partial_token_kind
    {
        finite,
        infinity,
        nan
    };

    struct partial_token_case
    {
        std::string_view text;
        std::size_t consumed;
        partial_token_kind kind;
        double finite_value = 0.0;
        bool negative = false;
    };

    template<class T>
    void check_partial_token_boundaries()
    {
        constexpr std::array cases{
            partial_token_case{
                "1.25tail", 4, partial_token_kind::finite, 1.25
            },
            partial_token_case{
                "1e+oops", 1, partial_token_kind::finite, 1.0
            },
            partial_token_case{
                "infinity;", 8, partial_token_kind::infinity
            },
            partial_token_case{
                "-inf tail", 4, partial_token_kind::infinity, 0.0, true
            },
            partial_token_case{
                "NaN(payload)", 3, partial_token_kind::nan
            },
            partial_token_case{
                "nan rest", 3, partial_token_kind::nan
            }
        };

        for (const auto& test : cases)
        {
            T value{};
            const auto result = bl::from_chars(
                test.text.data(), test.text.data() + test.text.size(), value);
            CAPTURE(test.text);
            REQUIRE(result.ec == std::errc{});
            REQUIRE(result.ptr == test.text.data() + test.consumed);

            if (test.kind == partial_token_kind::finite)
                CHECK(value == T{ test.finite_value });
            else if (test.kind == partial_token_kind::infinity)
            {
                CHECK(bl::isinf(value));
                CHECK(bl::signbit(value) == test.negative);
            }
            else
                CHECK(bl::isnan(value));
        }
    }

    template<class T>
    void check_extreme_decimal_saturation()
    {
        constexpr std::string_view overflow = "1e100000000";
        T positive{};
        const auto overflow_result = bl::from_chars(
            overflow.data(), overflow.data() + overflow.size(), positive);
        REQUIRE(overflow_result.ec == std::errc{});
        REQUIRE(overflow_result.ptr == overflow.data() + overflow.size());
        CHECK(bl::isinf(positive));
        CHECK_FALSE(bl::signbit(positive));

        constexpr std::string_view near_overflow = "1.8e308";
        const T rounded_overflow = bl::parse<T>(near_overflow);
        CHECK(bl::isinf(rounded_overflow));
        CHECK_FALSE(bl::signbit(rounded_overflow));

        constexpr std::string_view underflow = "-1e-100000000";
        T negative{ 1.0 };
        const auto underflow_result = bl::from_chars(
            underflow.data(), underflow.data() + underflow.size(), negative);
        REQUIRE(underflow_result.ec == std::errc{});
        REQUIRE(underflow_result.ptr == underflow.data() + underflow.size());
        CHECK(bl::iszero(negative));
        CHECK(bl::signbit(negative));
    }

    template<class T>
    void check_public_conversion_surface()
    {
        const T original{ 1.25 };
        std::array<char, 256> buffer{};

        const auto shortest =
            bl::to_chars(buffer.data(), buffer.data() + buffer.size(), original);
        REQUIRE(shortest.ec == std::errc{});

        T restored{};
        const auto shortest_input =
            bl::from_chars(buffer.data(), shortest.ptr, restored);
        CHECK(shortest_input.ec == std::errc{});
        CHECK(shortest_input.ptr == shortest.ptr);
        CHECK(restored == original);

        const auto general = bl::to_chars(
            buffer.data(),
            buffer.data() + buffer.size(),
            original,
            std::chars_format::general);
        REQUIRE(general.ec == std::errc{});

        const auto fixed = bl::to_chars(
            buffer.data(),
            buffer.data() + buffer.size(),
            original,
            std::chars_format::fixed,
            2);
        REQUIRE(fixed.ec == std::errc{});
        const std::string_view fixed_text{
            buffer.data(),
            static_cast<std::size_t>(fixed.ptr - buffer.data())
        };
        CHECK(fixed_text == "1.25");

        restored = T{};
        const auto fixed_input =
            bl::from_chars(buffer.data(), fixed.ptr, restored, std::chars_format::fixed);
        CHECK(fixed_input.ec == std::errc{});
        CHECK(fixed_input.ptr == fixed.ptr);
        CHECK(restored == original);

        const auto parsed = bl::try_parse<T>(fixed_text, std::chars_format::fixed);
        REQUIRE(parsed);
        CHECK(parsed.consumed == fixed_text.size());
        CHECK(parsed.value == original);

        restored = T{};
        CHECK(bl::try_parse(fixed_text, restored, std::chars_format::fixed));
        CHECK(restored == original);
        CHECK(bl::parse<T>(fixed_text, std::chars_format::fixed) == original);
        CHECK(bl::parse<T>("invalid", T{ 9.0 }) == T{ 9.0 });

        const auto static_text =
            bl::to_static_string(original, 2, std::ios_base::fixed);
        CHECK(static_text.view() == fixed_text);
        CHECK(bl::to_string(original, 2, std::ios_base::fixed) == fixed_text);
    }

    template<class T>
    void check_maximum_decimal_precision()
    {
        for (const T original : {
                 T{ std::numeric_limits<double>::max() },
                 T{ -std::numeric_limits<double>::max() } })
        {
            const std::string text = bl::to_string(
                original,
                std::numeric_limits<T>::max_digits10,
                std::ios_base::scientific);
            const auto restored = bl::try_parse<T>(text);
            REQUIRE(restored);
            REQUIRE(bl::isfinite(restored.value));

            T magnitude = original;
            if (magnitude < T{})
                magnitude = -magnitude;

            T difference = restored.value - original;
            if (difference < T{})
                difference = -difference;
            const T tolerance =
                magnitude * std::numeric_limits<T>::epsilon();
            CHECK(difference <= tolerance);
        }
    }

    [[nodiscard]] bool nonoverlapping(double high, double low) noexcept
    {
        if (low == 0.0)
            return true;
        return high != 0.0 && std::isfinite(high) && std::isfinite(low) &&
               high + low == high;
    }

    [[nodiscard]] bool is_canonical(const bl::fdd_s& value) noexcept
    {
        return nonoverlapping(value.hi, value.lo);
    }

    [[nodiscard]] bool is_canonical(const bl::fqd_s& value) noexcept
    {
        return nonoverlapping(value.x0, value.x1) &&
               nonoverlapping(value.x1, value.x2) &&
               nonoverlapping(value.x2, value.x3);
    }

    [[nodiscard]] bool same_encoding(double lhs, double rhs) noexcept
    {
        return std::bit_cast<std::uint64_t>(lhs) ==
               std::bit_cast<std::uint64_t>(rhs);
    }

    [[nodiscard]] bool same_tail_encoding(double lhs, double rhs) noexcept
    {
        return (lhs == 0.0 && rhs == 0.0) || same_encoding(lhs, rhs);
    }

    [[nodiscard]] bool same_encoding(
        const bl::fdd_s& lhs,
        const bl::fdd_s& rhs) noexcept
    {
        return same_encoding(lhs.hi, rhs.hi) &&
               same_tail_encoding(lhs.lo, rhs.lo);
    }

    [[nodiscard]] bool same_encoding(
        const bl::fqd_s& lhs,
        const bl::fqd_s& rhs) noexcept
    {
        return same_encoding(lhs.x0, rhs.x0) &&
               same_tail_encoding(lhs.x1, rhs.x1) &&
               same_tail_encoding(lhs.x2, rhs.x2) &&
               same_tail_encoding(lhs.x3, rhs.x3);
    }

    [[nodiscard]] std::string raw_limbs(const bl::fdd_s& value)
    {
        std::ostringstream out;
        out << std::hexfloat << value.hi << ", " << value.lo;
        return out.str();
    }

    [[nodiscard]] std::string raw_limbs(const bl::fqd_s& value)
    {
        std::ostringstream out;
        out << std::hexfloat
            << value.x0 << ", " << value.x1 << ", "
            << value.x2 << ", " << value.x3;
        return out.str();
    }

    [[nodiscard]] double random_limb(
        fltx::tests::random_bits& random,
        int exponent)
    {
        const double magnitude = std::ldexp(1.0 + random.unit(), exponent);
        return std::copysign(
            magnitude,
            (random.next() & 1u) != 0 ? -1.0 : 1.0);
    }

    [[nodiscard]] bl::fdd canonical_dd(
        fltx::tests::random_bits& random,
        int sample)
    {
        static constexpr std::array boundaries{
            bl::fdd{ 0.0, 0.0 },
            bl::fdd{ -0.0, 0.0 },
            bl::fdd{ std::numeric_limits<double>::denorm_min(), 0.0 },
            bl::fdd{ -std::numeric_limits<double>::denorm_min(), 0.0 },
            bl::fdd{ std::numeric_limits<double>::min(), 0.0 },
            bl::fdd{ -std::numeric_limits<double>::min(), 0.0 },
            bl::fdd{ 1.0, 0x1p-55 },
            bl::fdd{ 1.0, -0x1p-55 },
            bl::fdd{ -1.0, 0x1p-55 },
            bl::fdd{ -1.0, -0x1p-55 },
            bl::fdd{ 0x1p-900, 0x1p-955 },
            bl::fdd{ -0x1p-900, -0x1p-955 },
            bl::fdd{ 0x1p+900, 0x1p+845 },
            bl::fdd{ -0x1p+900, -0x1p+845 },
        };

        if (sample < static_cast<int>(boundaries.size()))
            return boundaries[static_cast<std::size_t>(sample)];

        const int exponent = static_cast<int>(random.next() % 1'701u) - 850;
        return {
            random_limb(random, exponent),
            random_limb(random, exponent - 56)
        };
    }

    [[nodiscard]] bl::fqd canonical_qd(
        fltx::tests::random_bits& random,
        int sample)
    {
        static constexpr std::array boundaries{
            bl::fqd{ 0.0, 0.0, 0.0, 0.0 },
            bl::fqd{ -0.0, 0.0, 0.0, 0.0 },
            bl::fqd{ std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 },
            bl::fqd{ -std::numeric_limits<double>::denorm_min(), 0.0, 0.0, 0.0 },
            bl::fqd{ std::numeric_limits<double>::min(), 0.0, 0.0, 0.0 },
            bl::fqd{ -std::numeric_limits<double>::min(), 0.0, 0.0, 0.0 },
            bl::fqd{ 1.0, 0x1p-55, -0x1p-110, 0x1p-165 },
            bl::fqd{ 1.0, -0x1p-55, 0x1p-110, -0x1p-165 },
            bl::fqd{ -1.0, 0x1p-55, -0x1p-110, 0x1p-165 },
            bl::fqd{ -1.0, -0x1p-55, 0x1p-110, -0x1p-165 },
            bl::fqd{ 0x1p-800, 0x1p-855, -0x1p-910, 0x1p-965 },
            bl::fqd{ -0x1p-800, -0x1p-855, 0x1p-910, -0x1p-965 },
            bl::fqd{ 0x1p+800, 0x1p+745, -0x1p+690, 0x1p+635 },
            bl::fqd{ -0x1p+800, -0x1p+745, 0x1p+690, -0x1p+635 },
        };

        if (sample < static_cast<int>(boundaries.size()))
            return boundaries[static_cast<std::size_t>(sample)];

        const int exponent = static_cast<int>(random.next() % 1'701u) - 850;
        return {
            random_limb(random, exponent),
            random_limb(random, exponent - 56),
            random_limb(random, exponent - 112),
            random_limb(random, exponent - 168)
        };
    }

    template<class T>
    void check_deterministic_round_trips(
        std::uint64_t seed,
        T (*make_value)(fltx::tests::random_bits&, int))
    {
        constexpr int sample_count = 10'000;
        constexpr int expansion_round_trip_digits =
            std::numeric_limits<T>::max_digits10;
        fltx::tests::random_bits random{ seed };

        for (int sample = 0; sample < sample_count; ++sample)
        {
            const T original = make_value(random, sample);
            if (!is_canonical(original))
            {
                CAPTURE(seed, sample);
                FAIL("raw-limb round-trip input is not canonical");
            }

            const std::string text = bl::to_string(
                original,
                expansion_round_trip_digits,
                std::ios_base::scientific);
            const auto restored = bl::try_parse<T>(text);

            if (!restored)
            {
                const std::string original_limbs = raw_limbs(original);
                const std::string restored_limbs = "parse failed";
                CAPTURE(seed, sample, original_limbs, text, restored_limbs);
                CHECK(restored);
            }
            else if (!same_encoding(restored.value, original))
            {
                const std::string original_limbs = raw_limbs(original);
                const std::string restored_limbs = raw_limbs(restored.value);
                CAPTURE(seed, sample, original_limbs, text, restored_limbs);
                CHECK(same_encoding(restored.value, original));
            }
        }

        SUCCEED("10,000 deterministic values round-tripped exactly");
    }

    template<class T>
    [[nodiscard]] T nominal_round_trip_value(
        fltx::tests::random_bits& random,
        int sample)
    {
        const T zero{};
        const T one{ 1.0 };
        const T two{ 2.0 };
        static const std::array boundaries{
            zero,
            T{ -0.0 },
            T{ std::numeric_limits<double>::denorm_min() },
            T{ -std::numeric_limits<double>::denorm_min() },
            T{ std::numeric_limits<double>::min() },
            T{ -std::numeric_limits<double>::min() },
            one,
            T{ -one },
            bl::nextafter(one, two),
            bl::nextafter(one, zero),
            bl::nextafter(T{ 8.0 }, T{ 16.0 }),
            bl::nextafter(T{ 8.0 }, zero),
            std::numeric_limits<T>::max(),
            std::numeric_limits<T>::lowest(),
        };

        if (sample < static_cast<int>(boundaries.size()))
            return boundaries[static_cast<std::size_t>(sample)];

        const int exponent = static_cast<int>(random.next() % 1'701u) - 850;
        T value{ random_limb(random, exponent) };
        const T target = (random.next() & 1u) != 0
            ? std::numeric_limits<T>::infinity()
            : -std::numeric_limits<T>::infinity();
        const int steps = static_cast<int>(random.next() % 17u);
        for (int step = 0; step < steps; ++step)
            value = bl::nextafter(value, target);
        return value;
    }

    template<class T>
    void check_nominal_default_round_trips(std::uint64_t seed)
    {
        constexpr int sample_count = 10'000;
        fltx::tests::random_bits random{ seed };

        for (int sample = 0; sample < sample_count; ++sample)
        {
            const T original = nominal_round_trip_value<T>(random, sample);
            REQUIRE(is_canonical(original));

            const std::string dynamic_text = bl::to_string(original);
            const auto static_text = bl::to_static_string(original);
            std::array<char, 128> chars{};
            const auto chars_result =
                bl::to_chars(chars.data(), chars.data() + chars.size(), original);
            REQUIRE(chars_result.ec == std::errc{});
            const std::string_view chars_text{
                chars.data(),
                static_cast<std::size_t>(chars_result.ptr - chars.data())
            };

            CAPTURE(seed, sample, dynamic_text, raw_limbs(original));
            CHECK(static_text.view() == dynamic_text);
            CHECK(chars_text == dynamic_text);
#if FLTX_HAS_STD_FORMAT
            CHECK(std::format("{}", original) == dynamic_text);
#endif

            const auto restored = bl::try_parse<T>(dynamic_text);
            REQUIRE(restored);
            CAPTURE(raw_limbs(restored.value));
            CHECK(bl::to_string(restored.value) == dynamic_text);

            T difference = restored.value - original;
            if (difference < T{})
                difference = -difference;
            const T neighbor = bl::nextafter(original, restored.value);
            T nominal_step = neighbor - original;
            if (nominal_step < T{})
                nominal_step = -nominal_step;
            const T half_step{ nominal_step * T{ 0.5 } };
            CHECK(difference <= half_step);
        }
    }
}

TEST_CASE("parse, format, and stream round trips preserve values", "[contracts][io]")
{
    check_parse_and_format<float>();
    check_parse_and_format<double>();
    check_parse_and_format<bl::fdd>();
    check_parse_and_format<bl::fqd>();
    check_maximum_decimal_precision<bl::fdd>();
    check_maximum_decimal_precision<bl::fqd>();

    const bl::fdd dd = bl::parse<bl::fdd>("1.00000000000000000000000000000001");
    const bl::fqd qd = bl::parse<bl::fqd>(
        "1.000000000000000000000000000000000000000000000000000000000000001");
    const std::string dd_text = bl::to_string(
        dd, std::numeric_limits<bl::fdd>::max_digits10, std::ios_base::scientific);
    const std::string qd_text = bl::to_string(
        qd, std::numeric_limits<bl::fqd>::max_digits10, std::ios_base::scientific);
    CHECK(bl::parse<bl::fdd>(dd_text) == dd);
    CHECK(bl::parse<bl::fqd>(qd_text) == qd);

    const bl::fqd qd_maximum = std::numeric_limits<bl::fqd>::max();
    const bl::fqd qd_lowest = std::numeric_limits<bl::fqd>::lowest();
    CHECK(bl::parse<bl::fqd>(bl::to_string(qd_maximum)) == qd_maximum);
    CHECK(bl::parse<bl::fqd>(bl::to_string(qd_lowest)) == qd_lowest);

    const bl::fdd dd_sparse_tail{ 1.0, 0x1p-55 };
    CHECK(bl::parse<bl::fdd>(bl::to_string(dd_sparse_tail)) == dd_sparse_tail);

    CHECK(bl::to_string(dd) == bl::to_string(
        dd, std::numeric_limits<bl::fdd>::max_digits10));
    CHECK(bl::to_string(qd) == bl::to_string(
        qd, std::numeric_limits<bl::fqd>::max_digits10));
    CHECK(bl::to_static_string(dd).view() ==
          bl::to_static_string(dd, std::numeric_limits<bl::fdd>::max_digits10).view());
    CHECK(bl::to_static_string(qd).view() ==
          bl::to_static_string(qd, std::numeric_limits<bl::fqd>::max_digits10).view());

    std::stringstream stream;
    stream << std::scientific
           << std::setprecision(std::numeric_limits<bl::fqd>::max_digits10)
           << qd;
    bl::fqd restored{};
    stream >> restored;
    CHECK(restored == qd);

    const bl::fdd_s dd_storage =
        bl::parse<bl::fdd_s>("1.00000000000000000000000000000001");
    const bl::fqd_s qd_storage = bl::parse<bl::fqd_s>(
        "1.000000000000000000000000000000000000000000000000000000000000001");
    CHECK(dd_storage.lo != 0.0);
    CHECK((qd_storage.x1 != 0.0 || qd_storage.x2 != 0.0 || qd_storage.x3 != 0.0));

    std::stringstream storage_stream;
    storage_stream
        << std::scientific
        << std::setprecision(std::numeric_limits<bl::fdd_s>::max_digits10)
        << dd_storage
        << ' '
        << std::setprecision(std::numeric_limits<bl::fqd_s>::max_digits10)
        << qd_storage;
    bl::fdd_s restored_dd_storage{};
    bl::fqd_s restored_qd_storage{};
    storage_stream >> restored_dd_storage >> restored_qd_storage;
    CHECK(restored_dd_storage == dd_storage);
    CHECK(restored_qd_storage == qd_storage);
}

TEST_CASE("parsing reports partial and invalid input without hidden overwrite", "[contracts][io]")
{
    check_invalid_input_is_transactional<float>();
    check_invalid_input_is_transactional<double>();
    check_invalid_input_is_transactional<bl::fdd>();
    check_invalid_input_is_transactional<bl::fqd>();

    constexpr std::string_view partial = "1.25tail";
    bl::fdd value{};
    const auto result = bl::from_chars(partial.data(), partial.data() + partial.size(), value);
    CHECK(result.ec == std::errc{});
    CHECK(result.ptr == partial.data() + 4);
    CHECK(value == bl::fdd{ 1.25 });

    std::istringstream stream{ "1.25tail" };
    bl::fdd unchanged{ 42.0 };
    CHECK_FALSE(static_cast<bool>(stream >> unchanged));
    CHECK(unchanged == bl::fdd{ 42.0 });
}

TEST_CASE("from_chars preserves partial token boundaries",
          "[contracts][io][charconv]")
{
    check_partial_token_boundaries<bl::fdd>();
    check_partial_token_boundaries<bl::fqd>();
}

TEST_CASE("I/O preserves signed zero and special values", "[contracts][io][special]")
{
    const auto inf = std::numeric_limits<bl::fqd>::infinity();
    const auto nan = std::numeric_limits<bl::fqd>::quiet_NaN();

    CHECK(bl::to_string(inf) == "inf");
    CHECK(bl::to_string(-inf) == "-inf");
    CHECK(bl::to_string(nan) == "nan");
    CHECK(bl::isinf(bl::parse<bl::fqd>("inf")));
    CHECK(bl::isnan(bl::parse<bl::fqd>("nan")));

    const auto negative_zero = bl::parse<bl::fdd>("-0");
    CHECK(bl::iszero(negative_zero));
    CHECK(bl::signbit(negative_zero));
    CHECK(bl::to_string(negative_zero, 3, std::ios_base::fixed) == "-0.000");
}

TEST_CASE("literals retain digits beyond binary64", "[contracts][io][literal]")
{
    using namespace bl::literals;

    constexpr bl::fdd dd = "1.00000000000000000000000000000001"_dd;
    constexpr bl::fqd qd =
        "1.000000000000000000000000000000000000000000000000000000000000001"_qd;

    STATIC_CHECK(dd > bl::fdd{ 1.0 });
    STATIC_CHECK(qd > bl::fqd{ 1.0 });
    STATIC_CHECK(0x1.4p+0_dd == bl::fdd{ 1.25 });
    STATIC_CHECK(0x1.4p+0_qd == bl::fqd{ 1.25 });
}

TEST_CASE("extended parsing preserves hexadecimal precision and range semantics",
          "[contracts][io][charconv]")
{
    using namespace bl::literals;

    STATIC_CHECK("0X1.8P+2"_dd == bl::fdd{ 6.0 });
    STATIC_CHECK("0X1.8P+2"_qd == bl::fqd{ 6.0 });
    STATIC_CHECK(
        bl::parse<bl::fqd>("+0X1.4P+0") == bl::fqd{ 1.25 });

    std::string half_hex = "1.";
    half_hex.append(52, '0');
    half_hex += "1p+0";
    const bl::fqd half_low_bit =
        bl::fqd{ 1.0 } + bl::ldexp(bl::fqd{ 1.0 }, -212);
    CHECK(bl::parse<bl::fqd>(half_hex, std::chars_format::hex) ==
          half_low_bit);

    std::string above_half_hex = "1.";
    above_half_hex.append(52, '0');
    above_half_hex += '1';
    above_half_hex.append(200, '0');
    above_half_hex += "1p+0";
    CHECK(bl::parse<bl::fqd>(above_half_hex, std::chars_format::hex) ==
          half_low_bit);

    check_extreme_decimal_saturation<bl::fdd>();
    check_extreme_decimal_saturation<bl::fqd>();
}

TEST_CASE("charconv-shaped helpers expose buffers, errors, and complete parsing", "[contracts][io][charconv]")
{
    check_public_conversion_surface<bl::f32>();
    check_public_conversion_surface<bl::f64>();
    check_public_conversion_surface<bl::fdd_s>();
    check_public_conversion_surface<bl::fdd>();
    check_public_conversion_surface<bl::fqd_s>();
    check_public_conversion_surface<bl::fqd>();

    std::array<char, 128> buffer{};
    const auto written = bl::to_chars(
        buffer.data(),
        buffer.data() + buffer.size(),
        bl::fdd{ 1.25 },
        std::chars_format::fixed,
        3);
    REQUIRE(written.ec == std::errc{});
    CHECK(std::string(buffer.data(), written.ptr) == "1.250");

    const auto parsed = bl::try_parse<bl::fqd>("1.25");
    REQUIRE(parsed);
    CHECK(parsed.consumed == 4);
    CHECK(parsed.value == bl::fqd{ 1.25 });

    bl::fdd unchanged{ 7.0 };
    CHECK_FALSE(bl::try_parse("1.25tail", unchanged));
    CHECK(unchanged == bl::fdd{ 7.0 });
    CHECK_THROWS_AS(bl::parse<bl::fdd>("not-a-number"), std::invalid_argument);

    for (const auto format : {
             std::chars_format::general,
             std::chars_format::fixed,
             std::chars_format::scientific,
             std::chars_format::hex })
    {
        std::array<char, 256> text{};
        const bl::fqd original{ 1.25 };
        const auto encoded =
            bl::to_chars(text.data(), text.data() + text.size(), original, format, 12);
        REQUIRE(encoded.ec == std::errc{});

        bl::fqd_s decoded{};
        const auto decoded_result =
            bl::from_chars(text.data(), encoded.ptr, decoded, format);
        CAPTURE(static_cast<int>(format));
        CHECK(decoded_result.ec == std::errc{});
        CHECK(decoded_result.ptr == encoded.ptr);
        CHECK(decoded == original);
    }

    std::array<char, 2> too_small{};
    const auto overflow = bl::to_chars(
        too_small.data(),
        too_small.data() + too_small.size(),
        bl::fdd_s{ 123.0, 0.0 });
    CHECK(overflow.ec == std::errc::value_too_large);
    CHECK(overflow.ptr == too_small.data() + too_small.size());

    constexpr std::string_view oversized_native = "1e10000";
    const auto out_of_range = bl::try_parse<bl::f32>(oversized_native);
    CHECK_FALSE(out_of_range);
    CHECK(out_of_range.ec == std::errc::result_out_of_range);

    bl::f32 unchanged_native = 7.0f;
    const auto native_range_result = bl::from_chars(
        oversized_native.data(),
        oversized_native.data() + oversized_native.size(),
        unchanged_native);
    CHECK(native_range_result.ptr ==
          oversized_native.data() + oversized_native.size());
    CHECK(native_range_result.ec == std::errc::result_out_of_range);
    CHECK(unchanged_native == 7.0f);

    CHECK(bl::parse<bl::f64>("0x1.8p+1") == 3.0);
    CHECK(bl::parse<bl::fdd>("+1.25") == bl::fdd{ 1.25 });

    static_assert(std::is_same_v<
        decltype(bl::to_static_string(bl::f32{})),
        bl::f32_io_string>);
    static_assert(std::is_same_v<
        decltype(bl::to_static_string(bl::f64{})),
        bl::f64_io_string>);
    static_assert(std::is_same_v<
        decltype(bl::to_static_string(bl::fdd{})),
        bl::fdd_io_string>);
    static_assert(std::is_same_v<
        decltype(bl::to_static_string(bl::fqd{})),
        bl::fqd_io_string>);
}

TEST_CASE("fixed-capacity strings provide a conventional constexpr string surface",
          "[contracts][io][static-string]")
{
    constexpr auto built = [] {
        bl::static_string<16> value{ "ab" };
        value.push_back('c');
        value.append(2, 'd');
        value.insert(1, "XY");
        value.insert(value.size(), 1, '!');
        return value;
    }();

    STATIC_CHECK(built.view() == "aXYbcdd!");
    STATIC_CHECK(built.size() == 8);
    STATIC_CHECK(!built.empty());
    STATIC_CHECK(built.front() == 'a');
    STATIC_CHECK(built[3] == 'b');
    STATIC_CHECK(built.end() - built.begin() == 8);
    STATIC_CHECK(bl::static_string<16>::static_capacity == 16);

    bl::static_string<8> runtime{ std::string_view{ "one" } };
    runtime = "two";
    CHECK(std::string(runtime) == "two");
    CHECK(std::string_view(runtime.data(), runtime.size()) == "two");
    CHECK(std::string_view(runtime.c_str()) == "two");
    runtime.resize(5);
    CHECK(runtime.size() == 5);
    runtime.clear();
    CHECK(runtime.empty());
    CHECK(runtime.c_str()[0] == '\0');
    CHECK_THROWS(runtime.insert(1, "x"));
    CHECK_THROWS(runtime.assign("123456789"));

    CHECK(
        bl::to_string(
            1.23456789,
            bl::precision_info{ 8, 2, 2 },
            std::ios_base::fixed) ==
        "1.23...89");
}

TEST_CASE("deterministic high-volume extended round trips preserve every value",
          "[contracts][io][roundtrip]")
{
    SECTION("dd")
    {
        check_deterministic_round_trips<bl::fdd>(
            0x1020304050607080ull,
            canonical_dd);
    }

    SECTION("qd")
    {
        check_deterministic_round_trips<bl::fqd>(
            0x8070605040302010ull,
            canonical_qd);
    }
}

TEST_CASE("default decimal output round trips the nominal value model",
          "[contracts][io][roundtrip][nominal]")
{
    SECTION("dd")
    {
        check_nominal_default_round_trips<bl::fdd>(
            0x6d2b79f5a4c381e0ull);
    }

    SECTION("qd")
    {
        check_nominal_default_round_trips<bl::fqd>(
            0x91e73ac46b5d280full);
    }
}

#if FLTX_HAS_STD_FORMAT
TEST_CASE("std format integration supports familiar numeric specifications", "[contracts][io][format]")
{
    CHECK(std::format("{}", bl::fdd{ 1.25 }) == "1.25");
    CHECK(std::format("{:.4g}", bl::fdd{ 1.25 }) == "1.25");
    CHECK(std::format("{:.3f}", bl::fdd{ 1.25 }) == "1.250");
    CHECK(std::format("{:.2f}", bl::fdd_s{ 2.5, 0.0 }) == "2.50");
    CHECK(std::format("{:.2f}", bl::fqd_s{ 2.5, 0.0, 0.0, 0.0 }) == "2.50");
    CHECK(std::format("{:+.2e}", bl::fqd{ 1.5 }) == "+1.50e+00");
    CHECK(std::format("{:+.2E}", bl::fqd{ 1.5 }) == "+1.50E+00");
    CHECK(std::format("{: .2f}", bl::fdd{ 1.25 }) == " 1.25");
    CHECK(std::format("{: .2f}", bl::fdd{ -1.25 }) == "-1.25");
    CHECK(std::format("{:+08.2f}", bl::fdd{ 1.25 }) == "+0001.25");
    CHECK(std::format("{:08.2f}", bl::fdd{ -1.25 }) == "-0001.25");
    CHECK(std::format("{:>8.2f}", bl::fdd{ 1.25 }) == "    1.25");
    CHECK(std::format("{:<8.2f}", bl::fdd{ 1.25 }) == "1.25    ");
    CHECK(std::format("{:*^8.2f}", bl::fdd{ 1.25 }) == "**1.25**");
    CHECK(std::format("{:#.0f}", bl::fdd{ 1.0 }) == "1.");
    CHECK(std::format("{:#.3g}", bl::fdd{ 1.0 }) == "1.00");
    CHECK(std::format("{:a}", bl::fdd{ 1.25 }) == "0x1.4p+0");
    CHECK(std::format("{:.2a}", bl::fdd{ 1.25 }) == "0x1.40p+0");
    CHECK(std::format("{:+#.0A}", bl::fqd{ 1.0 }) == "+0X1.P+0");
    CHECK(std::format("{:>12.1a}", bl::fdd{ 1.25 }) == "    0x1.4p+0");
    CHECK(std::format("{:.2F}", std::numeric_limits<bl::fdd>::infinity()) == "INF");
    CHECK(std::format("{:+.2F}", std::numeric_limits<bl::fdd>::quiet_NaN()) == "+NAN");
    CHECK(std::format("{:+A}", std::numeric_limits<bl::fqd>::infinity()) == "+INF");

    #if !defined(__EMSCRIPTEN__)
    const bl::fdd invalid_format_value{ 1.0 };
    CHECK_THROWS_AS(
        std::vformat("{:x}", std::make_format_args(invalid_format_value)),
        std::format_error);
    #endif
}

TEST_CASE("std format integration preserves long precision and padding",
          "[contracts][io][format]")
{
    const bl::fqd pi = std::numbers::pi_v<bl::fqd>;

    const std::string fixed60 = std::format("{:.60f}", pi);
    CHECK(fixed60.size() == 62u);
    CHECK(fixed60.find('.') == 1u);
    CHECK(fixed60.starts_with(
        "3.14159265358979323846264338327950288419716939937510"));

    const std::string scientific55 = std::format("{:.55e}", pi);
    CHECK(scientific55.size() == 61u);
    CHECK(scientific55.starts_with(
        "3.14159265358979323846264338327950288419716939937510"));
    CHECK(scientific55.ends_with("e+00"));

    const std::string general64 = std::format("{:.64g}", pi);
    CHECK(general64.size() == 65u);
    CHECK(general64.find('.') == 1u);
    CHECK(general64.starts_with(
        "3.14159265358979323846264338327950288419716939937510"));
    CHECK(general64.back() != '0');

    const std::string right80 = std::format("{:>80.60f}", pi);
    CHECK(right80.size() == 80u);
    CHECK(right80 == std::string(80u - fixed60.size(), ' ') + fixed60);

    const std::string left76 = std::format("{:_<76.60f}", pi);
    CHECK(left76.size() == 76u);
    CHECK(left76 == fixed60 + std::string(76u - fixed60.size(), '_'));

    const std::string centered67 = std::format("{:*^67.60f}", pi);
    CHECK(centered67.size() == 67u);
    CHECK(centered67 == std::string(2u, '*') + fixed60 + std::string(3u, '*'));

    CHECK(std::format("{:.10g}", bl::fqd{ 1.25 }) == "1.25");
    CHECK(std::format("{:#.10g}", bl::fqd{ 1.25 }) == "1.250000000");
    CHECK(std::format("{:.0f}", pi) == "3");
    CHECK(std::format("{:#.0f}", pi) == "3.");
}
#endif
