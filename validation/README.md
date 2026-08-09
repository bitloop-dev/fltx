# fltx validation system

`validation` contains the repository's correctness tests, numerical accuracy
runner, performance benchmarks, metrics publication pipeline, shared support,
and pinned comparison libraries. Behavioural suites live under `tests`, while
runtime, compile-time, and binary-size measurements live under `benchmarks`.

The validation executables have five roles. Each role has a strict-consumer
target and a matching `_fastmath` target compiled with the platform's real
consumer fast-math option (`/fp:fast` or `-ffast-math`):

- `fltx_contract_tests` checks public behaviour, I/O, random facilities,
  expression fusion, dispatch, and named numerical edge cases.
- `fltx_constexpr_tests` checks genuine compiler constant evaluation, including
  the 58-case-per-type MPFR corpus. Overload and return-type matrices belong to
  the contract suite.
- `fltx_accuracy` compares deterministic, named domains with an independent
  MPFR-backed oracle and writes one CSV row as soon as that domain finishes.
- `fltx_constexpr_accuracy` reuses the complete accuracy runner while forcing
  the library's constexpr algorithms at runtime, so they receive the same
  domains and threshold gates as the normal runtime algorithms.
- `fltx_benchmark` measures performance only. A full run uses broad,
  operation-specific corpora (81,920 base f128 samples and 40,960 base f256
  samples), calibrates fixed batches to at least 25 ms, and reports the median
  of seven trials. Every logical trial runs all registered implementations
  once, rotating which implementation runs first to balance ordering effects.

The initial comparison set is:

- f128: FLTX, Boost `cpp_double_double`, TLFloat `Quad`, and optional qdpp
  `dd_real`;
- f256: FLTX, Boost MPFR-backed 64-decimal-digit numbers, TLFloat `Octuple`,
  and optional qdpp `qd_real`.

Each comparison is registered only where the other library exposes a public
operation with the same intended purpose. Numeric accuracy comparisons use the
same named domains as FLTX, but MPFR evaluates the exact converted inputs
received by each implementation. If conversion collapses a nonzero
cancellation into identical operands, the comparison uses a representable
native-precision neighbour that preserves the intended nonzero result
direction. Parsing remains referenced to the original decimal text. Only FLTX
thresholds gate the process.

The f128/f256 arithmetic operations share one deterministic paired `general`
corpus: 70% log-random representative magnitudes (including structured operand
relationships), at least 20% broad exponents, and 5% near the finite exponent
limits. Add/subtract also reserve 5% for subnormals; multiply/divide use a
scale-safe broad case instead. Every smoke or larger run contains every
applicable stratum, while the detail report keeps one domain row per operation.

The vendored TLFloat library and comparisons are enabled by default through
`FLTX_METRICS_TLFLOAT`. `fltx_tlfloat_smoke` separately verifies its C++
`Quad`/`Octuple` headers and a compiled C-ABI symbol.

`fltx_compile_contract` builds every public header in isolation and compiles
the representative constexpr/runtime surface explicitly as both C++20 and
C++23.
`fltx_package_tests` installs fltx and builds strict, fast-math, and
injected-no-FMA consumers through `find_package`. Its dedicated install and
nested-build directories are recreated for every invocation, so stale
installed headers or CMake state cannot make the fixture pass.

## Preset metrics pipeline

The development metrics workflow can be run from one CMake build preset:

```powershell
python .\validation\metrics\run_preset_metrics.py --preset native-release
```

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

With no workflow flag the command uses the development profile and writes all
data and reports below `build/metrics`. Pass `--release` to use the full
publication profile and write canonical evidence below `validation/metrics`.
The pipeline checks each executable's configuration banner before measurement,
so fast-math evidence cannot be produced by a strict binary (or vice versa).

```powershell
python .\validation\metrics\run_preset_metrics.py `
    --preset native-release --release
