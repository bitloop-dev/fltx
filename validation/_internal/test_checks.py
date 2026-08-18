#!/usr/bin/env python3
"""Focused tests for the local CI-check orchestration commands."""

from __future__ import annotations

import io
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import mock

import check_pipeline
import supported_check_pipeline
from preset_support import PipelineError, PresetSelection


def selection() -> PresetSelection:
    return PresetSelection(
        build_name="native-build",
        configure_name="native-configure",
        binary_dir=Path("build/native"),
        configuration=None,
        generator="Ninja",
        compiler_hint="g++",
        target_hint="x64-linux",
        environment={"PRESET_ENV": "active"},
    )


class CheckPipelineTests(unittest.TestCase):
    def test_preset_checks_configure_then_build_the_ci_target(self) -> None:
        root = Path("source")
        selected = selection()
        with (
            mock.patch.object(
                check_pipeline,
                "resolve_preset",
                return_value=selected,
            ),
            mock.patch.object(
                check_pipeline,
                "needs_msvc_environment",
                return_value=False,
            ),
            mock.patch.object(check_pipeline, "_run") as run,
        ):
            check_pipeline.run_checks(root, "native-build")

        resolved_root = root.resolve()
        self.assertEqual(
            run.call_args_list,
            [
                mock.call(
                    ["cmake", "--preset", "native-configure"],
                    resolved_root,
                    selected.environment,
                ),
                mock.call(
                    [
                        "cmake",
                        "--build",
                        "--preset",
                        "native-build",
                        "--target",
                        "fltx_ci_checks",
                        "--parallel",
                    ],
                    resolved_root,
                    selected.environment,
                ),
            ],
        )

    def test_preset_checks_initialize_the_msvc_environment(self) -> None:
        selected = selection()
        developer_environment = {"VSCMD_ARG_TGT_ARCH": "x64"}
        with (
            mock.patch.object(
                check_pipeline,
                "resolve_preset",
                return_value=selected,
            ),
            mock.patch.object(
                check_pipeline,
                "needs_msvc_environment",
                return_value=True,
            ),
            mock.patch.object(
                check_pipeline,
                "msvc_target_architecture",
                return_value="x64",
            ),
            mock.patch.object(
                check_pipeline,
                "msvc_environment",
                return_value=developer_environment,
            ) as environment,
            mock.patch.object(check_pipeline, "_run") as run,
        ):
            check_pipeline.run_checks(Path("source"), "native-build")

        environment.assert_called_once_with(
            "x64",
            parent_environment=selected.environment,
        )
        self.assertTrue(all(
            call.args[2] is developer_environment for call in run.call_args_list
        ))

    def test_preset_main_reports_a_failed_check(self) -> None:
        errors = io.StringIO()
        with (
            mock.patch.object(
                check_pipeline,
                "run_checks",
                side_effect=PipelineError("contract failure"),
            ),
            redirect_stderr(errors),
        ):
            status = check_pipeline.main(
                Path("source"),
                ["--preset", "native-build"],
            )

        self.assertEqual(status, 1)
        self.assertIn("preset checks failed: contract failure", errors.getvalue())

    def test_all_supported_checks_run_each_host_preset(self) -> None:
        output = io.StringIO()
        with (
            mock.patch.object(
                supported_check_pipeline,
                "supported_presets",
                return_value=("first-release", "second-release"),
            ),
            mock.patch.object(check_pipeline, "run_checks") as run,
            redirect_stdout(output),
        ):
            status = supported_check_pipeline.main(Path("source"), [])

        self.assertEqual(status, 0)
        self.assertEqual(
            run.call_args_list,
            [
                mock.call(Path("source"), "first-release"),
                mock.call(Path("source"), "second-release"),
            ],
        )
        self.assertIn("running check preset 1/2: first-release", output.getvalue())
        self.assertIn("preset checks passed: second-release", output.getvalue())

    def test_all_supported_checks_stop_at_the_first_failure(self) -> None:
        errors = io.StringIO()
        with (
            mock.patch.object(
                supported_check_pipeline,
                "supported_presets",
                return_value=(
                    "first-release",
                    "second-release",
                    "third-release",
                ),
            ),
            mock.patch.object(
                check_pipeline,
                "run_checks",
                side_effect=(None, PipelineError("missing toolchain")),
            ) as run,
            redirect_stdout(io.StringIO()),
            redirect_stderr(errors),
        ):
            status = supported_check_pipeline.main(Path("source"), [])

        self.assertEqual(status, 1)
        self.assertEqual(run.call_count, 2)
        self.assertIn(
            "all-supported checks failed for second-release: missing toolchain",
            errors.getvalue(),
        )


if __name__ == "__main__":
    unittest.main()
