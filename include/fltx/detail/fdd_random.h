/**
 * fltx/detail/fdd_random.h - dd hooks for constexpr random facilities.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_FDD_RANDOM_INCLUDED
#define FLTX_DETAIL_FDD_RANDOM_INCLUDED
#include <cstdint>
#include <limits>

#include "fltx/fdd.h"
#include "fltx/fdd_limits.h"

namespace bl::detail::random
{
    template<>
    struct real_traits<fdd_s>
    {
        static constexpr bool enabled = true;
        static constexpr int digits = std::numeric_limits<fdd_s>::digits;

        [[nodiscard]] BL_FORCE_INLINE static constexpr fdd_s zero() noexcept { return fdd_s{ 0.0 }; }
        [[nodiscard]] BL_FORCE_INLINE static constexpr fdd_s one() noexcept { return fdd_s{ 1.0 }; }

        template<class UInt>
        [[nodiscard]] BL_FORCE_INLINE static constexpr fdd_s from_uint(UInt value) noexcept
        {
            fdd_s out{};
            out = static_cast<std::uint64_t>(value);
            return out;
        }
    };

    template<>
    struct real_traits<fdd>
    {
        static constexpr bool enabled = true;
        static constexpr int digits = std::numeric_limits<fdd>::digits;

        [[nodiscard]] BL_FORCE_INLINE static constexpr fdd zero() noexcept { return fdd{ 0.0 }; }
        [[nodiscard]] BL_FORCE_INLINE static constexpr fdd one() noexcept { return fdd{ 1.0 }; }

        template<class UInt>
        [[nodiscard]] BL_FORCE_INLINE static constexpr fdd from_uint(UInt value) noexcept
        {
            return fdd{ static_cast<std::uint64_t>(value) };
        }
    };
}

#endif
