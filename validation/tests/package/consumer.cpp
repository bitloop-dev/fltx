#include <fltx.h>

#include "../../support/config_banner.hpp"

#include <bit>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>

static_assert(std::is_same_v<bl::common_float_type_t<bl::f128, bl::f256>, bl::f256>);
static_assert(std::numeric_limits<bl::f128>::digits == 106);
static_assert(std::numeric_limits<bl::f256>::digits == 212);

namespace
{
    int fail(const char* message)
    {
        std::cerr << "fltx package consumer failed: " << message << '\n';
        return EXIT_FAILURE;
    }

    template<class T>
    bool fma_cancellation_contract()
    {
        const T unit{ 1.0 };
        const T increment = bl::ldexp(unit, -52);
        const T expected = -bl::ldexp(unit, -104);
        const T a = unit + increment;
        const T b = unit - increment;

        return bl::fma(a, b, -unit) == expected;
    }

    template<class T>
    bool product_cancellation_contract()
    {
        const T unit{ 1.0 };
        const T increment = bl::ldexp(unit, -52);
        const T expected = -bl::ldexp(unit, -104);
        const T a = unit + increment;
        const T b = unit - increment;

        return static_cast<T>(a * b - unit) == expected;
    }

    bool f128_multiply_special_contract()
    {
        constexpr std::uint64_t sign_mask = UINT64_C(0x8000000000000000);
        const double positive_zero = std::bit_cast<double>(UINT64_C(0));
        const double negative_zero = std::bit_cast<double>(sign_mask);

        const bl::f128_s positive =
            bl::f128_s{positive_zero, positive_zero} * bl::f128_s{2.0, positive_zero};
        const bl::f128_s double_negative =
            bl::f128_s{negative_zero, positive_zero} * bl::f128_s{-2.0, positive_zero};

        return std::bit_cast<std::uint64_t>(positive.hi) == UINT64_C(0) &&
               std::bit_cast<std::uint64_t>(positive.lo) == UINT64_C(0) &&
               std::bit_cast<std::uint64_t>(double_negative.hi) == UINT64_C(0) &&
               std::bit_cast<std::uint64_t>(double_negative.lo) == UINT64_C(0);
    }
}

int main()
{
    fltx::tests::support::print_config_banner("package consumer");

#if defined(FLTX_TESTS_EXPECT_FAST_MATH) && !defined(FLTX_FAST_MATH)
    return fail("consumer fast-math mode was not detected");
#elif defined(FLTX_TESTS_EXPECT_STRICT) && defined(FLTX_FAST_MATH)
    return fail("strict consumer was detected as fast-math");
#endif

#if defined(FLTX_TESTS_EXPECT_NO_LOCAL_FMA) && FLTX_TU_HAS_X86_FMA
    return fail("no-FMA consumer unexpectedly has translation-unit FMA");
#endif
#if defined(FLTX_TESTS_EXPECT_NO_LOCAL_FMA) && FLTX_DETAIL_X86_FMA_RUNTIME_CHECK
    if (bl::detail::fp::runtime_hardware_fma_enabled())
        return fail("injected runtime CPU probe did not disable FMA");
#endif

    const bl::f128 dd = bl::parse<bl::f128>("1.00000000000000000000000000000001");
    const bl::f256 qd = bl::parse<bl::f256>(
        "1.000000000000000000000000000000000000000000000000000000000000001");

    if (!(dd > bl::f128{ 1.0 }) || !(qd > bl::f256{ 1.0 }))
        return fail("decimal parsing lost low limbs");
    if (bl::sqrt(bl::f128{ 4.0 }) != bl::f128{ 2.0 })
        return fail("f128 sqrt contract");
    if (bl::sqrt(bl::f256{ 4.0 }) != bl::f256{ 2.0 })
        return fail("f256 sqrt contract");
    if (!fma_cancellation_contract<bl::f128>())
        return fail("f128 fused-cancellation contract");
    if (!product_cancellation_contract<bl::f128>())
        return fail("f128 product-cancellation contract");
    if (!f128_multiply_special_contract())
        return fail("f128 multiply signed-zero contract");
    if (!fma_cancellation_contract<bl::f256>())
        return fail("f256 fused-cancellation contract");
    if (!product_cancellation_contract<bl::f256>())
        return fail("f256 product-cancellation contract");

    std::cout << "fltx package consumer passed\n";
    return EXIT_SUCCESS;
}
