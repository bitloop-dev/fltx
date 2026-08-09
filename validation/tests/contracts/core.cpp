#include <catch2/catch_test_macros.hpp>

#include <array>
#include <compare>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <fltx.h>

namespace
{
    template<class T, class Scalar>
    void check_scalar_arithmetic(Scalar scalar)
    {
        const T six{ 6.0 };
        const T one_third{ T{ 2.0 } / T{ 6.0 } };

        CHECK(T{ six + scalar } == T{ 8.0 });
        CHECK(T{ six - scalar } == T{ 4.0 });
        CHECK(T{ six * scalar } == T{ 12.0 });
        CHECK(T{ six / scalar } == T{ 3.0 });
        CHECK(T{ scalar + six } == T{ 8.0 });
        CHECK(T{ scalar - six } == T{ -4.0 });
        CHECK(T{ scalar * six } == T{ 12.0 });
        CHECK(T{ scalar / six } == one_third);

        T value{ 6.0 };
        CHECK(&(value += scalar) == &value);
        CHECK(value == T{ 8.0 });
        CHECK(&(value -= scalar) == &value);
        CHECK(value == T{ 6.0 });
        CHECK(&(value *= scalar) == &value);
        CHECK(value == T{ 12.0 });
        CHECK(&(value /= scalar) == &value);
        CHECK(value == T{ 6.0 });
    }

    template<class T, class Integer>
    void check_wide_integer_arithmetic(Integer integer)
    {
        T exact{};
        exact = integer;
        const T zero{ 0.0 };
        const T one{ 1.0 };

        CHECK(T{ zero + integer } == exact);
        CHECK(T{ integer + zero } == exact);
        CHECK(T{ exact - integer } == zero);
        CHECK(T{ integer - exact } == zero);
        CHECK(T{ one * integer } == exact);
        CHECK(T{ integer * one } == exact);
        CHECK(T{ exact / integer } == one);
        CHECK(T{ integer / exact } == one);

        T value{ 0.0 };
        value += integer;
        CHECK(value == exact);
        value -= integer;
        CHECK(value == zero);
        value = 1.0;
        value *= integer;
        CHECK(value == exact);
        value /= integer;
        CHECK(value == one);
    }

    template<class T>
    void check_core_type()
    {
        const T one{ 1.0 };
        const T half{ 0.5 };

        const T sum = one + half;
        const T difference = one - half;
        const T product = one * half;
        const T quotient = one / half;
        CHECK(sum == T{ 1.5 });
        CHECK(difference == T{ 0.5 });
        CHECK(product == T{ 0.5 });
        CHECK(quotient == T{ 2.0 });
        CHECK(-one == T{ -1.0 });

        T value = one;
        value += half;
        value -= half;
        value *= T{ 2.0 };
        value /= T{ 2.0 };
        CHECK(value == one);

        CHECK(T{ 1.0 } == 1);
        CHECK(T{ 1.0 } < 2.0);
        CHECK(0.0 < T{ 1.0 });
        CHECK((T{ 1.0 } <=> T{ 2.0 }) == std::partial_ordering::less);
    }

    template<class T>
    void check_ordered_predicates()
    {
        const T one{ 1.0 };
        const T two{ 2.0 };
        const T nan = std::numeric_limits<T>::quiet_NaN();

        CHECK(bl::ispositive(one));
        CHECK_FALSE(bl::ispositive(T{ -1.0 }));
        CHECK_FALSE(bl::ispositive(T{ -0.0 }));
        CHECK(bl::isless(one, two));
        CHECK(bl::islessequal(one, one));
        CHECK(bl::isgreater(two, one));
        CHECK(bl::isgreaterequal(two, two));
        CHECK(bl::islessgreater(one, two));
        CHECK_FALSE(bl::islessgreater(one, one));

        CHECK_FALSE(bl::isless(nan, one));
        CHECK_FALSE(bl::islessequal(nan, one));
        CHECK_FALSE(bl::isgreater(nan, one));
        CHECK_FALSE(bl::isgreaterequal(nan, one));
        CHECK_FALSE(bl::islessgreater(nan, one));
        CHECK(bl::isunordered(nan, one));
        CHECK_FALSE(bl::isless(one, nan));
        CHECK_FALSE(bl::islessequal(one, nan));
        CHECK_FALSE(bl::isgreater(one, nan));
        CHECK_FALSE(bl::isgreaterequal(one, nan));
        CHECK_FALSE(bl::islessgreater(one, nan));
        CHECK(bl::isunordered(one, nan));
        CHECK_FALSE(bl::ispositive(nan));
    }

