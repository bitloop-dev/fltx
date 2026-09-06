#include <algorithm>
#include <ios>
#include <limits>
#include <string_view>
#include <type_traits>

#include <fltx/traits.h>

namespace
{
    template<class T, class Value, class Storage>
    consteval bool representation_types_match()
    {
        return std::same_as<bl::value_t<T>, Value> &&
               std::same_as<bl::storage_t<T>, Storage> &&
               std::same_as<bl::value_t<const volatile T&>, Value> &&
               std::same_as<bl::storage_t<const volatile T&>, Storage> &&
               std::same_as<bl::value_t<T&&>, Value> &&
               std::same_as<bl::storage_t<T&&>, Storage> &&
               std::same_as<bl::value_t<bl::value_t<T>>, Value> &&
               std::same_as<bl::storage_t<bl::storage_t<T>>, Storage> &&
               std::same_as<bl::value_t<bl::storage_t<T>>, Value> &&
               std::same_as<bl::storage_t<bl::value_t<T>>, Storage>;
    }

    // The traits must work before the scalar definitions are included.
    static_assert(representation_types_match<bl::fdd, bl::fdd, bl::fdd_s>());
    static_assert(representation_types_match<bl::fdd_s, bl::fdd, bl::fdd_s>());
    static_assert(representation_types_match<bl::fqd, bl::fqd, bl::fqd_s>());
    static_assert(representation_types_match<bl::fqd_s, bl::fqd, bl::fqd_s>());
    static_assert(representation_types_match<float, float, float>());
    static_assert(representation_types_match<double, double, double>());
    static_assert(representation_types_match<long double, long double, long double>());
    static_assert(representation_types_match<int, int, int>());
    static_assert(representation_types_match<bool, bool, bool>());
    struct unrelated_type;
    static_assert(representation_types_match<unrelated_type, unrelated_type, unrelated_type>());
    static_assert(representation_types_match<const bl::fdd*, const bl::fdd*, const bl::fdd*>());
    static_assert(std::same_as<bl::value_t<const void>, void>);
    static_assert(std::same_as<bl::storage_t<const void>, void>);
}

#include <fltx.h>

#ifndef FLTX_CXX_STANDARD_CONTRACT
#error FLTX_CXX_STANDARD_CONTRACT must identify the requested C++ standard.
#endif

static_assert(BL_CXX_LANGUAGE_VERSION >= 202002L);

#if defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER)
static_assert(!bl::detail::fp::dekker_product_needs_scaling(0x1p1000, 0.5));
#else
static_assert(bl::detail::fp::dekker_product_needs_scaling(0x1p1000, 0.5));
#endif

#if FLTX_CXX_STANDARD_CONTRACT >= 23
  #if !BL_HAS_IF_CONSTEVAL
    #error C++23 builds must use standard if consteval dispatch.
  #endif
  #if defined(BL_IF_CONSTEVAL_MSVC_CXX20_EXTENSION)
    #error C++23 builds must not select the MSVC C++20 extension path.
  #endif
  #if defined(BL_IF_CONSTEVAL_GCC_CXX20_EXTENSION)
    #error C++23 builds must not select the GCC C++20 extension path.
  #endif
#elif defined(_MSC_VER) && !defined(__clang__)
  #if !defined(BL_IF_CONSTEVAL_MSVC_CXX20_EXTENSION)
    #error MSVC C++20 builds must select the fast if consteval extension path.
  #endif
#elif defined(__GNUC__) && !defined(__clang__)
  #if !defined(BL_IF_CONSTEVAL_GCC_CXX20_EXTENSION)
    #error GCC C++20 builds must select the fast if consteval extension path.
  #endif
#endif

namespace
{
    using qd_product = decltype(std::declval<bl::fqd>() * std::declval<bl::fqd>());

    static_assert(representation_types_match<qd_product, bl::fqd, bl::fqd_s>());
    static_assert(representation_types_match<decltype(std::declval<bl::fdd>() + 1.0), bl::fdd, bl::fdd_s>());

    static_assert(std::is_same_v<decltype(std::declval<bl::fqd_s>() * std::declval<bl::fqd_s>()), bl::fqd_s>);

    template<class T>
    constexpr T add_same_type_pair(const T& x, const T& y) { return x + y; }

