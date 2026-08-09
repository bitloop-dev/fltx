#!/usr/bin/env python3
"""Incrementally build runners, reuse compatible evidence, and render reports."""

from __future__ import annotations

import argparse
import copy
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping, Sequence

import build_profile_comparison
from build_tables import Target
import run_metrics


RUNNER_SETS = {
    "strict": ("fltx_accuracy", "fltx_benchmark"),
    "fastmath": ("fltx_accuracy_fastmath", "fltx_benchmark_fastmath"),
}
RUNNER_TARGETS = tuple(
    target
    for targets in RUNNER_SETS.values()
    for target in targets
)
_MACRO = re.compile(r"\$\{([^{}]+)\}|\$(p?env)\{([^{}]+)\}")


class PipelineError(ValueError):
    """The requested preset or generated CMake metadata is unusable."""


@dataclass(frozen=True)
class PresetSelection:
    build_name: str
    configure_name: str
    binary_dir: Path
    configuration: str | None
    generator: str
    compiler_hint: str
    target_hint: str
    environment: Mapping[str, str]


def _read_json(path: Path) -> dict[str, object]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise PipelineError(f"cannot read {path}: {error}") from error
    if not isinstance(value, dict):
        raise PipelineError(f"{path}: expected a JSON object")
    return value


def _expand_text(
    value: str,
    *,
    source_dir: Path,
    preset_name: str,
    generator: str,
    environment: Mapping[str, str],
    parent_environment: Mapping[str, str],
    file_dir: Path | None = None,
) -> str:
    fixed = {
        "sourceDir": str(source_dir),
        "sourceParentDir": str(source_dir.parent),
        "sourceDirName": source_dir.name,
        "presetName": preset_name,
        "generator": generator,
        "hostSystemName": platform.system(),
        "pathListSep": os.pathsep,
        "dollar": "$",
        "fileDir": str(file_dir or source_dir),
    }

    def replace(match: re.Match[str]) -> str:
        cmake_name, environment_kind, environment_name = match.groups()
        if cmake_name is not None:
            if cmake_name not in fixed:
                raise PipelineError(
                    f"unsupported CMake preset macro ${{{cmake_name}}} in {value!r}"
                )
            return fixed[cmake_name]
        assert environment_kind is not None and environment_name is not None
        values = parent_environment if environment_kind == "penv" else environment
        return values.get(environment_name, "")

    expanded = value
    for _ in range(20):
        updated = _MACRO.sub(replace, expanded)
        if updated == expanded:
            return updated
        expanded = updated
    raise PipelineError(f"cyclic CMake preset macro expansion in {value!r}")


def _include_path(path: Path, value: object, source_dir: Path) -> Path:
    if not isinstance(value, str) or not value:
        raise PipelineError(f"{path}: preset include entries must be non-empty strings")
    expanded = _expand_text(
        value,
        source_dir=source_dir,
        preset_name="",
        generator="",
        environment=os.environ,
        parent_environment=os.environ,
        file_dir=path.parent,
    )
    include = Path(expanded)
    if not include.is_absolute():
        include = path.parent / include
    return include.resolve()


def _collect_preset_files(root: Path) -> list[tuple[Path, dict[str, object]]]:
    source_dir = root.resolve()
    visited: set[Path] = set()
    active: set[Path] = set()
    documents: list[tuple[Path, dict[str, object]]] = []

    def visit(path: Path) -> None:
        path = path.resolve()
        if path in visited:
            return
        if path in active:
            raise PipelineError(f"cyclic CMake preset include involving {path}")
        active.add(path)
        document = _read_json(path)
        includes = document.get("include", [])
        if isinstance(includes, str):
            includes = [includes]
        if not isinstance(includes, list):
            raise PipelineError(f"{path}: include must be a string or list")
        for include in includes:
            visit(_include_path(path, include, source_dir))
        active.remove(path)
        visited.add(path)
        documents.append((path, document))

    project = source_dir / "CMakePresets.json"
    if not project.is_file():
        raise PipelineError(f"missing CMake preset file: {project}")
    visit(project)
    user = source_dir / "CMakeUserPresets.json"
    if user.is_file():
        visit(user)
    return documents