    template<class T>
    void check_classification()
    {
        const T zero{ 0.0 };
        const T negative_zero{ -0.0 };
        const T infinity = std::numeric_limits<T>::infinity();
        const T nan = std::numeric_limits<T>::quiet_NaN();
        const T subnormal = std::numeric_limits<T>::denorm_min();

        CHECK(bl::iszero(zero));
        CHECK(bl::iszero(negative_zero));
        CHECK(bl::signbit(negative_zero));
        CHECK(bl::isfinite(T{ 1.0 }));
        CHECK(bl::isnormal(T{ 1.0 }));
        CHECK(bl::isinf(infinity));
        CHECK(bl::isnan(nan));
        CHECK(bl::isunordered(nan, T{ 1.0 }));
        if constexpr (std::same_as<T, bl::f128> || std::same_as<T, bl::f256>)
        {
            CHECK_FALSE(nan == nan);
            CHECK((nan <=> T{ 1.0 }) == std::partial_ordering::unordered);
        }
        CHECK(bl::fpclassify(zero) == FP_ZERO);
        CHECK(bl::fpclassify(infinity) == FP_INFINITE);
        CHECK(bl::fpclassify(nan) == FP_NAN);
        CHECK(bl::fpclassify(subnormal) == FP_SUBNORMAL);
        CHECK_FALSE(bl::isnormal(subnormal));
    }

    template<class T>
    void check_number_constants()
    {
        constexpr std::array constants{
            std::numbers::e_v<T>,
            std::numbers::log2e_v<T>,
            std::numbers::log10e_v<T>,
            std::numbers::pi_v<T>,
            std::numbers::inv_pi_v<T>,
            std::numbers::inv_sqrtpi_v<T>,
            std::numbers::ln2_v<T>,
            std::numbers::ln10_v<T>,
            std::numbers::sqrt2_v<T>,
            std::numbers::sqrt3_v<T>,
            std::numbers::inv_sqrt3_v<T>,
            std::numbers::egamma_v<T>,
            std::numbers::phi_v<T>
        };

        for (const T value : constants)
        {
            CHECK(bl::isfinite(value));
            CHECK(value > T{ 0.0 });
        }

        CHECK(std::numbers::e_v<T> > T{ 2.7 });
        CHECK(std::numbers::e_v<T> < T{ 2.8 });
        CHECK(std::numbers::pi_v<T> > T{ 3.14 });
        CHECK(std::numbers::pi_v<T> < T{ 3.15 });
        CHECK(std::numbers::ln2_v<T> > T{ 0.69 });
        CHECK(std::numbers::ln2_v<T> < T{ 0.70 });
    }

    template<class T>
    void check_numeric_limits(
        int digits,
        int digits10,
        int max_digits10,
        int min_exponent,
        int min_exponent10)
    {
        using limits = std::numeric_limits<T>;

        static_assert(std::same_as<decltype(limits::min()), T>);
        static_assert(std::same_as<decltype(limits::max()), T>);
        static_assert(std::same_as<decltype(limits::lowest()), T>);
        static_assert(std::same_as<decltype(limits::epsilon()), T>);
        static_assert(std::same_as<decltype(limits::round_error()), T>);
        static_assert(std::same_as<decltype(limits::infinity()), T>);
        static_assert(std::same_as<decltype(limits::quiet_NaN()), T>);
        static_assert(std::same_as<decltype(limits::signaling_NaN()), T>);
        static_assert(std::same_as<decltype(limits::denorm_min()), T>);

        CHECK(limits::is_specialized);
        CHECK(limits::digits == digits);
        CHECK(limits::digits10 == digits10);
        CHECK(limits::max_digits10 == max_digits10);
        CHECK(limits::is_signed);
        CHECK_FALSE(limits::is_integer);
        CHECK_FALSE(limits::is_exact);
        CHECK(limits::radix == 2);
        CHECK(limits::min_exponent == min_exponent);
        CHECK(limits::max_exponent == std::numeric_limits<double>::max_exponent);
        CHECK(limits::min_exponent10 == min_exponent10);
        CHECK(limits::max_exponent10 == std::numeric_limits<double>::max_exponent10);
        CHECK_FALSE(limits::is_iec559);
        CHECK(limits::is_bounded);
        CHECK_FALSE(limits::is_modulo);
        CHECK_FALSE(limits::traps);
        CHECK_FALSE(limits::tinyness_before);
        CHECK(limits::round_style == std::round_to_nearest);

        CHECK(limits::min() > T{ 0.0 });
        CHECK(bl::fpclassify(limits::min()) == FP_NORMAL);
        CHECK(bl::isfinite(limits::max()));
        CHECK(limits::lowest() == -limits::max());
        CHECK(limits::epsilon() > T{ 0.0 });
        CHECK(limits::round_error() == T{ 0.5 });
        CHECK(limits::has_infinity);
        CHECK(limits::has_quiet_NaN);
        CHECK(limits::has_signaling_NaN);
        CHECK(limits::has_denorm == std::denorm_present);
        CHECK_FALSE(limits::has_denorm_loss);
        CHECK(bl::isinf(limits::infinity()));
        CHECK(bl::isnan(limits::quiet_NaN()));
        CHECK(bl::isnan(limits::signaling_NaN()));
        CHECK(bl::fpclassify(limits::denorm_min()) == FP_SUBNORMAL);
    }

