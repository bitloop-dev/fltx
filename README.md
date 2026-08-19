<p align="center">
  <img src="res/logo.webp" alt="fltx logo" width="80"><br>
  Extended-precision floating point math library<br>
  Fast · Precise · Lightweight
</p>


[![Native Accuracy Matrix](https://github.com/bitloop-dev/fltx/actions/workflows/tests.yml/badge.svg)](https://github.com/bitloop-dev/fltx/actions/workflows/tests.yml)

`fltx` is a high-performance C++ library for double-double and quad-double arithmetic, with a broad std-style math library and extensive constexpr support throughout.

## Menu

- [Overview](https://github.com/willmh93/fltx#highlights): highlights and core types
- [Get Started](https://github.com/willmh93/fltx#quick-start): quick start and installation
- [Library Guide](https://github.com/willmh93/fltx#use-cases): use cases, headers, numeric types, math, and IO
- [Advanced Features](https://github.com/willmh93/fltx#template-dispatch): template dispatch
- [Developer](https://github.com/willmh93/fltx#building-fltx): building fltx and running tests
- [Project Details](https://github.com/willmh93/fltx#benchmarks): benchmarks, implementation notes, and license

## Highlights

Features:

- Fixed-size extended-precision scalar types: [`bl::f128`](include/fltx/f128.h) and [`bl::f256`](include/fltx/f256.h)
- Library-wide **constexpr** support for math, parsing, static formatting, and conversions
- Familiar `std::` style APIs; many calls work by swapping `std::` for `bl::`
- No required runtime dependencies
- Standard-library integration for streams, manipulators, `std::numeric_limits`, `std::numbers`, `std::hash`, and `std::format`

Accuracy:

- Accuracy validated against [boost::multiprecision::mpfr_float_backend](https://www.boost.org/doc/libs/latest/libs/multiprecision/doc/html/boost_multiprecision/tut/floats/mpfr_float.html)
- **Infinity** / **NaN** correctness
- Enable `FLTX_CONSTEXPR_PARITY` for bitwise-identical runtime and `constexpr` results (reduces performance)

Performance:

- Optimised for both runtime and compile-time performance
- Compatible with fast-math builds
- Internal [`bl::f256`](include/fltx/f256.h) expression fusion reduces intermediate rounding and temporary value materialisation for common arithmetic shapes.
- Hardware acceleration where supported, including x86/x64 SSE2 with FMA when available, AArch64 NEON, and WebAssembly SIMD128 via Emscripten
- Suitable for lightweight native builds, WebAssembly / Emscripten
- Optional runtime-to-compile-time dispatch helper for template-specialized kernels

## Core Types

| Type | Representation | Accuracy |
|---|---|---|
| [`bl::f32`](include/fltx/aliases.h) | Alias for native [`float`](https://en.cppreference.com/w/cpp/language/types) | Native [`float`](https://en.cppreference.com/w/cpp/language/types) precision |
| [`bl::f64`](include/fltx/aliases.h) | Alias for native [`double`](https://en.cppreference.com/w/cpp/language/types) | Native [`double`](https://en.cppreference.com/w/cpp/language/types) precision |
| [`bl::fdd`](include/fltx/f128.h) | Double-double | Minimum 29 decimal digits across arithmetic and math functions |
| [`bl::fqd`](include/fltx/f256.h) | Quad-double | Minimum 59 decimal digits across arithmetic and math functions |

The names refer to storage size:

```text
sizeof(bl::f128) == 16
sizeof(bl::f256) == 32
```

[`bl::f128`](include/fltx/f128.h) and [`bl::f256`](include/fltx/f256.h) are *not* IEEE [binary128](https://en.wikipedia.org/wiki/Quadruple-precision_floating-point_format#IEEE_754_quadruple-precision_binary_floating-point_format:_binary128) / [binary256](https://en.wikipedia.org/wiki/Octuple-precision_floating-point_format#IEEE_754_octuple-precision_binary_floating-point_format:_binary256) types.

They are fixed-size expansion types: `f128` is double-double, and `f256` is quad-double. The names are intentionally short and uniform with `f32`/`f64`; they describe the precision tier, not the underlying representation.

## Quick Start

```cpp
#include <iostream>
#include <iomanip>

#include <fltx.h>
using namespace bl;
using namespace bl::literals;

int main()
{
    constexpr f256 a = 1_qd / 3_qd;
    constexpr f256 b = 2_qd / 3_qd;
    constexpr f256 c = a + b;
    constexpr f256 d = bl::sin(a + b);

    std::cout
        << std::fixed
        << std::setprecision(std::numeric_limits<f256>::digits10)
        << "a = " << a << "\n"
        << "b = " << b << "\n"
        << "c = " << c << "\n"
        << "d = " << d << "\n";
}
```

Output:

```text
a = 0.333333333333333333333333333333333333333333333333333333333333333
b = 0.666666666666666666666666666666666666666666666666666666666666667
c = 1.000000000000000000000000000000000000000000000000000000000000000
d = 0.841470984807896506652502321630298999622563060798371065672751710
```

More examples:

| Example | Shows |
|---|---|
| [`example_basic.cpp`](examples/example_basic.cpp) | Basic `f128` / `f256` arithmetic, literals, and output. |
| [`example_constexpr_io.cpp`](examples/example_constexpr_io.cpp) | Compile-time parsing, formatting, and string conversion. |
| [`example_dispatch.cpp`](examples/example_dispatch.cpp) | Generic code dispatching across FLTX precision types. |
| [`example_mandelbrot.cpp`](examples/example_mandelbrot.cpp) | Higher-precision numeric work in a small visual workload. |
| [`example_consteval_ellipse.cpp`](examples/example_consteval_ellipse.cpp) | Consteval geometry using FLTX math. |
| [`example_consteval_library_sweep.cpp`](examples/example_consteval_library_sweep.cpp) | Library-wide constexpr coverage across the supported function groups. |
| [`example_random.cpp`](examples/example_random.cpp) | Random value generation for FLTX types. |
| [`example_pow.cpp`](examples/example_pow.cpp) | `bl::pow` / `bl::ipow` overload behavior, return-type policy, and fast paths. |
| [`example_newton_solver.cpp`](examples/example_newton_solver.cpp) | Generic Newton solving across `f64`, `f128`, and `f256`. |
| [`example_charconv.cpp`](examples/example_charconv.cpp) | Buffer-oriented `to_chars` / `from_chars` parsing and formatting. |
| [`example_std_integration.cpp`](examples/example_std_integration.cpp) | Standard-library integration with numbers, limits, hashing, comparisons, and optional `std::format`. |
| [`example_storage_interop.cpp`](examples/example_storage_interop.cpp) | Using `f128_s` / `f256_s` storage forms at fixed-size API boundaries. |
| [`example_special_values.cpp`](examples/example_special_values.cpp) | NaN, infinity, signed zero, classification, and layout helpers such as `frexp` / `modf`. |
| [`example_polynomial.cpp`](examples/example_polynomial.cpp) | Polynomial evaluation with `fma` and precision-sensitive cancellation. |
| [`example_consteval_coefficients.cpp`](examples/example_consteval_coefficients.cpp) | Compile-time generation of high-precision coefficient tables. |

## Installation

### vcpkg

Add the bitloop registry to `vcpkg-configuration.json`, pinning the registry commit you want to consume:

```json
{
  "registries": [
    {
      "kind": "git",
      "baseline": "5504123246482f0bbb58fd53783ae1b1e3fa88f9",
      "repository": "https://github.com/willmh93/bitloop-registry.git",
      "packages": ["fltx"]
    }
  ]
}
```

Then add `fltx` to `vcpkg.json`:

```json
{
  "name": "myapp",
  "version": "1.0.0",
  "dependencies": [
    "fltx"
  ]
}
```

### CMake

With vcpkg:

```cmake
find_package(fltx CONFIG REQUIRED)
target_link_libraries(main PRIVATE fltx::fltx)
```

Or include it directly:

```cmake
add_subdirectory(/path/to/fltx fltx-build)
target_link_libraries(main PRIVATE fltx::fltx)
```

The package has three performance controls:

| Option | Default | Meaning |
|---|---|---|
| `FLTX_FMA_MODE=AUTO\|OFF\|ASSUME` | `AUTO` | Safely detect x86 FMA at runtime, disable hardware FMA, or let the consumer guarantee FMA support. `ASSUME` exports `-mfma` where GNU-style compilers require it and performs no runtime feature check. |
| `FLTX_SIMD=ON\|OFF` | `ON` | Enable supported SIMD paths. Emscripten `ON` supplies `-msimd128` to both the library and consuming translation units. |
| `FLTX_INTERNAL_FAST_MATH=AUTO\|ON\|OFF` | `AUTO` | Control only the compiled `fltx` runtime sources. `AUTO` enables a platform only after its accuracy and performance gate is approved; the current approval list is empty. These flags are never exported to consumers. |

Normal build-type optimization flags remain controlled by the selected CMake configuration and toolchain.

## Use Cases

`fltx` is aimed at workloads where both precision and speed matter:

- simulations and iterative numerical kernels
- geometric transforms
- numerically sensitive reference code
- compile-time validation and test generation
- fixed-size interop boundaries that cannot use arbitrary-precision types

It is a good fit when you want an extended-precision type that can still be passed around like a normal scalar in ordinary C++ code.

## Public Headers

| Header | Provides |
|---|---|
| [`fltx.h`](include/fltx.h) | Umbrella header: Core types, IO, math, random, hashing and template dispatch helper |
| [`fltx/core.h`](include/fltx/core.h) | Core numeric types, storage types, arithmetic, conversions, and numeric integration |
| [`fltx/math.h`](include/fltx/math.h) | Core types plus the `constexpr` Math API |
| [`fltx/io.h`](include/fltx/io.h) | Limits, numbers, `charconv`-shaped helpers, string conversion, stream output, and literals |
| [`fltx/random.h`](include/fltx/random.h) | Standard-shaped random engines and distributions |
| [`fltx/hash.h`](include/fltx/hash.h) | `std::hash` specializations for the extended types |

Individual headers are also available when you want a smaller include surface:

| Header | Provides |
|---|---|
| [`fltx/f128.h`](include/fltx/f128.h)<br>[`fltx/f256.h`](include/fltx/f256.h) | Individual extended-precision types, storage types, and core operations |
| [`fltx/f32_math.h`](include/fltx/f32_math.h)<br>[`fltx/f64_math.h`](include/fltx/f64_math.h)<br>[`fltx/f128_math.h`](include/fltx/f128_math.h)<br>[`fltx/f256_math.h`](include/fltx/f256_math.h) | Math APIs for individual floating-point types |
| [`fltx/f128_io.h`](include/fltx/f128_io.h)<br>[`fltx/f256_io.h`](include/fltx/f256_io.h) | Extended-type string conversion, stream output, and `_dd` / `_qd` literals |
| [`fltx/charconv.h`](include/fltx/charconv.h) | `bl::to_chars`, `bl::from_chars`, `bl::parse<T>`, `bl::parse<T>(text, fallback)`, and `bl::try_parse<T>` for `f32`, `f64`, `f128`, and `f256` |
| [`fltx/aliases.h`](include/fltx/aliases.h) | Fundamental aliases such as `f32`, `f64`, `f128`, and `f256` |
| [`fltx/traits.h`](include/fltx/traits.h) | Concepts, type traits, `FloatType` enum, and `bl::to_string(FloatType)` |
| [`fltx/format.h`](include/fltx/format.h) | Optional `std::formatter` specializations when `<format>` is available |
| [`fltx/dispatch.h`](include/fltx/dispatch.h) | Includes standalone [`fltx/template_dispatch.h`](include/fltx/template_dispatch.h) runtime-to-compile-time dispatch-table helper,<br>plus mappings for `FloatType` values to `f32`, `f64`, `f128`, or `f256` |

## Numeric Types

The main user-facing types are:

```cpp
bl::f32     // float alias
bl::f64     // double alias
bl::f128    // double-double scalar type
bl::f256    // quad-double scalar type

bl::f128_s  // trivial storage form
bl::f256_s  // trivial storage form (no fused-expression support)
```

The trivial storage forms are useful when standard-layout storage matters, such as packed structures, unions, binary buffers, or interop boundaries. They convert cleanly to and from the full scalar types.

```cpp
bl::f128_s a { 5.0 }; // storage forms are trivially constructible
bl::f128   b = 5.0f;  // full scalar type

bl::f128_s c = a + b;
bl::f128   d = c;
```

Common type concepts and traits are available from [`fltx/traits.h`](include/fltx/traits.h):

```cpp
bl::fltx_f32<T>                  // true for bl::f32
bl::fltx_f64<T>                  // true for bl::f64
bl::fltx_f128<T>                 // true for bl::f128 or bl::f128_s
bl::fltx_f256<T>                 // true for bl::f256 or bl::f256_s

bl::fltx_extended_float<T>       // true for fltx f128/f256 scalar or storage types
bl::fltx_float<T>                // true for the fltx float family: f32, f64, f128, or f256
bl::fltx_arithmetic<T>           // true for native arithmetic or fltx extended floats

bl::is_f32_v<T>                  // bool-value form of bl::fltx_f32<T>
bl::is_f64_v<T>                  // bool-value form of bl::fltx_f64<T>
bl::is_f128_v<T>                 // bool-value form of bl::fltx_f128<T>
bl::is_f256_v<T>                 // bool-value form of bl::fltx_f256<T>

bl::is_fltx_extended_float_v<T>  // bool-value form of bl::fltx_extended_float<T>
bl::is_fltx_float_v<T>           // bool-value form of bl::fltx_float<T>
bl::is_arithmetic_v<T>           // bool-value form of bl::fltx_arithmetic<T>
bl::is_integral_v<T>             // true for native integral types (for consistency)
```

## Math API

[`fltx/math.h`](include/fltx/math.h) provides a `constexpr` interface modeled after the `<cmath>` API across [`bl::f32`](include/fltx/aliases.h), [`bl::f64`](include/fltx/aliases.h), [`bl::f128`](include/fltx/f128.h), and [`bl::f256`](include/fltx/f256.h).

This lets generic numeric code switch precision without changing its math calls:

```cpp
template<class T>
constexpr T radius(T x, T y)
{
    return bl::sqrt(x * x + y * y);
}

constexpr bl::f32   a = radius(1.0f, 2.0f);
constexpr bl::f64   b = radius(1.0,  2.0);
constexpr bl::f128  c = radius(1_dd, 2_dd);
constexpr bl::f256  d = radius(1_qd, 2_qd);
```

Supported function groups:

| constexpr | Category | Functions |
|---|---|---|
| ✅ | Arithmetic | `abs`, `fabs`, `fma` |
| ✅ | Rounding | `floor`, `ceil`, `trunc`, `round`, `lround`, `llround`, `round_decimals`, `round_significant`, `roundeven` |
| ✅ | Remainders | `fmod`, `remainder`, `remquo` |
| ✅ | Min / max / sign | `fmin`, `fmax`, `fdim`, `copysign`, `signbit` |
| ✅ | Roots / powers | `sqrt`, `cbrt`, `hypot`, `pow`, `ipow` |
| ✅ | Exp / log | `exp`, `exp2`, `expm1`, `log`, `log2`, `log10`, `log1p`, `logb`, `ilogb` |
| ✅ | Trigonometry | `sin`, `cos`, `tan`, `sincos`, `asin`, `acos`, `atan`, `atan2` |
| ✅ | Hyperbolic | `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh` |
| ✅ | Special functions | `erf`, `erfc`, `lgamma`, `tgamma` |
| ✅ | Classification / comparison | `fpclassify`, `isfinite`, `isinf`, `isnan`, `isnormal`, `isunordered`, `isgreater`, `isgreaterequal`, `isless`, `islessequal`, `islessgreater`, `iszero` |
| ✅ | Scaling / layout | `ldexp`, `scalbn`, `scalbln`, `frexp`, `modf`, `nextafter`, `nexttoward` |

For an example of the library-wide constexpr capabilities, see [`example_consteval_library_sweep.cpp`](examples/example_consteval_library_sweep.cpp).

## IO and Literals

[`fltx/io.h`](include/fltx/io.h) provides constexpr-capable parsing, formatting, stream output, string conversion, and the `_dd` / `_qd` literals:

### Literals example:

```cpp
#include <fltx/io.h>
using namespace bl;
using namespace bl::literals;

constexpr f128 pi_128 = 3.14159265358979323846264338327950288_dd;
constexpr f256 pi_256 = 3.1415926535897932384626433832795028841971693993751058209749445923078164_qd;
```

### Serialize example:

```cpp
// serialize
std::string    s1 = bl::to_string(pi_256);            // string with default precision
std::string    s2 = bl::to_string(pi_256, 16, true);  // string with fixed precision
constexpr auto s3 = bl::to_static_string(pi_256, 10); // static_string with fixed precision
```

### Deserialize example:

```cpp
// deserialize (fallback)
constexpr f256 value1 = bl::parse<f256>("__3.1415", 0_qd); // invalid input, use fallback value: 0.0

// deserialize (try/catch)
f256 value2;
try
{
    value2 = bl::parse<f256>("__3.1415");
}
catch (const std::exception& e)
{
    std::cout << e.what() << "\n"; // exception: "bl::parse invalid input"
}

// deserialize (error in result.ec)
constexpr auto result = bl::try_parse<f256>("__3.1415");
if (result)
    std::cout << "success: " << result.value;
else
    std::cout << "invalid input";
```

Use:
- `bl::to_static_string` for compile-time fixed-capacity string output.
- `bl::to_string` when you want a `std::string`.
- `bl::parse<T>` for strict whole-string parsing that returns the parsed value.
- `bl::parse<T>(text, fallback)` when a fallback value is enough.
- `bl::try_parse<T>` when you need error and consumed-character details.

String input is intentionally routed through the parsing APIs. Constructors for `f128` and `f256` are for numeric and storage-form values, not direct text parsing.

Stream output supports `std::setprecision`, `std::fixed`, `std::scientific`, `std::showpoint`, `std::showpos`, and `std::uppercase`.

### Buffer-oriented serializing/deserializing example:

```cpp
#include <fltx/io.h>

// serialize
char buffer[128]{};
auto written = bl::to_chars(buffer, buffer + sizeof(buffer), pi_256, std::chars_format::fixed, 32);
if (written.ec == std::errc{})
    std::cout << std::string_view{ buffer, static_cast<std::size_t>(written.ptr - buffer) };

// deserialize
std::string_view text = "3.1415";
f256 parsed{};
auto read = bl::from_chars(text.data(), text.data() + text.size(), parsed);
if (read.ec == std::errc{} && read.ptr == text.data() + text.size())
    std::cout << "success: " << parsed;
```

For buffer-oriented code, [`fltx/charconv.h`](include/fltx/charconv.h) provides:

- `bl::to_chars` writes into buffer and reports the end pointer plus `std::errc`.
- `bl::from_chars` parses the leading numeric token and reports where parsing stopped.

### std::format example:

Include [`fltx/format.h`](include/fltx/format.h) when you want `std::format` support for `f128`/`f256`.

The formatter supports common numeric presentation options such as precision, fixed/scientific/general notation, sign, width, alignment, fill, alternate form, and uppercase output.

```cpp
#include <fltx/format.h>

std::string fixed      = std::format("{:.32f}", pi_256);     // "3.14159265358979323846264338327950"
std::string scientific = std::format("{:+.16e}", pi_256);    // "+3.1415926535897932e+00"
std::string padded     = std::format("{:>25.20g}", pi_256);  // "    3.1415926535897932385"
std::string hex        = std::format("{:.8a}", pi_256);      // "0x1.921fb544p+1"
std::string hex_alt    = std::format("{:+#.12A}", pi_256);   // "+0X1.921FB54442D2P+1"
```

## Template Dispatch

[`fltx/dispatch.h`](include/fltx/dispatch.h) includes a small runtime-to-template dispatch layer.

This lets runtime values such as [`FloatType::F128`](include/fltx/traits.h) or [`FloatType::F256`](include/fltx/traits.h) select a compile-time type, so the called function still compiles as a normal template specialization.

```cpp
#include <iostream>
#include <fltx.h>

using namespace bl;

template<class T>
void run_kernel(int width, int height)
{
    T x = T{ width } / T{ height };
    T y = bl::sqrt(x) + bl::sin(x);

    std::cout << y << '\n';
}

int main()
{
    FloatType precision = FloatType::F256;

    bl_table_invoke(
        bl_dispatch_table(run_kernel, 1920, 1080),
        bl_enum_type(precision) // selects compile-time type f256 from runtime FloatType::F256 value
    );
}
```

<details>
<summary>Mapping a custom enum to a compile-time type</summary>

[`fltx/template_dispatch.h`](include/fltx/template_dispatch.h) is the lower-level dispatch utility used by [`fltx/dispatch.h`](include/fltx/dispatch.h). It can also be used directly when you want your own runtime enum to select one of several compile-time types:

```cpp
#include <fltx/template_dispatch.h>

enum struct Backend { Cpu, Gpu, COUNT };

struct CpuBackend {};
struct GpuBackend {};

bl_map_enum_to_type(Backend::Cpu, CpuBackend);
bl_map_enum_to_type(Backend::Gpu, GpuBackend);

template<class BackendT, bool Debug>
void run()
{
    if constexpr (Debug)
    {
        // Debug-only path.
    }
}

int main()
{
    Backend backend = Backend::Gpu;
    bool debug = true;

    bl_table_invoke(
        bl_dispatch_table(run),
        bl_enum_type(backend),
        debug
    );
}
```

</details>

## Building fltx

These steps are for contributors who want to build the `fltx` repository itself, including tests and benchmarks. They install the repo-local vcpkg dependencies used by the test suite, such as Catch2, Boost.Multiprecision, GMP, and MPFR.

If you only want to use `fltx` from your own project, you do not need this full setup. Use the vcpkg or CMake installation instructions above instead.

<details>
<summary>Windows</summary>

Install Visual Studio with the C++ desktop workload, then use a Developer PowerShell or Developer Command Prompt:

```powershell
git clone --recurse-submodules https://github.com/willmh93/fltx.git
cd fltx

.\vcpkg\bootstrap-vcpkg.bat

cmake --preset vs2026
cmake --build build\vs2026 --config Release
```

The `vs2026` preset uses Visual Studio/MSBuild with the MSVC compiler. If your Visual Studio version differs, create or select the matching CMake preset before configuring.

</details>

<details>
<summary>macOS</summary>

This is the fresh Apple Silicon macOS setup used for contributor builds on macOS Sequoia:

```bash
# 1. Install Homebrew, if missing
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. Add Homebrew to zsh PATH
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"

# 3. Install tools needed by CMake and vcpkg ports
brew install cmake pkg-config autoconf autoconf-archive automake libtool m4

# 4. Clone fltx with submodules
cd ~/Documents
git clone --recurse-submodules https://github.com/willmh93/fltx.git
cd fltx

# 5. Bootstrap repo-local vcpkg
./vcpkg/bootstrap-vcpkg.sh

# 6. Configure; this installs vcpkg dependencies like Catch2, GMP, and MPFR
cmake --preset macos-release

# 7. Build
cmake --build build/macos-release --parallel
```

For an already-cloned repo:

```bash
cd ~/Documents/fltx
git pull
git submodule update --init --recursive
./vcpkg/bootstrap-vcpkg.sh
cmake --preset macos-release
cmake --build build/macos-release --parallel
```

</details>

<details>
<summary>Linux</summary>

The Ubuntu/Debian flow is:

```bash
sudo apt update
sudo apt install -y git build-essential cmake ninja-build pkg-config autoconf autoconf-archive automake libtool m4

git clone --recurse-submodules https://github.com/willmh93/fltx.git
cd fltx

./vcpkg/bootstrap-vcpkg.sh

cmake --preset native-release
cmake --build build/native-release --parallel
```

Other distributions should use equivalent packages for a C++23 compiler, CMake, Ninja, pkg-config, and the autotools used by the GMP/MPFR vcpkg ports.

</details>

### Running Tests

Test executables are registered with CTest. After building, you can run the registered test cases from the command line:

```bash
ctest --preset native-release
```

Use the matching preset for other configured build trees, such as `macos-release`, `mingw-release`, or `wasm32-release`.

For a multi-config Visual Studio build, include the configuration:

```powershell
ctest --preset vs2026-release
```


```powershell
.\build\vs2026\tests\Release\metrics_tests.exe "[precision],[domain],[bench]"
```

## Benchmarks / Metrics

Tested on:
- **Windows:** AMD Ryzen 9 5950X, Memory 32 GB LPDDR5
- **Linux:** AMD Ryzen 9 5950X, Memory 32 GB LPDDR5
- **MacOS:** Apple M2 Pro, Memory 16 GB LPDDR5

Results:

### **bl::f128** vs reference libraries

<details>
<summary>windows_x64_MSVC</summary>
<img src="validation/metrics/generated/overview/windows_x86_64_MSVC_f128_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

<details>
<summary>windows_x64_MinGW</summary>
<img src="validation/metrics/generated/overview/windows_x86_64_MinGW_f128_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

<details>
<summary>windows_x64_ClangCL</summary>
<img src="validation/metrics/generated/overview/windows_x86_64_ClangCL_f128_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

<details>
<summary>WebAssembly_wasm32_Emscripten</summary>
<img src="validation/metrics/generated/overview/wasm32_wasm32_Emscripten_f128_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

### **bl::f256** vs reference libraries

<details>
<summary>windows_x64_MSVC</summary>
<img src="validation/metrics/generated/overview/windows_x86_64_MSVC_f256_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

<details>
<summary>windows_x64_MinGW</summary>
<img src="validation/metrics/generated/overview/windows_x86_64_MinGW_f256_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

<details>
<summary>windows_x64_ClangCL</summary>
<img src="validation/metrics/generated/overview/windows_x86_64_ClangCL_f256_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

<details>
<summary>WebAssembly_wasm32_Emscripten</summary>
<img src="validation/metrics/generated/overview/wasm32_wasm32_Emscripten_f256_overview_compact.svg" alt="fltx metrics table" width="100%">
</details>

## f256 Expression Fusion

[`bl::f256`](include/fltx/f256.h) uses an internal expression node system which recognises common arithmetic shapes, including product sums, dot-product-plus-bias expressions, and scaled linear combinations.

For example:

```cpp
f256 r = a * b + c * d + e;
```

is kept as a small compile-time expression and lowered to the existing fused product-sum body. This avoids materialising and normalising each intermediate quad-double result before the final value is needed.

The matcher also accepts selected equivalent spellings, such as reordered product/value terms and scaled linear forms. That gives common kernels like affine transforms, small matrix-vector operations, polynomial-style updates, and recurrence relations a faster path without requiring users to call specialised helpers or the implementation to maintain a full symbolic optimiser.

The fused bodies remain explicit and bounded, which keeps compile-time and code-size costs under control.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
