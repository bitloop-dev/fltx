#include <ios>
#include <limits>
#include <string_view>
#include <type_traits>

#include <fltx.h>

#ifndef FLTX_CXX_STANDARD_CONTRACT
#error FLTX_CXX_STANDARD_CONTRACT must identify the requested C++ standard.
#endif

static_assert(BL_CXX_LANGUAGE_VERSION >= 202002L);

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
        const auto first = bl::uniform_real_array<4>(
            T{ -2.0 },
            T{ 3.0 },
            bl::mt19937_64{ 0x1020304050607080ull });
        const auto second = bl::random_array<4>(
            bl::mt19937_64{ 0x1020304050607080ull },
            bl::uniform_real_distribution<T>{ T{ -2.0 }, T{ 3.0 } });

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
    constexpr bl::fqd expression_value{
        qd_literal * bl::fqd{ 2.0 } + bl::fqd{ 0.5 }
    };

    static_assert(dd_literal == bl::fdd{ 1.25 });
    static_assert(qd_literal == bl::fqd{ 1.25 });
    static_assert(expression_value == bl::fqd{ 3.0 });
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
    }
}
