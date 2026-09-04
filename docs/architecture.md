# fltx architecture

This is a navigation map of stable ownership and dependency flow. It is not a
file inventory, API reference, or generated index. Read exact source before
changing behaviour.

## Repository shape

| Area | Responsibility |
|---|---|
| `include/fltx.h` | Complete public umbrella include |
| `include/fltx/*.h` | Public types, operations, and focused include surfaces |
| `include/fltx/detail/*.h` | Shared and precision-specific implementation mechanics |
| `src/*.cpp` | Compiled runtime boundaries and heavyweight implementations |
| `cmake/` | Library build policy, source ownership, packaging, and developer-target wiring |
| `validation/tests/` | Behavioural, overload, header, language-mode, and genuine-constexpr contracts |
| `validation/accuracy/` | Deterministic MPFR-backed numerical domains and runner registration |
| `validation/benchmarks/` | Runtime, compile-time, and linked-size measurement |
| `validation/support/` | Validation-only domains, oracles, comparison adapters, and shared runner support |
| `validation/_internal/` | Preset resolution, host-toolchain setup, and local CI-check orchestration |
| `validation/metrics/` | Metrics orchestration, provenance, publication, and report generation |
| `examples/` | First-party usage examples and API consumers |

`vcpkg/` and `validation/extern/` are dependency trees, not first-party library
architecture. Generated metrics data and reports are evidence, not source
owners. Normal repository search excludes all of these through `.rgignore`.

## Public include structure

`include/fltx.h` composes the main surfaces:

```text
fltx.h
  core.h      native/fdd/fqd core-family umbrellas
  io.h        limits, char conversion, strings, streams
  math.h      f32/f64/fdd/fqd <cmath>-shaped APIs
  dispatch.h  runtime-to-template dispatch and FloatType mappings
  random.h    engines, distributions, and array helpers
  hash.h      std::hash integration
```

The focused headers remain supported entry points. `native.h`, `fdd.h`, and
`fqd.h` compose their respective core type families, while `native_math.h`,
`native_string.h`, `fdd_math.h`, `fqd_math.h`, `charconv.h`, and similar headers
expose smaller surfaces when consumers do not want the complete umbrella. The
native-family headers do not complete or include either extended type.

Supported standalone utilities live under `include/fltx/util`. In particular,
`util/template_dispatch.h` exposes the generic runtime-to-template dispatch
facility without the FLTX-specific `FloatType` mappings supplied by
`dispatch.h`.

The extended types have two public forms:

- `fdd_s` and `fqd_s` are trivial storage forms containing two and four
  `double` limbs respectively;
- `fdd` and `fqd` are scalar value forms built on those storage layouts;
- `fqd` additionally participates in the bounded expression-fusion system in
  `detail/fqd_expressions.h`, while `fqd_s` remains the eager storage form.

`aliases.h` supplies the type names, `traits.h` owns concepts and promotion
traits, and the per-precision `*_type.h`, `*_limits.h`, `*_conversions.h`,
`*_comparison.h`, `*_classification.h`, and `*_arithmetic.h` headers build the
core numeric surface.

## Library layers and execution paths

### Header implementation layer

Reusable constexpr-capable machinery lives under `include/fltx/detail`:

- `common_fp.h`, `common_math.h`, `math_utils.h`, and `math_promotion.h` own
  cross-precision floating-point mechanics and promotion;
- `common_decimal.h`, `common_io.h`, `native_float_decimal.h`, and
  `native_float_io.h` own shared decimal and text conversion mechanics;
- `fdd_*` and `fqd_*` headers own expansion arithmetic, kernels, constants,
  math implementations, conversions, random traits, and expression lowering;
- `simd.h`, `fqd_simd_config.h`, and `build_info.h` own compiled/consumer SIMD
  and FMA configuration boundaries.

Internal namespaces reflect the layers:

```text
bl::detail::_dd / _qd            primitive expansion operations and kernels
bl::detail::_dd_impl / _qd_impl  algorithm implementations
bl::detail::_dd_runtime / _qd_runtime compiled entry-point declarations
bl::detail::_qd_expr              bounded fqd expression nodes and lowering
```

