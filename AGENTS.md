# fltx repository guidance

## Start with the current source

- Check the worktree before editing and preserve unrelated changes.
- Locate the exact first-party owner with repository search, then read it before
  changing behaviour. Start in `include`, `src`, `cmake`, `validation`, and
  `examples`; do not scan vendored, generated, or build trees unless they are
  part of the task. `.rgignore` keeps those trees out of normal searches; use
  `rg --no-ignore` with an explicit path when they are intentionally in scope.
- Read `docs/architecture.md` for unfamiliar or cross-cutting work. Treat the
  source as authoritative if documentation and implementation disagree, and
  correct the documentation when the discrepancy is architectural.
- Use `validation/COVERAGE.md` to find the existing validation owner and
  `validation/README.md` for current target and metrics semantics.

## Architecture and API policy

- Consider architectural consequences before implementing. Ask for
  confirmation when a proposal would weaken an established boundary, add an
  awkward dependency, or create a competing implementation path. Simple local
  changes with limited side effects do not need a pause.
- Public headers under `include/fltx` expose the supported API. Reusable
  mechanics belong under `include/fltx/detail`; runtime-only or heavyweight
  work belongs in `src` when putting it in a header would harm compile time,
  dependency exposure, or clarity.
- Keep validation-only dependencies and support code out of the library.
- Check constant-evaluation and runtime paths together for arithmetic, math,
  conversion, and parsing changes. A fix in one route is not assumed to cover
  the other.
- Be cautious with new includes in public headers. Every public header must
  remain independently compilable.
- The project has no external compatibility obligation before its first public
  release. Prefer the cleanest coherent API, update first-party callers and
  validation together, and do not add deprecated aliases or forwarding shims
  unless explicitly requested.
- Keep tightly coupled design, implementation, and verification together.
  Delegation is optional and is useful only for bounded work that can proceed
  independently without overlapping edits; the primary agent owns integration.

## Build and verification

- Inspect available presets with `cmake --list-presets=all`. Checked-in
  `CMakePresets.json` contains portable project presets; ignored
  `CMakeUserPresets.json` may provide machine-local toolchains or isolated build
  trees. Never assume a local preset name, tool path, or dependency location.
- For Emscripten, set `EMSDK` to the active emsdk root and make its required
  tools, including Node, available on `PATH`.
- Run the smallest relevant check first, then broaden in proportion to the
  change. Report what ran and what was not exercised. The authoritative target
  descriptions and commands are in `validation/README.md`.
- Extend an existing contract, constexpr corpus, accuracy domain, or benchmark
  owner instead of creating overlapping coverage. Do not relax thresholds or
  reinterpret failed evidence merely to make a run pass.
- Development, filtered, interrupted, and phase-only metrics stay under
  `build/`. Publish or overwrite canonical data under `validation/metrics`
  only when the user explicitly requests a complete release run.

## Documentation maintenance

Update `docs/architecture.md` only when a subsystem is added, removed, moved,
or its ownership/dependency flow materially changes. Routine implementation,
test, and refactoring work does not require an architecture-document update.
