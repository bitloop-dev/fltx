/**
 * fltx/dispatch.h - FloatType dispatch helpers for f32, f64, fdd, and fqd.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DISPATCH_INCLUDED
#define FLTX_DISPATCH_INCLUDED
#include "fltx/core.h"
#include "fltx/traits.h"
#include "fltx/util/template_dispatch.h"

bl_map_enum_to_type(bl::FloatType::F32, bl::f32);
bl_map_enum_to_type(bl::FloatType::F64, bl::f64);
bl_map_enum_to_type(bl::FloatType::FDD, bl::fdd);
bl_map_enum_to_type(bl::FloatType::FQD, bl::fqd);

#endif