    template<class Value, class Integer>
    consteval bool integer_assignment_is_exact()
    {
        constexpr Integer integer = std::numeric_limits<Integer>::max();
        Value value{};
        value = integer;
        if (static_cast<Integer>(value) != integer) return false;
        // Exercise each spelling's compound overload with exact small operands;
        // the runtime core contract owns wide-integer compound arithmetic.
        constexpr Integer two = 2;
        value = two;
        value -= two;
        if (value != Value{ 0.0 }) return false;
        value += two;
        value /= two;
        if (value != Value{ 1.0 }) return false;
        value *= two;
        return value == Value{ 2.0 };
    }

    template<class Value>
    consteval bool integer_spellings_are_supported()
    {
        return integer_assignment_is_exact<Value, long>() &&
               integer_assignment_is_exact<Value, unsigned long>() &&
               integer_assignment_is_exact<Value, long long>() &&
               integer_assignment_is_exact<Value, unsigned long long>();
    }

    static_assert(integer_spellings_are_supported<bl::fdd_s>());
    static_assert(integer_spellings_are_supported<bl::fdd>());
    static_assert(integer_spellings_are_supported<bl::fqd_s>());
    static_assert(integer_spellings_are_supported<bl::fqd>());

    template<class T>
    consteval bool midpoint_extremes_are_constant_evaluated()
    {
        constexpr T maximum = std::numeric_limits<T>::max();
        constexpr T tiny = std::numeric_limits<T>::denorm_min();
        const T half = bl::ldexp(maximum, -1);
        return bl::midpoint(maximum, maximum) == maximum &&
               bl::midpoint(T{ -maximum }, maximum) == T{ 0.0 } &&
               bl::midpoint(tiny, maximum) == half &&
               bl::midpoint(maximum, tiny) == half &&
               bl::midpoint(tiny, tiny) == tiny &&
               bl::midpoint(T{ 0.0 }, tiny) == T{ 0.0 };
    }

    static_assert(midpoint_extremes_are_constant_evaluated<bl::fdd>());
    static_assert(midpoint_extremes_are_constant_evaluated<bl::fqd>());

    template<class Value, class... T>
    consteval bool eager_arithmetic_preserves_value_type()
    {
        return (requires(Value a, const Value c, T b)
        {
            { +a } -> std::same_as<Value>;
            { -a } -> std::same_as<Value>;
            { +c } -> std::same_as<Value>;
            { -c } -> std::same_as<Value>;
            { a + b } -> std::same_as<Value>;
            { b + c } -> std::same_as<Value>;
            { a - b } -> std::same_as<Value>;
            { b - c } -> std::same_as<Value>;
            { a * b } -> std::same_as<Value>;
            { b * c } -> std::same_as<Value>;
            { a / b } -> std::same_as<Value>;
            { b / c } -> std::same_as<Value>;
        } && ...);
    }

    static_assert(eager_arithmetic_preserves_value_type<
        bl::fdd, bl::fdd, bl::fdd_s, float, double,
        bool, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t,
        short, unsigned short, int, unsigned int, long, unsigned long,
        long long, unsigned long long>());
    static_assert(eager_arithmetic_preserves_value_type<bl::fqd, bl::fqd_s, bl::fdd, bl::fdd_s>());

    constexpr bl::fdd dd_a{ 2.0 }, dd_b{ 3.0 }, dd_c{ 4.0 };
    static_assert(std::max(dd_a, dd_b * dd_c) == bl::fdd{ 12.0 });
    static_assert(add_same_type_pair(dd_a * dd_b, dd_b * dd_c + dd_a) == bl::fdd{ 20.0 });
    static_assert(bl::fdd{ 1.0 } + UINT64_C(9007199254740993) == bl::fdd{ 9007199254740994.0 });
    static_assert(bl::fqd{ 1.0 } - bl::fdd{ 1.0, 0x1p-60 } == bl::fqd{ -0x1p-60 });
    static_assert(bl::fdd{ 1.0, 0x1p-60 } - bl::fqd_s{ 1.0 } == bl::fqd{ 0x1p-60 });

    #if !defined(FLTX_ENABLE_FQD_EXPRESSIONS) || !FLTX_ENABLE_FQD_EXPRESSIONS
    static_assert(std::is_same_v<qd_product, bl::fqd>);
    static_assert(eager_arithmetic_preserves_value_type<
        bl::fqd,
        bl::fqd, bl::fqd_s, bl::fdd, bl::fdd_s, float, double,
        bool, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t,
        short, unsigned short, int, unsigned int, long, unsigned long,
        long long, unsigned long long>());

