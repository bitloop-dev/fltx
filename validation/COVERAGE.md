# fltx coverage ledger

This is an implementation ledger, not a statement of intended coverage. An
entry names only validation coverage that exists today. `—` means that the lane is
not implemented yet.

Lanes:

- **C**: behavioural Catch2 contract;
- **A**: differential accuracy row;
- **CE**: genuine constant evaluation (`static_assert`);
- **CA**: fixed-simulation constexpr-algorithm accuracy row;
- **O**: overload/return-type compile check;
- **B**: benchmark row;
- **H**: isolated public-header compile probe.

**CA** is provided by `fltx_constexpr_accuracy`, which links an isolated
`FLTX_SIMULATE_FIXED_CONSTEVAL_MODE` library to the complete MPFR accuracy
runner. It exercises the constexpr algorithms with the same deterministic
domains, special values, and thresholds as normal runtime accuracy. Cross-path
bit identity is intentionally not a contract.

Every public header has an **H** probe through `fltx_compile_contract`, with
`FLTX_ENABLE_FQD_EXPRESSIONS` both undefined and set to `1`.
The representative C++20 surface also compiles with
`FLTX_DISABLE_MATH_USES_CHECKED_DEKKER`, checking the opt-out and constexpr native `pow`.
C++20/C++23 probes check both expression modes, including full-value unary/binary
return types, mixed arithmetic and same-type generic deduction. Storage-only
arithmetic retains its storage return types. With
`FLTX_VALIDATION_FQD_EXPRESSIONS=OFF`, `contracts/expressions.cpp` checks eager
generic boundaries and storage-kernel parity; the suites otherwise opt in.
The `native.h`, `native_math.h`, and `native_string.h` probes additionally
require the fdd/fqd value and storage types to remain incomplete, enforcing the
native-only include boundary.
`contracts/overload_matrix.cpp` separately instantiates every `<cmath>`-shaped
operation family across the native, integer, fdd, fqd, and mixed-rank
promotion boundaries. Header probes prove self-containment; the matrix proves
public call and return-type contracts.

## Core and integration