    template<class Value, class Storage>
    void check_wrapper_numeric_limits()
    {
        using value_limits = std::numeric_limits<Value>;
        using storage_limits = std::numeric_limits<Storage>;

        STATIC_CHECK(value_limits::is_specialized == storage_limits::is_specialized);
        STATIC_CHECK(value_limits::digits == storage_limits::digits);
        STATIC_CHECK(value_limits::digits10 == storage_limits::digits10);
        STATIC_CHECK(value_limits::max_digits10 == storage_limits::max_digits10);
        STATIC_CHECK(value_limits::is_signed == storage_limits::is_signed);
        STATIC_CHECK(value_limits::is_integer == storage_limits::is_integer);
        STATIC_CHECK(value_limits::is_exact == storage_limits::is_exact);
        STATIC_CHECK(value_limits::radix == storage_limits::radix);
        STATIC_CHECK(value_limits::min_exponent == storage_limits::min_exponent);
        STATIC_CHECK(value_limits::max_exponent == storage_limits::max_exponent);
        STATIC_CHECK(value_limits::min_exponent10 == storage_limits::min_exponent10);
        STATIC_CHECK(value_limits::max_exponent10 == storage_limits::max_exponent10);
        STATIC_CHECK(value_limits::has_denorm == storage_limits::has_denorm);
        STATIC_CHECK(value_limits::has_denorm_loss == storage_limits::has_denorm_loss);
        STATIC_CHECK(value_limits::is_iec559 == storage_limits::is_iec559);
        STATIC_CHECK(value_limits::is_bounded == storage_limits::is_bounded);
        STATIC_CHECK(value_limits::is_modulo == storage_limits::is_modulo);
        STATIC_CHECK(value_limits::traps == storage_limits::traps);
        STATIC_CHECK(value_limits::tinyness_before == storage_limits::tinyness_before);
        STATIC_CHECK(value_limits::round_style == storage_limits::round_style);

        STATIC_CHECK(value_limits::min() == Value{ storage_limits::min() });
        STATIC_CHECK(value_limits::max() == Value{ storage_limits::max() });
        STATIC_CHECK(value_limits::lowest() == Value{ storage_limits::lowest() });
        STATIC_CHECK(value_limits::epsilon() == Value{ storage_limits::epsilon() });
        STATIC_CHECK(value_limits::round_error() == Value{ storage_limits::round_error() });
        STATIC_CHECK(value_limits::infinity() == Value{ storage_limits::infinity() });
        STATIC_CHECK(value_limits::denorm_min() == Value{ storage_limits::denorm_min() });
        CHECK(bl::isnan(value_limits::quiet_NaN()));
        CHECK(bl::isnan(value_limits::signaling_NaN()));
    }

