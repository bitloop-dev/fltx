#ifndef FLTX_TESTS_BENCHMARK_RUNNER_INCLUDED
#define FLTX_TESTS_BENCHMARK_RUNNER_INCLUDED

#include "../../support/csv.hpp"
#include "../../support/domains.hpp"
#include "../../support/implementations.hpp"
#include "../../support/source_identity.hpp"
#include "integer_observer.hpp"
#include "samples.hpp"
#include "timing_policy.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fltx::tests::benchmark
{
    inline constexpr std::string_view schema_version = "6";
#if !defined(FLTX_TESTS_ENABLE_EXTERNAL_COMPARISONS) || \
    !FLTX_TESTS_ENABLE_EXTERNAL_COMPARISONS || \
    (defined(FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH) && \
     FLTX_TESTS_EXPECT_CONSUMER_FAST_MATH)
    inline constexpr bool external_implementations_enabled = false;
#else
    inline constexpr bool external_implementations_enabled = true;
#endif
    inline volatile std::size_t timing_batches = 0;
    inline volatile std::uint64_t timing_sink = 0;
    inline constexpr double minimum_credible_iteration_ns = 0.001;
    inline constexpr double full_minimum_trial_ns = 25'000'000.0;
    inline constexpr double standard_minimum_trial_ns =
        timing_policy::normal_trial_target_ns;
    inline constexpr double smoke_minimum_trial_ns = 3'000'000.0;
    inline constexpr std::size_t maximum_batches = 1u << 20;

    struct options
    {
        std::string precision;
        std::string output;
        std::string run_id;
        std::string source_revision;
        std::string filter;
        std::string sample_mode = "custom";
        std::size_t samples = 64;
        std::size_t trials = 5;
        double minimum_trial_ns = full_minimum_trial_ns;
    };

    struct timing_result
    {
        double median_ns = 0.0;
        std::size_t batches = 0;
        std::size_t operations = 0;
        std::vector<double> trials_ns;
        std::vector<double> elapsed_trials_ns;
    };

    struct implementation_info
    {
        std::string id;
        std::string short_label;
        std::string label;
        std::string api;
        bool enabled = true;
    };

    [[nodiscard]] inline implementation_info describe(
        implementations::identity identity,
        std::string_view api)
    {
        return {
            std::string(identity.id),
            std::string(identity.short_label),
            std::string(identity.label),
            std::string(api.empty() ? identity.api : api),
            identity.enabled
        };
    }

    template<class Value, class Eval>
    struct implementation_spec
    {
        using value_type = Value;

        implementation_info info;
        Eval evaluate;
    };

    template<class Value, class Eval>
    [[nodiscard]] auto implementation(
        implementations::identity identity,
        std::string_view api,
        Eval evaluate)
    {
        implementation_info info = describe(identity, api);
        info.enabled = info.enabled && external_implementations_enabled;
        return implementation_spec<Value, Eval>{std::move(info), std::move(evaluate)};
    }

    template<class Trial>
    struct task_spec
    {
        implementation_info info;
        Trial trial;
    };

    template<class Trial>
    [[nodiscard]] auto task(
        implementations::identity identity,
        std::string_view api,
        Trial trial)
    {
        implementation_info info = describe(identity, api);
        info.enabled = info.enabled && external_implementations_enabled;
        return task_spec<Trial>{std::move(info), std::move(trial)};
    }

    class checksum_accumulator
    {
    public:
        template<class First, class Second>
        void add(const std::pair<First, Second>& value)
        {
            add(value.first);
            add(value.second);
        }

        template<class Value>
        void add(const Value& value)
        {
            observer_.add(implementations::observe(value));
        }

        [[nodiscard]] std::uint64_t value() const noexcept
        {
            return observer_.value();
        }

    private:
        integer_observer observer_;
    };

    [[nodiscard]] inline std::string number(double value)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(6) << value;
        return out.str();
    }

    template<class Trial>
    [[nodiscard]] double elapsed_ns(std::size_t batches, Trial& trial)
    {
        std::atomic_signal_fence(std::memory_order_seq_cst);
        const auto start = std::chrono::steady_clock::now();
        timing_batches = batches;
        const std::uint64_t checksum = trial(timing_batches);
        timing_sink = checksum;
        std::atomic_signal_fence(std::memory_order_seq_cst);
        const auto elapsed = std::chrono::steady_clock::now() - start;
        return std::chrono::duration<double, std::nano>(elapsed).count();
    }

    template<class Trial>
    [[nodiscard]] std::size_t calibrated_batches(Trial& trial, double minimum_ns)
    {
        std::size_t batches = 1;
        for (int attempt = 0; attempt < 5; ++attempt)
        {
            const double elapsed = elapsed_ns(batches, trial);
            if (elapsed >= minimum_ns || batches == maximum_batches)
                break;

            const double scale = minimum_ns / std::max(elapsed, 1.0);
            const auto requested = static_cast<std::size_t>(
                std::ceil(static_cast<double>(batches) * scale * 1.1));
            batches = std::min(
                maximum_batches,
                std::max(batches + 1, requested));
        }
        return batches;
    }

    template<class Trial>
    [[nodiscard]] timing_result measure_ns(
        std::size_t operations_per_batch,
        std::size_t trials,
        double minimum_ns,
        Trial trial)
    {
        if (operations_per_batch == 0 || trials == 0)
            throw std::runtime_error("benchmark needs samples and trials");

        const std::size_t batches = calibrated_batches(trial, minimum_ns);
        timing_result result;
        result.batches = batches * trials;
        result.operations = operations_per_batch * batches * trials;
        result.trials_ns.reserve(trials);
        result.elapsed_trials_ns.reserve(trials);
        for (std::size_t i = 0; i < trials; ++i)
        {
            const double elapsed = elapsed_ns(batches, trial);
            result.elapsed_trials_ns.push_back(elapsed);
            result.trials_ns.push_back(
                elapsed / static_cast<double>(operations_per_batch * batches));
        }

        std::vector<double> sorted = result.trials_ns;
        std::sort(sorted.begin(), sorted.end());
        result.median_ns = sorted[sorted.size() / 2];
        return result;
    }

    template<class Float>
    class runner
    {
    public:
        runner(csv_writer& output, const options& settings)
            : output_(output), settings_(settings)
        {
            values_ = domains::moderate(
                std::min<std::size_t>(settings.samples, 4096), -4.0, 4.0).values;
        }

        template<class Value>
        [[nodiscard]] std::vector<Value> values() const
        {
            std::vector<Value> out;
            out.reserve(values_.size());
            for (const sample& value : values_)
                out.push_back(implementations::value_traits<Value>::from_sample(value));
            return out;
        }

        template<class Eval, class... Specs>
        void unary(
            std::string_view group,
            std::string_view operation,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            const auto inputs = samples::make(
                operation, settings_.samples, settings_.sample_mode);
            std::vector<measurement> measurements;
            measurements.push_back(make_unary(
                fltx_info(operation), inputs,
                implementation<Float>(
                    implementations::primary_identity<Float>,
                    fltx_api(operation),
                    std::move(evaluate))));
            (add_unary(measurements, inputs, std::move(comparisons)), ...);
            run(group, operation, inputs.size(), std::move(measurements));
        }

        template<class Eval, class... Specs>
        void unary_result(
            std::string_view group,
            std::string_view operation,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            const auto inputs = samples::make(
                operation, settings_.samples, settings_.sample_mode);
            std::vector<measurement> measurements;
            measurements.push_back(make_unary_result(
                fltx_info(operation), inputs,
                implementation<Float>(
                    implementations::primary_identity<Float>,
                    fltx_api(operation),
                    std::move(evaluate))));
            (add_unary_result(measurements, inputs, std::move(comparisons)), ...);
            run(group, operation, inputs.size(), std::move(measurements));
        }

        template<class Eval, class... Specs>
        void binary(
            std::string_view group,
            std::string_view operation,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            const auto inputs = samples::make(
                operation, settings_.samples, settings_.sample_mode);
            std::vector<measurement> measurements;
            measurements.push_back(make_binary(
                fltx_info(operation), inputs,
                implementation<Float>(
                    implementations::primary_identity<Float>,
                    fltx_api(operation),
                    std::move(evaluate))));
            (add_binary(measurements, inputs, std::move(comparisons)), ...);
            run(group, operation, inputs.size(), std::move(measurements));
        }

        template<class Eval, class... Specs>
        void binary_result(
            std::string_view group,
            std::string_view operation,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            const auto inputs = samples::make(
                operation, settings_.samples, settings_.sample_mode);
            std::vector<measurement> measurements;
            measurements.push_back(make_binary_result(
                fltx_info(operation), inputs,
                implementation<Float>(
                    implementations::primary_identity<Float>,
                    fltx_api(operation),
                    std::move(evaluate))));
            (add_binary_result(measurements, inputs, std::move(comparisons)), ...);
            run(group, operation, inputs.size(), std::move(measurements));
        }

        template<class Eval, class... Specs>
        void ternary(
            std::string_view group,
            std::string_view operation,
            Eval evaluate,
            Specs... comparisons)
        {
            if (!selected(operation))
                return;

            const auto inputs = samples::make(
                operation, settings_.samples, settings_.sample_mode);
            std::vector<measurement> measurements;
            measurements.push_back(make_ternary(
                fltx_info(operation), inputs,
                implementation<Float>(
                    implementations::primary_identity<Float>,
                    fltx_api(operation),
                    std::move(evaluate))));
            (add_ternary(measurements, inputs, std::move(comparisons)), ...);
            run(group, operation, inputs.size(), std::move(measurements));
        }

        template<class FltxTrial, class... Tasks>
        void workload(
            std::string_view operation,
            std::size_t iterations,
            FltxTrial fltx_trial,
            Tasks... comparisons)
        {
            if (!selected(operation))
                return;

            std::vector<measurement> measurements;
            measurements.push_back(make_workload_task(
                fltx_info(operation),
                iterations,
                task(
                    implementations::primary_identity<Float>,
                    "same workload",
                    std::move(fltx_trial))));
            (add_workload_task(
                measurements,
                iterations,
                std::move(comparisons)), ...);
            run("mixed_workloads", operation, iterations, std::move(measurements));
        }

        template<class FltxTrial, class... Tasks>
        void measured_task(
            std::string_view group,
            std::string_view operation,
            std::size_t operations_per_batch,
            FltxTrial fltx_trial,
            Tasks... comparisons)
        {
            if (!selected(operation))
                return;

            std::vector<measurement> measurements;
            measurements.push_back(make_task(
                fltx_info(operation),
                operations_per_batch,
                task(
                    implementations::primary_identity<Float>,
                    fltx_api(operation),
                    std::move(fltx_trial))));
            (add_task(
                measurements,
                operations_per_batch,
                std::move(comparisons)), ...);
            run(
                group,
                operation,
                operations_per_batch,
                std::move(measurements));
        }

    private:
        template<class Value>
        struct input_values
        {
            using value_type = Value;

            std::vector<Value> x;
            std::vector<Value> y;
            std::vector<Value> z;
        };

        struct measurement
        {
            implementation_info info;
            std::size_t operations_per_batch;
            std::function<double(std::size_t)> elapsed;
        };

        template<class Trial>
        [[nodiscard]] static measurement make_measurement(
            implementation_info info,
            std::size_t operations_per_batch,
            Trial trial)
        {
            auto shared_trial =
                std::make_shared<std::decay_t<Trial>>(std::move(trial));
            return {
                std::move(info),
                operations_per_batch,
                [shared_trial = std::move(shared_trial)](std::size_t batches) {
                    return elapsed_ns(batches, *shared_trial);
                }
            };
        }

        template<class Value>
        [[nodiscard]] static input_values<Value> convert_inputs(
            const std::vector<samples::input>& inputs)
        {
            input_values<Value> out;
            out.x.reserve(inputs.size());
            out.y.reserve(inputs.size());
            out.z.reserve(inputs.size());
            for (const samples::input& input : inputs)
            {
                out.x.push_back(
                    implementations::value_traits<Value>::from_sample(input.x));
                out.y.push_back(
                    implementations::value_traits<Value>::from_sample(input.y));
                out.z.push_back(
                    implementations::value_traits<Value>::from_sample(input.z));
            }
            return out;
        }

        [[nodiscard]] implementation_info fltx_info(
            std::string_view operation) const
        {
            return describe(
                implementations::primary_identity<Float>,
                fltx_api(operation));
        }

        [[nodiscard]] static std::string fltx_api(std::string_view operation)
        {
            return "bl::" + std::string(operation);
        }

        template<class Spec>
        [[nodiscard]] measurement make_unary(
            implementation_info,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            using value = typename Spec::value_type;
            auto converted = convert_inputs<value>(inputs).x;
            const std::size_t operations = converted.size();
            return make_measurement(
                std::move(spec.info),
                operations,
                [values = std::move(converted),
                 evaluate = std::move(spec.evaluate)](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        visit_rotated(values.size(), batch, [&](std::size_t i) {
                            const value result{ evaluate(values[i]) };
                            checksum.add(result);
                        });
                    }
                    return checksum.value();
                });
        }

        template<class Spec>
        [[nodiscard]] measurement make_unary_result(
            implementation_info,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            using value = typename Spec::value_type;
            auto converted = convert_inputs<value>(inputs).x;
            const std::size_t operations = converted.size();
            return make_measurement(
                std::move(spec.info),
                operations,
                [values = std::move(converted),
                 evaluate = std::move(spec.evaluate)](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        visit_rotated(values.size(), batch, [&](std::size_t i) {
                            checksum.add(evaluate(values[i]));
                        });
                    }
                    return checksum.value();
                });
        }

        template<class Spec>
        [[nodiscard]] measurement make_binary(
            implementation_info,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            using value = typename Spec::value_type;
            auto converted = convert_inputs<value>(inputs);
            const std::size_t operations = converted.x.size();
            return make_measurement(
                std::move(spec.info),
                operations,
                [values = std::move(converted),
                 evaluate = std::move(spec.evaluate)](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        visit_rotated(values.x.size(), batch, [&](std::size_t i) {
                            const value result{
                                evaluate(values.x[i], values.y[i])
                            };
                            checksum.add(result);
                        });
                    }
                    return checksum.value();
                });
        }

        template<class Spec>
        [[nodiscard]] measurement make_binary_result(
            implementation_info,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            using value = typename Spec::value_type;
            auto converted = convert_inputs<value>(inputs);
            const std::size_t operations = converted.x.size();
            return make_measurement(
                std::move(spec.info),
                operations,
                [values = std::move(converted),
                 evaluate = std::move(spec.evaluate)](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        visit_rotated(values.x.size(), batch, [&](std::size_t i) {
                            checksum.add(evaluate(values.x[i], values.y[i]));
                        });
                    }
                    return checksum.value();
                });
        }

        template<class Spec>
        [[nodiscard]] measurement make_ternary(
            implementation_info,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            using value = typename Spec::value_type;
            auto converted = convert_inputs<value>(inputs);
            const std::size_t operations = converted.x.size();
            return make_measurement(
                std::move(spec.info),
                operations,
                [values = std::move(converted),
                 evaluate = std::move(spec.evaluate)](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                    {
                        visit_rotated(values.x.size(), batch, [&](std::size_t i) {
                            const value result{ evaluate(
                                values.x[i], values.y[i], values.z[i]) };
                            checksum.add(result);
                        });
                    }
                    return checksum.value();
                });
        }

        template<class Spec>
        [[nodiscard]] measurement make_task(
            implementation_info,
            std::size_t operations_per_batch,
            Spec spec)
        {
            return make_measurement(
                std::move(spec.info),
                operations_per_batch,
                std::move(spec.trial));
        }

        template<class Spec>
        [[nodiscard]] measurement make_workload_task(
            implementation_info,
            std::size_t operations_per_batch,
            Spec spec)
        {
            return make_measurement(
                std::move(spec.info),
                operations_per_batch,
                [trial = std::move(spec.trial)](std::size_t batches) {
                    checksum_accumulator checksum;
                    for (std::size_t batch = 0; batch < batches; ++batch)
                        checksum.add(trial(batch));
                    return checksum.value();
                });
        }

        template<class Spec>
        void add_unary(
            std::vector<measurement>& out,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            if (spec.info.enabled)
                out.push_back(make_unary(spec.info, inputs, std::move(spec)));
        }

        template<class Spec>
        void add_unary_result(
            std::vector<measurement>& out,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            if (spec.info.enabled)
                out.push_back(
                    make_unary_result(spec.info, inputs, std::move(spec)));
        }

        template<class Spec>
        void add_binary(
            std::vector<measurement>& out,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            if (spec.info.enabled)
                out.push_back(make_binary(spec.info, inputs, std::move(spec)));
        }

        template<class Spec>
        void add_binary_result(
            std::vector<measurement>& out,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            if (spec.info.enabled)
                out.push_back(
                    make_binary_result(spec.info, inputs, std::move(spec)));
        }

        template<class Spec>
        void add_ternary(
            std::vector<measurement>& out,
            const std::vector<samples::input>& inputs,
            Spec spec)
        {
            if (spec.info.enabled)
                out.push_back(make_ternary(spec.info, inputs, std::move(spec)));
        }

        template<class Spec>
        void add_task(
            std::vector<measurement>& out,
            std::size_t operations_per_batch,
            Spec spec)
        {
            if (spec.info.enabled)
                out.push_back(make_task(
                    spec.info,
                    operations_per_batch,
                    std::move(spec)));
        }

        template<class Spec>
        void add_workload_task(
            std::vector<measurement>& out,
            std::size_t operations_per_batch,
            Spec spec)
        {
            if (spec.info.enabled)
            {
                out.push_back(make_workload_task(
                    spec.info,
                    operations_per_batch,
                    std::move(spec)));
            }
        }

        template<class Visit>
        static void visit_rotated(
            std::size_t count,
            std::size_t batch,
            Visit visit)
        {
            const std::size_t first = batch % count;
            for (std::size_t i = first; i < count; ++i)
                visit(i);
            for (std::size_t i = 0; i < first; ++i)
                visit(i);
        }

        [[nodiscard]] bool selected(std::string_view operation) const
        {
            return settings_.filter.empty() ||
                   operation.find(settings_.filter) != std::string_view::npos;
        }

        [[nodiscard]] static std::string trial_list(
            const std::vector<double>& trials)
        {
            std::string out;
            for (const double trial : trials)
            {
                if (!out.empty())
                    out.push_back('|');
                out += number(trial);
            }
            return out;
        }

        [[nodiscard]] static double total_elapsed(const timing_result& timing)
        {
            double total = 0.0;
            for (const double elapsed : timing.elapsed_trials_ns)
                total += elapsed;
            return total;
        }

        static void append_elapsed(
            timing_result& result,
            std::size_t operations_per_batch,
            std::size_t batches,
            double elapsed)
        {
            const std::size_t operations = operations_per_batch * batches;
            result.batches += batches;
            result.operations += operations;
            result.trials_ns.push_back(
                elapsed / static_cast<double>(operations));
            result.elapsed_trials_ns.push_back(elapsed);
        }

        static void finish_timing(timing_result& result)
        {
            std::vector<double> sorted = result.trials_ns;
            std::sort(sorted.begin(), sorted.end());
            result.median_ns = sorted[sorted.size() / 2];
        }

        void run(
            std::string_view group,
            std::string_view operation,
            std::size_t samples,
            std::vector<measurement> measurements)
        {
            if (measurements.empty() || measurements.front().info.id != "fltx")
                throw std::logic_error("benchmark operation must start with fltx");

            std::unordered_set<std::string> ids;
            for (const measurement& value : measurements)
            {
                if (!ids.insert(value.info.id).second)
                {
                    throw std::logic_error(
                        "duplicate benchmark implementation " + value.info.id);
                }
            }

            std::vector<timing_result> results(measurements.size());
            for (timing_result& result : results)
            {
                result.trials_ns.reserve(settings_.trials);
                result.elapsed_trials_ns.reserve(settings_.trials);
            }

            if (settings_.sample_mode == "small" ||
                settings_.sample_mode == "standard")
            {
                timing_policy::for_each_measurement_primary_first(
                    measurements.size(),
                    [&](std::size_t index) {
                        measurement& value = measurements[index];
                        const double pilot = value.elapsed(1);
                        const double ns_per_iteration = pilot /
                            static_cast<double>(value.operations_per_batch);
                        const auto policy = timing_policy::select(ns_per_iteration);
                        const auto calibration = timing_policy::calibrate_after_pilot(
                            value.elapsed,
                            pilot,
                            policy.target_trial_ns,
                            maximum_batches);

                        std::size_t requested =
                            std::min(settings_.trials, policy.trials);
                        if (requested % 2 == 0)
                            --requested;
                        requested = std::max<std::size_t>(1, requested);
                        const std::size_t trials = timing_policy::fitting_trials(
                            requested,
                            calibration.elapsed_ns);

                        if (calibration.elapsed_ns > timing_policy::maximum_row_ns)
                        {
                            std::cerr
                                << "warning: benchmark row exceeded the adaptive "
                                   "five-second budget during calibration; using "
                                   "that measurement as its sole trial: "
                                << implementations::precision_name<Float> << ' '
                                << operation << ' ' << value.info.id << '\n';
                            append_elapsed(
                                results[index],
                                value.operations_per_batch,
                                calibration.batches,
                                calibration.elapsed_ns);
                            return;
                        }

                        for (std::size_t trial = 0; trial < trials; ++trial)
                        {
                            append_elapsed(
                                results[index],
                                value.operations_per_batch,
                                calibration.batches,
                                value.elapsed(calibration.batches));
                        }
                    });
            }
            else
            {
                timing_policy::for_each_measurement_primary_first(
                    measurements.size(),
                    [&](std::size_t index) {
                        measurement& value = measurements[index];
                        const auto calibration = timing_policy::calibrate_once(
                            value.elapsed,
                            settings_.minimum_trial_ns,
                            maximum_batches);
                        for (std::size_t trial = 0;
                             trial < settings_.trials;
                             ++trial)
                        {
                            append_elapsed(
                                results[index],
                                value.operations_per_batch,
                                calibration.batches,
                                value.elapsed(calibration.batches));
                        }
                    });
            }
            for (timing_result& result : results)
                finish_timing(result);

            for (std::size_t i = 0; i < results.size(); ++i)
            {
                if (results[i].median_ns < minimum_credible_iteration_ns)
                {
                    throw std::runtime_error(
                        "benchmark produced an implausibly small timing; "
                        "the workload may have been optimized away: " +
                        std::string(implementations::precision_name<Float>) +
                        " " + std::string(operation) + " " +
                        measurements[i].info.id);
                }
            }

            const double fltx_ns = results.front().median_ns;
            for (std::size_t i = 0; i < measurements.size(); ++i)
            {
                const bool is_fltx = i == 0;
                output_.row({
                    std::string(schema_version),
                    settings_.run_id,
                    settings_.source_revision,
                    fltx::tests::support::source_fingerprint,
                    std::string(implementations::precision_name<Float>),
                    std::string(group),
                    std::string(operation),
                    measurements[i].info.id,
                    measurements[i].info.short_label,
                    measurements[i].info.label,
                    measurements[i].info.api,
                    std::to_string(samples),
                    number(results[i].median_ns),
                    trial_list(results[i].trials_ns),
                    std::to_string(results[i].operations),
                    number(total_elapsed(results[i])),
                    is_fltx
                        ? std::string{}
                        : number(results[i].median_ns / fltx_ns)
                });
            }

            std::cout << std::left
                      << std::setw(8)
                      << implementations::precision_name<Float>
                      << std::setw(34)
                      << operation
                      << std::right
                      << std::fixed
                      << std::setprecision(2)
                      << std::setw(12)
                      << fltx_ns
                      << " ns";
            for (std::size_t i = 1; i < measurements.size(); ++i)
            {
                std::cout << "  "
                          << std::setw(7)
                          << results[i].median_ns / fltx_ns
                          << "x "
                          << measurements[i].info.short_label;
            }
            if (measurements.size() == 1)
                std::cout << "  fltx only";
            std::cout << '\n' << std::flush;
        }

        csv_writer& output_;
        const options& settings_;
        std::vector<sample> values_;
    };

    void run_operations_dd(csv_writer& output, const options& settings);
    void run_operations_qd(csv_writer& output, const options& settings);
    void run_workloads_dd(csv_writer& output, const options& settings);
    void run_workloads_qd(csv_writer& output, const options& settings);
}

#endif
