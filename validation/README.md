# fltx validation system

`validation` contains the repository's correctness tests, numerical accuracy
runner, performance benchmarks, metrics publication pipeline, shared support,
and pinned comparison libraries. Behavioural suites live under `tests`, while
runtime, compile-time, and binary-size measurements live under `benchmarks`.

The validation executables have five roles. Each role has a strict-consumer
target and a matching `_fastmath` target compiled with the platform's real
consumer fast-math option (`/fp:fast` or `-ffast-math`):

MinGW/GCC validation deliberately applies only `-ffast-math`, including at
link time, so it measures the compiler's unmodified consumer configuration.
On MinGW, GCC may consequently link `crtfastmath.o` and globally enable DAZ/FTZ,
which flushes binary64 subnormals and can discard low expansion limbs. Consumers
that want fast-math transformations while retaining gradual underflow can add
`-mno-daz-ftz`; the standard fast-math reports do not use that mitigation.

For dd/qd basic `+`, `-`, `*`, and `/`, consumer fast-math also selects
finite-input runtime kernels that omit NaN, infinity, and signed-zero handling.
Strict consumers and genuine constant evaluation retain the checked semantics.

- `fltx_contract_tests` checks public behaviour, I/O, random facilities,
  expression fusion, dispatch, and named numerical edge cases.
- `fltx_constexpr_tests` checks genuine compiler constant evaluation, including
  the 58-case-per-type MPFR corpus for all four types in the strict consumer.
  The fast-math target retains that corpus for dd/qd; native numerical
  gates use the strict consumer because fast-math also relaxes the compiler's
  constant evaluator. Exact native special-value assertions still compile in
  both profiles. Overload and return-type matrices belong to the contract suite.
- `fltx_accuracy` compares deterministic, named domains with an independent
  MPFR-backed oracle and writes one CSV row as soon as that domain finishes.
- `fltx_constexpr_accuracy` reuses the complete accuracy runner while forcing
  the library's constexpr algorithms at runtime, so they receive the same
  domains and threshold reporting as the normal runtime algorithms.
- `fltx_benchmark` measures performance only. A full run uses broad,
  operation-specific corpora (81,920 base dd samples and 40,960 base qd
  samples), calibrates fixed batches to at least 25 ms, and reports the median
  of seven trials. Every logical trial runs all registered implementations
  once, rotating which implementation runs first to balance ordering effects.

The initial comparison set is:

- dd: FLTX, Boost `cpp_double_double`, TLFloat `Quad`, and optional qdpp
  `dd_real`;
- qd: FLTX, Boost MPFR-backed 64-decimal-digit numbers, TLFloat `Octuple`,
  and optional qdpp `qd_real`.

Each comparison is registered only where the other library exposes a public
operation with the same intended purpose. Numeric accuracy comparisons use the
same named domains as FLTX, but MPFR evaluates the exact converted inputs
received by each implementation. If conversion collapses a nonzero
cancellation into identical operands, the comparison uses a representable
native-precision neighbour that preserves the intended nonzero result
direction. Parsing remains referenced to the original decimal text. Only FLTX
thresholds gate the process.

The dd/qd arithmetic operations share one deterministic paired `general`
corpus: 70% log-random representative magnitudes (including structured operand
relationships), at least 20% broad exponents, and 5% near the finite exponent
limits. Add/subtract also reserve 5% for subnormals; multiply/divide use a
scale-safe broad case instead. Every smoke or larger run contains every
applicable stratum, while the detail report keeps one domain row per operation.

The vendored TLFloat library and comparisons are enabled by default through
`FLTX_METRICS_TLFLOAT` on supported targets. The Windows ARM64 presets disable
it because upstream TLFloat currently uses the x64-only `_umul128` intrinsic
under MSVC-compatible frontends. Where enabled, `fltx_tlfloat_smoke`
separately verifies its C++ `Quad`/`Octuple` headers and a compiled C-ABI
symbol.

`fltx_compile_contract` builds every public header in isolation and compiles
the representative constexpr/runtime surface explicitly as both C++20 and
C++23. Comparison-library smoke executables are owned separately by
`fltx_metrics_compile_contract`; they are metrics integration checks, not FLTX
compile contracts.
`fltx_package_tests` installs fltx and builds strict, fast-math, and
injected-no-FMA consumers through `find_package`. Its dedicated install and
nested-build directories are recreated for every invocation, so stale
installed headers or CMake state cannot make the fixture pass.

