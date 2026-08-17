#include "../../benchmarks/runtime/integer_observer.hpp"
#include "../../benchmarks/runtime/timing_policy.hpp"
#include "../../benchmarks/runtime/samples.hpp"
#include "../../support/native_fp.hpp"

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace policy = fltx::tests::benchmark::timing_policy;
namespace samples = fltx::tests::benchmark::samples;

TEST_CASE("validation observes native floating encodings without classification intrinsics",
          "[contracts][benchmark-policy]")
{
    namespace native_fp = fltx::tests::native_fp;

    const double negative_zero = native_fp::signed_zero<double>(true);
    const double infinity = native_fp::positive_infinity<double>();
    const double nan = native_fp::quiet_nan<double>();

    CHECK(native_fp::sign_bit(negative_zero));
    CHECK(native_fp::is_inf(infinity));
    CHECK_FALSE(native_fp::is_finite(infinity));
    CHECK(native_fp::is_nan(nan));
    CHECK(native_fp::is_finite(native_fp::denorm_min<double>()));
}

TEST_CASE("benchmark observation accumulates floating representations as integers",
          "[contracts][benchmark-policy]")
{
    fltx::tests::benchmark::integer_observer observer;
    observer.add({1.0, -0.0, 2.0, -3.0});
    observer.add({-1.0, 0.0, -2.0, 3.0});

    const auto lane0 = std::bit_cast<std::uint64_t>(1.0) +
                       std::bit_cast<std::uint64_t>(-1.0);
    const auto lane1 = std::bit_cast<std::uint64_t>(-0.0) +
                       std::bit_cast<std::uint64_t>(0.0);
    const auto lane2 = std::bit_cast<std::uint64_t>(2.0) +
                       std::bit_cast<std::uint64_t>(-2.0);
    const auto lane3 = std::bit_cast<std::uint64_t>(-3.0) +
                       std::bit_cast<std::uint64_t>(3.0);
    CHECK(observer.value() ==
          (lane0 ^ std::rotl(lane1, 13) ^ std::rotl(lane2, 29) ^
           std::rotl(lane3, 47)));
}

TEST_CASE("benchmark measurements run primary first",
          "[contracts][benchmark-policy]")
{
    std::vector<std::size_t> order;
    policy::for_each_measurement_primary_first(
        4,
        [&](std::size_t index) { order.push_back(index); });
    CHECK(order == std::vector<std::size_t>{0, 1, 2, 3});
}

TEST_CASE("standard benchmark timing tiers are adaptive",
          "[contracts][benchmark-policy]")
{
    CHECK(policy::identity == "adaptive-v2");
    const auto fast = policy::select(1.0);
    CHECK(fast.trials == 7);
    CHECK(fast.target_trial_ns == 15'000'000.0);

    const auto ordinary = policy::select(20.0);
    CHECK(ordinary.trials == 3);
    CHECK(ordinary.target_trial_ns == 8'000'000.0);

    const auto slow = policy::select(10'000.0);
    CHECK(slow.trials == 3);
    CHECK(slow.target_trial_ns == 0.0);
}

TEST_CASE("standard benchmark row budget keeps an odd trial count",
          "[contracts][benchmark-policy]")
{
    CHECK(policy::fitting_trials(7, 15'000'000.0) == 7);
    CHECK(policy::fitting_trials(3, 2'000'000'000.0) == 1);
    CHECK(policy::fitting_trials(7, 1'000'000'000.0) == 5);
    CHECK(policy::fitting_trials(3, 6'000'000'000.0) == 1);
}

TEST_CASE("standard benchmark calibration runs once per row",
          "[contracts][benchmark-policy]")
{
    std::size_t calls = 0;
    const auto measured = policy::calibrate_once(
        [&](std::size_t batches) {
            ++calls;
            return static_cast<double>(batches) * 1'000'000.0;
        },
        8'000'000.0,
        1u << 20);

    CHECK(measured.elapsed_ns >= 8'000'000.0);
    CHECK(measured.calls == calls);
    CHECK(calls < 5);

    calls = 0;
    const auto slow = policy::calibrate_once(
        [&](std::size_t) {
            ++calls;
            return 6'000'000'000.0;
        },
        0.0,
        1u << 20);
    CHECK(slow.batches == 1);
    CHECK(slow.calls == 1);
    CHECK(calls == 1);
}

TEST_CASE("adaptive benchmark corpora match operation cost",
          "[contracts][benchmark-policy]")
{
    CHECK(samples::count_for("add", 8192, "standard") == 81920);
    CHECK(samples::count_for("sqrt", 4096, "standard") == 40960);
    CHECK(samples::count_for("sin", 8192, "standard") == 2048);
    CHECK(samples::count_for("erf", 8192, "standard") == 256);
    CHECK(samples::count_for("tgamma", 4096, "standard") == 128);

    CHECK(samples::count_for("add", 4096, "small") == 40960);
    CHECK(samples::count_for("sqrt", 2048, "small") == 20480);
    CHECK(samples::count_for("sin", 4096, "small") == 1024);
    CHECK(samples::count_for("erf", 4096, "small") == 128);
    CHECK(samples::count_for("tgamma", 2048, "small") == 64);

    CHECK(samples::count_for("add", 12, "smoke") == 12);
    CHECK(samples::count_for("sin", 12, "smoke") == 3);
    CHECK(samples::count_for("add", 81920, "full") == 81920);
    CHECK(samples::count_for("erf", 81920, "full") == 20480);
}
