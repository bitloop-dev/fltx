/**
 * fltx/detail/trig_reduce_consts.h - Shared fixed-point trig reduction constants.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_TRIG_REDUCE_CONSTS_INCLUDED
#define FLTX_DETAIL_TRIG_REDUCE_CONSTS_INCLUDED
#include <cstdint>

namespace bl::detail::trig_reduce
{
    inline constexpr std::uint32_t two_over_pi_fixed_words[] = {
        0xcaf27f1du, 0x9f3a1f35u, 0x6b1e5ef8u, 0xc33d26efu,
        0x98327dbbu, 0x32c2de4fu, 0x3f7e33e8u, 0xa5ff0705u,
        0x5719053eu, 0xddaf44d1u, 0x8b961ca6u, 0x8359c476u,
        0xdce8092au, 0x19c367cdu, 0x8c6b47c4u, 0x60e27bc0u,
        0xca73a8c9u, 0x06061556u, 0x4d732731u, 0x8dffd880u,
        0x14a06840u, 0x6599855fu, 0x5ee61b08u, 0xa9e39161u,
        0x9af4361du, 0xf0cfbc20u, 0xfc7b6babu, 0x56033046u,
        0x1f8d5d08u, 0x6bfb5fb1u, 0x8a5292eau, 0x3d0739f7u,
        0xebe5f17bu, 0x7527bac7u, 0x9e5fea2du, 0x4f463f66u,
        0x27cb09b7u, 0x6d367ecfu, 0x5a0a6d1fu, 0xef2f118bu,
        0xde05980fu, 0x1ff897ffu, 0xbdf9283bu, 0x9c845f8bu,
        0x835339f4u, 0x3991d639u, 0xb45f7e41u, 0xe99c7026u,
        0x2ebb4484u, 0xe88235f5u, 0xb129a73eu, 0xfe1deb1cu,
        0x09d1921cu, 0x06492eeau, 0x424dd2e0u, 0xb7246e3au,
        0xdebbc561u, 0xfe5163abu, 0x3c439041u, 0xdb629599u,
        0xf534ddc0u, 0xfc2757d1u, 0x4e441529u, 0xa2f9836eu
    };

    inline constexpr int two_over_pi_fixed_bits = 2048;
}

#endif // FLTX_DETAIL_TRIG_REDUCE_CONSTS_INCLUDED