| Public family | Types exercised | C | CE / CA / O | A / B | Notes |
|---|---|---|---|---|---|
| fdd/fqd storage and value construction, assignment, scalar/integer and cross-precision conversion | fdd/fqd | `contracts/core.cpp` | CE: `constexpr/core.cpp`, Boolean assertions in `contracts/core.cpp`; O: `contracts/overloads.cpp`, `contracts/overload_matrix.cpp` | — | Covers every fundamental arithmetic type, explicit narrowing, implicit widening, exact 64-bit boundaries, low-limb integer truncation, signed zero, `long double`, aggregate storage, both cross-precision directions, and bitwise Boolean conversion of zeros, subnormals, infinities and NaNs in strict/fast-math modes |
| unary, binary and compound `+ - * /` | fdd/fqd | `contracts/core.cpp` | CE: `constexpr/core.cpp`, `constexpr/accuracy.cpp`; CA: all; O: `contracts/overloads.cpp` | A: f32/f64/fdd/fqd; B: fdd/fqd | Every distinct scalar category and cross-precision route is instantiated; runtime fast-math waives NaN, infinity, and signed-zero guarantees for basic fdd/fqd arithmetic while genuine constant evaluation remains checked |
| comparison, `<=>`, unordered NaN behaviour | fdd/fqd | `contracts/core.cpp` | CE: `constexpr/core.cpp`, six-operator matrix in `contracts/core.cpp` | B: six fdd/fqd relational operators | Runtime and constexpr pairwise ordering includes signed zeros, infinities, NaNs, and equal heads with distinct tails |
| `approx_eq` and parity tolerances | same-precision f32/f64/fdd/fqd, including fqd expressions | `contracts/approx_comparison.cpp` | CE: default and custom relative-threshold cases; O: accepted same-precision and rejected mixed/four-argument calls | — | Covers named defaults, native runtime/constexpr math parity, scaling, signed zero, infinities, NaNs, invalid tolerances, symmetry, exact threshold boundaries, and mixed expression shapes |
| classification, signed zero, subnormals, infinities and NaNs | all | `contracts/core.cpp` | CE: `constexpr/core.cpp` for fdd/fqd | — | Includes unordered comparisons |
| `abs`, `fabs`, `sqr`, `clamp`, `recip` | fdd/fqd | `contracts/math.cpp` | CE: `abs`, `clamp`, `recip`; CA: all except `clamp`; O: complete type matrix | A: all except `clamp`; B: fdd/fqd helpers | `fabs` and `sqr` are instantiated as constexpr-capable APIs; dd reciprocal exact-power contracts include exponents ±1000 |
| aliases, concepts, `value_t`, `storage_t`, rank, `common_float_type_t`, `FloatType` | all | `contracts/core.cpp` | O: `contracts/overloads.cpp`, `contracts/overload_matrix.cpp`, `standard/cxx_standard_contract.cpp`, `contracts/expressions.cpp` | — | Includes cv/ref normalization, value/storage mappings before scalar definitions, idempotence and cross-normalization, unrelated-type passthrough, every fqd expression node, both expression policies in C++20/C++23, and every precision-rank boundary |
| `numeric_limits`, `eps`, `highest`, `std::numbers` | fdd/fqd storage/value | `contracts/core.cpp` | compile-time forwarding assertions | — | Covers nominal 106/212-bit epsilon, normal/subnormal range, exponent metadata, finite extrema, denormal policy, and constants; sparse arithmetic storage remains unrestricted |
| `std::hash` | fdd/fqd storage/value | `contracts/core.cpp` | — | — | Signed zero, every limb, storage/value agreement, unordered containers, and deterministic infinity/NaN hashing |
| runtime/template dispatch helpers | all | `contracts/dispatch.cpp` | — | — | Covers type tags, raw/sparse enums, free/member/callable tables, multiple dimensions, invocation, counts, domain sizes and reports |
| installed `find_package` consumers | fdd/fqd | `package/consumer.cpp` | strict, fast-math, injected-no-FMA AUTO, expression-enabled and explicit-`0` variants | — | One library serves every consumer; includes constexpr/runtime expression boundaries, value layout, FMA and cancellation/EFT-sensitive products |

## Math

The fdd/fqd accuracy runner currently owns these numerical operations:

`add`, `subtract`, `multiply`, `divide`, `fma`, `sqrt`, `cbrt`, `hypot`,
`sin`, `cos`, `tan`, `atan`, `atan2`, `asin`, `acos`, `exp`, `exp2`,
`expm1`, `log`, `log2`, `log10`, `log1p`, `pow`, `ipow`, `fmod`,
`remainder`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `erf`,
`erfc`, `lgamma`, `tgamma`, `abs`, `fabs`, `sqr`, `recip`, `fmin`,
`fmax`, `fdim`, `copysign`, `floor`, `ceil`, `trunc`, `round`,
`roundeven`, `lround`, `llround`, `round_decimals`,
`round_significant`, `remquo`, `modf`, `ldexp`, `scalbn`,
`scalbln`, `frexp`, `ilogb`, and `logb`.