## Preset check pipeline

Run the complete pull-request and branch check target from one CMake build
preset with:

```powershell
python .\validation\run_preset_checks.py `
    --preset windows-x64-msvc-release
```

The command always configures the selected preset and then builds
`fltx_ci_checks`. Success means that preset completed the same strict contracts,
genuine constexpr checks, package checks, first-party Python tooling tests, and
standard FLTX-only accuracy policy used by CI. The complete
consumer-fast-math contract executable is built but remains a manually runnable
diagnostic suite: real fast-math permits reassociation, signed-zero loss, and
subnormal flushing, so those semantic assertions are not badge gates. Runtime
f32/f64 threshold findings remain visible as advisory platform-libm baseline
evidence. The native fixed-simulation consumer-fast-math lane is also advisory
because runtime fast-math code generation cannot faithfully model genuine
compiler constant evaluation. All consumer-fast-math accuracy lanes are
advisory for the same reason: reassociation can invalidate expansion arithmetic
without changing the strict library contract. Genuine strict f32/f64 numerical
evaluation and strict fixed-simulation remain gating; exact native special-value
assertions compile in both consumer profiles, and strict dd/qd accuracy
remains gating.
The artificial fixed-simulation fast-math runner records `-` for its accuracy
special-value column: clang may emit a trap merely for constructing its
deliberate NaN/infinity probes under fast-math. Real runtime fast-math and every
strict lane retain those probes, while the genuine constexpr corpus remains the
authority for constant-evaluation special values.

To run that target for every release preset supported by the current host OS
and architecture, use:

```powershell
python .\validation\run_all_supported_checks.py
```

Every selected preset is required. The command runs them in host-matrix order
and stops at the first failure. It shares CMake preset resolution and automatic
Visual Studio developer-environment setup with the metrics pipeline, while
remaining independent of metrics evidence and report generation.

## Preset metrics pipeline

The development metrics workflow can be run from one CMake build preset:

```powershell
python .\validation\metrics\run_preset_metrics.py `
    --preset windows-x64-msvc-release
```

To run every release preset supported by the current host OS and architecture,
omit the preset and use the host-matrix entry point. It accepts the same
workflow arguments as the single-preset command:

```powershell
python .\validation\metrics\run_all_supported_metrics.py --consumer-mode all
python .\validation\metrics\run_all_supported_metrics.py `
    --consumer-mode all --quick
python .\validation\metrics\run_all_supported_metrics.py `
    --consumer-mode all --standard --publish
```

The host matrix is explicit rather than inferred from installed tools:

| Host | Presets |
|---|---|
| Windows x64 | MSVC, clang-cl, MinGW, wasm32 Emscripten |
| Windows ARM64 | MSVC, clang-cl |
| Linux x64/ARM64 | native GCC and Clang for the host architecture |
| macOS x64/ARM64 | native AppleClang for the host architecture |

Every selected preset is required and runs in order; a missing toolchain fails
the command rather than silently producing a partial matrix. The alternate
`vs2026` and `xcode` generator presets are excluded because their native
compiler coverage is already represented by the release matrix. Windows x64
is the designated automatic wasm32 metrics host so canonical Emscripten
evidence is not overwritten by several hosts. Other hosts can still run the
`wasm32-emscripten-release` preset explicitly with the single-preset command.

The command configures the preset, builds `fltx_accuracy` and `fltx_benchmark`
plus their `_fastmath` variants, and locates their artifacts through the CMake
File API. It runs strict-consumer metrics by default. Pass
`--consumer-mode fastmath` for only fast-math metrics, or
`--consumer-mode all` to run strict and then fast-math metrics and generate
both sets of data and reports. The modes retain separate evidence and
provenance, including the source fingerprint needed for later comparisons.
After the requested mode finishes, the pipeline also regenerates strict versus
fast-math profile comparisons whenever a compatible pair exists. An `all` run
requires those comparison reports; a focused run simply skips them when the
other mode is unavailable or stale.

