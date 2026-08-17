#include "runner.hpp"
#include "../../support/config_banner.hpp"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
    [[nodiscard]] bool describes_build(int argc, char** argv) noexcept
    {
        return argc == 2 && std::string_view{ argv[1] } == "--describe";
    }

    [[nodiscard]] fltx::tests::benchmark::options parse_arguments(int argc, char** argv)
    {
        fltx::tests::benchmark::options out;
        bool sample_mode_set = false;
        bool samples_set = false;
        bool trials_set = false;
        for (int i = 1; i < argc; ++i)
        {
            const std::string argument = argv[i];
            auto value = [&]() -> std::string {
                if (++i >= argc)
                    throw std::runtime_error("missing value after " + argument);
                return argv[i];
            };

            if (argument == "--precision")
                out.precision = value();
            else if (argument == "--output")
                out.output = value();
            else if (argument == "--run-id")
                out.run_id = value();
            else if (argument == "--source-revision")
                out.source_revision = value();
            else if (argument == "--filter")
                out.filter = value();
            else if (argument == "--samples")
            {
                out.samples = static_cast<std::size_t>(std::stoull(value()));
                samples_set = true;
            }
            else if (argument == "--trials")
            {
                out.trials = static_cast<std::size_t>(std::stoull(value()));
                trials_set = true;
            }
            else if (argument == "--sample-mode")
            {
                out.sample_mode = value();
                sample_mode_set = true;
                if (out.sample_mode != "smoke" && out.sample_mode != "small" &&
                    out.sample_mode != "standard" && out.sample_mode != "full")
                    throw std::runtime_error(
                        "--sample-mode must be smoke, small, standard or full");
            }
            else if (argument == "--help")
            {
                std::cout
                    << "fltx_benchmark --precision f128|f256 --output FILE "
                       "--run-id ID --source-revision REV "
                       "[--sample-mode smoke|small|standard|full] [--samples N] "
                       "[--trials N (maximum for small/standard)] "
                       "[--filter TEXT]\n"
                       "fltx_benchmark --describe\n";
                std::exit(0);
            }
            else
                throw std::runtime_error("unknown argument: " + argument);
        }

        if (out.precision != "f128" && out.precision != "f256")
            throw std::runtime_error("--precision must be f128 or f256");
        if (sample_mode_set)
        {
            if (!samples_set)
            {
                if (out.sample_mode == "smoke")
                    out.samples = 12;
                else if (out.sample_mode == "small")
                    out.samples = out.precision == "f128" ? 4096 : 2048;
                else if (out.sample_mode == "standard")
                    out.samples = out.precision == "f128" ? 8192 : 4096;
                else
                    out.samples = out.precision == "f128" ? 81920 : 40960;
            }
            if (!trials_set)
                out.trials = out.sample_mode == "smoke" ? 3 : 7;
            out.minimum_trial_ns = out.sample_mode == "smoke"
                ? fltx::tests::benchmark::smoke_minimum_trial_ns
                : out.sample_mode == "small" || out.sample_mode == "standard"
                    ? fltx::tests::benchmark::standard_minimum_trial_ns
                    : fltx::tests::benchmark::full_minimum_trial_ns;
        }
        else if (samples_set || trials_set)
        {
            out.sample_mode = "custom";
        }
        if (out.output.empty() || out.run_id.empty() || out.source_revision.empty())
            throw std::runtime_error("--output, --run-id and --source-revision are required");
        if (out.samples == 0 || out.trials == 0)
            throw std::runtime_error("--samples and --trials must be greater than zero");
        return out;
    }
}

int main(int argc, char** argv)
{
    try
    {
        if (describes_build(argc, argv))
        {
            fltx::tests::support::print_config_banner("benchmark");
            return 0;
        }

        const auto settings = parse_arguments(argc, argv);
        fltx::tests::support::print_config_banner(
            "benchmark",
            settings.sample_mode.c_str(),
            settings.samples,
            settings.trials);
        std::cout << "[timing] minimum-trial-ms="
                  << settings.minimum_trial_ns / 1'000'000.0 << '\n';
        if (settings.sample_mode == "small" || settings.sample_mode == "standard")
        {
            std::cout
                << "[timing] policy="
                << fltx::tests::benchmark::timing_policy::identity
                << " fast-threshold-ns="
                << fltx::tests::benchmark::timing_policy::fast_threshold_ns
                << " slow-threshold-ns="
                << fltx::tests::benchmark::timing_policy::slow_threshold_ns
                << " fast-trials="
                << fltx::tests::benchmark::timing_policy::fast_trials
                << " ordinary-trials="
                << fltx::tests::benchmark::timing_policy::ordinary_trials
                << " fast-target-ms="
                << fltx::tests::benchmark::timing_policy::fast_trial_target_ns /
                    1'000'000.0
                << " normal-target-ms="
                << fltx::tests::benchmark::timing_policy::normal_trial_target_ns /
                    1'000'000.0
                << " maximum-row-ms="
                << fltx::tests::benchmark::timing_policy::maximum_row_ns /
                    1'000'000.0
                << '\n';
        }
        fltx::tests::csv_writer output(settings.output);
        output.row({
            "schema_version", "run_id", "source_revision", "source_fingerprint",
            "precision", "group", "operation",
            "implementation", "implementation_short", "implementation_label",
            "api", "samples", "ns_iter", "trials_ns", "iterations",
            "elapsed_ns", "speed_ratio"
        });
        const std::size_t first_result_row = output.rows_written();

        if (settings.precision == "f128")
        {
            fltx::tests::benchmark::run_operations_f128(output, settings);
            fltx::tests::benchmark::run_workloads_f128(output, settings);
        }
        else
        {
            fltx::tests::benchmark::run_operations_f256(output, settings);
            fltx::tests::benchmark::run_workloads_f256(output, settings);
        }

        const std::size_t result_rows = output.rows_written() - first_result_row;
        if (result_rows == 0)
            throw std::runtime_error("benchmark filter matched no operations");

        output.finish();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "fltx_benchmark: " << error.what() << '\n';
        return 2;
    }
}