The f32/f64 accuracy runner owns the same applicable numerical set. Its
exponent, subnormal, cancellation and argument-reduction samples are generated
at the native type's range so float inputs do not overflow or collapse to
double-only test values. CTest owns the deterministic smoke runs;
`fltx_ci_checks` runs the 4,096-sample standard corpus for all four types through
both the normal and fixed-simulation runners. Explicit full and release metrics
workflows retain the 65,536-sample corpus, and `fltx_native_accuracy_full`
retains a standalone complete f32/f64 orchestration target. Detailed output
remains under the build tree. Native runtime f32/f64 threshold misses are
advisory platform-libm baseline evidence in both the orchestration target and
`fltx_ci_checks`; runner integrity still gates. Runtime and fixed-simulation accuracy under
consumer fast-math are advisory for every precision because reassociation can
invalidate expansion arithmetic and optimized runtime execution cannot
faithfully model the compiler's constant evaluator. Their direct CTest smoke
checks mirror that policy. Genuine f32/f64 numerical constant evaluation gates
the strict consumer, exact native special-value assertions compile in both
consumer profiles, the strict native fixed-simulation remains gating, and
strict fdd/fqd accuracy thresholds remain gating. The full consumer-fast-math contract
executable is compiled and remains directly runnable, but its semantic suite is
diagnostic rather than gating because that compiler profile permits
reassociation, signed-zero loss, and subnormal flushing. Native accuracy is
never published in the fdd/fqd summary tables.
The fixed-simulation consumer-fast-math accuracy runner reports `-` for its
special-value field because clang may lower the deliberate NaN/infinity probe
construction to a trap in that artificial mode. Real runtime fast-math, all
strict accuracy lanes, and the genuine constexpr corpus retain special-value
ownership.

| Public family | C | CE | CA | O | A | B |
|---|---|---|---|---|---|---|
| `abs`, `fabs`, `sqr`, `recip`, `fmin`, `fmax`, `fdim`, `copysign`, `clamp` | `contracts/math.cpp` (all types) | `abs`, `clamp`, `recip` | all except `clamp` | complete matrix | all except `clamp` | fdd/fqd helpers |
| `min`, `max`, `minmax`, mixed `clamp`, `lerp`, `midpoint` | `contracts/math.cpp`, `contracts/expressions.cpp`: ties, NaNs, finite extremes, subnormals, interpolation bounds/monotonicity, exact 64-bit selection/midpoints; `min`/`max` initializer lists including mixed expression shapes and owning results | `constexpr/core.cpp`: expression calls, initializer lists, exact values, extremes/subnormals | — | mixed native/storage/value/expression return types, including `min`/`max` lists | — | — |
| `floor`, `ceil`, `trunc`, `round`, `roundeven` | `contracts/math.cpp` (all types), including runtime/constexpr tails around positive and negative `round` ties, integral heads, large heads and signed zero | yes | all types | complete matrix | all types | fdd/fqd |
| `lround`, `llround` | `contracts/math.cpp` | yes | all types | complete matrix | all types | fdd/fqd |
| `round_decimals`, `round_significant` | `contracts/math.cpp` | yes | all types | complete matrix | all types | fdd/fqd |
| `fma` | `contracts/math.cpp`, `contracts/edge_cases.cpp`, `package/consumer.cpp` | arithmetic corpus | all types | complete mixed-rank matrix | all types | yes |
| `fmod`, `remainder`, `remquo` | `contracts/math.cpp` (all types), `contracts/edge_cases.cpp` | MPFR corpus plus `remquo` decomposition | all types | complete matrix | all three for all types | fdd/fqd |
| `modf`, `frexp`, `ldexp`, `scalbn`, `scalbln`, `ilogb`, `logb` | `contracts/math.cpp` (all types) | yes | all types | complete matrix | all types | fdd/fqd |
| `nextafter`, `nexttoward` | `contracts/math.cpp` (all types), `contracts/edge_cases.cpp` | exact nominal steps and boundaries in `constexpr/core.cpp` | — | complete target-type matrix, including extended `long double` where available | — | fdd/fqd use binade-relative 106/212-bit steps without rounding ordinary math results |
| `sqrt`, `cbrt`, `hypot` | `contracts/math.cpp` (all types) | MPFR corpus; exact-family checks for all | all types | complete matrix | all types | all |
| `exp`, `exp2`, `expm1` | special values in `contracts/math.cpp` | MPFR corpus | all types | complete matrix | all types | all |
| `log`, `log2`, `log10`, `log1p`, `log_as_double` | `log_as_double` and special values | first four in MPFR corpus; `log_as_double` representative | first four | first four complete | first four for all types | first four |
| `pow`, `ipow` | exact values in `contracts/math.cpp` | MPFR `pow` corpus plus `ipow` | all types | complete exponent/promotion matrix | both for all types | both |
| `sin`, `cos`, `tan`, `sincos` overloads | `sincos` for all types plus sin/cos special values in `contracts/math.cpp` | first three in MPFR corpus plus `sincos` | first three | complete matrix | first three for all types | all four |
| `asin`, `acos`, `atan`, `atan2` | special values and canonical results in `contracts/math.cpp` | MPFR corpus | all types | complete matrix | all types | yes |
| `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh` | special values and canonical results in `contracts/math.cpp` | MPFR corpus | all types | complete matrix | all types | yes |
| `erf`, `erfc`, `lgamma`, `tgamma` | special values and canonical results in `contracts/math.cpp` | MPFR corpus | all types | complete matrix | all types | all |