    template<class Value, class Storage>
    void check_wrapper_number_constants()
    {
        STATIC_CHECK(std::numbers::e_v<Value> == Value{ std::numbers::e_v<Storage> });
        STATIC_CHECK(std::numbers::log2e_v<Value> == Value{ std::numbers::log2e_v<Storage> });
        STATIC_CHECK(std::numbers::log10e_v<Value> == Value{ std::numbers::log10e_v<Storage> });
        STATIC_CHECK(std::numbers::pi_v<Value> == Value{ std::numbers::pi_v<Storage> });
        STATIC_CHECK(std::numbers::inv_pi_v<Value> == Value{ std::numbers::inv_pi_v<Storage> });
        STATIC_CHECK(std::numbers::inv_sqrtpi_v<Value> == Value{ std::numbers::inv_sqrtpi_v<Storage> });
        STATIC_CHECK(std::numbers::ln2_v<Value> == Value{ std::numbers::ln2_v<Storage> });
        STATIC_CHECK(std::numbers::ln10_v<Value> == Value{ std::numbers::ln10_v<Storage> });
        STATIC_CHECK(std::numbers::sqrt2_v<Value> == Value{ std::numbers::sqrt2_v<Storage> });
        STATIC_CHECK(std::numbers::sqrt3_v<Value> == Value{ std::numbers::sqrt3_v<Storage> });
        STATIC_CHECK(std::numbers::inv_sqrt3_v<Value> == Value{ std::numbers::inv_sqrt3_v<Storage> });
        STATIC_CHECK(std::numbers::egamma_v<Value> == Value{ std::numbers::egamma_v<Storage> });
        STATIC_CHECK(std::numbers::phi_v<Value> == Value{ std::numbers::phi_v<Storage> });
    }

    template<class T>
    [[nodiscard]] std::size_t hash_value(const T& value) noexcept
    {
        return std::hash<T>{}(value);
    }

    template<class T>
    void check_special_value_hashes()
    {
        const T infinity = std::numeric_limits<T>::infinity();
        const T nan = std::numeric_limits<T>::quiet_NaN();
        CHECK(hash_value(infinity) == hash_value(infinity));
        CHECK(hash_value(nan) == hash_value(nan));
    }
}

using namespace bl::int_literals;

static_assert(std::same_as<bl::i8, std::int8_t>);
static_assert(std::same_as<bl::i16, std::int16_t>);
static_assert(std::same_as<bl::i32, std::int32_t>);
static_assert(std::same_as<bl::i64, std::int64_t>);
static_assert(std::same_as<bl::u8, std::uint8_t>);
static_assert(std::same_as<bl::u16, std::uint16_t>);
static_assert(std::same_as<bl::u32, std::uint32_t>);
static_assert(std::same_as<bl::u64, std::uint64_t>);
static_assert(std::same_as<decltype(1_u8), std::uint8_t>);
static_assert(std::same_as<decltype(1_i8), std::int8_t>);
static_assert(std::same_as<decltype(1_u16), std::uint16_t>);
static_assert(std::same_as<decltype(1_i16), std::int16_t>);
static_assert(std::same_as<decltype(1_u32), std::uint32_t>);
static_assert(std::same_as<decltype(1_i32), std::int32_t>);
static_assert(std::same_as<decltype(1_u64), std::uint64_t>);
static_assert(std::same_as<decltype(1_i64), std::int64_t>);
static_assert(255_u8 == std::uint8_t{ 255 });
static_assert(127_i8 == std::int8_t{ 127 });
static_assert(65535_u16 == std::uint16_t{ 65535 });
static_assert(32767_i16 == std::int16_t{ 32767 });
static_assert(123456_u32 == std::uint32_t{ 123456 });
static_assert(123456_i32 == std::int32_t{ 123456 });
static_assert(123456789_u64 == std::uint64_t{ 123456789 });
static_assert(123456789_i64 == std::int64_t{ 123456789 });

