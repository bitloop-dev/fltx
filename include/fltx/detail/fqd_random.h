/**
 * fltx/detail/fqd_random.h - qd hooks for constexpr random facilities.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_FQD_RANDOM_INCLUDED
#define FLTX_DETAIL_FQD_RANDOM_INCLUDED
#include <cstdint>
#include <limits>

#include "fltx/fqd.h"
#include "fltx/fqd_limits.h"

namespace bl::detail::random
{
    template<>
    struct real_traits<fqd_s>
    {
        static constexpr bool enabled = true;
        static constexpr int digits = std::numeric_limits<fqd_s>::digits;

        [[nodiscard]] BL_FORCE_INLINE static constexpr fqd_s zero() noexcept { return fqd_s{ 0.0 }; }
        [[nodiscard]] BL_FORCE_INLINE static constexpr fqd_s one() noexcept { return fqd_s{ 1.0 }; }

        template<class UInt>
        [[nodiscard]] BL_FORCE_INLINE static constexpr fqd_s from_uint(UInt value) noexcept
        {
            fqd_s out{};
            out = static_cast<std::uint64_t>(value);
            return out;
        }
    };

    template<>
    struct real_traits<fqd>
    {
        static constexpr bool enabled = true;
        static constexpr int digits = std::numeric_limits<fqd>::digits;

        [[nodiscard]] BL_FORCE_INLINE static constexpr fqd zero() noexcept { return fqd{ 0.0 }; }
        [[nodiscard]] BL_FORCE_INLINE static constexpr fqd one() noexcept { return fqd{ 1.0 }; }

        template<class UInt>
        [[nodiscard]] BL_FORCE_INLINE static constexpr fqd from_uint(UInt value) noexcept
        {
            return fqd{ static_cast<std::uint64_t>(value) };
        }
    };
}

#endif
