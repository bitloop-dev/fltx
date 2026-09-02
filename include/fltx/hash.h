/**
 * fltx/hash.h - std::hash specializations for fltx types.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_HASH_INCLUDED
#define FLTX_HASH_INCLUDED

#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>

#include "fltx/fdd_type.h"
#include "fltx/fqd_type.h"

namespace bl::detail
{
    [[nodiscard]] BL_FORCE_INLINE constexpr std::uint64_t hash_double_bits(double value) noexcept
    {
        return value == 0.0 ? 0u : std::bit_cast<std::uint64_t>(value);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr std::size_t hash_mix(std::size_t seed, std::uint64_t value) noexcept
    {
        if constexpr (sizeof(std::size_t) >= sizeof(std::uint64_t))
        {
            std::uint64_t x = value + 0x9e3779b97f4a7c15ull + (static_cast<std::uint64_t>(seed) << 6) + (static_cast<std::uint64_t>(seed) >> 2);
            x ^= x >> 30;
            x *= 0xbf58476d1ce4e5b9ull;
            x ^= x >> 27;
            x *= 0x94d049bb133111ebull;
            x ^= x >> 31;
            return static_cast<std::size_t>(x);
        }
        else
        {
            std::uint32_t x = static_cast<std::uint32_t>(value) ^
                              static_cast<std::uint32_t>(value >> 32) ^
                              static_cast<std::uint32_t>(seed);
            x ^= x >> 16;
            x *= 0x7feb352du;
            x ^= x >> 15;
            x *= 0x846ca68bu;
            x ^= x >> 16;
            return static_cast<std::size_t>(x);
        }
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr std::size_t hash_seed(std::uint64_t seed64, std::uint32_t seed32) noexcept
    {
        if constexpr (sizeof(std::size_t) >= sizeof(std::uint64_t))
            return static_cast<std::size_t>(seed64);
        else
            return static_cast<std::size_t>(seed32);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr std::size_t hash_dd(const fdd_s& value) noexcept
    {
        std::size_t seed = hash_seed(0x4ddc2d0f5b0d3911ull, 0x5b0d3911u);
        seed = hash_mix(seed, hash_double_bits(value.hi));
        seed = hash_mix(seed, hash_double_bits(value.lo));
        return seed;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr std::size_t hash_qd(const fqd_s& value) noexcept
    {
        std::size_t seed = hash_seed(0x94d049bb133111ebull, 0x133111ebu);
        seed = hash_mix(seed, hash_double_bits(value.x0));
        seed = hash_mix(seed, hash_double_bits(value.x1));
        seed = hash_mix(seed, hash_double_bits(value.x2));
        seed = hash_mix(seed, hash_double_bits(value.x3));
        return seed;
    }

} // namespace bl::detail

template<>
struct std::hash<bl::fdd_s>
{
    [[nodiscard]] std::size_t operator()(const bl::fdd_s& value) const noexcept
    {
        return bl::detail::hash_dd(value);
    }
};

template<>
struct std::hash<bl::fdd>
{
    [[nodiscard]] std::size_t operator()(const bl::fdd& value) const noexcept
    {
        return bl::detail::hash_dd(static_cast<const bl::fdd_s&>(value));
    }
};

template<>
struct std::hash<bl::fqd_s>
{
    [[nodiscard]] std::size_t operator()(const bl::fqd_s& value) const noexcept
    {
        return bl::detail::hash_qd(value);
    }
};

template<>
struct std::hash<bl::fqd>
{
    [[nodiscard]] std::size_t operator()(const bl::fqd& value) const noexcept
    {
        return bl::detail::hash_qd(static_cast<const bl::fqd_s&>(value));
    }
};

#endif
