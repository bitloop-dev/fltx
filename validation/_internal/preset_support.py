#!/usr/bin/env python3
"""Shared CMake preset and host-toolchain support for validation workflows."""

from __future__ import annotations

import copy
import json
import os
import platform
import re
import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping, Sequence


WASM_PRESET = "wasm32-emscripten-release"
SUPPORTED_PRESETS = {
    ("Windows", "x86_64"): (
        "windows-x64-msvc-release",
        "windows-x64-clangcl-release",
        "windows-x64-mingw-release",
        WASM_PRESET,
    ),
    ("Windows", "arm64"): (
        "windows-arm64-msvc-release",
        "windows-arm64-clangcl-release",
    ),
    ("Linux", "x86_64"): (
        "linux-x64-gcc-release",
        "linux-x64-clang-release",
    ),
    ("Linux", "arm64"): (
        "linux-arm64-gcc-release",
        "linux-arm64-clang-release",
    ),
    ("Darwin", "x86_64"): (
        "macos-x64-appleclang-release",
    ),
    ("Darwin", "arm64"): (
        "macos-arm64-appleclang-release",
    ),
}
_ARCHITECTURE_ALIASES = {
    "amd64": "x86_64",
    "x64": "x86_64",
    "x86_64": "x86_64",
    "aarch64": "arm64",
    "arm64": "arm64",
}
_MACRO = re.compile(r"\$\{([^{}]+)\}|\$(p?env)\{([^{}]+)\}")


class PipelineError(ValueError):
    """The requested preset or host toolchain is unusable."""


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


def supported_presets(
    host_system: str | None = None,
    host_machine: str | None = None,
) -> tuple[str, ...]:
    system = host_system or platform.system()
    machine = host_machine or platform.machine()
    architecture = _ARCHITECTURE_ALIASES.get(machine.casefold(), machine.casefold())
    presets = SUPPORTED_PRESETS.get((system, architecture))
    if presets is None:
        raise PipelineError(f"unsupported validation host {system}/{machine}")
    return presets


def read_json(path: Path) -> dict[str, object]:
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
        document = read_json(path)
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


def needs_msvc_environment(
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
    alternatives = ("mingw", "emscripten", "gcc", "g++", "wasm32")
    return not any(value in hints for value in alternatives)


def msvc_target_architecture(selection: PresetSelection) -> str:
    hints = f"{selection.compiler_hint} {selection.target_hint}".casefold()
    return "arm64" if "arm64" in hints else "x64"


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


def msvc_environment(
    target_architecture: str,
    parent_environment: Mapping[str, str] | None = None,
) -> dict[str, str]:
    """Capture a clean native MSVC/clang-cl environment from Visual Studio."""

    if target_architecture not in {"x64", "arm64"}:
        raise PipelineError(
            f"unsupported Visual Studio target architecture {target_architecture!r}"
        )

    parent = dict(os.environ if parent_environment is None else parent_environment)
    vsdevcmd = _find_vsdevcmd(parent)
    command = (
        f'call "{vsdevcmd}" -no_logo -arch={target_architecture} '
        f'-host_arch={target_architecture} '
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
        raise PipelineError(
            f"cannot initialize the {target_architecture} Visual Studio "
            f"environment: {error}"
        ) from error
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
    if (
        _environment_value(environment, "VSCMD_ARG_TGT_ARCH").casefold()
        != target_architecture
    ):
        raise PipelineError(
            f"Visual Studio did not initialize a {target_architecture} target environment"
        )
    if not _environment_value(environment, "LIB") or not _environment_value(
        environment,
        "PATH",
    ):
        raise PipelineError(
            f"Visual Studio returned an incomplete {target_architecture} build environment"
        )
    return environment