Public constexpr wrappers select the constant-evaluation implementation or a
compiled runtime entry point. Therefore changes to math, arithmetic,
conversion, or parsing commonly have two routes to inspect:

```text
public wrapper
  constant evaluation -> detail primitives / *_impl in headers
  runtime             -> detail::*_runtime declaration -> src/*.cpp -> *_impl
```

`config.h` centralizes compiler feature selection and the constant-evaluation
switch. `build_info.cpp` captures the compiled library configuration and
runtime hardware-FMA availability; `fma_x86.cpp` supplies separately compiled
x86 FMA backends when the build policy enables them.

### Compiled runtime layer

`cmake/fltxSources.cmake` is the source of truth for compiled library sources:

| Sources | Ownership |
|---|---|
| `fdd.cpp`, `fqd.cpp` | Runtime conversions, hot arithmetic helpers, and fqd fused-expression bodies |
| `fdd_math.cpp`, `fqd_math.cpp` | Core roots, rounding, remainder, and decomposition entry points |
| `fdd_transcendental.cpp`, `fqd_transcendental.cpp` | Exponential, logarithmic, power, trigonometric, hyperbolic, and special-function entry points |
| `fltx_io.cpp` | Runtime string formatting and extended-type parsing entry points |
| `build_info.cpp` | Compiled configuration identity and runtime feature detection |
| `fma_x86.cpp` | Optional complete-operation x86 FMA backend |

Heavy runtime work should remain behind these boundaries when header placement
would enlarge consumer translation units. Header code remains necessary where
constant evaluation or templates require the implementation to be visible.

## Subsystem ownership

| Subsystem | Public entry points | Detail/runtime owners | Primary validation owners |
|---|---|---|---|
| Types, storage, traits, limits | `core.h`, `native.h`, `fdd.h`, `fqd.h`, `traits.h`, `limits.h` | type/conversion/common-FP headers; `fdd.cpp`, `fqd.cpp`, `build_info.cpp` | `tests/contracts/core.cpp`, overload matrices, isolated-header probes, `tests/constexpr/core.cpp`, package tests |
| Arithmetic and fqd expressions | `fdd_arithmetic.h`, `fqd_arithmetic.h` | `detail/fdd_arithmetic.h`, `detail/fqd_arithmetic.h`, `detail/fqd_expansion.h`, `detail/fqd_expressions.h`, `fqd.cpp` | core, expression, and edge-case contracts; accuracy runners; runtime operation benchmarks |
| Math and classification | `math.h`, `native_math.h`, per-type math/classification/numbers headers | common math/promotion plus per-precision basic, kernel, and transcendental headers; math/transcendental sources | `tests/contracts/math.cpp`, overload matrices, constexpr corpus, `accuracy/`, runtime benchmarks |
| Text and streams | `io.h`, `charconv.h`, `string.h`, `static_string.h`, `string_options.h`, per-type string headers, `stream.h`, `format.h` | common/native/precision I/O and decimal headers; `fltx_io.cpp` | I/O contracts, constexpr corpus, isolated-header probes, parse/format accuracy rows, I/O benchmarks |
| Random facilities | `random.h` | header-only common logic plus `detail/fdd_random.h` and `detail/fqd_random.h` | random contracts, constexpr cases, selected runtime benchmarks |
| Dispatch | `dispatch.h`, `util/template_dispatch.h` | header-only FLTX mappings and standalone dispatch-table utility | dispatch contracts and isolated-header probes |
| Standard integration | hash, limits, numbers, format specializations | corresponding focused public/detail headers | core and I/O contracts plus isolated-header probes |
| Build and package policy | top-level `CMakeLists.txt`, `CMakePresets.json` | `cmake/fltxBuildPolicy.cmake`, `cmake/fltxSources.cmake`, validation CMake modules | header/C++ standard contracts, in-tree and installed-package consumers |

`validation/COVERAGE.md` is the detailed ownership ledger. It should be updated
when the implemented validation surface changes; this architecture map should
not duplicate its operation-by-operation inventory.

## Validation flow

