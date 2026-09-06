#include <fltx.h>

#include "../../support/config_banner.hpp"

#include <algorithm>
#include <bit>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>

static_assert(std::is_same_v<bl::common_float_type_t<bl::fdd, bl::fqd>, bl::fqd>);
static_assert(std::numeric_limits<bl::fdd>::digits == 106);
static_assert(std::numeric_limits<bl::fqd>::digits == 212);
static_assert(sizeof(bl::fqd) == 4 * sizeof(double));
static_assert(std::is_trivially_copyable_v<bl::fqd>);
static_assert(std::is_aggregate_v<bl::fqd_s>);

using qd_product = decltype(bl::fqd{} * bl::fqd{});
#if defined(FLTX_ENABLE_FQD_EXPRESSIONS) && FLTX_ENABLE_FQD_EXPRESSIONS
static_assert(bl::fltx_expression<qd_product>);
#else
static_assert(std::is_same_v<qd_product, bl::fqd>);
#endif

namespace
{
    constexpr bl::fqd add_pair(const auto& x, const auto& y) { return x + y; }

    constexpr bool expression_policy_contract(bl::fqd a, bl::fqd b, bl::fqd c)
    {
        const auto product = a * b;
        const auto sum = b * c + a;
        if (add_pair(product, sum) != bl::fqd{ 20.0 } ||
            bl::min({ a, product, sum }) != a ||
            bl::sqrt(product * product) != bl::fqd{ 6.0 })
            return false;

        #if defined(FLTX_ENABLE_FQD_EXPRESSIONS) && FLTX_ENABLE_FQD_EXPRESSIONS
        return std::max<bl::fqd>(a, b * c) == bl::fqd{ 12.0 };
        #else
        return std::max(a, b * c) == bl::fqd{ 12.0 };
        #endif
    }

    static_assert(expression_policy_contract(bl::fqd{ 2.0 }, bl::fqd{ 3.0 }, bl::fqd{ 4.0 }));

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

    bool dd_multiply_special_contract()
    {
        constexpr std::uint64_t sign_mask = UINT64_C(0x8000000000000000);
        const double positive_zero = std::bit_cast<double>(UINT64_C(0));
        const double negative_zero = std::bit_cast<double>(sign_mask);

        const bl::fdd_s positive =
            bl::fdd_s{positive_zero, positive_zero} * bl::fdd_s{2.0, positive_zero};
        const bl::fdd_s double_negative =
            bl::fdd_s{negative_zero, positive_zero} * bl::fdd_s{-2.0, positive_zero};

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
#if defined(FLTX_TESTS_EXPECT_NO_LOCAL_FMA) && FLTX_X86_FMA_RUNTIME_CHECK
    if (bl::detail::fp::runtime_hardware_fma_enabled())
        return fail("injected runtime CPU probe did not disable FMA");
#endif

    const bl::fdd dd = bl::parse<bl::fdd>("1.00000000000000000000000000000001");
    const bl::fqd qd = bl::parse<bl::fqd>(
        "1.000000000000000000000000000000000000000000000000000000000000001");

    if (!(dd > bl::fdd{ 1.0 }) || !(qd > bl::fqd{ 1.0 }))
        return fail("decimal parsing lost low limbs");
    if (bl::sqrt(bl::fdd{ 4.0 }) != bl::fdd{ 2.0 })
        return fail("fdd sqrt contract");
    if (bl::sqrt(bl::fqd{ 4.0 }) != bl::fqd{ 2.0 })
        return fail("fqd sqrt contract");
    if (!fma_cancellation_contract<bl::fdd>())
        return fail("fdd fused-cancellation contract");
    if (!product_cancellation_contract<bl::fdd>())
        return fail("fdd product-cancellation contract");
    if (!dd_multiply_special_contract())
        return fail("fdd multiply signed-zero contract");
    if (!fma_cancellation_contract<bl::fqd>())
        return fail("fqd fused-cancellation contract");
    if (!product_cancellation_contract<bl::fqd>())
        return fail("fqd product-cancellation contract");
    if (!expression_policy_contract(
            bl::parse<bl::fqd>("2"), bl::parse<bl::fqd>("3"), bl::parse<bl::fqd>("4")))
        return fail("fqd expression-policy contract");

    std::cout << "fltx package consumer passed\n";
    return EXIT_SUCCESS;
}