The public commands use mutually exclusive, value-free profile flags. With no
profile flag, `--standard` is implied. `--quick` selects the half-sized
`small` sample profile and `--full` selects the full profile. Publication is
orthogonal: add `--publish` to any profile to write canonical evidence and
reports beneath `validation/metrics/{data,generated}`. Without `--publish`,
the selected profile stays under `_unversioned`. `--output-root` remains
available for custom isolation and creates `data/` and `generated/` below the
supplied path; it cannot be combined with `--publish`. Published runs still
require complete accuracy and benchmark results, a known source revision, and
optimized Release runners.
The pipeline checks each executable's configuration banner before measurement,
so fast-math evidence cannot be produced by a strict binary (or vice versa).

```powershell
python .\validation\metrics\run_preset_metrics.py `
    --preset windows-x64-msvc-release `
    --consumer-mode all `
    --full

# Publish the standard profile after its evidence has been accepted.
python .\validation\metrics\run_preset_metrics.py `
    --preset windows-x64-msvc-release `
    --consumer-mode all `
    --standard `
    --publish
```

| Workflow | Internal profile | Accuracy samples | Benchmark base (dd/qd) | Trials | Timing |
|---|---|---:|---:|---:|---:|
| `--quick` | `small` | 2,048 | 4,096 / 2,048 | adaptive 1 / 3 / 7 | 8 or 15 ms target |
| Default / `--standard` | `standard` | 4,096 | 8,192 / 4,096 | adaptive 1 / 3 / 7 | 8 or 15 ms target |
| `--full` | `full` | 65,536 | 81,920 / 40,960 | 7 | 25 ms minimum |

The quick and standard workflows run the dd and qd accuracy phases
concurrently, then benchmark sequentially. They share the same adaptive corpus
and timing policy, with every quick corpus exactly half its standard size.
Cheap primitives use large corpora and seven 15 ms trials; ordinary operations
use three 8 ms trials; operations measured at 10 microseconds or slower use
three single-batch trials. A soft five-second row budget can reduce the trial
count to one or three. In standard mode, the four slow gamma/error functions
use 256 dd or 128 qd samples; quick mode uses 128 or 64.

The remaining public Python command in `validation/metrics/` rebuilds every
report from existing data without running the metrics executables:

```powershell
python .\validation\metrics\rebuild_tables.py `
    --targets windows/x86_64/MSVC
```

Omit `--targets` when all canonical datasets belong to a compatible run. The
supporting Python modules live under `validation/metrics/_internal/` and are invoked by
these entry points or by CMake. Every report build produces both the complete
and `_compact` overview/performance variants. Compact reports hide displayed
speed ratios, narrow the benchmark columns, and colour each nanosecond timing
from the same hidden relative speed ratio used by the complete layout.

Configure and build are incremental. Before running the expensive phases, the
pipeline validates any existing evidence against the freshly built runners'
source fingerprint, compiler/configuration/flags, host, sample profile,
manifests, summaries, and file hashes. Compatible evidence is reused and all
seven SVGs per consumer mode are regenerated; `--force-rerun` explicitly
bypasses reuse.
Renderer-only Python changes do not alter the runner source fingerprint.

Canonical published output and the profile-separated unversioned output are:

```text
validation/metrics/
  data/                                  # --publish evidence
  generated/                             # --publish reports
  _unversioned/
    quick/
      data/                              # --quick evidence
      generated/                         # --quick reports
    standard/
      data/                              # default/--standard evidence
      generated/                         # default/--standard reports
    full/
      data/                              # --full evidence
      generated/                         # --full reports
```

Unpublished profiles replace only their compatible unversioned profile
snapshot; `--publish` writes canonical data under `validation/metrics/data`
and matching reports under `validation/metrics/generated`.
The unversioned trees are ignored by Git and repository search. Report
filenames are identical across the roots, with `_compact` distinguishing
presentation variants and `_fastmath` identifying consumer-fast-math evidence
and reports. Strict and fast-math data also have separate run metadata and
publication locks. Comparison reports use
names such as `windows_x86_64_MSVC_qd_strict_fastmath_comp.svg`. They show strict
values first and colour the parenthesized `fast-math - strict` delta. Pairing
requires matching source, target, sample policy, host, and build/library
configuration; run IDs and collection times may differ.
Both overview layouts and the profile comparison keep Inf/NaN support in its
existing column and report signed-zero preservation separately in a narrow
`±0` column. A tick or cross summarizes deterministic signed-zero contract
probes for that operation; `-` means the operation has no applicable
accuracy probe (including predicates, integer-return APIs, and benchmark-only
rows whose semantics are covered by the contract suites).
Opposite zero signs are numerically exact in the bits-accuracy columns, so this
semantic distinction cannot distort mean, worst, or domain-pass results.
Consumer-fast-math overviews and strict/fast-math comparisons also show a
narrow `Subnorm` column. Their headline accuracy and domain count exclude the
dedicated subnormal corpus, while a tick or cross reports whether that corpus
passes its normal release threshold; `-` means no separate subnormal corpus is
defined for the operation. Strict overviews retain their existing aggregate
accuracy presentation.
The output target is inferred from the system, target architecture, compiler
ID, and compiler frontend embedded in both runners. This separates x64 from
ARM64 and distinguishes Windows clang-cl from ordinary Clang. On Windows,
MSVC and clang-cl Ninja presets run in the matching x64 or ARM64 Visual Studio
developer environment discovered through `vswhere`; MinGW, Emscripten, and
Visual Studio-generator presets keep their own environments.

