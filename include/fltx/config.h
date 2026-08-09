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

#ifndef BL_CXX_LANGUAGE_VERSION
  #if defined(_MSVC_LANG) && (!defined(__cplusplus) || (_MSVC_LANG > __cplusplus))
  #define BL_CXX_LANGUAGE_VERSION _MSVC_LANG
  #else
  #define BL_CXX_LANGUAGE_VERSION __cplusplus
  #endif
#endif

#if BL_CXX_LANGUAGE_VERSION < 202002L
#error fltx requires C++20 or newer.
#endif

#ifndef BL_HAS_IF_CONSTEVAL
  #if defined(__INTELLISENSE__) && defined(_MSC_VER) && !defined(__clang__) && \
      (BL_CXX_LANGUAGE_VERSION <= 202002L)
    // IntelliSense does not consistently parse MSVC's C++20 if-consteval
    // extension. Use the portable spelling for editor analysis only; the
    // compiler never defines __INTELLISENSE__ and retains if consteval.
    #define BL_HAS_IF_CONSTEVAL 0
  #elif defined(__cpp_if_consteval) && (__cpp_if_consteval >= 202106L)
    #define BL_HAS_IF_CONSTEVAL 1
  #elif defined(_MSC_VER) && !defined(__clang__) && \
        defined(__cpp_consteval) && (_MSC_VER >= 1936)
    // MSVC accepts C++23's if consteval in C++20 mode as extension C5282.
    // Using it avoids a severe optimizer/code-size regression in large
    // constexpr/runtime dispatch graphs.
    #define BL_HAS_IF_CONSTEVAL 1
    #define BL_IF_CONSTEVAL_MSVC_CXX20_EXTENSION 1
  #elif defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 12)
    // GCC's standard C++20 branch optimizes well at -O2, but at -O0 it can
    // retain and force-inline the constexpr graph into runtime code. GCC 12+
    // accepts if consteval in C++20 mode and discards that graph correctly.
    #define BL_HAS_IF_CONSTEVAL 1
    #define BL_IF_CONSTEVAL_GCC_CXX20_EXTENSION 1
  #else
    #define BL_HAS_IF_CONSTEVAL 0
  #endif
#endif

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTELLISENSE__) && \
    (BL_CXX_LANGUAGE_VERSION <= 202002L) && !BL_HAS_IF_CONSTEVAL
#error fltx C++20 support requires MSVC 19.36 or newer; upgrade MSVC or compile as C++23.
#endif

#if defined(__GNUC__) && !defined(__clang__) && \
    (BL_CXX_LANGUAGE_VERSION <= 202002L) && !BL_HAS_IF_CONSTEVAL
#error fltx C++20 support requires GCC 12 or newer; upgrade GCC or compile as C++23.
#endif

#if !BL_HAS_IF_CONSTEVAL
#include <type_traits>
#endif

#if defined(BL_IF_CONSTEVAL_MSVC_CXX20_EXTENSION)
  #define BL_IF_CONSTEVAL_WARNING_PUSH \
    __pragma(warning(push))                    \
    __pragma(warning(disable: 5282))
  #define BL_IF_CONSTEVAL_WARNING_POP __pragma(warning(pop))
#elif defined(BL_IF_CONSTEVAL_GCC_CXX20_EXTENSION)
  #define BL_IF_CONSTEVAL_WARNING_PUSH              \
    _Pragma("GCC diagnostic push")                         \
    _Pragma("GCC diagnostic ignored \"-Wc++23-extensions\"")
  #define BL_IF_CONSTEVAL_WARNING_POP _Pragma("GCC diagnostic pop")
#else
  #define BL_IF_CONSTEVAL_WARNING_PUSH
  #define BL_IF_CONSTEVAL_WARNING_POP
#endif

#if defined(_MSC_VER) && !defined(__clang__)
  #define BL_DEPRECATED_DECLARATIONS_WARNING_PUSH \
    __pragma(warning(push))                               \
    __pragma(warning(disable: 4996))
  #define BL_DEPRECATED_DECLARATIONS_WARNING_POP __pragma(warning(pop))