static_assert(bl::fltx_f32<bl::f32>);
static_assert(bl::fltx_f64<bl::f64>);
static_assert(bl::fltx_f128<bl::f128>);
static_assert(bl::fltx_f128<bl::f128_s>);
static_assert(bl::fltx_f256<bl::f256>);
static_assert(bl::fltx_f256<bl::f256_s>);
static_assert(bl::fltx_float<const bl::f128>);
static_assert(bl::fltx_extended_float<volatile bl::f256_s>);
static_assert(bl::fltx_floating_point<long double>);
static_assert(bl::fltx_arithmetic<std::uint64_t>);
static_assert(!bl::fltx_float<long double>);
static_assert(!bl::fltx_extended_float<double>);
static_assert(bl::is_f32_v<const float>);
static_assert(bl::is_f64_v<const double>);
static_assert(bl::is_f128_v<const bl::f128_s>);
static_assert(bl::is_f256_v<const bl::f256>);
static_assert(bl::is_fltx_extended_float_v<bl::f128>);
static_assert(bl::is_fltx_float_v<bl::f256>);
static_assert(bl::is_floating_point_v<long double>);
static_assert(bl::is_arithmetic_v<std::int32_t>);
static_assert(bl::is_integral_v<std::uint16_t>);
static_assert(bl::fltx_precision_rank_v<float> == 1);
static_assert(bl::fltx_precision_rank_v<double> == 2);
static_assert(bl::fltx_precision_rank_v<long double> == 3);
static_assert(bl::fltx_precision_rank_v<bl::f128> == 4);
static_assert(bl::fltx_precision_rank_v<bl::f256> == 5);
static_assert(std::same_as<bl::common_float_type_t<float, double>, double>);
static_assert(std::same_as<bl::common_float_type_t<double, bl::f128>, bl::f128>);
static_assert(std::same_as<bl::common_float_type_t<bl::f128, bl::f256>, bl::f256>);
static_assert(std::same_as<bl::common_float_type_t<float, int>, double>);
static_assert(std::same_as<bl::common_float_type_t<double, long double>, long double>);
static_assert(std::same_as<bl::common_float_type_t<int, bl::f128, bl::f256>, bl::f256>);
static_assert(static_cast<int>(bl::FloatType::COUNT) == 4);
static_assert(bl::to_string(bl::FloatType::COUNT) == "unknown");
static_assert(bl::to_string(static_cast<bl::FloatType>(99)) == "unknown");

TEST_CASE("extended types preserve arithmetic and conversion contracts", "[contracts][core]")
{
    check_core_type<bl::f32>();
    check_core_type<bl::f64>();
    check_core_type<bl::f128>();
    check_core_type<bl::f256>();

    constexpr std::int64_t exact = (std::int64_t{ 1 } << 60) + 3;
    CHECK(static_cast<std::int64_t>(static_cast<double>(bl::f128{ exact })) != exact);
    CHECK(bl::f128{ exact } == exact);
    CHECK(bl::f256{ exact } == exact);

    const bl::f128 dd{ 1.0, 0x1p-60 };
    const bl::f256 qd{ dd };
    CHECK(static_cast<bl::f128>(qd) == dd);
    CHECK(static_cast<double>(bl::f128{ 1.5 }) == 1.5);
    CHECK(static_cast<float>(bl::f256{ 1.5 }) == 1.5f);
    CHECK(static_cast<int>(bl::f128_s{ 7.75 }) == 7);

    bl::f128_s assigned128{};
    assigned128 = exact;
    CHECK(assigned128 == exact);
    assigned128 = std::uint32_t{ 42 };
    CHECK(assigned128 == 42);

    bl::f256_s assigned256{};
    assigned256 = static_cast<std::uint64_t>(exact);
    CHECK(assigned256 == exact);
    assigned256 = std::int16_t{ -42 };
    CHECK(assigned256 == -42);

    assigned128 = qd;
    CHECK(assigned128 == dd);
    assigned256 = dd;
    CHECK(assigned256 == qd);
}