def _preset_catalog(
    documents: Sequence[tuple[Path, dict[str, object]]],
    key: str,
) -> dict[str, dict[str, object]]:
    catalog: dict[str, dict[str, object]] = {}
    for path, document in documents:
        values = document.get(key, [])
        if not isinstance(values, list):
            raise PipelineError(f"{path}: {key} must be a list")
        for value in values:
            if not isinstance(value, dict) or not isinstance(value.get("name"), str):
                raise PipelineError(f"{path}: invalid entry in {key}")
            name = value["name"]
            if name in catalog:
                raise PipelineError(f"duplicate {key} preset {name!r}")
            catalog[name] = copy.deepcopy(value)
    return catalog


def _merge_mapping(
    inherited: Mapping[str, object],
    child: Mapping[str, object],
) -> dict[str, object]:
    result = copy.deepcopy(dict(inherited))
    for key, value in child.items():
        if key == "inherits":
            continue
        if value is None:
            result.pop(key, None)
        elif isinstance(value, dict) and isinstance(result.get(key), dict):
            result[key] = _merge_mapping(result[key], value)
        else:
            result[key] = copy.deepcopy(value)
    return result


def _resolve_preset(
    catalog: Mapping[str, dict[str, object]],
    name: str,
    kind: str,
) -> dict[str, object]:
    resolved: dict[str, dict[str, object]] = {}
    active: set[str] = set()

    def resolve(current: str) -> dict[str, object]:
        if current in resolved:
            return copy.deepcopy(resolved[current])
        if current in active:
            raise PipelineError(f"cyclic inheritance in {kind} preset {current!r}")
        value = catalog.get(current)
        if value is None:
            raise PipelineError(f"unknown {kind} preset {current!r}")
        active.add(current)
        parents = value.get("inherits", [])
        if isinstance(parents, str):
            parents = [parents]
        if not isinstance(parents, list) or not all(
            isinstance(parent, str) for parent in parents
        ):
            raise PipelineError(f"{kind} preset {current!r} has invalid inheritance")

        combined: dict[str, object] = {}
        # CMake gives earlier parents precedence when multiple parents define
        # the same field, so merge them last.
        for parent in reversed(parents):
            combined = _merge_mapping(combined, resolve(parent))
        combined = _merge_mapping(combined, value)
        combined["name"] = current
        active.remove(current)
        resolved[current] = combined
        return copy.deepcopy(combined)

    return resolve(name)


def _expanded_environment(
    raw: object,
    *,
    source_dir: Path,
    preset_name: str,
    generator: str,
    parent_environment: Mapping[str, str],
) -> dict[str, str]:
    if raw is None:
        return dict(parent_environment)
    if not isinstance(raw, dict):
        raise PipelineError(f"configure preset {preset_name!r} has invalid environment")

    effective = dict(parent_environment)
    pending: dict[str, str] = {}
    for key, value in raw.items():
        if not isinstance(key, str):
            raise PipelineError(f"configure preset {preset_name!r} has invalid environment")
        if value is None:
            effective.pop(key, None)
        elif isinstance(value, str):
            pending[key] = value
        else:
            raise PipelineError(f"configure preset {preset_name!r} has invalid environment")

    for _ in range(max(1, len(pending) + 1)):
        changed = False
        for key, value in pending.items():
            expanded = _expand_text(
                value,
                source_dir=source_dir,
                preset_name=preset_name,
                generator=generator,
                environment=effective,
                parent_environment=parent_environment,
            )
            if effective.get(key) != expanded:
                effective[key] = expanded
                changed = True
        if not changed:
            break
    return effective


def _cache_string(cache: object, name: str) -> str:
    if not isinstance(cache, dict):
        return ""
    value = cache.get(name)
    if isinstance(value, dict):
        value = value.get("value")
    return value if isinstance(value, str) else ""


