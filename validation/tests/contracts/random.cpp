#include <catch2/catch_test_macros.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <type_traits>
#include <vector>

#include <fltx/random.h>

namespace
{
    struct modulo_six_engine
    {
        using result_type = unsigned int;

        unsigned int value = 0;

        [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
        [[nodiscard]] static constexpr result_type max() noexcept { return 5; }

        [[nodiscard]] constexpr result_type operator()() noexcept
        {
            const result_type result = value;
            value = (value + 1) % 6;
            return result;
        }
    };

    template<class Distribution>
    void check_stream_round_trip(Distribution distribution)
    {
        std::stringstream stream;
        stream << distribution;
        Distribution restored;
        stream >> restored;
        REQUIRE(stream);
        CHECK(restored == distribution);
    }

    template<class Real>
    void check_real_distribution_surface()
    {
        using uniform = bl::uniform_real_distribution<Real>;
        using exponential = bl::exponential_distribution<Real>;
        using normal = bl::normal_distribution<Real>;
        using lognormal = bl::lognormal_distribution<Real>;

        bl::mt19937_64 engine{ 0x12345678u };

        constexpr typename uniform::param_type uniform_params{
            Real{ -2.0 },
            Real{ 3.0 }
        };
        uniform uniform_dist{ uniform_params };
        CHECK(uniform_dist.param() == uniform_params);
        CHECK(uniform_dist.a() == Real{ -2.0 });
        CHECK(uniform_dist.b() == Real{ 3.0 });
        CHECK(uniform_dist.min() == Real{ -2.0 });
        CHECK(uniform_dist.max() == Real{ 3.0 });
        const Real uniform_override = uniform_dist(
            engine,
            typename uniform::param_type{ Real{ 10.0 }, Real{ 11.0 } });
        CHECK(uniform_override >= Real{ 10.0 });
        CHECK(uniform_override < Real{ 11.0 });
        CHECK(uniform_dist.param() == uniform_params);
        uniform_dist.param(
            typename uniform::param_type{ Real{ -4.0 }, Real{ -3.0 } });
        CHECK(uniform_dist.a() == Real{ -4.0 });
        uniform_dist.reset();
        check_stream_round_trip(uniform_dist);

        constexpr typename exponential::param_type exponential_params{ Real{ 0.75 } };
        exponential exponential_dist{ exponential_params };
        CHECK(exponential_dist.lambda() == Real{ 0.75 });
        CHECK(exponential_dist.min() == Real{ 0.0 });
        CHECK(exponential_dist.max() == std::numeric_limits<Real>::max());
        CHECK(exponential_dist.param() == exponential_params);
        CHECK(exponential_dist(
            engine,
            typename exponential::param_type{ Real{ 0.5 } }) >= Real{ 0.0 });
        CHECK(exponential_dist.param() == exponential_params);
        exponential_dist.param(
            typename exponential::param_type{ Real{ 1.25 } });
        CHECK(exponential_dist.lambda() == Real{ 1.25 });
        exponential_dist.reset();
        check_stream_round_trip(exponential_dist);

        constexpr typename normal::param_type normal_params{
            Real{ -1.0 },
            Real{ 0.25 }
        };
        normal normal_dist{ normal_params };
        CHECK(normal_dist.mean() == Real{ -1.0 });
        CHECK(normal_dist.stddev() == Real{ 0.25 });
        CHECK(normal_dist.min() == std::numeric_limits<Real>::lowest());
        CHECK(normal_dist.max() == std::numeric_limits<Real>::max());
        CHECK(normal_dist.param() == normal_params);
        CHECK(bl::isfinite(normal_dist(
            engine,
            typename normal::param_type{ Real{ 10.0 }, Real{ 2.0 } })));
        CHECK(normal_dist.param() == normal_params);
        check_stream_round_trip(normal_dist);
        normal_dist.reset();
        normal_dist.param(
            typename normal::param_type{ Real{ 3.0 }, Real{ 4.0 } });
        CHECK(normal_dist.mean() == Real{ 3.0 });
        CHECK(normal_dist.stddev() == Real{ 4.0 });

        constexpr typename lognormal::param_type lognormal_params{
            Real{ 0.1 },
            Real{ 0.75 }
        };
        lognormal lognormal_dist{ lognormal_params };
        CHECK(lognormal_dist.m() == Real{ 0.1 });
        CHECK(lognormal_dist.s() == Real{ 0.75 });
        CHECK(lognormal_dist.min() == Real{ 0.0 });
        CHECK(lognormal_dist.max() == std::numeric_limits<Real>::max());
        CHECK(lognormal_dist.param() == lognormal_params);
        CHECK(lognormal_dist(
            engine,
            typename lognormal::param_type{ Real{ 0.2 }, Real{ 0.5 } }) >
            Real{ 0.0 });
        CHECK(lognormal_dist.param() == lognormal_params);
        check_stream_round_trip(lognormal_dist);
        lognormal_dist.reset();
        lognormal_dist.param(
            typename lognormal::param_type{ Real{ -0.5 }, Real{ 1.25 } });
        CHECK(lognormal_dist.m() == Real{ -0.5 });
        CHECK(lognormal_dist.s() == Real{ 1.25 });
    }
}

static_assert(bl::uniform_random_bit_generator<modulo_six_engine>);
static_assert(std::same_as<bl::default_random_engine, bl::mt19937>);
static_assert(!std::copy_constructible<bl::seed_seq>);
static_assert(std::constructible_from<bl::random_device, const std::string&>);

TEST_CASE("fltx Mersenne Twister engines match the standard engines", "[contracts][random]")
{
    bl::mt19937 ours32{ 5489u };
    std::mt19937 standard32{ 5489u };
    bl::mt19937_64 ours64{ 5489u };
    std::mt19937_64 standard64{ 5489u };

    for (int i = 0; i < 1000; ++i)
    {
        REQUIRE(ours32() == standard32());
        REQUIRE(ours64() == standard64());
    }

    bl::mt19937 default_engine;
    bl::mt19937 default_seeded{ bl::mt19937::default_seed };
    CHECK(default_engine == default_seeded);
    CHECK(bl::mt19937::min() == std::mt19937::min());
    CHECK(bl::mt19937::max() == std::mt19937::max());

    const bl::mt19937 unchanged{ 7u };
    bl::mt19937 malformed_target = unchanged;
    std::istringstream malformed{ "999999" };
    malformed >> malformed_target;
    CHECK(malformed.fail());
    CHECK(malformed_target == unchanged);
}

TEST_CASE("seed sequences and engine state match standard behavior", "[contracts][random]")
{
    bl::seed_seq ours{ 1u, 2u, 3u, 4u, 5u };
    std::seed_seq standard{ 1u, 2u, 3u, 4u, 5u };
    std::array<std::uint32_t, 16> ours_words{};
    std::array<std::uint32_t, 16> standard_words{};
    ours.generate(ours_words.begin(), ours_words.end());
    standard.generate(standard_words.begin(), standard_words.end());
    CHECK(ours_words == standard_words);
    CHECK(ours.size() == 5);

    std::vector<std::uint32_t> saved_seed;
    ours.param(std::back_inserter(saved_seed));
    CHECK(saved_seed == std::vector<std::uint32_t>{ 1u, 2u, 3u, 4u, 5u });

    bl::seed_seq iterator_seed{ saved_seed.begin(), saved_seed.end() };
    std::array<std::uint32_t, 16> iterator_words{};
    iterator_seed.generate(iterator_words.begin(), iterator_words.end());
    CHECK(iterator_words == ours_words);

    bl::seed_seq ours_engine_seed{ 4u, 3u, 2u, 1u };
    std::seed_seq standard_engine_seed{ 4u, 3u, 2u, 1u };
    bl::mt19937 ours_seeded_engine{ ours_engine_seed };
    std::mt19937 standard_seeded_engine{ standard_engine_seed };
    CHECK(ours_seeded_engine() == standard_seeded_engine());
    ours_seeded_engine.seed(17u);
    standard_seeded_engine.seed(17u);
    CHECK(ours_seeded_engine() == standard_seeded_engine());

    bl::mt19937_64 engine{ 42u };
    engine.discard(37);
    std::stringstream state;
    state << engine;
    bl::mt19937_64 restored;
    state >> restored;
    REQUIRE(restored == engine);
    CHECK(restored() == engine());

    bl::seed_seq empty_ours;
    std::seed_seq empty_standard;
    empty_ours.generate(ours_words.begin(), ours_words.end());
    empty_standard.generate(standard_words.begin(), standard_words.end());
    CHECK(ours_words == standard_words);
}

TEST_CASE("integer and real distributions are deterministic and bounded", "[contracts][random]")
{
    bl::mt19937_64 integer_engine{ 0x1020304050607080ull };
    bl::mt19937_64 left{ 0x1020304050607080ull };
    bl::mt19937_64 right{ 0x1020304050607080ull };
    bl::uniform_int_distribution<int> integers{ -17, 23 };
    bl::uniform_real_distribution<bl::f128> dd{ bl::f128{ -2.0 }, bl::f128{ 3.0 } };
    bl::uniform_real_distribution<bl::f256> qd{ bl::f256{ -2.0 }, bl::f256{ 3.0 } };

    for (int i = 0; i < 128; ++i)
    {
        const int sample = integers(integer_engine);
        CHECK(sample >= -17);
        CHECK(sample <= 23);

        const bl::f128 a = dd(left);
        const bl::f128 b = dd(right);
        CHECK(a >= bl::f128{ -2.0 });
        CHECK(a < bl::f128{ 3.0 });
        CHECK(a == b);
    }

    bl::mt19937_64 qd_engine{ 7u };
    const auto sample = qd(qd_engine);
    CHECK(sample >= bl::f256{ -2.0 });
    CHECK(sample < bl::f256{ 3.0 });

    bl::uniform_int_distribution<std::int64_t> signed_full{
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max()
    };
    bl::uniform_int_distribution<std::uint64_t> unsigned_full{
        0,
        std::numeric_limits<std::uint64_t>::max()
    };
    bl::mt19937_64 wide_engine{ 99u };
    for (int i = 0; i < 32; ++i)
    {
        const auto signed_value = signed_full(wide_engine);
        const auto unsigned_value = unsigned_full(wide_engine);
        CHECK(signed_value >= signed_full.min());
        CHECK(signed_value <= signed_full.max());
        CHECK(unsigned_value >= unsigned_full.min());
        CHECK(unsigned_value <= unsigned_full.max());
    }

    modulo_six_engine canonical_engine;
    modulo_six_engine bounded_engine;
    bl::uniform_int_distribution<int> four_values{ 0, 3 };
    for (int i = 0; i < 16; ++i)
    {
        const double canonical = bl::generate_canonical<double, 4>(canonical_engine);
        CHECK(canonical >= 0.0);
        CHECK(canonical < 1.0);

        const int bounded = four_values(bounded_engine);
        CHECK(bounded >= 0);
        CHECK(bounded <= 3);
    }
}

TEST_CASE("extended random helpers are deterministic", "[contracts][random]")
{
    const auto a = bl::uniform_real_array<8>(bl::f128{ -1.0 }, bl::f128{ 1.0 }, 123u);
    const auto b = bl::uniform_real_array<8>(bl::f128{ -1.0 }, bl::f128{ 1.0 }, 123u);
    CHECK(a == b);

    const auto normal_a =
        bl::normal_array<8, bl::f128>(bl::f128{ 1.0 }, bl::f128{ 0.5 }, 123u);
    const auto normal_b =
        bl::normal_array<8, bl::f128>(bl::f128{ 1.0 }, bl::f128{ 0.5 }, 123u);
    CHECK(normal_a == normal_b);

    bl::mt19937 canonical_engine{ 1234u };
    const bl::f256 canonical =
        bl::generate_canonical<bl::f256, std::numeric_limits<bl::f256>::digits>(
            canonical_engine);
    CHECK(canonical >= bl::f256{ 0.0 });
    CHECK(canonical < bl::f256{ 1.0 });
}

TEST_CASE("all real distributions expose their distinct public operations",
          "[contracts][random][distribution]")
{
    check_real_distribution_surface<bl::f128>();
    check_real_distribution_surface<bl::f256>();

    bl::mt19937_64 engine{ 1001u };
    CHECK(bl::exponential_distribution<float>{ 1.5f }(engine) >= 0.0f);
    CHECK(bl::isfinite(bl::normal_distribution<double>{ 1.0, 2.0 }(engine)));
    CHECK(bl::lognormal_distribution<long double>{ 0.0L, 0.5L }(engine) > 0.0L);
}

TEST_CASE("integer distributions expose params, overrides, equality, and streams",
          "[contracts][random][distribution]")
{
    using distribution = bl::uniform_int_distribution<int>;
    constexpr distribution::param_type initial{ -7, 13 };
    constexpr distribution::param_type same{ -7, 13 };
    STATIC_CHECK(initial == same);
    STATIC_CHECK(initial.a() == -7);
    STATIC_CHECK(initial.b() == 13);

    distribution values{ initial };
    CHECK(values.a() == -7);
    CHECK(values.b() == 13);
    CHECK(values.min() == -7);
    CHECK(values.max() == 13);
    CHECK(values.param() == initial);

    bl::mt19937 engine{ 321u };
    const int override_value =
        values(engine, distribution::param_type{ 10, 12 });
    CHECK(override_value >= 10);
    CHECK(override_value <= 12);
    CHECK(values.param() == initial);

    values.param(distribution::param_type{ -2, 2 });
    CHECK(values == distribution{ -2, 2 });
    values.reset();
    check_stream_round_trip(values);

    bl::uniform_int_distribution<unsigned int> one_value{ 7u, 7u };
    CHECK(one_value(engine) == 7u);
}

TEST_CASE("array helpers cover engine, seed, explicit, and default overloads",
          "[contracts][random][array]")
{
    bl::mt19937_64 first_engine{ 12u };
    bl::mt19937_64 second_engine{ 12u };
    CHECK(
        bl::uniform_real_array<4>(-1.0, 1.0, first_engine) ==
        bl::uniform_real_array<4>(-1.0, 1.0, second_engine));
    CHECK(
        bl::uniform_real_array<4, bl::f128>(first_engine) ==
        bl::uniform_real_array<4, bl::f128>(second_engine));
    CHECK(
        bl::uniform_real_array<4, bl::f256>(17u) ==
        bl::uniform_real_array<4, bl::f256>(17u));

    bl::mt19937_64 third_engine{ 24u };
    bl::mt19937_64 fourth_engine{ 24u };
    CHECK(
        bl::normal_array<4>(1.0, 0.5, third_engine) ==
        bl::normal_array<4>(1.0, 0.5, fourth_engine));
    CHECK(
        bl::normal_array<4, bl::f128>(third_engine) ==
        bl::normal_array<4, bl::f128>(fourth_engine));
    CHECK(
        bl::normal_array<4, bl::f256>(29u) ==
        bl::normal_array<4, bl::f256>(29u));

    const auto constants = bl::random_array<4>(
        bl::mt19937{ 1u },
        bl::uniform_int_distribution<int>{ 5, 5 });
    CHECK(constants == std::array{ 5, 5, 5, 5 });
}

TEST_CASE("stateful distributions reset cached samples", "[contracts][random]")
{
    using normal = bl::normal_distribution<double>;
    using lognormal = bl::lognormal_distribution<double>;

    constexpr normal::param_type normal_params{ -1.0, 0.25 };
    constexpr lognormal::param_type lognormal_params{ 0.1, 0.75 };
    normal normal_dist{ normal_params };
    normal normal_fresh{ normal_params };
    lognormal lognormal_dist{ lognormal_params };
    lognormal lognormal_fresh{ lognormal_params };
    bl::mt19937_64 engine{ 777u };

    normal override_dist{ normal_params };
    const double override_sample =
        override_dist(engine, normal::param_type{ 10.0, 2.0 });
    CHECK(bl::isfinite(override_sample));
    CHECK(override_dist.param() == normal_params);

    static_cast<void>(normal_dist(engine));
    static_cast<void>(lognormal_dist(engine));
    CHECK(normal_dist != normal_fresh);
    CHECK(lognormal_dist != lognormal_fresh);

    normal_dist.reset();
    lognormal_dist.reset();
    CHECK(normal_dist == normal_fresh);
    CHECK(lognormal_dist == lognormal_fresh);
}

TEST_CASE("extended normal distributions retain only their parameters",
          "[contracts][random]")
{
    bl::mt19937_64 engine{ 777u };

    bl::normal_distribution<bl::f128> normal128{ bl::f128{ -1.0 }, bl::f128{ 0.25 } };
    const auto fresh_normal128 = normal128;
    static_cast<void>(normal128(engine));
    CHECK(normal128 == fresh_normal128);

    bl::lognormal_distribution<bl::f128> lognormal128{ bl::f128{ 0.1 }, bl::f128{ 0.75 } };
    const auto fresh_lognormal128 = lognormal128;
    static_cast<void>(lognormal128(engine));
    CHECK(lognormal128 == fresh_lognormal128);

    bl::normal_distribution<bl::f256> normal256{ bl::f256{ -1.0 }, bl::f256{ 0.25 } };
    const auto fresh_normal256 = normal256;
    static_cast<void>(normal256(engine));
    CHECK(normal256 == fresh_normal256);

    bl::lognormal_distribution<bl::f256> lognormal256{ bl::f256{ 0.1 }, bl::f256{ 0.75 } };
    const auto fresh_lognormal256 = lognormal256;
    static_cast<void>(lognormal256(engine));
    CHECK(lognormal256 == fresh_lognormal256);
}

TEST_CASE("extended normal distributions have stable sample moments",
          "[contracts][random]")
{
    constexpr std::size_t sample_count = 32'768;

    const auto check_moments = []<class Real>() {
        bl::mt19937_64 engine{ 0x1020304050607080ull };
        bl::normal_distribution<Real> distribution;
        double sum = 0.0;
        double sum_squares = 0.0;

        for (std::size_t index = 0; index < sample_count; ++index)
        {
            const double sample = static_cast<double>(distribution(engine));
            sum += sample;
            sum_squares += sample * sample;
        }

        const double mean = sum / static_cast<double>(sample_count);
        const double variance =
            sum_squares / static_cast<double>(sample_count) - mean * mean;
        CHECK(mean > -0.03);
        CHECK(mean < 0.03);
        CHECK(variance > 0.94);
        CHECK(variance < 1.06);
    };

    check_moments.template operator()<bl::f128>();
    check_moments.template operator()<bl::f256>();
}

TEST_CASE("random device exposes the standard runtime-only shape", "[contracts][random]")
{
    bl::random_device device;
    const auto sample = device();
    CHECK(sample >= bl::random_device::min());
    CHECK(sample <= bl::random_device::max());
    CHECK(device.entropy() >= 0.0);
}
