#pragma once

// TLFloat 1.17.1 uses standard type traits without including their header.
#include <type_traits>

// Keep the C++ header first: it establishes TLFloat's bigint definitions
// before the C compatibility header.
// clang-format off
#include <tlfloat/tlfloat.hpp>
#include <tlfloat/tlfloat.h>
// clang-format on