    constexpr bl::fqd eager_a{ 2.0 }, eager_b{ 3.0 }, eager_c{ 4.0 };
    static_assert(std::max(eager_a, eager_b * eager_c) == bl::fqd{ 12.0 });
    static_assert(std::min({ eager_a, eager_b * eager_c, eager_a * eager_b + eager_c }) == eager_a);
    static_assert(add_same_type_pair(eager_a * eager_b, eager_b * eager_c + eager_a) == bl::fqd{ 20.0 });
    static_assert(bl::fqd{ 1.0 } + UINT64_C(9007199254740993) == bl::fqd{ 9007199254740994.0 });
    #else
    static_assert(bl::fltx_expression<qd_product>);
    #endif

    template<class T>
    consteval bool core_is_constant_evaluated()
    {
        T value{ 1.25 };
        value += T{ 0.75 };
        value *= T{ 3.0 };
        value -= T{ 1.0 };
        value /= T{ 2.0 };

        return value == T{ 2.5 } &&
               -value < T{ 0.0 } &&
               bl::clamp(value, T{ 0.0 }, T{ 2.0 }) == T{ 2.0 } &&
               bl::isfinite(value) &&
               bl::isnormal(value) &&
               !bl::isnan(value) &&
               !bl::signbit(value) &&
               bl::recip(T{ 4.0 }) == T{ 0.25 };
    }

    template<class T>
    consteval bool math_is_constant_evaluated()
    {
        T integral{};
        int exponent = 0;
        int quotient = 0;
        T sine{};
        T cosine{};

        const T fraction = bl::modf(T{ 3.25 }, &integral);
        const T mantissa = bl::frexp(T{ 8.0 }, &exponent);
        const T remainder = bl::remquo(T{ 5.0 }, T{ 2.0 }, &quotient);
        const T exp_log = bl::exp(bl::log(T{ 1.25 }));
        const bool sincos_ok = bl::sincos(T{ 0.0 }, sine, cosine);

        return bl::sqrt(T{ 4.0 }) == T{ 2.0 } &&
               exp_log > T{ 1.249 } && exp_log < T{ 1.251 } &&
               bl::floor(T{ -2.25 }) == T{ -3.0 } &&
               bl::ceil(T{ -2.25 }) == T{ -2.0 } &&
               bl::trunc(T{ -2.75 }) == T{ -2.0 } &&
               bl::roundeven(T{ 2.5 }) == T{ 2.0 } &&
               fraction == T{ 0.25 } && integral == T{ 3.0 } &&
               mantissa == T{ 0.5 } && exponent == 4 &&
               remainder == T{ 1.0 } && quotient != 0 &&
               bl::ipow(T{ 2.0 }, 10) == T{ 1024.0 } &&
               bl::sqr(T{ 2.0 } * T{ 2.0 }) == T{ 16.0 } &&
               bl::ipow(T{ 2.0 } * T{ 2.0 }, -1) == T{ 0.25 } &&
               bl::approx_eq(T{ 2.0 } * T{ 2.0 }, T{ 4.0 }) &&
               bl::min(T{ 2.0 } * T{ 2.0 }, 5) == T{ 4.0 } &&
               bl::max(5, T{ 2.0 } * T{ 2.0 }) == T{ 5.0 } &&
               bl::minmax(T{ 2.0 } * T{ 2.0 }, T{ 5.0 }).second == T{ 5.0 } &&
               bl::clamp(T{ 2.0 } * T{ 2.0 }, 0, 3) == T{ 3.0 } &&
               bl::lerp(T{ 2.0 } * T{ 2.0 }, T{ 5.0 }, 0.5) == T{ 4.5 } &&
               bl::midpoint(T{ 2.0 } * T{ 2.0 }, T{ 5.0 }) == T{ 4.5 } &&
               sincos_ok && sine == T{ 0.0 } && cosine == T{ 1.0 };
    }

    template<class T>
    consteval bool io_is_constant_evaluated()
    {
        const auto parsed = bl::try_parse<T>("12.5");
        const auto rejected = bl::try_parse<T>("12.5tail");
        const T fallback = bl::parse<T>("bad", T{ 7.0 });
        const auto text = bl::to_static_string(
            T{ 12.5 },
            1,
            std::ios_base::fixed);

        return parsed && parsed.consumed == 4 && parsed.value == T{ 12.5 } &&
               !rejected && rejected.consumed == 4 &&
               fallback == T{ 7.0 } &&
               text.view() == std::string_view{ "12.5" };
    }