## Quick MSVC run

```powershell
cmake --preset windows-x64-msvc-release
cmake --build --preset windows-x64-msvc-release --target `
    fltx_compile_contract fltx_accuracy fltx_accuracy_fastmath `
    fltx_constexpr_accuracy fltx_constexpr_accuracy_fastmath `
    fltx_benchmark fltx_benchmark_fastmath

.\build\windows-x64-msvc-release\validation\fltx_contract_tests.exe
.\build\windows-x64-msvc-release\validation\fltx_constexpr_tests.exe
.\build\windows-x64-msvc-release\validation\fltx_constexpr_accuracy.exe `
    --precision dd --sample-mode smoke
```

Boost comparisons are always enabled in strict metrics because Boost, GMP, and
MPFR are already test dependencies. The vendored qdpp headers are enabled by
default for the strict accuracy and benchmark runners, including Emscripten.
The Windows clang-cl presets explicitly disable qdpp because it treats every
`_MSC_VER` frontend as having baseline-safe x86 FMA intrinsics; clang-cl
correctly rejects those intrinsics when the translation unit does not target
FMA. Both Windows ARM64 presets also disable qdpp because those x86 intrinsics
are unavailable on the target. Metrics builds leave these dependency choices
to the selected preset. Required GitHub checks use the FLTX-only accuracy runner
and do not build these comparison integrations.
Consumer-fast-math runners contain FLTX rows only and do not parse qdpp headers,
because qdpp deliberately rejects aggressive fast-math. The qdpp smoke target
also compiles and executes representative `dd_real` and `qd_real` operations.
To test a different qdpp checkout, override its include directory:

```powershell
cmake --preset windows-x64-msvc-release `
    -DFLTX_METRICS_QDPP_INCLUDE_DIR=C:/path/to/qdpp/include
cmake --build --preset windows-x64-msvc-release --target `
    fltx_qdpp_smoke fltx_accuracy fltx_benchmark
```

`FLTX_METRICS_QDPP=OFF` remains available as an explicit opt-out for a
dependency-isolation build.

For a direct noncanonical full dd/qd run:

```powershell
python validation/metrics/_internal/run_metrics.py `
    --accuracy build/windows-x64-msvc-release/validation/fltx_accuracy.exe `
    --benchmark build/windows-x64-msvc-release/validation/fltx_benchmark.exe `
    --platform windows `
    --architecture x86_64 `
    --compiler MSVC `
    --sample-mode full `
    --output-root validation/metrics/_unversioned/direct-full/data