TEST_CASE("scalar and cross-precision arithmetic exercise each overload family", "[contracts][core]")
{
    check_scalar_arithmetic<bl::f128_s>(2.0f);
    check_scalar_arithmetic<bl::f128_s>(2.0);
    check_scalar_arithmetic<bl::f128_s>(std::int32_t{ 2 });
    check_scalar_arithmetic<bl::f128_s>(std::uint64_t{ 2 });
    check_scalar_arithmetic<bl::f128>(std::int64_t{ 2 });

    check_scalar_arithmetic<bl::f256_s>(2.0f);
    check_scalar_arithmetic<bl::f256_s>(2.0);
    check_scalar_arithmetic<bl::f256_s>(std::int32_t{ 2 });
    check_scalar_arithmetic<bl::f256_s>(std::uint64_t{ 2 });
    check_scalar_arithmetic<bl::f256>(std::int64_t{ 2 });

    constexpr std::int64_t signed_wide = (std::int64_t{ 1 } << 60) + 3;
    constexpr std::uint64_t unsigned_wide = (std::uint64_t{ 1 } << 63) + 5;
    check_wide_integer_arithmetic<bl::f128_s>(signed_wide);
    check_wide_integer_arithmetic<bl::f128_s>(unsigned_wide);
    check_wide_integer_arithmetic<bl::f256_s>(signed_wide);
    check_wide_integer_arithmetic<bl::f256_s>(unsigned_wide);

    const bl::f128_s narrow{ 2.0 };
    const bl::f256_s wide{ 6.0 };
    CHECK(wide + narrow == bl::f256_s{ 8.0 });
    CHECK(wide - narrow == bl::f256_s{ 4.0 });
    CHECK(wide * narrow == bl::f256_s{ 12.0 });
    CHECK(wide / narrow == bl::f256_s{ 3.0 });
    CHECK(narrow + wide == bl::f256_s{ 8.0 });
    CHECK(narrow - wide == bl::f256_s{ -4.0 });
    CHECK(narrow * wide == bl::f256_s{ 12.0 });
    CHECK(narrow / wide == bl::f256_s{ 2.0 } / bl::f256_s{ 6.0 });

    bl::f256_s compound{ 6.0 };
    CHECK(&(compound += narrow) == &compound);
    CHECK(compound == bl::f256_s{ 8.0 });
    CHECK(&(compound -= narrow) == &compound);
    CHECK(compound == bl::f256_s{ 6.0 });
    CHECK(&(compound *= narrow) == &compound);
    CHECK(compound == bl::f256_s{ 12.0 });
    CHECK(&(compound /= narrow) == &compound);
    CHECK(compound == bl::f256_s{ 6.0 });
}

TEST_CASE("classification and ordering handle IEEE special values", "[contracts][core]")
{
    check_classification<bl::f32>();
    check_classification<bl::f64>();
    check_classification<bl::f128>();
    check_classification<bl::f256>();
    check_ordered_predicates<bl::f128>();
    check_ordered_predicates<bl::f256>();
}

TEST_CASE("limits, constants, and traits describe both expansion types", "[contracts][core]")
{
    STATIC_CHECK(bl::f128_s::eps() == std::numeric_limits<bl::f128_s>::epsilon());
    STATIC_CHECK(bl::f256_s::eps() == std::numeric_limits<bl::f256_s>::epsilon());
    STATIC_CHECK(std::numeric_limits<bl::f128_s>::highest() ==
                 std::numeric_limits<bl::f128_s>::max());
    STATIC_CHECK(std::numeric_limits<bl::f256_s>::highest() ==
                 std::numeric_limits<bl::f256_s>::max());
    STATIC_CHECK(std::numeric_limits<bl::f128_s>::epsilon() ==
                 bl::f128_s{ 0x1p-105, 0.0 });
    STATIC_CHECK(std::numeric_limits<bl::f256_s>::epsilon() ==
                 bl::f256_s{ 0x1p-211, 0.0, 0.0, 0.0 });
    STATIC_CHECK(std::numeric_limits<bl::f128_s>::min() ==
                 bl::f128_s{ 0x1p-969, 0.0 });
    STATIC_CHECK(std::numeric_limits<bl::f256_s>::min() ==
                 bl::f256_s{ 0x1p-863, 0.0, 0.0, 0.0 });
    STATIC_CHECK(std::numeric_limits<bl::f128_s>::max() == bl::f128_s{
        0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+969 });
    STATIC_CHECK(std::numeric_limits<bl::f256_s>::max() == bl::f256_s{
        0x1.fffffffffffffp+1023,
        0x1.fffffffffffffp+969,
        0x1.fffffffffffffp+915,
        0x1.fffffffffffffp+861 });
    STATIC_CHECK(bl::to_string(bl::FloatType::F32) == "f32");
    STATIC_CHECK(bl::to_string(bl::FloatType::F64) == "f64");
    STATIC_CHECK(bl::to_string(bl::FloatType::F128) == "f128");
    STATIC_CHECK(bl::to_string(bl::FloatType::F256) == "f256");

    check_numeric_limits<bl::f128_s>(106, 31, 33, -968, -291);
    check_numeric_limits<bl::f128>(106, 31, 33, -968, -291);
    check_numeric_limits<bl::f256_s>(212, 63, 65, -862, -259);
    check_numeric_limits<bl::f256>(212, 63, 65, -862, -259);
    check_wrapper_numeric_limits<bl::f128, bl::f128_s>();
    check_wrapper_numeric_limits<bl::f256, bl::f256_s>();

    check_number_constants<bl::f128_s>();
    check_number_constants<bl::f128>();
    check_number_constants<bl::f256_s>();
    check_number_constants<bl::f256>();
    check_wrapper_number_constants<bl::f128, bl::f128_s>();
    check_wrapper_number_constants<bl::f256, bl::f256_s>();
}