    template<class T>
    consteval bool random_is_constant_evaluated()
    {
        const auto low = T{ -1.0 } * T{ 2.0 };
        const auto high = T{ 1.0 } * T{ 2.0 } + T{ 1.0 };
        const auto first = bl::uniform_real_array<4>(
            low,
            high,
            bl::mt19937_64{ 0x1020304050607080ull });
        const auto second = bl::random_array<4>(
            bl::mt19937_64{ 0x1020304050607080ull },
            bl::uniform_real_distribution<T>{ low, high });

        bl::mt19937_64 distribution_engine{ 777u };
        bl::exponential_distribution<T> exponential{ T{ 0.75 } };
        bl::normal_distribution<T> normal{ T{ 1.0 }, T{ 0.5 } };
        bl::lognormal_distribution<T> lognormal{ T{ 0.0 }, T{ 0.5 } };
        const T exponential_sample = exponential(distribution_engine);
        const T normal_sample = normal(distribution_engine);
        const T lognormal_sample = lognormal(distribution_engine);

        return first == second &&
               first.front() >= T{ -2.0 } && first.front() < T{ 3.0 } &&
               first.back() >= T{ -2.0 } && first.back() < T{ 3.0 } &&
               exponential_sample >= T{ 0.0 } &&
               bl::isfinite(normal_sample) &&
               lognormal_sample > T{ 0.0 };
    }

    using namespace bl::literals;

    constexpr bl::fdd dd_literal = "1.25"_dd;
    constexpr bl::fqd qd_literal = "1.25"_qd;
    constexpr double native_pow = bl::pow(4.0, 1.5);
    constexpr bl::fqd expression_value{
        qd_literal * bl::fqd{ 2.0 } + bl::fqd{ 0.5 }
    };
    constexpr auto expression_distribution = bl::uniform_real_distribution{
        qd_literal * bl::fqd{ 2.0 }, qd_literal * bl::fqd{ 2.0 } + bl::fqd{ 1.0 } };

    static_assert(dd_literal == bl::fdd{ 1.25 });
    static_assert(qd_literal == bl::fqd{ 1.25 });
    static_assert(native_pow > 7.999999999999 && native_pow < 8.000000000001);
    static_assert(expression_value == bl::fqd{ 3.0 });
    static_assert(bl::min({ qd_literal, qd_literal * qd_literal,
                           qd_literal * qd_literal + 1.0 }) == qd_literal);
    static_assert(bl::max({ qd_literal, qd_literal * qd_literal,
                           qd_literal * qd_literal + 1.0 }) == bl::fqd{ 2.5625 });
    static_assert(expression_distribution.a() == bl::fqd{ 2.5 });
    static_assert(expression_distribution.b() == bl::fqd{ 3.5 });
    static_assert(core_is_constant_evaluated<bl::fdd>());
    static_assert(core_is_constant_evaluated<bl::fqd>());
    static_assert(math_is_constant_evaluated<bl::fdd>());
    static_assert(math_is_constant_evaluated<bl::fqd>());
    static_assert(io_is_constant_evaluated<bl::f32>());
    static_assert(io_is_constant_evaluated<bl::f64>());
    static_assert(io_is_constant_evaluated<bl::fdd>());
    static_assert(io_is_constant_evaluated<bl::fqd>());
    static_assert(random_is_constant_evaluated<bl::fdd>());
    static_assert(random_is_constant_evaluated<bl::fqd>());

    template<class T, bool Add>
    int dispatch_probe(int value)
    {
        if constexpr (Add)
            return value + bl::fltx_precision_rank_v<T>;
        return value - bl::fltx_precision_rank_v<T>;
    }

    [[maybe_unused]] void instantiate_runtime_and_dispatch_contracts(
        bl::fdd dd,
        bl::fqd qd)
    {
        const auto table = bl_dispatch_table(dispatch_probe, 10);
        (void)bl_table_invoke(
            table,
            bl_enum_type(bl::FloatType::FQD),
            true);

        // These calls must remain valid thin wrappers around compiled runtime
        // implementations when they are not constant-evaluated.
        (void)bl::sin(dd);
        (void)bl::sin(qd);
        #if FLTX_HAS_STD_FORMAT
        (void)std::format("{:.20g}", qd * qd + qd);
        #endif
    }
}