The validation layers answer different questions:

```text
contracts          public behaviour, overloads, edge cases, I/O, random, dispatch
genuine constexpr compiler-evaluated behaviour
accuracy           deterministic numerical comparison against MPFR
constexpr accuracy runtime execution of the fixed constexpr-algorithm route
benchmarks         performance, compile-time, and linked-size evidence
package checks     installed find_package consumer behaviour
```

CMake target construction is split across `cmake/validation/`:

- `fltxValidationTests.cmake` owns contracts, genuine constexpr, isolated
  public-header probes, and C++20/C++23 compile contracts;
- `fltxValidationRunners.cmake` owns accuracy, benchmark, source-identity, and
  comparison smoke runners;
- `fltxValidationDependencies.cmake` owns validation-only comparison
  dependencies;
- `fltxValidationPackage.cmake` owns in-tree and installed-package consumers;
- `fltxValidationCommon.cmake` owns shared target preparation and build
  identity.

The local CI-check entry points are intentionally separate from metrics
collection. Their accuracy runner emits and gates FLTX rows only; comparison
implementations and dependency smoke checks stay on metrics-owned targets:

```text
run_preset_checks.py ------------------+-> _internal/check_pipeline.py
run_all_supported_checks.py -----------+   -> configure the selected preset
                                           -> build fltx_ci_checks
```

Shared preset inheritance, host-matrix selection, and Visual Studio developer
environment setup live in `validation/_internal/preset_support.py` so checks
and metrics use one toolchain-resolution path.

Read `validation/README.md` for current commands and runner semantics. Read
`validation/COVERAGE.md` before adding coverage so new work extends the
existing owner rather than creating a parallel test lane.

## Metrics flow

The normal public entry points are intentionally few:

```text
run_preset_metrics.py -------------------------------+
run_all_supported_metrics.py                         |
  -> _internal/supported_preset_pipeline.py ----------+
  -> _internal/preset_pipeline.py
  -> configure/build and locate C++ runners through the CMake File API
  -> _internal/run_metrics.py and run_native_accuracy.py
  -> rebuild_tables.py / _internal/report_pipeline.py
  -> SVG renderers
```

`manifest.py` owns expected operation/implementation registration.
`source_fingerprint.py` owns source identity. `run_metrics.py` owns runner
preflight, streamed evidence, aggregation, provenance, staging, and atomic
publication. The `build_*` modules render existing evidence and must not repair
or reinterpret source rows.

By default, quick, standard, and full runs keep profile-separated evidence and
reports beneath
`validation/metrics/_unversioned/<workflow>/{data,generated}`. Publication is
an orthogonal explicit choice: `--publish` sends the selected profile's
complete evidence to `validation/metrics/data` and regenerates its matching
reports beneath `validation/metrics/generated`. Every published profile keeps
the known-revision, optimized-Release, compatibility, locking, and
transactional-publication requirements.

## Change navigation

For a public API change, trace the focused public header into its detail and
runtime owners, then search first-party consumers in `validation` and
`examples`. Update them directly rather than preserving a superseded API.

For an arithmetic, math, conversion, or parsing change, inspect both branches
of the constant-evaluation/runtime split and select validation from the
existing coverage row. Performance evidence complements correctness; it does
not replace contracts or accuracy coverage.

For a new compiled source, update `cmake/fltxSources.cmake`. For a new
validation source, add it to the owning validation CMake module rather than the
top-level build file.

For metrics work, start at `run_preset_metrics.py` or `rebuild_tables.py`, then
enter `_internal` only at the stage being changed. Renderer-only work should
reuse compatible data; collection/publishing work must preserve source
identity, manifests, provenance, and transactional publication.

For a local CI reproduction, start at `run_preset_checks.py`; use
`run_all_supported_checks.py` only when every required host toolchain is
available.

## Maintenance rule

Update this document only when a subsystem is added, removed, moved, or its
ownership/dependency flow materially changes. Do not update it for ordinary
function additions, algorithm tuning, test cases, or refactoring within an
existing owner. Avoid volatile line numbers, counts, generated inventories,
and timestamps.
