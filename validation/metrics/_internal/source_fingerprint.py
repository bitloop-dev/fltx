#!/usr/bin/env python3
"""Identify the build- and result-affecting source inputs for metrics runners."""

from __future__ import annotations

import argparse
import hashlib
import os
import stat
import subprocess
from dataclasses import dataclass
from pathlib import Path


_HASH_FORMAT = b"fltx-runner-source-fingerprint-v3\0"
_RUNNER_ROOT_FILES = {
    "CMakeLists.txt",
    "CMakePresets.json",
    "vcpkg-configuration.json",
    "vcpkg.json",
}
_RUNNER_PREFIXES = (
    "cmake/",
    "include/",
    "src/",
    "validation/extern/qdpp/",
    "validation/extern/tlfloat/",
    "validation/accuracy/",
    "validation/benchmarks/runtime/",
    "validation/support/",
)


@dataclass(frozen=True)
class SourceIdentity:
    revision: str
    fingerprint: str


def valid_fingerprint(value: object) -> bool:
    return (
        isinstance(value, str)
        and len(value) == 64
        and all(character in "0123456789abcdef" for character in value)
    )


def _included(path: str) -> bool:
    normalized = path.replace("\\", "/").lstrip("./")
    return (
        normalized in _RUNNER_ROOT_FILES
        or normalized == "validation/CMakeLists.txt"
        or any(normalized.startswith(prefix) for prefix in _RUNNER_PREFIXES)
    )


def _git(root: Path, *arguments: str) -> bytes:
    return subprocess.check_output(
        ["git", *arguments],
        cwd=root,
        stderr=subprocess.DEVNULL,
    )


def _git_paths(root: Path, *arguments: str) -> list[str]:
    return [
        os.fsdecode(path)
        for path in _git(root, *arguments).split(b"\0")
        if path
    ]


def _fallback_paths(root: Path) -> list[str]:
    paths: list[str] = []
    for directory, names, files in os.walk(root):
        relative_directory = Path(directory).relative_to(root)
        names[:] = [
            name
            for name in names
            if name not in {".git", ".vs", "__pycache__", "build", "vcpkg_installed"}
            and not name.startswith("cmake-build-")
        ]
        for name in files:
            path = (relative_directory / name).as_posix()
            if _included(path):
                paths.append(path)
    return paths


def _index_modes(root: Path) -> dict[str, str]:
    modes: dict[str, str] = {}
    for entry in _git(root, "ls-files", "--stage", "-z").split(b"\0"):
        if not entry:
            continue
        metadata, raw_path = entry.split(b"\t", 1)
        mode, _, stage = metadata.split()
        if stage == b"0":
            modes[os.fsdecode(raw_path)] = mode.decode("ascii")
    return modes


def _worktree_mode(path: Path) -> str:
    mode = path.lstat().st_mode
    if stat.S_ISLNK(mode):
        return "120000"
    if stat.S_ISREG(mode):
        return "100755" if mode & stat.S_IXUSR else "100644"
    return f"special-{stat.S_IFMT(mode):o}"


def _clean_blob_ids(root: Path, paths: list[str]) -> dict[str, str]:
    """Hash working files through the same clean filters Git uses for blobs."""

    regular = [
        path
        for path in paths
        if "\n" not in path and "\r" not in path
    ]
    identities: dict[str, str] = {}
    if regular:
        output = subprocess.check_output(
            ["git", "hash-object", "--stdin-paths"],
            cwd=root,
            input=b"".join(os.fsencode(path) + b"\n" for path in regular),
            stderr=subprocess.DEVNULL,
        ).splitlines()
        if len(output) != len(regular):
            raise subprocess.CalledProcessError(1, ["git", "hash-object", "--stdin-paths"])
        identities.update(
            (path, blob.decode("ascii"))
            for path, blob in zip(regular, output)
        )

    # --stdin-paths is line-delimited. Preserve unusual but valid Git paths by
    # hashing those few files separately rather than making the common case
    # pay for one Git process per source file.
    for path in paths:
        if "\n" not in path and "\r" not in path:
            continue
        identities[path] = _git(
            root,
            "hash-object",
            f"--path={path}",
            "--",
            path,
        ).decode("ascii").strip()
    return identities


def _gitlink_state(root: Path, path: str) -> str:
    candidate = root.joinpath(*path.replace("\\", "/").split("/"))
    try:
        submodule_root = _git(
            candidate,
            "rev-parse",
            "--show-toplevel",
        ).decode(errors="surrogateescape").strip()
        if os.path.normcase(str(Path(submodule_root).resolve())) != os.path.normcase(
            str(candidate.resolve())
        ):
            return "missing"
        head = _git(
            candidate,
            "rev-parse",
            "--verify",
            "HEAD",
        ).decode("ascii").strip()
        status = _git(
            candidate,
            "status",
            "--porcelain=v1",
            "-z",
            "--untracked-files=all",
        )
        if not status:
            return head

        digest = hashlib.sha256()
        digest.update(_git(candidate, "diff", "--binary", "HEAD", "--"))
        untracked = _git_paths(
            candidate,
            "ls-files",
            "--others",
            "--exclude-standard",
            "-z",
        )
        for nested_path, blob in sorted(
            _clean_blob_ids(candidate, untracked).items()
        ):
            digest.update(nested_path.encode("utf-8", errors="surrogateescape"))
            digest.update(b"\0")
            digest.update(blob.encode("ascii"))
            digest.update(b"\0")
        return f"{head}-dirty-{digest.hexdigest()}"
    except (OSError, UnicodeError, subprocess.CalledProcessError):
        return "missing"