```

| Workflow | Accuracy samples | Benchmark base (f128/f256) | Trials | Timing |
|---|---:|---:|---:|---:|
| Default | 4,096 | 8,192 / 4,096 | adaptive 1 / 3 / 7 | 8 or 15 ms target |
| `--release` | 65,536 | 81,920 / 40,960 | 7 | 25 ms minimum |

The default workflow runs the f128 and f256 accuracy phases concurrently, then
benchmarks sequentially. Cheap primitives use release-sized benchmark corpora
and seven 15 ms trials; ordinary operations use three 8 ms trials; operations
measured at 10 microseconds or slower use three single-batch trials. A soft
five-second row budget can reduce the trial count to one or three. The four
slow gamma/error functions use 256 f128 or 128 f256 samples.

The only other public Python command in `validation/metrics/` rebuilds every report from
existing data without running the metrics executables:

```powershell
python .\validation\metrics\rebuild_tables.py --targets windows/MSVC
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

The two output roots deliberately have the same shape:

```text
build/metrics/                     validation/metrics/
  data/                              data/
  generated/                         generated/
    accuracy/                          accuracy/
    overview/                          overview/
    performance/                       performance/
    profile_comparison/                profile_comparison/
```

The default workflow replaces the compatible local snapshot under
`build/metrics/data`; `--release` publishes canonical data under
`validation/metrics/data`. Report filenames are identical across the roots,
with `_compact` distinguishing presentation variants and `_fastmath`
identifying consumer-fast-math evidence and reports. Strict and fast-math data
also have separate run metadata and publication locks. Comparison reports use
names such as `windows_MSVC_f256_strict_fastmath_comp.svg`. They show strict
values first and colour the parenthesized `fast-math - strict` delta. Pairing
requires matching source, target, sample policy, host, and build/library
configuration; run IDs and collection times may differ.
The output target is inferred from the build identity embedded in both
runners; Windows MSVC and MinGW, Emscripten, Linux GCC and Clang, and macOS
AppleClang builds have canonical labels.
On Windows, a native Ninja preset with no explicit alternative compiler is
run in an x64 Visual Studio developer environment discovered through
`vswhere`. This makes the command independent of whether it was launched from
plain PowerShell or from an x86/x64 Developer PowerShell; MinGW, Clang,
Emscripten, and Visual Studio-generator presets keep their own environments.

## Quick MSVC run

```powershell
cmake --preset msvc-release-codex
cmake --build --preset msvc-release-codex --target `
    fltx_compile_contract fltx_accuracy fltx_accuracy_fastmath `
    fltx_constexpr_accuracy fltx_constexpr_accuracy_fastmath `
    fltx_benchmark fltx_benchmark_fastmath

.\build\msvc-release-codex\validation\Release\fltx_contract_tests.exe
.\build\msvc-release-codex\validation\Release\fltx_constexpr_tests.exe
.\build\msvc-release-codex\validation\Release\fltx_constexpr_accuracy.exe `
    --precision f128 --sample-mode smoke
```

Boost comparisons are always enabled because Boost, GMP, and MPFR are already
test dependencies. The vendored qdpp headers are enabled by default for both
accuracy and benchmark runners, including Emscripten. The qdpp smoke target
also compiles and executes representative `dd_real` and `qd_real` operations.
To test a different qdpp checkout, override its include directory:

```powershell
cmake --preset msvc-release-codex `
    -DFLTX_METRICS_QDPP_INCLUDE_DIR=C:/path/to/qdpp/include
cmake --build --preset msvc-release-codex --target `
    fltx_qdpp_smoke fltx_accuracy fltx_benchmark
```

`FLTX_METRICS_QDPP=OFF` remains available as an explicit opt-out for a
dependency-isolation build.

Then run the complete f128/f256 publication:

```powershell
python validation/metrics/_internal/run_metrics.py `
    --accuracy build/msvc-release-codex/validation/Release/fltx_accuracy.exe `
    --benchmark build/msvc-release-codex/validation/Release/fltx_benchmark.exe `
    --platform windows `
    --compiler MSVC `
    --sample-mode full
