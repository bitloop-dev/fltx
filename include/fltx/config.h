/**
 * fltx/config.h - Core macros, precision controls, and constant-evaluation helpers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_CONFIG_INCLUDED
#define FLTX_CONFIG_INCLUDED
#include <cstdint>
#include <limits>

// Environment checks

static_assert(sizeof(double) == sizeof(std::uint64_t),
    "fltx requires double to be 64 bits.");

static_assert(std::numeric_limits<double>::is_iec559 &&
              std::numeric_limits<double>::radix == 2 &&
              std::numeric_limits<double>::digits == 53 &&
              std::numeric_limits<double>::min_exponent == -1021 &&
              std::numeric_limits<double>::max_exponent == 1024,
    "fltx requires double to use the IEEE 754 binary64 format.");

#if !defined(NDEBUG) && defined(_MSC_VER)
#define BL_CONSTEXPR_DEBUG_ASSERT(cond, msg) \
    do { if consteval { if (!(cond)) throw msg; } else { if (!(cond)) __debugbreak(); } } while (false)
#elif !defined(NDEBUG) && (defined(__clang__) || defined(__GNUC__))
#define BL_CONSTEXPR_DEBUG_ASSERT(cond, msg) \
    do { if consteval { if (!(cond)) throw msg; } else { if (!(cond)) __builtin_trap(); } } while (false)
#else
#define BL_CONSTEXPR_DEBUG_ASSERT(cond, msg) \
    do { if consteval { if (!(cond)) throw msg; } } while (false)
#endif


#ifndef BL_CXX_LANGUAGE_VERSION
  #if defined(_MSVC_LANG) && (!defined(__cplusplus) || (_MSVC_LANG > __cplusplus))
  #define BL_CXX_LANGUAGE_VERSION _MSVC_LANG
  #else
  #define BL_CXX_LANGUAGE_VERSION __cplusplus
  #endif
#endif

#if BL_CXX_LANGUAGE_VERSION <= 202002L
#error fltx requires C++23 or newer.
#endif

#if !defined(__cpp_if_consteval) || (__cpp_if_consteval < 202106L)
#error fltx requires C++23 if consteval support.
#endif

#ifndef BL_FAST_MATH
  #if defined(__FAST_MATH__)
  #define BL_FAST_MATH
  #elif defined(_MSC_VER) && defined(_M_FP_FAST)
  #define BL_FAST_MATH
  #endif
#endif

// Inlining policy:
//
// - Tiny leaf helpers are force-inlined when the call overhead is a poor tradeoff.
//
// - Medium internal helpers may be force-inlined, but only when their main callers
//   are large out-of-line runtime bodies. This lets those .cpp implementations be
//   optimized aggressively once, without copying the whole call graph into every
//   user translation unit.
//
// - Large functions should stay out-of-line. Parsing, formatting, transcendental
//   math, and other heavy implementations are too expensive to inline/optimize at
//   every call site.
//
// - Public wrappers should stay thin: either inline a genuinely tiny operation,
//   or dispatch to an out-of-line implementation. Avoid making public headers
//   pull large optimization graphs into consumer builds.

#ifndef BL_FORCE_INLINE
  #if defined(_MSC_VER)
  #define BL_FORCE_INLINE __forceinline
  #elif defined(__clang__) || defined(__GNUC__)
  #define BL_FORCE_INLINE inline __attribute__((always_inline))
  #else
  #define BL_FORCE_INLINE inline
  #endif
#endif

#ifndef BL_NO_INLINE
  #if defined(_MSC_VER)
  #define BL_NO_INLINE __declspec(noinline)
  #elif defined(__clang__) || defined(__GNUC__)
  #define BL_NO_INLINE __attribute__((noinline))
  #else
  #define BL_NO_INLINE
  #endif
#endif

// MSVC-only noinline pressure valve for constexpr helpers that otherwise cause excessive
// inlining/optimization work; GCC/Clang are left free to inline them.

#if defined(_MSC_VER)
  #define BL_MSVC_NOINLINE BL_NO_INLINE
#else
  #define BL_MSVC_NOINLINE
#endif

// Enable FMA kernels when the target clearly has cheap hardware FMA.
//
// MSVC x86/x64 builds assume FMA by default because MSVC does not consistently
// expose a portable __FMA__-style target macro for consumer translation units.
// Unsupported CPUs/VMs can opt out with FLTX_DISABLE_FMA_AVAILABLE=1; the FMA
// runtime preflight emits a diagnostic before the unchecked instruction path is
// used in normal MSVC startup.

#if !defined(FLTX_ASSUME_X86_FMA_AVAILABLE)
  #if defined(_MSC_VER) && !defined(__EMSCRIPTEN__) && \
      (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
  #define FLTX_ASSUME_X86_FMA_AVAILABLE 1
  #else
  #define FLTX_ASSUME_X86_FMA_AVAILABLE 0
  #endif
#endif

#if !defined(FLTX_DISABLE_FMA_AVAILABLE)
  #ifndef FMA_AVAILABLE
    #ifndef __EMSCRIPTEN__
      #if defined(__FMA__) || defined(__FMA4__)
        #define FMA_AVAILABLE
      #elif FLTX_ASSUME_X86_FMA_AVAILABLE
        #define FMA_AVAILABLE
        #define BL_FLTX_ASSUMED_X86_FMA 1
      #elif defined(_MSC_VER) && (defined(__AVX2__) || defined(__AVX512F__))
        #define FMA_AVAILABLE
      #endif
    #endif
  #endif
#endif

#if !defined(BL_FLTX_HAS_X86_FMA)
  #if !defined(__EMSCRIPTEN__) && \
      (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)) && \
      (defined(__FMA__) || FLTX_ASSUME_X86_FMA_AVAILABLE || \
       (defined(_MSC_VER) && (defined(__AVX2__) || defined(__AVX512F__))))
  #define BL_FLTX_HAS_X86_FMA 1
  #else
  #define BL_FLTX_HAS_X86_FMA 0
  #endif
#endif

#if defined(_MSC_VER)
  #ifndef BL_PUSH_PRECISE
  #define BL_PUSH_PRECISE __pragma(float_control(precise, on, push)) \
                          __pragma(fp_contract(off))
  #endif
  #ifndef BL_POP_PRECISE
  #define BL_POP_PRECISE  __pragma(float_control(pop))
  #endif
#elif defined(__EMSCRIPTEN__)
  #ifndef BL_PUSH_PRECISE
  #define BL_PUSH_PRECISE _Pragma("clang fp reassociate(off)") \
                          _Pragma("clang fp contract(off)")
  #endif
  #ifndef BL_POP_PRECISE
  #define BL_POP_PRECISE  _Pragma("clang fp reassociate(on)")  \
                          _Pragma("clang fp contract(fast)")
  #endif
#elif defined(__clang__)
  // Clang's fp pragmas here do not restore a previous stack state. Keep strict
  // FP semantics after protected blocks; expansion arithmetic relies on it.
  #ifndef BL_PUSH_PRECISE
  #define BL_PUSH_PRECISE _Pragma("clang fp reassociate(off)") \
                          _Pragma("clang fp contract(off)")
  #endif
  #ifndef BL_POP_PRECISE
  #define BL_POP_PRECISE  _Pragma("clang fp reassociate(off)") \
                          _Pragma("clang fp contract(off)")
  #endif
#elif defined(__GNUC__)
  #ifndef BL_PUSH_PRECISE
  #define BL_PUSH_PRECISE _Pragma("GCC push_options")               \
                          _Pragma("GCC optimize(\"no-fast-math\")") \
                          _Pragma("STDC FP_CONTRACT OFF")
  #endif
  #ifndef BL_POP_PRECISE
  #define BL_POP_PRECISE _Pragma("GCC pop_options")
  #endif
#else
  #define BL_PUSH_PRECISE
  #define BL_POP_PRECISE
#endif

// Improves pow/cosh/sinh/lgamma domain scores with minimal overhead (default on)
#if !defined(FLTX_DISABLE_MATH_USES_CHECKED_DEKKER) && !defined(FLTX_MATH_USES_CHECKED_DEKKER)
#define FLTX_MATH_USES_CHECKED_DEKKER
#endif

#if defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE) && defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
#error FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE and FLTX_SIMULATE_FIXED_CONSTEVAL_MODE are separate modes; enable only one.
#endif

#if defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE) || defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
#define FLTX_HAS_SIMULATED_CONSTEVAL_MODE
#endif

namespace bl
{
    #if defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE)
    namespace _fltx_debug
    {
        inline bool simulate_consteval_path = false;

        BL_FORCE_INLINE void set_simulated_consteval_path(bool enabled) noexcept { simulate_consteval_path = enabled; }
        BL_FORCE_INLINE void set_forced_constexpr_path() noexcept { set_simulated_consteval_path(true); }
        BL_FORCE_INLINE void set_forced_runtime_path() noexcept { set_simulated_consteval_path(false); }
    }
    #endif

    namespace detail
    {
        [[nodiscard]] BL_FORCE_INLINE constexpr bool is_constant_evaluated() noexcept
        {
            // In simulated-consteval mode, tests can run ordinary runtime calls
            // through the branches that would be selected during constant
            // evaluation. FLTX_CONSTEXPR_PARITY itself is intentionally not part of
            // this decision; it requests bitwise-compatible results, not forced
            // constexpr-path execution.
            if consteval
            {
                return true;
            }

            #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
            return true;
            #elif defined(FLTX_SIMULATE_TOGGLE_CONSTEVAL_MODE)
            return bl::_fltx_debug::simulate_consteval_path;
            #else
            return false;
            #endif
        }

        [[nodiscard]] BL_FORCE_INLINE constexpr bool use_constexpr_parity() noexcept
        {
            // Result-parity policy only. Callers may use this to decide whether to
            // canonicalize a math result.
            #if defined(FLTX_CONSTEXPR_PARITY)
            return true;
            #else
            return false;
            #endif
        }

        [[nodiscard]] BL_FORCE_INLINE constexpr bool use_constexpr_math() noexcept
        {
            // Select constexpr-safe math algorithms. In normal builds this tracks
            // actual constant evaluation. Toggle simulated mode lets tests select
            // constexpr-safe paths at runtime for parity/domain checks. Fixed
            // simulated mode always selects those paths so benchmarks do not measure
            // the toggle branch. FLTX_CONSTEXPR_PARITY also takes this path so runtime
            // and constant-evaluated results are bitwise comparable.
            #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
            return true;
            #else
            return is_constant_evaluated() || use_constexpr_parity();
            #endif
        }

    } // namespace detail

} // namespace bl


// Route public constexpr-capable APIs to the constexpr implementation during
// constant evaluation, and to the optimized runtime implementation otherwise.
// Test modes can force or toggle the constexpr path at runtime so constexpr-only
// code can be tested/benchmarked against the runtime path.

#ifndef BL_CONSTEXPR_RUNTIME_DISPATCH
  #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
    #define BL_CONSTEXPR_RUNTIME_DISPATCH(CONSTEVAL_EXPR, RUNTIME_EXPR) \
        do                                                              \
        {                                                               \
            if consteval                                                \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
            else                                                        \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
        } while (false)
  #elif !defined(FLTX_CONSTEXPR_PARITY) && !defined(FLTX_HAS_SIMULATED_CONSTEVAL_MODE)
    #define BL_CONSTEXPR_RUNTIME_DISPATCH(CONSTEVAL_EXPR, RUNTIME_EXPR) \
        do                                                              \
        {                                                               \
            if consteval                                                \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
            else                                                        \
            {                                                           \
                return (RUNTIME_EXPR);                                  \
            }                                                           \
        } while (false)
  #else
    #define BL_CONSTEXPR_RUNTIME_DISPATCH(CONSTEVAL_EXPR, RUNTIME_EXPR) \
        do                                                              \
        {                                                               \
            if consteval                                                \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
            else                                                        \
            {                                                           \
                if (bl::detail::use_constexpr_math())                   \
                    return (CONSTEVAL_EXPR);                            \
                return (RUNTIME_EXPR);                                  \
            }                                                           \
        } while (false)
  #endif
#endif


// Convenience macro for f128/f128_s and f256/f256_s for identical std::numeric_limits

#ifndef BL_DEFINE_FLOAT_WRAPPER_NUMERIC_LIMITS
#define BL_DEFINE_FLOAT_WRAPPER_NUMERIC_LIMITS(wrapper_type, storage_type)                                   \
template<>                                                                                                   \
struct std::numeric_limits<wrapper_type>                                                                     \
{                                                                                                            \
    using base = std::numeric_limits<storage_type>;                                                          \
                                                                                                             \
    static constexpr bool is_specialized = base::is_specialized;                                             \
                                                                                                             \
    static constexpr wrapper_type min()           noexcept { return wrapper_type{ base::min() }; }           \
    static constexpr wrapper_type max()           noexcept { return wrapper_type{ base::max() }; }           \
    static constexpr wrapper_type lowest()        noexcept { return wrapper_type{ base::lowest() }; }        \
    static constexpr wrapper_type epsilon()       noexcept { return wrapper_type{ base::epsilon() }; }       \
    static constexpr wrapper_type round_error()   noexcept { return wrapper_type{ base::round_error() }; }   \
    static constexpr wrapper_type infinity()      noexcept { return wrapper_type{ base::infinity() }; }      \
    static constexpr wrapper_type quiet_NaN()     noexcept { return wrapper_type{ base::quiet_NaN() }; }     \
    static constexpr wrapper_type signaling_NaN() noexcept { return wrapper_type{ base::signaling_NaN() }; } \
    static constexpr wrapper_type denorm_min()    noexcept { return wrapper_type{ base::denorm_min() }; }    \
                                                                                                             \
    static constexpr bool has_infinity       = base::has_infinity;                                           \
    static constexpr bool has_quiet_NaN      = base::has_quiet_NaN;                                          \
    static constexpr bool has_signaling_NaN  = base::has_signaling_NaN;                                      \
    static constexpr int  digits             = base::digits;                                                 \
    static constexpr int  digits10           = base::digits10;                                               \
    static constexpr int  max_digits10       = base::max_digits10;                                           \
                                                                                                             \
    static constexpr bool is_signed          = base::is_signed;                                              \
    static constexpr bool is_integer         = base::is_integer;                                             \
    static constexpr bool is_exact           = base::is_exact;                                               \
    static constexpr int  radix              = base::radix;                                                  \
                                                                                                             \
    static constexpr int  min_exponent       = base::min_exponent;                                           \
    static constexpr int  max_exponent       = base::max_exponent;                                           \
    static constexpr int  min_exponent10     = base::min_exponent10;                                         \
    static constexpr int  max_exponent10     = base::max_exponent10;                                         \
                                                                                                             \
    static constexpr bool is_iec559          = base::is_iec559;                                              \
    static constexpr bool is_bounded         = base::is_bounded;                                             \
    static constexpr bool is_modulo          = base::is_modulo;                                              \
    static constexpr bool traps              = base::traps;                                                  \
    static constexpr bool tinyness_before    = base::tinyness_before;                                        \
                                                                                                             \
    static constexpr std::float_round_style round_style = base::round_style;                                 \
};
#endif

#endif // FLTX_CONFIG_INCLUDED