def _hash_git_tree(root: Path) -> str:
    modes = _index_modes(root)
    paths = sorted(set(_git_paths(
        root,
        "ls-files",
        "--cached",
        "--others",
        "--exclude-standard",
        "-z",
    )))

    files: list[str] = []
    identities: dict[str, str] = {}
    for path in paths:
        if not _included(path):
            continue
        candidate = root.joinpath(*path.replace("\\", "/").split("/"))
        if modes.get(path) == "160000":
            identities[path] = f"gitlink:{_gitlink_state(root, path)}"
        elif not candidate.exists() and not candidate.is_symlink():
            identities[path] = "missing"
        elif candidate.is_symlink():
            target = os.fsencode(os.readlink(candidate))
            blob = subprocess.check_output(
                ["git", "hash-object", "--stdin"],
                cwd=root,
                input=target,
                stderr=subprocess.DEVNULL,
            ).decode("ascii").strip()
            identities[path] = f"blob:{blob}"
        elif candidate.is_file():
            files.append(path)
        else:
            identities[path] = "special"

    identities.update(
        (path, f"blob:{blob}")
        for path, blob in _clean_blob_ids(root, files).items()
    )

    digest = hashlib.sha256()
    digest.update(_HASH_FORMAT)
    for path in paths:
        if not _included(path):
            continue
        candidate = root.joinpath(*path.replace("\\", "/").split("/"))
        mode = modes.get(path)
        if mode is None:
            try:
                mode = _worktree_mode(candidate)
            except FileNotFoundError:
                mode = "missing"
        digest.update(mode.encode("ascii"))
        digest.update(b"\0")
        digest.update(path.replace("\\", "/").encode(
            "utf-8",
            errors="surrogateescape",
        ))
        digest.update(b"\0")
        digest.update(identities[path].encode("ascii"))
        digest.update(b"\0")
    return digest.hexdigest()


def _hash_fallback_tree(root: Path, paths: list[str]) -> str:
    digest = hashlib.sha256()
    digest.update(_HASH_FORMAT)

    for path in sorted(set(paths)):
        normalized = path.replace("\\", "/")
        if not _included(normalized):
            continue
        digest.update(normalized.encode("utf-8", errors="surrogateescape"))
        digest.update(b"\0")
        candidate = root.joinpath(*normalized.split("/"))
        try:
            mode = candidate.lstat().st_mode
        except FileNotFoundError:
            digest.update(b"missing\0")
            continue

        if stat.S_ISLNK(mode):
            digest.update(b"symlink\0")
            digest.update(os.fsencode(os.readlink(candidate)))
        elif stat.S_ISREG(mode):
            digest.update(b"file\0")
            with candidate.open("rb") as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b""):
                    digest.update(block)
        else:
            digest.update(b"special\0")
        digest.update(b"\0")
    return digest.hexdigest()


def _git_is_dirty(root: Path) -> bool:
    changed = _git_paths(root, "diff", "--name-only", "-z", "HEAD", "--")
    if any(_included(path) for path in changed):
        return True
    return any(
        _included(path)
        for path in _git_paths(root, "ls-files", "--others", "--exclude-standard", "-z")
    )


def source_identity(root: Path) -> SourceIdentity:
    """Return a content fingerprint and a readable revision for ``root``."""

    root = root.resolve()
    try:
        revision = _git(root, "rev-parse", "--verify", "HEAD").decode("ascii").strip()
        fingerprint = _hash_git_tree(root)
        readable_revision = (
            f"{revision}-dirty-{fingerprint[:12]}"
            if _git_is_dirty(root)
            else revision
        )
        return SourceIdentity(readable_revision, fingerprint)
    except (OSError, UnicodeError, subprocess.CalledProcessError):
        fingerprint = _hash_fallback_tree(root, _fallback_paths(root))
        return SourceIdentity("unknown", fingerprint)


def write_header(path: Path, identity: SourceIdentity) -> bool:
    """Atomically update the generated C++ header, returning whether it changed."""

    content = (
        "#pragma once\n\n"
        f"#define FLTX_METRICS_BUILD_SOURCE_FINGERPRINT \"{identity.fingerprint}\"\n"
    )
    try:
        if path.read_text(encoding="utf-8") == content:
            return False
    except OSError:
        pass

    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".writing")
    temporary.write_text(content, encoding="utf-8", newline="\n")
    os.replace(temporary, path)
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    identity = source_identity(args.root)
    changed = write_header(args.output, identity)
    state = "updated" if changed else "unchanged"
    print(f"fltx source fingerprint {identity.fingerprint} ({state})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
