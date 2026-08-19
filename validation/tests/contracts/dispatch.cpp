#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <type_traits>

#include <fltx/dispatch.h>

enum class dispatch_mode
{
    subtract = 2,
    add = 7,
    COUNT = 8
};

template<>
struct bl_enum_domain_traits<dispatch_mode>
    : bl_enum_domain_values<dispatch_mode, dispatch_mode::subtract, dispatch_mode::add>
{};

static_assert(std::is_same_v<
    bl_enum_type_map_t<bl::FloatType, bl::FloatType::F32>,
    bl::f32>);
static_assert(std::is_same_v<
    bl_enum_type_map_t<bl::FloatType, bl::FloatType::F64>,
    bl::f64>);
static_assert(std::is_same_v<
    bl_enum_type_map_t<bl::FloatType, bl::FloatType::F128>,
    bl::f128>);
static_assert(std::is_same_v<
    bl_enum_type_map_t<bl::FloatType, bl::FloatType::F256>,
    bl::f256>);

namespace
{
    template<class T, bool Add>
    int dispatch_probe(int value)
    {
        if constexpr (Add)
            return value + bl::fltx_precision_rank_v<T>;
        return value - bl::fltx_precision_rank_v<T>;
    }

    template<class T, class U, bool Add>
    int two_type_probe(int value)
    {
        constexpr int ranks = bl::fltx_precision_rank_v<T> + bl::fltx_precision_rank_v<U>;
        return Add ? value + ranks : value - ranks;
    }

    template<dispatch_mode Mode, bool Twice>
    int mode_probe(int value)
    {
        constexpr int delta = Twice ? 2 : 1;
        if constexpr (Mode == dispatch_mode::add)
            return value + delta;
        return value - delta;
    }

    struct member_probe
    {
        template<class T, bool Add>
        int apply(int value) const
        {
            return dispatch_probe<T, Add>(value);
        }
    };

    struct callable_probe
    {
        template<class T, bool Add>
        int operator()(int value) const
        {
            return dispatch_probe<T, Add>(value);
        }
    };
}

TEST_CASE("runtime float selection maps to the documented template type", "[contracts][dispatch]")
{
    const auto table = bl_dispatch_table(dispatch_probe, 10);

    CHECK(bl_table_invoke(table, bl_enum_type(bl::FloatType::F32), true) == 11);
    CHECK(bl_table_invoke(table, bl_enum_type(bl::FloatType::F64), true) == 12);
    CHECK(bl_table_invoke(table, bl_enum_type(bl::FloatType::F128), true) == 14);
    CHECK(bl_table_invoke(table, bl_enum_type(bl::FloatType::F256), true) == 15);
    CHECK(bl_table_invoke(table, bl_enum_type(bl::FloatType::F256), false) == 5);
}

TEST_CASE("type tags, member functions, and callable objects share dispatch semantics", "[contracts][dispatch]")
{
    CHECK(bl_table_invoke(
        bl_dispatch_table(dispatch_probe, 10),
        bl_type<bl::f128>,
        true) == 14);

    CHECK(bl_table_invoke<bl::f64>(
        bl_dispatch_table(dispatch_probe, 10),
        false) == 8);

    const member_probe object;
    CHECK(bl_table_invoke(
        bl_dispatch_table_memfn(object, member_probe::apply, 10),
        bl_enum_type(bl::FloatType::F256),
        true) == 15);

    const callable_probe callable;
    CHECK(bl_table_invoke(
        bl_dispatch_table_callable(callable, 10),
        bl_type<bl::f32>,
        false) == 9);

    CHECK(bl_table_invoke(
        bl_dispatch_table(two_type_probe, 10),
        bl_enum_type(bl::FloatType::F128),
        bl_enum_type(bl::FloatType::F64),
        true) == 16);
}

TEST_CASE("raw enum dispatch supports explicit sparse domains", "[contracts][dispatch]")
{
    STATIC_CHECK(bl_enum_domain_traits<dispatch_mode>::size == 2);
    STATIC_CHECK(bl_enum_domain_traits<dispatch_mode>::index(dispatch_mode::subtract) == 0);
    STATIC_CHECK(bl_enum_domain_traits<dispatch_mode>::index(dispatch_mode::add) == 1);
    STATIC_CHECK(bl_enum_domain_traits<dispatch_mode>::value(0) == dispatch_mode::subtract);
    STATIC_CHECK(bl_enum_domain_traits<dispatch_mode>::value(1) == dispatch_mode::add);

    const auto table = bl_dispatch_table(mode_probe, 10);
    CHECK(bl_table_invoke(table, dispatch_mode::subtract, false) == 9);
    CHECK(bl_table_invoke(table, dispatch_mode::subtract, true) == 8);
    CHECK(bl_table_invoke(table, dispatch_mode::add, false) == 11);
    CHECK(bl_table_invoke(table, dispatch_mode::add, true) == 12);
}

TEST_CASE("dispatch domain helpers report the actual table shape", "[contracts][dispatch]")
{
    const auto type = bl_enum_type(bl::FloatType::F128);
    CHECK(bl_dispatch_arg_domain_size(type) == 4);
    CHECK(bl_enum_type_domain_size(type) == 4);
    CHECK(bl_dispatch_arg_domain_size(bl_type<bl::f128>) == 1);
    CHECK(bl_dispatch_arg_domain_size(dispatch_mode::add) == 2);
    CHECK(bl_dispatch_table_variant_count(type, true) == 8);
    CHECK(bl_table_variants_count(type, dispatch_mode::add, true) == 16);
    CHECK(bl_dispatch_table_domain_sizes(type, true) == std::array<std::size_t, 2>{ 4, 2 });
    CHECK(bl_dispatch_table_domain_sizes(
        bl_type<bl::f128>,
        type,
        dispatch_mode::add,
        true) == std::array<std::size_t, 4>{ 1, 4, 2, 2 });

    const std::string report = bl_dispatch_table_report("probe", type, true);
    CHECK(report.find("probe: 8 variants") != std::string::npos);
    CHECK(report.find("arg[0] domain = 4") != std::string::npos);
    CHECK(report.find("arg[1] domain = 2") != std::string::npos);

    const std::string default_label = bl_dispatch_table_report(nullptr, bl_type<bl::f64>);
    CHECK(default_label.find("dispatch: 1 variants") != std::string::npos);
}
