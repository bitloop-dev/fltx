/**
 * fltx/traits.h - Compile-time fltx type traits and precision metadata.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_TRAITS_INCLUDED
#define FLTX_TRAITS_INCLUDED
#include <concepts>
#include <string_view>
#include <type_traits>

#include "fltx/aliases.h"

namespace bl
{
    template<class T> concept fltx_f32  = std::same_as<std::remove_cv_t<T>, f32>;
    template<class T> concept fltx_f64  = std::same_as<std::remove_cv_t<T>, f64>;
    template<class T> concept fltx_f128 = std::same_as<std::remove_cv_t<T>, f128_s> ||
                                          std::same_as<std::remove_cv_t<T>, f128>;
    template<class T> concept fltx_f256 = std::same_as<std::remove_cv_t<T>, f256_s> ||
                                          std::same_as<std::remove_cv_t<T>, f256>;

    template<class T> concept fltx_extended_float = fltx_f128<T> || fltx_f256<T>;
    template<class T> concept fltx_float          = fltx_f32<T>  || fltx_f64<T> || fltx_extended_float<T>;

    template<class T> concept fltx_floating_point = std::is_floating_point_v<T> || fltx_extended_float<T>;
    template<class T> concept fltx_arithmetic     = std::is_arithmetic_v<T>     || fltx_extended_float<T>;

    template<class T> inline constexpr bool is_f32_v  = fltx_f32<T>;
    template<class T> inline constexpr bool is_f64_v  = fltx_f64<T>;
    template<class T> inline constexpr bool is_f128_v = fltx_f128<T>;
    template<class T> inline constexpr bool is_f256_v = fltx_f256<T>;

    template<class T> inline constexpr bool is_fltx_extended_float_v = fltx_extended_float<T>;
    template<class T> inline constexpr bool is_fltx_float_v          = fltx_float<T>;

    template<class T> inline constexpr bool is_floating_point_v = fltx_floating_point<T>;
    template<class T> inline constexpr bool is_arithmetic_v     = fltx_arithmetic<T>;
    template<class T> inline constexpr bool is_integral_v       = std::is_integral_v<T>;

    template<class T>
    inline constexpr int fltx_precision_rank_v =
        fltx_f256<std::remove_cvref_t<T>>                 ? 5 :
        fltx_f128<std::remove_cvref_t<T>>                 ? 4 :
        std::same_as<std::remove_cvref_t<T>, long double> ? 3 :
        (std::same_as<std::remove_cvref_t<T>, f64> ||
         std::is_integral_v<std::remove_cvref_t<T>>)      ? 2 :
        std::same_as<std::remove_cvref_t<T>, f32>         ? 1 : 0;

    namespace detail::traits
    {
        template<class T>
        using clean_t = std::remove_cvref_t<T>;

        template<class... Ts>
        [[nodiscard]] consteval int max_precision_rank() noexcept
        {
            int rank = 0;
            ((rank = rank < fltx_precision_rank_v<Ts> ? fltx_precision_rank_v<Ts> : rank), ...);
            return rank;
        }

        template<class... Ts>
        struct common_float_type_impl
        {
            static_assert(sizeof...(Ts) > 0,
                "bl::common_float_type_t requires at least one type.");
            static_assert((fltx_arithmetic<clean_t<Ts>> && ...),
                "bl::common_float_type_t requires arithmetic or fltx extended floating-point types.");
            static_assert(((fltx_precision_rank_v<Ts> != 0) && ...),
                "bl::common_float_type_t does not support one of these arithmetic types.");

            static constexpr int rank = max_precision_rank<Ts...>();
            using type =
                std::conditional_t<rank == 5, f256,
                std::conditional_t<rank == 4, f128,
                std::conditional_t<rank == 3, long double,
                std::conditional_t<rank == 2, f64,
                f32>>>>;
        };

    } // namespace detail::traits

    template<class... Ts>
    struct common_float_type
    {
        using type = typename detail::traits::common_float_type_impl<Ts...>::type;
    };

    template<class... Ts>
    using common_float_type_t = typename common_float_type<Ts...>::type;

    enum struct FloatType : int
    {
        F32,
        F64,
        F128,
        F256,
        COUNT
    };

    [[nodiscard]] constexpr std::string_view to_string(FloatType type) noexcept
    {
        switch (type)
        {
        case FloatType::F32:  return "f32";
        case FloatType::F64:  return "f64";
        case FloatType::F128: return "f128";
        case FloatType::F256: return "f256";
        default:              return "unknown";
        }
    }

} // namespace bl

#endif