Accuracy domains are assigned explicitly in `accuracy/fdd.cpp`,
`accuracy/fqd.cpp`, and `accuracy/native.cpp`. The table deliberately does not
claim domains for operations without rows.

The new selection/interpolation helpers have behavioural and genuine-constexpr
contracts, but no standalone MPFR accuracy or benchmark rows yet.

The lack of standalone accuracy rows for `clamp`, `nextafter`, `nexttoward`,
`sincos`, and `log_as_double` is explicit. `clamp` and stepping currently use
exact behavioural contracts; `sincos` is checked semantically while its two
components are independently measured as `sin` and `cos`; `log_as_double` has
behavioural and genuine-constexpr ownership. These are intentional exact or
component-owned contracts rather than missing numerical rows.

`contracts/math.cpp` applies the same operation-family special-value matrices
to f32, f64, fdd and fqd. They cover NaN propagation, both infinities,
positive and negative zero, invalid domains, poles and signed results across
arithmetic helpers, roots and powers, trigonometric/hyperbolic/special
functions, rounding, remainder, decomposition and stepping. Consumer
fast-math excludes runtime fdd/fqd `+`, `-`, `*`, and `/` from that matrix;
their genuine constant-evaluation special-value behavior remains covered.

The rounding contracts include long/long-long minimum `+0.5`-limb witnesses,
the `2^52`/`2^53` integer boundaries, and varied
decimal/significant-figure tie cases.

Matched-bit scores are relative-error scores in the ordinary range. Near the
component type's absolute spacing limit, the runner preserves the same
accuracy slack relative to the precision that an expansion can physically
represent there. In other words, one underlying float/double ULP is mapped to
the nominal precision before applying the normal threshold. Exact zero remains
strict: a spurious minimum subnormal still scores zero bits. This
resolution-adjusted score prevents underflow from being reported as an
algorithmic precision loss without concealing wrong zeros, infinities, signs,
or multi-ULP errors.

Accuracy rows for exact families use exact MPFR equality rather than treating
the normal bit threshold as sufficient. Zero results must also carry the
expected sign. Exact rows currently cover absolute value, minimum/maximum,
copy-sign, integer-valued rounding, decomposition and binary scaling families.
Before emitting rows, every runner also compares representative `sin`, `exp`,
`log1p`, and `erfc` references from the 400-decimal-digit oracle with an
independent 800-decimal-digit backend.

An unfiltered fdd/fqd run writes 127 independently gated operation/domain
rows; the native f32/f64 runs retain their 119 numerical rows. Every row keeps
its deterministic seed and worst witness, so no aggregate score can make a
failed domain pass.

Any current threshold failures are recorded in `KNOWN_FAILURES.md`; they remain
gating failures rather than threshold exceptions.

## I/O, random, expressions, and numerical edge cases