```

For a direct consumer-fast-math run, use `fltx_accuracy_fastmath.exe` and
`fltx_benchmark_fastmath.exe` and add `--consumer-mode fastmath`. The compiled
fltx library is the same target in both runs; only the consumer translation
units and executable link receive the consumer fast-math option.

`--platform` and `--compiler` remain available for explicit validation, but
may be omitted together to infer the canonical target from the runners.

The runners print their configuration and requested sample profile once,
stream rows in real time, and flush each row to a `.partial.csv`. Benchmark
detail rows retain every trial plus total timed iterations and elapsed time.
A complete passing run atomically publishes
the detail files and canonical `validation/metrics/data/<platform>/<compiler>_<type>.csv`
files. Filtered and smoke runs stay under `build/` and cannot replace published
results. The banner records the ordered implementation set for each precision.
The manifest requires 85 FLTX benchmark rows, 75 Boost rows per precision, and,
when enabled, 60 f128 or 62 f256 qdpp rows and 71 TLFloat rows per precision.
Accuracy contributes 123 FLTX and 114 Boost domain rows per precision. When
enabled, qdpp contributes 87 f128 or 91 f256 rows and TLFloat contributes 107
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

f32 and f64 use the same 119 independently gated accuracy rows, but they are
not merged into the published f128/f256 summary tables. Run both complete
native suites with one command:

```powershell
cmake --build --preset msvc-release-codex --target fltx_native_accuracy_full
```

The target invokes both precisions even if the first fails. Every invocation
gets a new directory under `build/<preset>/validation/native_accuracy`, retaining
the streamed `.partial.csv` witnesses for known failures. The equivalent
direct command is:

```powershell
python validation/metrics/_internal/run_native_accuracy.py `
    --accuracy build/msvc-release-codex/validation/Release/fltx_accuracy.exe
```

## CI policy

The pull-request and branch badge command is:

```powershell
cmake --build --preset msvc-release-codex --target fltx_ci_checks
```

It builds isolated public-header and C++ standard probes, both comparison
library smoke targets, and the in-tree package consumer. It then runs contract,
genuine constexpr, dependency, package, Python tooling, and the complete
65,536-sample accuracy corpus for all four types through normal and
fixed-constexpr runners in both strict and consumer-fast-math modes. The Linux GCC/Clang, Windows
MSVC/MinGW, macOS AppleClang, and WebAssembly Emscripten workflows all invoke
this same target. Any currently reproducible
failure remains recorded in [KNOWN_FAILURES.md](KNOWN_FAILURES.md); CI does not
weaken a threshold or suppress a contract to manufacture a green result.

Authoritative installed-package variants, compile and binary-size telemetry,
performance, and canonical metrics publication remain outside that pull-request target.
The release matrix configures separate `AUTO`, `OFF`, and `ASSUME` build trees,
then runs `fltx_native_accuracy_full`, `fltx_package_tests`, and a full
internal metrics-runner invocation with qdpp enabled when comparative ratios are
wanted. Every command is gating: thresholds and exit codes must reflect the
library contract rather than the desired automation colour.

Compile and size telemetry have distinct opt-in targets:

```powershell
cmake --build --preset msvc-release-codex --target fltx_compile_benchmarks_full
cmake --build --preset msvc-release-codex --target fltx_binary_size
```

The first writes a summary covering public-header cost and expression growth at
25, 100, and 200 functions, plus a separate matrix comparing eager `f256_s`
with public `f256` expressions for 49 individual fused shapes plus the
aggregate all-shapes case. The second reports
dead-code-eliminated linked-size deltas for baseline, arithmetic/expression,
math, I/O, and random-use buckets for both precisions.

After matching full runs exist for all requested targets:

```powershell
python validation/metrics/rebuild_tables.py `
    --input validation/metrics/data `
    --targets windows/MSVC windows/MinGW wasm32/Wasm32 `
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