def resolve_preset(root: Path, name: str) -> PresetSelection:
    documents = _collect_preset_files(root)
    build_catalog = _preset_catalog(documents, "buildPresets")
    configure_catalog = _preset_catalog(documents, "configurePresets")
    build = _resolve_preset(build_catalog, name, "build")

    configure_name = build.get("configurePreset")
    if not isinstance(configure_name, str) or not configure_name:
        raise PipelineError(
            f"build preset {name!r} does not resolve to a configurePreset"
        )
    configure = _resolve_preset(configure_catalog, configure_name, "configure")
    binary_value = configure.get("binaryDir")
    if not isinstance(binary_value, str) or not binary_value:
        raise PipelineError(
            f"configure preset {configure_name!r} does not define binaryDir"
        )
    generator = configure.get("generator", "")
    if not isinstance(generator, str):
        raise PipelineError(f"configure preset {configure_name!r} has invalid generator")
    environment = _expanded_environment(
        configure.get("environment"),
        source_dir=root.resolve(),
        preset_name=configure_name,
        generator=generator,
        parent_environment=os.environ,
    )
    binary_text = _expand_text(
        binary_value,
        source_dir=root.resolve(),
        preset_name=configure_name,
        generator=generator,
        environment=environment,
        parent_environment=os.environ,
    )
    binary_dir = Path(binary_text)
    if not binary_dir.is_absolute():
        binary_dir = root / binary_dir

    configuration = build.get("configuration")
    if configuration is not None and not isinstance(configuration, str):
        raise PipelineError(f"build preset {name!r} has invalid configuration")
    cache = configure.get("cacheVariables")
    compiler_hint = (
        _cache_string(cache, "CMAKE_CXX_COMPILER")
        or environment.get("CXX", "")
    )
    target_hint = " ".join(
        value
        for value in (
            _cache_string(cache, "VCPKG_TARGET_TRIPLET"),
            _cache_string(cache, "VCPKG_CHAINLOAD_TOOLCHAIN_FILE"),
            _cache_string(cache, "CMAKE_TOOLCHAIN_FILE"),
        )
        if value
    )
    return PresetSelection(
        build_name=name,
        configure_name=configure_name,
        binary_dir=binary_dir.resolve(),
        configuration=configuration,
        generator=generator,
        compiler_hint=compiler_hint,
        target_hint=target_hint,
        environment=environment,
    )


def needs_msvc_x64_environment(
    selection: PresetSelection,
    *,
    host_system: str | None = None,
) -> bool:
    """Identify Windows Ninja presets that rely on the ambient MSVC toolchain."""

    if (host_system or platform.system()).casefold() != "windows":
        return False
    generator = selection.generator.casefold()
    if generator.startswith("visual studio"):
        return False
    if generator not in {"ninja", "ninja multi-config"}:
        return False

    hints = f"{selection.compiler_hint} {selection.target_hint}".casefold()
    alternatives = (
        "mingw",
        "emscripten",
        "clang",
        "gcc",
        "g++",
        "wasm32",
    )
    return not any(value in hints for value in alternatives)


def _environment_value(environment: Mapping[str, str], name: str) -> str:
    expected = name.casefold()
    return next(
        (
            value
            for key, value in environment.items()
            if key.casefold() == expected
        ),
        "",
    )