TEST_CASE("hashing follows equality and includes every limb", "[contracts][hash]")
{
    const bl::f128_s f128_zero_a{ 0.0, -0.0 };
    const bl::f128_s f128_zero_b{ -0.0, 0.0 };
    CHECK(f128_zero_a == f128_zero_b);
    CHECK(hash_value(f128_zero_a) == hash_value(f128_zero_b));
    CHECK(hash_value(bl::f128{ f128_zero_a }) == hash_value(bl::f128{ f128_zero_b }));

    const bl::f256_s f256_zero_a{ 0.0, -0.0, 0.0, -0.0 };
    const bl::f256_s f256_zero_b{ -0.0, 0.0, -0.0, 0.0 };
    CHECK(f256_zero_a == f256_zero_b);
    CHECK(hash_value(f256_zero_a) == hash_value(f256_zero_b));
    CHECK(hash_value(bl::f256{ f256_zero_a }) == hash_value(bl::f256{ f256_zero_b }));

    const bl::f256_s base{ 1.0, 0.0, 0.0, 0.0 };
    CHECK(hash_value(base) != hash_value(bl::f256_s{ 1.0, 0x1p-80, 0.0, 0.0 }));
    CHECK(hash_value(base) != hash_value(bl::f256_s{ 1.0, 0.0, 0x1p-160, 0.0 }));
    CHECK(hash_value(base) != hash_value(bl::f256_s{ 1.0, 0.0, 0.0, 0x1p-240 }));

    const bl::f128_s base128{ 1.0, 0.0 };
    CHECK(hash_value(base128) != hash_value(bl::f128_s{ 1.0, 0x1p-80 }));
    CHECK(hash_value(bl::f128{ base128 }) == hash_value(base128));
    CHECK(hash_value(bl::f256{ base }) == hash_value(base));

    STATIC_CHECK(noexcept(std::hash<bl::f128_s>{}(std::declval<const bl::f128_s&>())));
    STATIC_CHECK(noexcept(std::hash<bl::f128>{}(std::declval<const bl::f128&>())));
    STATIC_CHECK(noexcept(std::hash<bl::f256_s>{}(std::declval<const bl::f256_s&>())));
    STATIC_CHECK(noexcept(std::hash<bl::f256>{}(std::declval<const bl::f256&>())));
    static_assert(std::same_as<decltype(hash_value(bl::f128_s{})), std::size_t>);
    static_assert(std::same_as<decltype(hash_value(bl::f128{})), std::size_t>);
    static_assert(std::same_as<decltype(hash_value(bl::f256_s{})), std::size_t>);
    static_assert(std::same_as<decltype(hash_value(bl::f256{})), std::size_t>);

    std::unordered_map<bl::f128_s, int> f128_storage_values;
    f128_storage_values.emplace(bl::f128_s{ 3.0, 0x1p-82 }, 128);
    CHECK(f128_storage_values.at(bl::f128_s{ 3.0, 0x1p-82 }) == 128);

    std::unordered_map<bl::f128, std::string_view> f128_values;
    f128_values.emplace(bl::f128{ -3.0, 0x1p-82 }, "f128");
    CHECK(f128_values.at(bl::f128{ -3.0, 0x1p-82 }) == "f128");

    std::unordered_set<bl::f256_s> f256_storage_values;
    f256_storage_values.emplace(5.0, 0x1p-80, -0x1p-160, 0x1p-240);
    CHECK(f256_storage_values.contains(
        bl::f256_s{ 5.0, 0x1p-80, -0x1p-160, 0x1p-240 }));

    std::unordered_set<bl::f256> f256_values;
    f256_values.emplace(-5.0, 0x1p-80, -0x1p-160, 0x1p-240);
    CHECK(f256_values.contains(
        bl::f256{ -5.0, 0x1p-80, -0x1p-160, 0x1p-240 }));

    check_special_value_hashes<bl::f128_s>();
    check_special_value_hashes<bl::f128>();
    check_special_value_hashes<bl::f256_s>();
    check_special_value_hashes<bl::f256>();
}