| Public family | Implemented owner | Constant evaluation | Known limit |
|---|---|---|---|
| `parse_result<T>`, `from_chars`, `try_parse`, `parse` | `contracts/io.cpp`, `accuracy/fdd.cpp`, `accuracy/fqd.cpp` | complete-token success, trailing-input rejection and fallback in `constexpr/core.cpp` | Result value/error/consumed/bool state; native, storage and value forms plus fallback, partial, invalid, range and format behavior; deterministic long-decimal accuracy against one shared MPFR oracle |
| `to_chars`, `to_string`, `to_static_string` | `contracts/io.cpp` | fixed-format `to_static_string` | All three `to_chars` forms and all native/storage/value families; fdd/fqd `to_chars` and `to_string` benchmarks |
| `precision_info`, `trailing_zero_policy`, `static_string<N>`, `f32_io_string`, `f64_io_string`, `fdd_io_string`, `fqd_io_string` | `contracts/io.cpp` | construction, mutation, capacity and returned alias types | Focused public string-surface contract rather than exhaustive `std::string` emulation |
| `_dd`, `_qd`, integer literals | `contracts/io.cpp`, `contracts/core.cpp` | decimal and hex literal assertions | |
| stream insertion/extraction | `contracts/io.cpp` | runtime only | |
| `std::formatter` | `contracts/io.cpp` when available, plus header probe | formatter parsing is compile-time through the library | General/fixed/scientific/hex, signs, alternate form, zero fill, alignment, special values, invalid specs, long fqd precision, and expression/value formatting parity; platform-dependent availability |
| deterministic formatter/parser round trip | `contracts/io.cpp` | sparse fdd and finite-extreme fqd cases in `constexpr/core.cpp` | Defaults use the normalized-expansion `max_digits10` values of 34/67 digits. General-format coverage includes a sparse-tail fdd regression; 10,000 independently constructed, bounded-spacing raw-limb values per type round-trip exactly in scientific format at those precision settings, including signed zero, mixed-sign tails, and extreme boundaries |
| `seed_seq`, `mt19937`, `mt19937_64` | `contracts/random.cpp` | `mt19937_64` through array generation | Constructors, seed/generate/param, min/max, discard, comparison, valid/malformed streams and standard-engine equivalence |
| `random_device` | `contracts/random.cpp` | runtime only | |
| `uniform_int_distribution`, `generate_canonical` | `contracts/random.cpp` | fdd/fqd canonical generation | Constructors, params, bounds, override calls, equality and streams, including full-width integer ranges |
| uniform real distribution and array helpers | `contracts/random.cpp` | deterministic fdd/fqd and expression arrays in `constexpr/core.cpp` | Native/fdd/fqd/expression parameters, expression CTAD, and every engine/seed/default array overload; fdd/fqd distribution benchmark |
| exponential, normal, lognormal distributions | `contracts/random.cpp` | fdd/fqd samples and expression parameters in `constexpr/core.cpp` | Every accessor, param/reset/override/equality/stream form; expression CTAD and normal-array parity; native cached-state and extended parameter-only behavior; fdd/fqd normal benchmark |
| fqd expression lifetime, routing and materialization | `contracts/expressions.cpp`, `contracts/overload_matrix.cpp` | conversions, const/lvalue composition, ADL and mixed math promotion in `constexpr/core.cpp` | Covers delayed temporaries and storage members, every node shape, owning copies through generic helpers, explicit conversion to all fundamental arithmetic targets, promotion by value type, independent expected values, and a fusion-sensitive residual |
| numerical edge cases | `contracts/edge_cases.cpp`, fmod `quotient_reduction` accuracy domain | boundary stepping also checked in CE | Covers large expansion limbs, deep-limb and huge-quotient remainder reduction, exact half ties, normal/subnormal stepping, and fdd FMA with an overflowing leading product |

## Benchmark ownership

`benchmarks/runtime/operations.cpp` currently owns 77 rows for each of fdd and fqd:

- arithmetic: `add`, `subtract`, `multiply`, `divide`, `fma`, `abs`,
  `fabs`, `sqr`, `recip`, `fmin`, `fmax`, `fdim`, `copysign`;
- rounding: `floor`, `ceil`, `trunc`, `round`, `roundeven`, `lround`,
  `llround`, `round_decimals`, `round_significant`;