#elif defined(__clang__) || defined(__GNUC__)
  #define BL_DEPRECATED_DECLARATIONS_WARNING_PUSH       \
    _Pragma("GCC diagnostic push")                            \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
  #define BL_DEPRECATED_DECLARATIONS_WARNING_POP _Pragma("GCC diagnostic pop")
#else
  #define BL_DEPRECATED_DECLARATIONS_WARNING_PUSH
  #define BL_DEPRECATED_DECLARATIONS_WARNING_POP
#endif

#if BL_HAS_IF_CONSTEVAL
  #define BL_IF_CONSTEVAL if consteval
#else
  #define BL_IF_CONSTEVAL if (std::is_constant_evaluated())
#endif

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
#define BL_CONSTEXPR_DEBUG_ASSERT(cond, msg)                         \
    do                                                              \
    {                                                               \
        BL_IF_CONSTEVAL_WARNING_PUSH                         \
        BL_IF_CONSTEVAL                                      \
        {                                                           \
            if (!(cond))                                            \
                throw msg;                                          \
        }                                                           \
        else                                                        \
        {                                                           \
            if (!(cond))                                            \
                __debugbreak();                                     \
        }                                                           \
        BL_IF_CONSTEVAL_WARNING_POP                          \
    } while (false)
#elif !defined(NDEBUG) && (defined(__clang__) || defined(__GNUC__))
#define BL_CONSTEXPR_DEBUG_ASSERT(cond, msg)                         \
    do                                                              \
    {                                                               \
        BL_IF_CONSTEVAL_WARNING_PUSH                         \
        BL_IF_CONSTEVAL                                      \
        {                                                           \
            if (!(cond))                                            \
                throw msg;                                          \
        }                                                           \
        else                                                        \
        {                                                           \
            if (!(cond))                                            \
                __builtin_trap();                                   \
        }                                                           \
        BL_IF_CONSTEVAL_WARNING_POP                          \
    } while (false)
#else
#define BL_CONSTEXPR_DEBUG_ASSERT(cond, msg)                         \
    do                                                              \
    {                                                               \
        BL_IF_CONSTEVAL_WARNING_PUSH                         \
        BL_IF_CONSTEVAL                                      \
        {                                                           \
            if (!(cond))                                            \
                throw msg;                                          \
        }                                                           \
        BL_IF_CONSTEVAL_WARNING_POP                          \
    } while (false)
#endif

#ifndef FLTX_FAST_MATH
  #if defined(__FAST_MATH__)
  #define FLTX_FAST_MATH
  #elif defined(_MSC_VER) && defined(_M_FP_FAST)
  #define FLTX_FAST_MATH
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

#ifndef BL_VECTORCALL
  // ABI-affecting optimization: apply only to individually benchmarked runtime
  // entry points. Callers and the compiled library must see the same declaration.
  #if defined(_MSC_VER)
  #define BL_VECTORCALL __vectorcall
  #else
  #define BL_VECTORCALL
  #endif
#endif

// MSVC-only noinline pressure valve for constexpr helpers that otherwise cause excessive
// optimization work; GCC/Clang retain control of their own inlining decisions.

#if defined(_MSC_VER)
  #define BL_MSVC_NOINLINE BL_NO_INLINE
#else
  #define BL_MSVC_NOINLINE
#endif

// Public-header feature policy. CMake may define these macros on fltx::fltx,
// but they are intentionally just policy switches, not CPU or FP compiler flags.

#if !defined(FLTX_HEADER_SIMD_OFF)
#define FLTX_HEADER_SIMD_OFF 0
#endif

#if !defined(FLTX_HEADER_FMA_AUTO)
#define FLTX_HEADER_FMA_AUTO 0
#endif

#if !defined(FLTX_HEADER_FMA_OFF)
#define FLTX_HEADER_FMA_OFF 0
#endif

#if !defined(FLTX_HEADER_FMA_ASSUME)
#define FLTX_HEADER_FMA_ASSUME 0
#endif

#if !defined(FLTX_HAS_COMPILED_X86_FMA_BACKEND)
#define FLTX_HAS_COMPILED_X86_FMA_BACKEND 0
#endif