```

For a direct consumer-fast-math run, use `fltx_accuracy_fastmath.exe` and
`fltx_benchmark_fastmath.exe` and add `--consumer-mode fastmath`. The compiled
fltx library is the same target in both runs; only the consumer translation
units and executable link receive the consumer fast-math option. Preset
fast-math accuracy runs are advisory: every strict-threshold miss remains a
visible `FAIL` with its measured margin and worst case, but does not prevent the
benchmark phase or report generation. Strict consumer metrics remain gating.

`--platform`, `--architecture`, and `--compiler` remain available for explicit
validation, but must be supplied together and may all be omitted to infer the
canonical target from the runners.

The runners print their configuration and requested sample profile once,
stream rows in real time, and flush each row to a `.partial.csv`. Benchmark
detail rows retain every trial plus total timed iterations and elapsed time.
A complete passing run atomically commits the detail files and combined
`<platform>/<architecture>/<compiler>_<type>.csv` files beneath its selected
data root. The public preset pipeline selects the canonical root only when
`--publish` is present. Filtered and smoke runs stay under `build/` and cannot
replace published results. The banner records the ordered implementation set
for each precision.
The manifest requires 85 FLTX benchmark rows, 75 Boost rows per precision, and,
when enabled, 60 dd or 62 qd qdpp rows and 71 TLFloat rows per precision.
Accuracy contributes 127 FLTX and 118 Boost domain rows per precision. When
enabled, qdpp contributes 87 dd or 91 qd rows and TLFloat contributes 107
rows per precision. These are registration checks, not assumptions in the
runner or SVG renderer.

Operation families follow the public metrics taxonomy used by the original
report: Arithmetic contains only add, subtract, multiply, and divide;
decomposition, scaling, stepping, FMA, and related helpers are
Floating-point utilities; `sqr` is grouped with roots and powers immediately
before `sqrt`; exponentials, logarithms,
trigonometric, hyperbolic, inverse-hyperbolic, special-function, remainder,
comparison, I/O, random, and mixed-workload rows each have distinct groups.
The group is recorded by the C++ operation registration itself, so reports do
not repair or reinterpret incorrectly classified CSV rows.

Either phase can also be run independently by supplying only `--accuracy` or
only `--benchmark`. A phase-only run retains its CSV and metadata under
`build/metrics/runs/<run-id>/`; it never replaces the combined canonical
results. This makes focused correctness or timing checks cheap while ensuring
that published tables only combine phases measured from the same run.

All requested accuracy and benchmark phases run even if an earlier phase
fails. In that case nothing is published, but the complete streamed evidence
and failed-run metadata remain together in the staging directory printed by
the command.

Every accuracy and benchmark build embeds a SHA-256 fingerprint of its source
checkout. Before starting any measured work, the internal metrics runner inspects every
requested executable and rejects stale or differently configured builds.
Rebuild the requested targets after changing source; this preflight avoids
discovering the problem after a long full run. The orchestrator also passes the
profile's sample and trial counts explicitly, so the Python profile is the
single source of truth.
Canonical runs always use the revision computed from that checkout; it cannot
be overridden on the command line. The detail CSVs,
configuration banner, and run metadata retain that identity; generated metrics
and SVG output directories are excluded from the digest. Run metadata also
records the requested sample counts, host/runtime identity, and SHA-256 hashes
of both runner artifacts and every published CSV. The configuration metadata
also records whether `FLTX_SIMULATE_FIXED_CONSTEVAL_MODE` was enabled for the
consumer translation unit.

## Native accuracy

f32 and f64 use the same 119 accuracy rows as a native/libm policy baseline,
but they are not merged into the dd/qd summary tables. The preset metrics
pipeline records the matching strict or fast-math baseline after each requested
dd/qd metrics mode. Run both complete native suites independently with:

```powershell
cmake --build --preset windows-x64-msvc-release --target fltx_native_accuracy_full
```

The target invokes both precisions and gives every invocation a new directory
under `build/<preset>/validation/native_accuracy`. Numerical threshold misses
are retained as advisory baseline data; crashes, stale binaries, mode
mismatches, and incomplete manifests still fail. The equivalent direct strict
command is:

```powershell
python validation/metrics/_internal/run_native_accuracy.py `
    --accuracy build/windows-x64-msvc-release/validation/fltx_accuracy.exe
```

Pass `--consumer-mode fastmath` with `fltx_accuracy_fastmath.exe` to record the
native fast-math baseline. Each completed bundle includes f32/f64 CSVs plus a
fingerprinted metadata manifest with the host, compiler configuration, runner
hash, and consumer mode.

## CI policy

The recommended local pull-request and branch check command is:

```powershell
python .\validation\run_preset_checks.py `
    --preset windows-x64-msvc-release
