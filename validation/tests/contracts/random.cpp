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
    bl::uniform_real_distribution<bl::fdd> dd{ bl::fdd{ -2.0 }, bl::fdd{ 3.0 } };
    bl::uniform_real_distribution<bl::fqd> qd{ bl::fqd{ -2.0 }, bl::fqd{ 3.0 } };

    for (int i = 0; i < 128; ++i)
    {
        const int sample = integers(integer_engine);
        CHECK(sample >= -17);
        CHECK(sample <= 23);

        const bl::fdd a = dd(left);
        const bl::fdd b = dd(right);
        CHECK(a >= bl::fdd{ -2.0 });
        CHECK(a < bl::fdd{ 3.0 });
        CHECK(a == b);
    }

    bl::mt19937_64 qd_engine{ 7u };
    const auto sample = qd(qd_engine);
    CHECK(sample >= bl::fqd{ -2.0 });
    CHECK(sample < bl::fqd{ 3.0 });

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
    const auto a = bl::uniform_real_array<8>(bl::fdd{ -1.0 }, bl::fdd{ 1.0 }, 123u);
    const auto b = bl::uniform_real_array<8>(bl::fdd{ -1.0 }, bl::fdd{ 1.0 }, 123u);
    CHECK(a == b);

    const auto normal_a =
        bl::normal_array<8, bl::fdd>(bl::fdd{ 1.0 }, bl::fdd{ 0.5 }, 123u);
    const auto normal_b =
        bl::normal_array<8, bl::fdd>(bl::fdd{ 1.0 }, bl::fdd{ 0.5 }, 123u);
    CHECK(normal_a == normal_b);

    bl::mt19937 canonical_engine{ 1234u };
    const bl::fqd canonical =
        bl::generate_canonical<bl::fqd, std::numeric_limits<bl::fqd>::digits>(
            canonical_engine);
    CHECK(canonical >= bl::fqd{ 0.0 });
    CHECK(canonical < bl::fqd{ 1.0 });
}

TEST_CASE("all real distributions expose their distinct public operations",
          "[contracts][random][distribution]")
{
    check_real_distribution_surface<bl::fdd>();
    check_real_distribution_surface<bl::fqd>();

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
        bl::uniform_real_array<4, bl::fdd>(first_engine) ==
        bl::uniform_real_array<4, bl::fdd>(second_engine));
    CHECK(
        bl::uniform_real_array<4, bl::fqd>(17u) ==
        bl::uniform_real_array<4, bl::fqd>(17u));

    bl::mt19937_64 third_engine{ 24u };
    bl::mt19937_64 fourth_engine{ 24u };
    CHECK(
        bl::normal_array<4>(1.0, 0.5, third_engine) ==
        bl::normal_array<4>(1.0, 0.5, fourth_engine));
    CHECK(
        bl::normal_array<4, bl::fdd>(third_engine) ==
        bl::normal_array<4, bl::fdd>(fourth_engine));
    CHECK(
        bl::normal_array<4, bl::fqd>(29u) ==
        bl::normal_array<4, bl::fqd>(29u));

    const auto constants = bl::random_array<4>(
        bl::mt19937{ 1u },
        bl::uniform_int_distribution<int>{ 5, 5 });
    CHECK(constants == std::array{ 5, 5, 5, 5 });
}