- remainder: `fmod`, `remainder`, `remquo`;
- decomposition: `modf`, `ldexp`, `scalbn`, `scalbln`, `frexp`, `ilogb`,
  `logb`;
- stepping: `nextafter`, `nexttoward`;
- comparison: `equal`, `not_equal`, `less`, `less_equal`, `greater`,
  `greater_equal`;
- roots: `sqrt`, `cbrt`, `hypot`;
- trigonometric: `sin`, `cos`, `sincos`, `tan`, `atan`, `atan2`, `asin`,
  `acos`;
- powers: `exp`, `exp2`, `expm1`, `log`, `log2`, `log10`, `log1p`, `pow`,
  `ipow`;
- hyperbolic: `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`;
- special: `erf`, `erfc`, `lgamma`, `tgamma`;
- expressions: `product_sum`;
- I/O: `to_string`, `to_chars`, `parse`;
- random: `mt19937_64`, `uniform_real`, `normal`.

`benchmarks/runtime/workloads.cpp` owns `chained_arithmetic`, `affine_trig`,
`mandelbrot`, `matrix_vector_4x4`, `horner_polynomial`, `newton_root`,
`log_sum_exp`, and `haversine`, bringing the manifest to 85 rows per
precision. `product_sum` is intentionally the same source expression for both
precisions: fqd can route it through its fused expression evaluator, while
fdd provides the eager analogue. Each benchmark uses calibrated equal-work
batches, reports a median, and retains each normalized trial in the detailed
CSV. `to_chars` is compared with qdpp's public buffer-writing API and labelled
`qdpp write`.
Dispatch remains untimed because its call overhead is already included in the
operations it selects.

The runner uses a normalized implementation model rather than a single
reference. Every logical trial runs FLTX and all available comparisons once,
with a rotating start position. The manifests contain 85 FLTX rows,
75 Boost rows per precision, 71 TLFloat rows per precision, and 60 fdd or 62
fqd qdpp rows. qdpp's fqd-only difference is its public `min`/`max`;
`dd_real` exposes neither. Boost and TLFloat deliberately omit operations for
which they have no matching public API. The internal metrics manifest validates each
enabled implementation independently; disabling one optional implementation
does not weaken the others' manifests.

Accuracy uses the same samples, domains, seed, and 400-digit MPFR result for
every registered implementation. It records 127 FLTX and 118 Boost rows per
precision and, when enabled, 87 fdd or 91 fqd qdpp rows and 107 TLFloat
rows. These counts are asserted by the internal metrics manifest.

`metrics/_internal/test_tools.py` also owns operation-filter orchestration:
exact selections preserve each operation's domain and implementation manifest,
isolate reusable evidence, reject filtered publication, and pass through both
consumer modes and native baselines. The same host matrix includes all twelve
release presets; executable performance coverage depends on native host access.
Parse accuracy uses the original exact decimal value rather than a
format-rounded surrogate. Matched bits therefore retain the same numerical
meaning across expansion and IEEE representations; exact-sample ceilings and
underflow adjustment use each implementation's declared precision and range.
Competitor accuracy is informational; only FLTX's source-controlled thresholds
affect the exit code.

## Release validation

The remaining release-evidence tasks are:

1. resolve or explicitly approve every finding in
   [KNOWN_FAILURES.md](KNOWN_FAILURES.md);
2. run FMA `AUTO`, `OFF`, and valid `ASSUME` configurations, including the
   injected unavailable-FMA `AUTO` path, on every applicable target;
3. repeat the mutation campaign under MinGW and Emscripten and tie every run to
   committed test-owner sources;
4. freeze compile-time, linked-size, and clean-library-build budgets, then run
   `fltx_compile_benchmarks_full` and `fltx_binary_size` on the release matrix;
5. publish complete, same-fingerprint canonical evidence for Windows MSVC and
   MinGW, Linux GCC and Clang, macOS AppleClang, and Emscripten;
6. reconcile the README's numerical and comparative claims with the final
   worst-domain and benchmark evidence in `RELEASE_CHECKLIST.md`.
