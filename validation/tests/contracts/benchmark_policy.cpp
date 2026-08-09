#include "../../benchmarks/runtime/timing_policy.hpp"
#include "../../benchmarks/runtime/samples.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

namespace policy = fltx::tests::benchmark::timing_policy;
namespace samples = fltx::tests::benchmark::samples;

TEST_CASE("standard benchmark timing tiers are adaptive",
          "[contracts][benchmark-policy]")
{
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

TEST_CASE("standard benchmark corpora match operation cost",
          "[contracts][benchmark-policy]")
{
    CHECK(samples::count_for("add", 8192, "standard") == 81920);
    CHECK(samples::count_for("sqrt", 4096, "standard") == 40960);
    CHECK(samples::count_for("sin", 8192, "standard") == 2048);
    CHECK(samples::count_for("erf", 8192, "standard") == 256);
    CHECK(samples::count_for("tgamma", 4096, "standard") == 128);

    CHECK(samples::count_for("add", 12, "smoke") == 12);
    CHECK(samples::count_for("sin", 12, "smoke") == 3);
    CHECK(samples::count_for("add", 81920, "full") == 81920);
    CHECK(samples::count_for("erf", 81920, "full") == 20480);
}