TEST_CASE("random helpers deduce owning values from expression parameters",
          "[contracts][random][expressions]")
{
    const auto half = bl::fqd{ 0.25 } * bl::fqd{ 2.0 };
    const auto one = half + bl::fqd{ 0.5 };
    const auto two = bl::fqd{ 4.0 } * bl::fqd{ 0.5 };
    bl::uniform_real_distribution uniform{ half, one };
    bl::normal_distribution normal{ one, half };
    bl::exponential_distribution exponential{ two };
    bl::lognormal_distribution lognormal{ one, half };
    STATIC_CHECK(std::same_as<decltype(uniform), bl::uniform_real_distribution<bl::fqd>>);
    STATIC_CHECK(std::same_as<decltype(normal), bl::normal_distribution<bl::fqd>>);
    STATIC_CHECK(std::same_as<decltype(exponential), bl::exponential_distribution<bl::fqd>>);
    STATIC_CHECK(std::same_as<decltype(lognormal), bl::lognormal_distribution<bl::fqd>>);
    STATIC_CHECK(std::same_as<decltype(bl::uniform_real_distribution{ 0, 1 }), bl::uniform_real_distribution<double>>);
    STATIC_CHECK(std::same_as<decltype(bl::uniform_real_distribution{ 0.0f, 1.0f }), bl::uniform_real_distribution<float>>);
    STATIC_CHECK(std::same_as<decltype(bl::normal_distribution{ 1.0f, 0.5 }), bl::normal_distribution<double>>);
    STATIC_CHECK(std::same_as<decltype(bl::exponential_distribution{ 2 }), bl::exponential_distribution<double>>);
    STATIC_CHECK(std::same_as<decltype(bl::normal_distribution{ bl::fdd_s{ 1.0 }, bl::fdd_s{ 0.5 } }), bl::normal_distribution<bl::fdd_s>>);
    STATIC_CHECK(std::same_as<decltype(bl::exponential_distribution{ bl::fqd_s{ 2.0 } }), bl::exponential_distribution<bl::fqd_s>>);
    STATIC_CHECK(std::same_as<decltype(bl::lognormal_distribution{ bl::fdd{ 1.0 }, bl::fqd{ 0.5 } }), bl::lognormal_distribution<bl::fqd>>);
    CHECK(bl::uniform_real_distribution{ 0, bl::fqd{ one } } == bl::uniform_real_distribution{ 0, one });
    CHECK(bl::normal_distribution{ bl::fdd{ 1.0 }, bl::fqd{ half } } == normal);
    CHECK(uniform == bl::uniform_real_distribution<bl::fqd>{ bl::fqd{ 0.5 }, bl::fqd{ 1.0 } });
    CHECK(normal == bl::normal_distribution<bl::fqd>{ bl::fqd{ 1.0 }, bl::fqd{ 0.5 } });
    CHECK(exponential == bl::exponential_distribution<bl::fqd>{ bl::fqd{ 2.0 } });
    CHECK(lognormal == bl::lognormal_distribution<bl::fqd>{ bl::fqd{ 1.0 }, bl::fqd{ 0.5 } });
    CHECK(bl::uniform_real_distribution{ half } == bl::uniform_real_distribution<bl::fqd>{ bl::fqd{ 0.5 } });
    CHECK(bl::normal_distribution{ one } == bl::normal_distribution<bl::fqd>{ bl::fqd{ 1.0 } });
    CHECK(bl::lognormal_distribution{ one } == bl::lognormal_distribution<bl::fqd>{ bl::fqd{ 1.0 } });
    CHECK(bl::uniform_real_distribution{ half, two } == bl::uniform_real_distribution<bl::fqd>{ bl::fqd{ 0.5 }, bl::fqd{ 2.0 } });
    CHECK(bl::uniform_real_distribution{ 0, one } == bl::uniform_real_distribution<bl::fqd>{ bl::fqd{ 0.0 }, bl::fqd{ 1.0 } });
    CHECK(bl::normal_distribution{ bl::fdd{ 1.0 }, half } == normal);
    CHECK(bl::lognormal_distribution{ one, 0.5 } == lognormal);

    const auto uniform_values = bl::uniform_real_array<4>(half, one, 17u);
    STATIC_CHECK(std::same_as<typename decltype(uniform_values)::value_type, bl::fqd>);
    CHECK(uniform_values == bl::uniform_real_array<4>(bl::fqd{ 0.5 }, bl::fqd{ 1.0 }, 17u));
    CHECK(uniform_values == bl::uniform_real_array<4, bl::fqd>(half, one, bl::mt19937_64{ 17u }));
    CHECK(bl::uniform_real_array<4>(0, one, 17u) ==
          bl::uniform_real_array<4>(bl::fqd{ 0.0 }, bl::fqd{ 1.0 }, 17u));
    CHECK(bl::normal_array<4>(one, half, bl::mt19937_64{ 29u }) ==
          bl::normal_array<4>(bl::fqd{ 1.0 }, bl::fqd{ 0.5 }, 29u));
    CHECK(bl::normal_array<4, bl::fqd>(one, half, 29u) ==
          bl::normal_array<4>(bl::fqd{ 1.0 }, bl::fqd{ 0.5 }, 29u));
    CHECK(bl::normal_array<4, bl::fdd>(one, half, 29u) ==
          bl::normal_array<4>(bl::fdd{ 1.0 }, bl::fdd{ 0.5 }, 29u));

    bl::mt19937_64 actual_engine{ 71u }, expected_engine{ 71u };
    CHECK(uniform(actual_engine) == bl::uniform_real_distribution<bl::fqd>{ bl::fqd{ 0.5 }, bl::fqd{ 1.0 } }(expected_engine));
    CHECK(normal(actual_engine) == bl::normal_distribution<bl::fqd>{ bl::fqd{ 1.0 }, bl::fqd{ 0.5 } }(expected_engine));
    CHECK(exponential(actual_engine) == bl::exponential_distribution<bl::fqd>{ bl::fqd{ 2.0 } }(expected_engine));
    CHECK(lognormal(actual_engine) == bl::lognormal_distribution<bl::fqd>{ bl::fqd{ 1.0 }, bl::fqd{ 0.5 } }(expected_engine));
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

    bl::normal_distribution<bl::fdd> normal_dd{ bl::fdd{ -1.0 }, bl::fdd{ 0.25 } };
    const auto fresh_normal_dd = normal_dd;
    static_cast<void>(normal_dd(engine));
    CHECK(normal_dd == fresh_normal_dd);

    bl::lognormal_distribution<bl::fdd> lognormal_dd{ bl::fdd{ 0.1 }, bl::fdd{ 0.75 } };
    const auto fresh_lognormal_dd = lognormal_dd;
    static_cast<void>(lognormal_dd(engine));
    CHECK(lognormal_dd == fresh_lognormal_dd);

    bl::normal_distribution<bl::fqd> normal_qd{ bl::fqd{ -1.0 }, bl::fqd{ 0.25 } };
    const auto fresh_normal_qd = normal_qd;
    static_cast<void>(normal_qd(engine));
    CHECK(normal_qd == fresh_normal_qd);

    bl::lognormal_distribution<bl::fqd> lognormal_qd{ bl::fqd{ 0.1 }, bl::fqd{ 0.75 } };
    const auto fresh_lognormal_qd = lognormal_qd;
    static_cast<void>(lognormal_qd(engine));
    CHECK(lognormal_qd == fresh_lognormal_qd);
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

    check_moments.template operator()<bl::fdd>();
    check_moments.template operator()<bl::fqd>();
}

TEST_CASE("random device exposes the standard runtime-only shape", "[contracts][random]")
{
    bl::random_device device;
    const auto sample = device();
    CHECK(sample >= bl::random_device::min());
    CHECK(sample <= bl::random_device::max());
    CHECK(device.entropy() >= 0.0);
}