#if (FLTX_HEADER_FMA_AUTO + FLTX_HEADER_FMA_OFF + FLTX_HEADER_FMA_ASSUME) == 0
#undef FLTX_HEADER_FMA_AUTO
#define FLTX_HEADER_FMA_AUTO 1
#endif

#if (FLTX_HEADER_FMA_AUTO + FLTX_HEADER_FMA_OFF + FLTX_HEADER_FMA_ASSUME) > 1
#error Select only one fltx header FMA policy: AUTO, OFF, or ASSUME.
#endif

#if !defined(FLTX_X86_TARGET)
  #if !defined(__EMSCRIPTEN__) && \
      (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
  #define FLTX_X86_TARGET 1
  #else
  #define FLTX_X86_TARGET 0
  #endif
#endif

#if !defined(FLTX_ARM64_TARGET)
  #if !defined(__EMSCRIPTEN__) && (defined(__aarch64__) || defined(_M_ARM64))
  #define FLTX_ARM64_TARGET 1
  #else
  #define FLTX_ARM64_TARGET 0
  #endif
#endif

#if !defined(FLTX_TU_HAS_X86_FMA)
  #if FLTX_X86_TARGET && (defined(__FMA__) || \
      (defined(_MSC_VER) && (defined(__AVX2__) || defined(__AVX512F__) || defined(_M_AVX2))))
  #define FLTX_TU_HAS_X86_FMA 1
  #else
  #define FLTX_TU_HAS_X86_FMA 0
  #endif
#endif

#if FLTX_HEADER_FMA_ASSUME && !FLTX_X86_TARGET && !FLTX_ARM64_TARGET
#error FLTX_HEADER_FMA_ASSUME requires a target with baseline or explicitly enabled hardware FMA.
#endif

#if FLTX_HEADER_FMA_ASSUME && FLTX_X86_TARGET && !defined(_MSC_VER) && !FLTX_TU_HAS_X86_FMA
#error FLTX_HEADER_FMA_ASSUME requires GNU/Clang translation units to be compiled with FMA enabled, for example -mfma.
#endif

#if !defined(FLTX_HAS_X86_FMA)
  #if !FLTX_HEADER_FMA_OFF && FLTX_X86_TARGET && (FLTX_TU_HAS_X86_FMA || FLTX_HEADER_FMA_ASSUME)
  #define FLTX_HAS_X86_FMA 1
  #else
  #define FLTX_HAS_X86_FMA 0
  #endif
#endif

#if !defined(FLTX_DETAIL_MSVC_GUARDED_X86_FMA)
  #if FLTX_HEADER_FMA_AUTO && FLTX_X86_TARGET && defined(_MSC_VER) && !FLTX_HAS_X86_FMA
  #define FLTX_DETAIL_MSVC_GUARDED_X86_FMA 1
  #else
  #define FLTX_DETAIL_MSVC_GUARDED_X86_FMA 0
  #endif
#endif

#if !defined(FLTX_DETAIL_X86_FMA_RUNTIME_CHECK)
  #if FLTX_DETAIL_MSVC_GUARDED_X86_FMA || \
      (FLTX_HEADER_FMA_AUTO && FLTX_X86_TARGET && \
       FLTX_HAS_COMPILED_X86_FMA_BACKEND && !FLTX_TU_HAS_X86_FMA)
  #define FLTX_DETAIL_X86_FMA_RUNTIME_CHECK 1
  #else
  #define FLTX_DETAIL_X86_FMA_RUNTIME_CHECK 0
  #endif
#endif

#if !defined(FLTX_DETAIL_USE_SCALAR_X86_FMA)
  #if FLTX_HAS_X86_FMA
  #define FLTX_DETAIL_USE_SCALAR_X86_FMA 1
  #else
  #define FLTX_DETAIL_USE_SCALAR_X86_FMA 0
  #endif
#endif

#if !defined(FLTX_DETAIL_USE_BASELINE_ARM64_FMA)
  #if !FLTX_HEADER_FMA_OFF && FLTX_ARM64_TARGET
  #define FLTX_DETAIL_USE_BASELINE_ARM64_FMA 1
  #else
  #define FLTX_DETAIL_USE_BASELINE_ARM64_FMA 0
  #endif
#endif