def _find_vsdevcmd(environment: Mapping[str, str]) -> Path:
    candidates: list[Path] = []
    installation = _environment_value(environment, "VSINSTALLDIR")
    if installation:
        candidates.append(Path(installation) / "Common7" / "Tools" / "VsDevCmd.bat")

    vswhere_candidates = []
    for variable in ("ProgramFiles(x86)", "ProgramFiles"):
        parent = _environment_value(environment, variable)
        if parent:
            vswhere_candidates.append(
                Path(parent) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
            )
    discovered = shutil.which(
        "vswhere.exe",
        path=_environment_value(environment, "PATH") or None,
    )
    if discovered:
        vswhere_candidates.append(Path(discovered))

    for vswhere in vswhere_candidates:
        if not vswhere.is_file():
            continue
        try:
            process = subprocess.run(
                [
                    str(vswhere),
                    "-latest",
                    "-products",
                    "*",
                    "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-property",
                    "installationPath",
                ],
                env=dict(environment),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=False,
            )
        except OSError:
            continue
        if process.returncode:
            continue
        paths = [line.strip() for line in process.stdout.splitlines() if line.strip()]
        if paths:
            candidates.append(
                Path(paths[-1]) / "Common7" / "Tools" / "VsDevCmd.bat"
            )

    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    raise PipelineError(
        "native Windows Ninja builds require the x64 MSVC developer environment, "
        "but VsDevCmd.bat could not be found"
    )