```

The underlying CMake command used by GitHub Actions is:

```powershell
cmake --build --preset windows-x64-msvc-release --target fltx_ci_checks
```

It builds isolated public-header and C++ standard probes, both contract
executables, the in-tree package consumer, and FLTX-only accuracy runners. It
runs the strict contract suite, genuine constexpr checks, package and
first-party Python tooling checks, and the 4,096-sample standard accuracy corpus
for all four types through normal and fixed-constexpr runners in both strict and
consumer-fast-math modes. The deterministic domain anchors and explicit ternary
operand tuples run in every profile; the full metrics profile retains the
65,536-sample corpus for deeper evidence. cppdd, qdpp, TLFloat, their
comparison rows, and their dependency smoke checks remain owned by the metrics
targets and cannot fail this gate. The complete consumer-fast-math contract suite stays
available as diagnostic evidence but is not a badge gate because the compiler
profile explicitly relaxes the semantics asserted by that suite. Runtime
f32/f64 numerical threshold misses are advisory because those paths deliberately
delegate to the platform libm; crashes, incomplete output, and runner faults
still fail. Both runtime and fixed-simulation accuracy under consumer fast-math
are advisory for every precision: the compiler may legally reassociate the
error-free transformations underlying expansion arithmetic, and optimized
runtime code cannot faithfully reproduce the compiler's constant evaluator.
The fixed-simulation fast-math runner omits deliberate NaN/infinity accuracy
probes (reported as `-`) because clang can compile those probe constructions to
a trap under fast-math; this does not affect real runtime or strict
special-value coverage.
The genuine f32/f64 constexpr numerical corpus gates the strict consumer. Exact
native special-value assertions are compiled in both consumer profiles, strict
native fixed-simulation remains gating, and strict dd/qd accuracy
thresholds remain gating.
The Linux GCC/Clang, Windows MSVC/MinGW, macOS AppleClang, and WebAssembly
Emscripten workflows all invoke this same target. Any currently reproducible
gating failure remains recorded in
[KNOWN_FAILURES.md](KNOWN_FAILURES.md); gating thresholds are not reduced to
manufacture a green result.

Authoritative installed-package variants, compile and binary-size telemetry,
performance, and canonical metrics publication remain outside that pull-request target.
The release matrix configures separate `AUTO`, `OFF`, and `ASSUME` build trees,
then runs `fltx_native_accuracy_full`, `fltx_package_tests`, and a full
internal metrics-runner invocation with qdpp enabled when comparative ratios are
wanted. Package and dd/qd contract thresholds remain gating; native
accuracy is retained as observational policy evidence for comparing the
consumer-facing extended types against the platform's float/double behavior.

Compile and size telemetry have distinct opt-in targets:

```powershell
cmake --build --preset windows-x64-msvc-release --target fltx_compile_benchmarks_full
cmake --build --preset windows-x64-msvc-release --target fltx_binary_size
```

The first writes a summary covering public-header cost and expression growth at
25, 100, and 200 functions, plus a separate matrix comparing eager `fqd_s`
with public `fqd` expressions for 49 individual fused shapes plus the
aggregate all-shapes case. The second reports
dead-code-eliminated linked-size deltas for baseline, arithmetic/expression,
math, I/O, and random-use buckets for both precisions.

After matching full runs exist for all requested targets:

```powershell
python validation/metrics/rebuild_tables.py `
    --input validation/metrics/data `
    --targets windows/x86_64/MSVC windows/x86_64/MinGW `
        webassembly/wasm32/Emscripten `
    --output validation/metrics/generated
```

Add `--consumer-mode fastmath` to rebuild the corresponding `_fastmath`
reports from consumer-fast-math evidence.

This creates the complete and compact performance/overview reports plus the
FLTX-only cross-platform accuracy-consistency report.
Performance cells show FLTX time followed by every available implementation's
ratio against FLTX. Accuracy cells show FLTX average/minimum measured bits and
domain pass counts for each target; competitor accuracy remains in the
per-target overview and CSV evidence. Report headlines show uncapped measured
bits accurate for every implementation. Parse rows score
the binary result against the exact decimal input; formatting rows score the
emitted decimal against the binary source with MPFR and bypass every library
parser. Those directional scores are intentionally not expected to match.
Detailed CSV rows and tooltips retain the same measurements and original
threshold margins.

The compact per-target overview uses a different, implementation-local
presentation: each competitor block shows `implementation speed / FLTX speed`.
Thus `0.49×` means that implementation runs at 49% of FLTX's speed, while
`2.00×` means it runs twice as fast. The FLTX block is the unratioed baseline.
See [COVERAGE.md](COVERAGE.md) for ownership and
[KNOWN_FAILURES.md](KNOWN_FAILURES.md) for findings that currently prevent
canonical publication.
