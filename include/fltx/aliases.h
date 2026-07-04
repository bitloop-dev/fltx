/**
 * fltx/aliases.h - Cheap scalar aliases and fltx type forward declarations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_ALIASES_INCLUDED
#define FLTX_ALIASES_INCLUDED
#include <cstdint>

namespace bl
{
    struct f128;
    struct f128_s;
    struct f256;
    struct f256_s;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using f32 = float;
    using f64 = double;

    namespace int_literals
    {
        inline constexpr std::uint8_t operator "" _u8(unsigned long long arg) noexcept
        {
            return static_cast<std::uint8_t>(arg);
        }

        inline constexpr std::int8_t operator "" _i8(unsigned long long arg) noexcept
        {
            return static_cast<std::int8_t>(arg);
        }

        inline constexpr std::uint16_t operator "" _u16(unsigned long long arg) noexcept
        {
            return static_cast<std::uint16_t>(arg);
        }

        inline constexpr std::int16_t operator "" _i16(unsigned long long arg) noexcept
        {
            return static_cast<std::int16_t>(arg);
        }

        inline constexpr std::uint32_t operator "" _u32(unsigned long long arg) noexcept
        {
            return static_cast<std::uint32_t>(arg);
        }

        inline constexpr std::int32_t operator "" _i32(unsigned long long arg) noexcept
        {
            return static_cast<std::int32_t>(arg);
        }

        inline constexpr std::uint64_t operator "" _u64(unsigned long long arg) noexcept
        {
            return static_cast<std::uint64_t>(arg);
        }

        inline constexpr std::int64_t operator "" _i64(unsigned long long arg) noexcept
        {
            return static_cast<std::int64_t>(arg);
        }
    }
} // namespace bl

#endif