def msvc_x64_environment(
    parent_environment: Mapping[str, str] | None = None,
) -> dict[str, str]:
    """Capture a clean x64 MSVC command environment from Visual Studio."""

    parent = dict(
        os.environ if parent_environment is None else parent_environment
    )
    vsdevcmd = _find_vsdevcmd(parent)
    command = (
        f'call "{vsdevcmd}" -no_logo -arch=x64 -host_arch=x64 '
        ">nul && set"
    )
    try:
        process = subprocess.run(
            command,
            shell=True,
            executable=_environment_value(parent, "COMSPEC") or "cmd.exe",
            env=parent,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
    except OSError as error:
        raise PipelineError(f"cannot initialize the x64 MSVC environment: {error}") from error
    if process.returncode:
        detail = process.stderr.strip() or process.stdout.strip()
        suffix = f": {detail}" if detail else ""
        raise PipelineError(
            f"VsDevCmd.bat failed with status {process.returncode}{suffix}"
        )

    environment: dict[str, str] = {}
    for line in process.stdout.splitlines():
        if not line or line.startswith("=") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key:
            environment[key] = value
    if _environment_value(environment, "VSCMD_ARG_TGT_ARCH").casefold() != "x64":
        raise PipelineError("Visual Studio did not initialize an x64 target environment")
    if not _environment_value(environment, "LIB") or not _environment_value(
        environment,
        "PATH",
    ):
        raise PipelineError("Visual Studio returned an incomplete x64 build environment")
    return environment


def prepare_file_api_query(binary_dir: Path) -> None:
    query = binary_dir / ".cmake" / "api" / "v1" / "query" / "client-fltx-metrics"
    query.mkdir(parents=True, exist_ok=True)
    (query / "query.json").write_text(
        json.dumps(
            {"requests": [{"kind": "codemodel", "version": {"major": 2}}]},
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def _codemodel_path(binary_dir: Path) -> Path:
    reply = binary_dir / ".cmake" / "api" / "v1" / "reply"
    indexes = sorted(
        reply.glob("index-*.json"),
        key=lambda path: path.stat().st_mtime_ns,
        reverse=True,
    )
    for index_path in indexes:
        index = _read_json(index_path)
        client = index.get("reply", {})
        if not isinstance(client, dict):
            continue
        response = client.get("client-fltx-metrics", {})
        if not isinstance(response, dict):
            continue
        query = response.get("query.json", {})
        if not isinstance(query, dict):
            continue
        responses = query.get("responses", [])
        if not isinstance(responses, list):
            continue
        for item in responses:
            if (
                isinstance(item, dict)
                and item.get("kind") == "codemodel"
                and isinstance(item.get("jsonFile"), str)
            ):
                return reply / item["jsonFile"]
    raise PipelineError(
        f"CMake did not produce a File API codemodel under {reply}"
    )


def discover_runner_artifacts(
    binary_dir: Path,
    configuration: str | None,
) -> dict[str, Path]:
    codemodel_path = _codemodel_path(binary_dir)
    codemodel = _read_json(codemodel_path)
    configurations = codemodel.get("configurations")
    if not isinstance(configurations, list) or not configurations:
        raise PipelineError(f"{codemodel_path}: missing configurations")

    selected: dict[str, object] | None = None
    if configuration is not None:
        selected = next(
            (
                value
                for value in configurations
                if isinstance(value, dict)
                and str(value.get("name", "")).casefold() == configuration.casefold()
            ),
            None,
        )
        if selected is None:
            names = [
                str(value.get("name", ""))
                for value in configurations
                if isinstance(value, dict)
            ]
            raise PipelineError(
                f"CMake File API has no {configuration!r} configuration "
                f"(available: {', '.join(names)})"
            )
    elif len(configurations) == 1 and isinstance(configurations[0], dict):
        selected = configurations[0]
    else:
        raise PipelineError(
            "multi-config build preset must specify its configuration"
        )
    assert selected is not None

    targets = selected.get("targets")
    if not isinstance(targets, list):
        raise PipelineError(f"{codemodel_path}: selected configuration has no targets")
    reply = codemodel_path.parent
    artifacts: dict[str, Path] = {}
    for target_name in RUNNER_TARGETS:
        references = [
            value
            for value in targets
            if isinstance(value, dict) and value.get("name") == target_name
        ]
        if len(references) != 1 or not isinstance(references[0].get("jsonFile"), str):
            raise PipelineError(
                f"CMake File API expected one {target_name!r} target, "
                f"found {len(references)}"
            )
        target_path = reply / references[0]["jsonFile"]
        target = _read_json(target_path)
        if target.get("type") != "EXECUTABLE":
            raise PipelineError(f"{target_path}: {target_name} is not an executable")
        values = target.get("artifacts")
        if not isinstance(values, list) or not values:
            raise PipelineError(f"{target_path}: {target_name} has no artifact")
        candidates = [
            value["path"]
            for value in values
            if isinstance(value, dict) and isinstance(value.get("path"), str)
        ]
        name_on_disk = target.get("nameOnDisk")
        if isinstance(name_on_disk, str):
            named = [value for value in candidates if Path(value).name == name_on_disk]
            if named:
                candidates = named
        if len(candidates) != 1:
            raise PipelineError(
                f"{target_path}: cannot identify the primary {target_name} artifact"
            )
        artifact = Path(candidates[0])
        if not artifact.is_absolute():
            artifact = binary_dir / artifact
        artifact = artifact.resolve()
        if not artifact.is_file():
            raise PipelineError(f"built runner artifact does not exist: {artifact}")
        artifacts[target_name] = artifact
    return artifacts


def _run(
    command: Sequence[str],
    root: Path,
    environment: Mapping[str, str] | None = None,
) -> None:
    print(f"> {subprocess.list2cmdline(list(command))}", flush=True)
    try:
        result = subprocess.run(
            command,
            cwd=root,
            env=dict(environment) if environment is not None else None,
            check=False,
        )
    except OSError as error:
        raise PipelineError(f"cannot run {command[0]}: {error}") from error
    if result.returncode:
        raise PipelineError(
            f"command exited with status {result.returncode}: "
            f"{subprocess.list2cmdline(list(command))}"
        )


def metrics_root(root: Path, release: bool) -> Path:
    return root / ("validation/metrics" if release else "build/metrics")


def existing_runner_artifacts(
    root: Path,
    selection: PresetSelection,
    sample_mode: str,
    environment: Mapping[str, str] | None = None,
) -> dict[str, Path] | None:
    """Return source-current runner artifacts without invoking CMake."""

    try:
        artifacts = discover_runner_artifacts(
            selection.binary_dir,
            selection.configuration,
        )
        identities = set()
        for consumer_mode, (accuracy_target, benchmark_target) in RUNNER_SETS.items():
            identities.add(
                run_metrics.validate_runner_artifacts(
                    root,
                    {
                        "fltx_accuracy": artifacts[accuracy_target],
                        "fltx_benchmark": artifacts[benchmark_target],
                    },
                    sample_mode=sample_mode,
                    consumer_mode=consumer_mode,
                    environment=environment,
                )
            )
        if len(identities) != 1:
            raise PipelineError(
                "strict and consumer-fast-math runners identify different targets"
            )
        platform_name, compiler_name = next(iter(identities))
    except (PipelineError, run_metrics.MetricsError, OSError) as error:
        print(
            f"existing metrics runners cannot be reused: {error}; "
            "configuring/building",
            flush=True,
        )
        return None

    print(
        "existing metrics runners match the current source fingerprint "
        f"for {platform_name}/{compiler_name}; CMake configure/build skipped",
        flush=True,
    )
    return artifacts


def run_pipeline(
    root: Path,
    preset: str,
    *,
    release: bool = False,
    force_rerun: bool = False,
    consumer_mode: str = "strict",
) -> list[Path]:
    root = root.resolve()
    if consumer_mode == "all":
        requested_modes = tuple(RUNNER_SETS)
    elif consumer_mode in RUNNER_SETS:
        requested_modes = (consumer_mode,)
    else:
        raise PipelineError(f"unsupported consumer mode {consumer_mode!r}")
    sample_mode = "full" if release else "standard"
    output_root = metrics_root(root, release)
    selection = resolve_preset(root, preset)
    environment = selection.environment
    artifacts = existing_runner_artifacts(
        root,
        selection,
        sample_mode,
        environment,
    )
    if artifacts is None:
        cmake_environment: Mapping[str, str] | None = None
        if needs_msvc_x64_environment(selection):
            cmake_environment = msvc_x64_environment()
            environment = cmake_environment
        prepare_file_api_query(selection.binary_dir)
        try:
            _run(
                ["cmake", "--preset", selection.configure_name],
                root,
                cmake_environment,
            )
        except PipelineError as error:
            hints = f"{selection.compiler_hint} {selection.target_hint}".casefold()
            if "mingw" not in hints:
                raise
            raise PipelineError(
                f"{error}\nMinGW preset {selection.configure_name!r} requires "
                f"a MinGW compiler and {selection.generator} in the invoking "
                "environment or a local CMake user preset; the vcpkg triplet "
                "selects dependency binaries but does not provide these tools."
            ) from error
        _run(
            [
                "cmake",
                "--build",
                "--preset",
                selection.build_name,
                "--target",
                *RUNNER_TARGETS,
            ],
            root,
            cmake_environment,
        )
        artifacts = discover_runner_artifacts(
            selection.binary_dir,
            selection.configuration,
        )

    generated: list[Path] = []
    output = output_root / "generated"
    comparison_target: str | None = None
    comparison_input_root: str | None = None
    comparison_identity: tuple[str, str] | None = None
    for mode in requested_modes:
        accuracy_target, benchmark_target = RUNNER_SETS[mode]
        handoff = (
            root
            / "build"
            / "metrics"
            / "pipeline"
            / uuid.uuid4().hex
            / f"{mode}_run_result.json"
        )
        _run(
            [
                sys.executable,
                str(
                    root
                    / "validation"
                    / "metrics"
                    / "_internal"
                    / "run_metrics.py"
                ),
                "--accuracy",
                str(artifacts[accuracy_target]),
                "--benchmark",
                str(artifacts[benchmark_target]),
                "--consumer-mode",
                mode,
                "--sample-mode",
                sample_mode,
                "--output-root",
                str(output_root / "data"),
                "--result-file",
                str(handoff),
                *([] if force_rerun else ["--reuse-compatible"]),
            ],
            root,
            environment,
        )
        result = _read_json(handoff)
        expected_fields = {
            "schema_version",
            "run_id",
            "sample_mode",
            "consumer_mode",
            "platform",
            "compiler",
            "target",
            "input_root",
            "metadata",
        }
        if (
            set(result) != expected_fields
            or result.get("sample_mode") != sample_mode
            or result.get("consumer_mode") != mode
        ):
            raise PipelineError(f"{handoff}: invalid metrics result handoff")
        target = result.get("target")
        input_root = result.get("input_root")
        if not isinstance(target, str) or not isinstance(input_root, str):
            raise PipelineError(f"{handoff}: invalid metrics target or input root")
        identity = (str(result["platform"]), str(result["compiler"]))
        if comparison_target is None:
            comparison_target = target
            comparison_input_root = input_root
            comparison_identity = identity
        elif (
            comparison_target != target
            or comparison_input_root != input_root
            or comparison_identity != identity
        ):
            raise PipelineError(
                "consumer modes produced incompatible metrics targets"
            )

        _run(
            [
                sys.executable,
                str(root / "validation" / "metrics" / "rebuild_tables.py"),
                "--input",
                input_root,
                "--output",
                str(output),
                "--targets",
                target,
                "--consumer-mode",
                mode,
            ],
            root,
            environment,
        )
        mode_suffix = run_metrics.consumer_mode_suffix(mode)
        mode_outputs = [
            output / "accuracy" / f"accuracy_table{mode_suffix}.svg",
            output / "performance" / f"performance_table{mode_suffix}.svg",
            output
            / "performance"
            / f"performance_table{mode_suffix}_compact.svg",
        ]
        for precision in run_metrics.PRECISIONS:
            base = (
                f"{result['platform']}_{result['compiler']}_{precision}"
                f"{mode_suffix}_overview"
            )
            mode_outputs.extend(
                (
                    output / "overview" / f"{base}.svg",
                    output / "overview" / f"{base}_compact.svg",
                )
            )
        missing = [path for path in mode_outputs if not path.is_file()]
        if missing:
            raise PipelineError(
                "renderers did not create expected outputs: "
                + ", ".join(str(path) for path in missing)
            )
        generated.extend(mode_outputs)

    assert comparison_target is not None
    assert comparison_input_root is not None
    assert comparison_identity is not None
    comparison_output = output / "profile_comparison"
    _run(
        [
            sys.executable,
            str(
                root
                / "validation"
                / "metrics"
                / "_internal"
                / "build_profile_comparison.py"
            ),
            "--input",
            comparison_input_root,
            "--output",
            str(comparison_output),
            "--target",
            comparison_target,
            *(["--required"] if consumer_mode == "all" else []),
        ],
        root,
        environment,
    )
    platform_name, compiler_name = comparison_identity
    comparison_outputs = build_profile_comparison.output_paths(
        comparison_output,
        Target(platform_name, compiler_name),
    )
    existing_comparisons = tuple(
        path for path in comparison_outputs if path.is_file()
    )
    if existing_comparisons and len(existing_comparisons) != len(
        comparison_outputs
    ):
        raise PipelineError(
            "profile comparison renderer created an incomplete output set"
        )
    if consumer_mode == "all" and not existing_comparisons:
        raise PipelineError(
            "profile comparison renderer did not create required outputs"
        )
    generated.extend(existing_comparisons)
    return generated


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", required=True, help="CMake build preset")
    parser.add_argument(
        "--release",
        action="store_true",
        help="run and publish the full canonical metrics profile",
    )
    parser.add_argument(
        "--force-rerun",
        action="store_true",
        help=(
            "run the metrics phases even when compatible evidence exists; "
            "source-current runner executables are still reused"
        ),
    )
    parser.add_argument(
        "--consumer-mode",
        choices=("strict", "fastmath", "all"),
        default="strict",
        help="run strict consumers, fast-math consumers, or both in sequence",
    )
    return parser.parse_args(argv)


def main(root: Path, argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        outputs = run_pipeline(
            root,
            args.preset,
            release=args.release,
            force_rerun=args.force_rerun,
            consumer_mode=args.consumer_mode,
        )
    except PipelineError as error:
        print(f"preset metrics failed: {error}", file=sys.stderr)
        return 1
    print("generated metrics reports:", flush=True)
    for output in outputs:
        print(output, flush=True)
    return 0
