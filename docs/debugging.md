# Native debugger display

The packaged `fltx.natvis` displays `bl::fdd`, `bl::fqd`, and their `_s` storage
forms as approximate decimal values. For example, an extended π value displays
as `3.14159265358979323846`, and `fdd{1.0, 1e-20}` displays as
`1.00000000000000000001`. Expanding a value still shows its original limbs.

Ordinary magnitudes use fixed notation, following the native double debugger's
exponent cutoffs of -4 through 16. Smaller and larger magnitudes use scientific
notation. A decimal carry across a decade can retain scientific notation at
the boundary. The extra preview digits remain visible in either format.

This uses the existing CMake `/NATVIS` integration, including the installed
target consumed through vcpkg. Consumers need no extension, additional source,
or formatting call. Rebuild/relink the consuming executable after updating the
package so its embedded visualizer is refreshed.

The summary prints 21 digits, with roughly 20 useful significant digits. It is
a debugger preview, not a correctly rounded replacement for `bl::to_chars` or
`bl::to_string`, and does not show the full DD/QD precision. It uses the lower
limbs without evaluating any functions in the debugged application. There is
no object-size, allocation, or arithmetic-runtime overhead.

The extended preview applies when the leading limb's absolute value is at
least `1e-299` and less than `1e299`, with small trailing limbs. Zero, nonfinite
values, subnormals, extreme magnitudes, and unusual unnormalized storage fall
back to the simpler leading-limb display. An ellipsis indicates nonzero lower
limbs in that fallback. Very close to a power of ten, the scientific mantissa
can start with `0.999…`; its exponent still gives the appropriate value.

The native Visual Studio 2026 debugger has been exercised with all four types,
negative values, decimal carries, zero padding, signed zero, nonfinite values,
notation switching, and magnitude fallbacks. Other debugger implementations have not been verified;
the original visualizers remain as lower-priority alternatives.

Selecting among fixed layouts adds debugger evaluation cost. Automated Watch
checks on the development machine took approximately one second per value
after the initial evaluation, compared with tens of milliseconds for the
scientific-only version. This affects debugger inspection, not application
execution.

## Maintaining and checking the visualizer

`validation/_internal/generate_natvis.py` owns the marked decimal-preview section
of `include/fltx.natvis`. It generates small tables of decimal scale factors and
expression-based intrinsics. This generator is not part of consumer builds.
The expression-tree visualizers and simple fallbacks remain handwritten.

Run the following in an MSVC developer terminal from the repository root:

```powershell
python validation/_internal/generate_natvis.py --check
python validation/_internal/test_natvis.py --emit-probe build/natvis/numerical.cpp
cl /nologo /std:c++20 /EHsc /Od /fp:strict build/natvis/numerical.cpp /Fo:build/natvis/numerical.obj /Fe:build/natvis/numerical.exe
python validation/_internal/test_natvis.py --probe build/natvis/numerical.exe
```

The probe compiles the expressions extracted from the shipped XML. Its output
is checked against Python Decimal sums of the exact binary64 limbs at 750-digit
precision. The corpus covers both signs, every supported decimal decade and
its neighbours, sparse lower limbs, randomized DD/QD expansions, and fallback
conditions. The preview's numerical error bound in this check is `1e-20`
relative; it deliberately does not claim correct rounding of the final digit.

Also verify the file in Visual Studio after changing expressions or display
specifiers. Build a Debug consumer of the FLTX CMake target, stop after
initializing values such as `fdd{1.0, 1e-20}` and an extended π or e, and inspect
the scalar and storage forms in Watch. Check the expanded limbs and the
special-value fallbacks. Compiling the expressions alone does not validate
the debugger's parser, intrinsic support, or `hb` zero-padded digit formatting.
