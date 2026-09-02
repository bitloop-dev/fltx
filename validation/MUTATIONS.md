# Mutation checks

Mutation testing verifies that each important defect class has a specific test
owner. Apply one mutation in a disposable worktree, run the narrow owner listed
below, and require that owner to fail. The unmodified control must pass under
the same preset.

A recorded run includes the source fingerprint, compiler, preset, mutation,
owner command, and failure output. Discard the disposable worktree after every
case so mutations cannot affect one another.

## Contract owners

| Mutation | Required owner | Expected detection |
| --- | --- | --- |
| Route `fdd roundeven` through nearest-away-from-zero | `contracts/math.cpp` | `roundeven(2.5)` differs from 2 |
| Implement `fdd signbit` as `x.hi < 0.0` | `contracts/io.cpp` | parsed negative zero loses its sign bit |
| Make `fdd nextafter` return its input | `contracts/math.cpp` | the adjacent value is not greater than 1 |
| Disable complete-input checking in `try_parse` | `contracts/io.cpp` | `1.25tail` is accepted and overwrites the destination |
| Drop low limbs while materializing a product expression | `contracts/expressions.cpp` | delayed and direct materialization lose low-limb information |
| Drop the low limbs in the generic `fqd(Expr&&)` constructor | `contracts/expressions.cpp` | direct-list construction loses low-limb information |
| Omit `x3` from `hash_qd` | `contracts/core.cpp` | values differing only in `x3` hash identically |
| Advance a Mersenne Twister engine twice per result | `contracts/random.cpp` | fltx diverges from the corresponding standard engine |
| Return the lower bound from `uniform_real_distribution::max()` | `contracts/random.cpp` | `max()` does not report the upper bound |
| Keep a cached variate after `normal_distribution::reset()` | `contracts/random.cpp` | reset distributions differ from fresh instances |
| Make `fdd isunordered` ignore NaNs | `contracts/core.cpp` | `isunordered(NaN, 1)` returns false |
| Ignore the injected unavailable-FMA probe | `package/consumer.cpp` | the no-FMA `AUTO` consumer selects the FMA path |

## Accuracy owners

| Mutation | Required owner | Expected detection |
| --- | --- | --- |
| Discard the low limb of every `fdd + fdd` result | `accuracy/fdd.cpp`, filter `add` | one or more arithmetic domains miss their threshold |
| Replace the hardware `two_prod` correction with zero | `accuracy/fdd.cpp`, filter `multiply` | multiplication domains miss their thresholds |
| Bypass checked/scaled Dekker in an FMA-OFF build | `accuracy/fdd.cpp`, filter `atan2` | the extreme-finite domain misses its threshold |
| Perturb the leading sine coefficient | `accuracy/fdd.cpp`, filter `sin` | sine domains miss their thresholds |

## Required configurations

Run all contract mutations with MSVC, MinGW, and Emscripten release presets.
Run FMA-sensitive mutations in `AUTO` and `OFF` configurations, plus valid
`ASSUME` configurations where the target supports them. Fixed-consteval
dispatch mutations use `fltx_constexpr_accuracy`; runtime dispatch mutations
use `fltx_accuracy`.