#if !defined(FLTX_DETAIL_HAS_RUNTIME_FMA_PATH)
  #if FLTX_DETAIL_USE_SCALAR_X86_FMA || FLTX_DETAIL_MSVC_GUARDED_X86_FMA || \
      FLTX_DETAIL_USE_BASELINE_ARM64_FMA
  #define FLTX_DETAIL_HAS_RUNTIME_FMA_PATH 1
  #else
  #define FLTX_DETAIL_HAS_RUNTIME_FMA_PATH 0
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
  // WebAssembly Clang does not support the float_control push/pop stack.
  // Restore the incoming mode explicitly after each protected EFT block.
  #ifndef BL_PUSH_PRECISE
  #define BL_PUSH_PRECISE _Pragma("clang fp reassociate(off)") \
                          _Pragma("clang fp contract(off)")
  #endif
  #ifndef BL_POP_PRECISE
    #if defined(__FAST_MATH__)
    #define BL_POP_PRECISE _Pragma("clang fp reassociate(on)") \
                           _Pragma("clang fp contract(fast)")
    #elif defined(FLTX_DETAIL_FP_CONTRACT_FAST)
    #define BL_POP_PRECISE _Pragma("clang fp reassociate(off)") \
                           _Pragma("clang fp contract(fast)")
    #else
    #define BL_POP_PRECISE _Pragma("clang fp reassociate(off)") \
                           _Pragma("clang fp contract(off)")
    #endif
  #endif
#elif defined(__clang__)
  // Clang supports the same file-scope float_control stack as MSVC. Keep
  // contraction disabled inside EFT declarations and restore the exact
  // incoming translation-unit mode afterwards.
  #ifndef BL_PUSH_PRECISE
  #define BL_PUSH_PRECISE _Pragma("float_control(precise, on, push)") \
                          _Pragma("clang fp contract(off)")
  #endif
  #ifndef BL_POP_PRECISE
  #define BL_POP_PRECISE _Pragma("float_control(pop)")
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

namespace bl
{
    namespace detail
    {
        [[nodiscard]] BL_FORCE_INLINE constexpr bool is_constant_evaluated() noexcept
        {
            // Fixed simulated-consteval mode runs every constexpr-capable branch
            // at runtime for accuracy testing and performance profiling.
            BL_IF_CONSTEVAL_WARNING_PUSH
            BL_IF_CONSTEVAL
            {
                return true;
            }
            BL_IF_CONSTEVAL_WARNING_POP

            #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
            return true;
            #else
            return false;
            #endif
        }

    } // namespace detail

} // namespace bl


// Route public constexpr-capable APIs to the constexpr implementation during
// constant evaluation, and to the optimized runtime implementation otherwise.
// Fixed simulation can force the constexpr path at runtime so its accuracy and
// performance can be measured over the complete runtime-generated corpus.

#ifndef BL_CONSTEXPR_RUNTIME_DISPATCH
  #if defined(FLTX_SIMULATE_FIXED_CONSTEVAL_MODE)
    #define BL_CONSTEXPR_RUNTIME_DISPATCH(CONSTEVAL_EXPR, RUNTIME_EXPR) \
        do                                                              \
        {                                                               \
            BL_IF_CONSTEVAL_WARNING_PUSH                         \
            BL_IF_CONSTEVAL                                      \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
            else                                                        \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
            BL_IF_CONSTEVAL_WARNING_POP                          \
        } while (false)
  #else
    #define BL_CONSTEXPR_RUNTIME_DISPATCH(CONSTEVAL_EXPR, RUNTIME_EXPR) \
        do                                                              \
        {                                                               \
            BL_IF_CONSTEVAL_WARNING_PUSH                         \
            BL_IF_CONSTEVAL                                      \
            {                                                           \
                return (CONSTEVAL_EXPR);                                \
            }                                                           \
            else                                                        \
            {                                                           \
                return (RUNTIME_EXPR);                                  \
            }                                                           \
            BL_IF_CONSTEVAL_WARNING_POP                          \
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
    static constexpr std::float_denorm_style has_denorm = base::has_denorm;                                  \
    static constexpr bool has_denorm_loss    = base::has_denorm_loss;                                        \
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
