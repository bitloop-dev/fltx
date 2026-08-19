/**
 * fltx/detail/f256_simd_config.h - f256 SIMD configuration defaults.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_F256_SIMD_CONFIG_INCLUDED
#define FLTX_DETAIL_F256_SIMD_CONFIG_INCLUDED

#include "fltx/detail/simd.h"

#if !defined(FLTX_F256_ENABLE_SIMD)
#  if defined(FLTX_F256_ENABLE_TRIG_SIMD)
#    define FLTX_F256_ENABLE_SIMD FLTX_F256_ENABLE_TRIG_SIMD
#  elif FLTX_HAS_SSE2 || FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD
#    define FLTX_F256_ENABLE_SIMD 1
#  else
#    define FLTX_F256_ENABLE_SIMD 0
#  endif
#endif

#if !defined(FLTX_F256_ENABLE_TRIG_SIMD)
#  define FLTX_F256_ENABLE_TRIG_SIMD FLTX_F256_ENABLE_SIMD
#endif

#if FLTX_F256_ENABLE_SIMD && !(FLTX_HAS_SSE2 || FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
#  error "FLTX_F256_ENABLE_SIMD requires SSE2, AArch64 NEON, or WebAssembly SIMD128 support."
#endif

#endif
