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
    template<class T, class = void>
    struct fltx_expression_traits
    {
        static constexpr bool is_expression = false;
        using value_type = std::remove_cvref_t<T>;
        using storage_type = std::remove_cvref_t<T>;
    };

    template<class T>
    struct fltx_expression_traits<
        T,
        std::void_t<typename std::remove_cvref_t<T>::fltx_expression_value_type>>
    {
        static constexpr bool is_expression = true;
        using value_type = typename std::remove_cvref_t<T>::fltx_expression_value_type;
        using storage_type = typename std::remove_cvref_t<T>::fltx_expression_storage_type;
    };

    template<class T>
    concept fltx_expression = fltx_expression_traits<T>::is_expression;

    template<class T> concept fltx_f32 = std::same_as<std::remove_cv_t<T>, f32>;
    template<class T> concept fltx_f64 = std::same_as<std::remove_cv_t<T>, f64>;
    template<class T> concept fltx_fdd = std::same_as<std::remove_cv_t<T>, fdd_s> ||
                                         std::same_as<std::remove_cv_t<T>, fdd>;
    template<class T> concept fltx_fqd = std::same_as<std::remove_cv_t<T>, fqd_s> ||
                                         std::same_as<std::remove_cv_t<T>, fqd>;

    template<class T> concept fltx_extended_float = fltx_fdd<T> || fltx_fqd<T>;
    template<class T> concept fltx_float          = fltx_f32<T> || fltx_f64<T> || fltx_extended_float<T>;
    template<class T> concept fltx_arithmetic     = std::is_arithmetic_v<T>    || fltx_extended_float<T>;

    // Normalize representations and deferred expressions, stripping cv/ref qualifiers.
    template<class T>
    using value_t = std::conditional_t<fltx_fdd<typename fltx_expression_traits<T>::value_type>, fdd,
                    std::conditional_t<fltx_fqd<typename fltx_expression_traits<T>::value_type>, fqd,
                    typename fltx_expression_traits<T>::value_type>>;

    template<class T>
    using storage_t = std::conditional_t<fltx_fdd<value_t<T>>, fdd_s,
                      std::conditional_t<fltx_fqd<value_t<T>>, fqd_s, value_t<T>>>;

    template<class T> inline constexpr bool is_f32_v = fltx_f32<T>;
    template<class T> inline constexpr bool is_f64_v = fltx_f64<T>;
    template<class T> inline constexpr bool is_fdd_v = fltx_fdd<T>;
    template<class T> inline constexpr bool is_fqd_v = fltx_fqd<T>;

    template<class T> inline constexpr bool is_fltx_extended_float_v = fltx_extended_float<T>;
    template<class T> inline constexpr bool is_fltx_float_v          = fltx_float<T>;

    template<class T> inline constexpr bool is_arithmetic_v     = fltx_arithmetic<T>;
    template<class T> inline constexpr bool is_integral_v       = std::is_integral_v<T>;

    // Promotion uses an expression's value type, not its node representation.
    template<class T>
    inline constexpr int fltx_precision_rank_v =
        fltx_fqd<value_t<T>>                  ? 5 :
        fltx_fdd<value_t<T>>                  ? 4 :
        std::same_as<value_t<T>, long double> ? 3 :
        (std::same_as<value_t<T>, f64> ||
         std::is_integral_v<value_t<T>>)      ? 2 :
        std::same_as<value_t<T>, f32>         ? 1 : 0;

    namespace detail::traits
    {
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
            static_assert((fltx_arithmetic<value_t<Ts>> && ...),
                "bl::common_float_type_t requires arithmetic or fltx extended floating-point types.");
            static_assert(((fltx_precision_rank_v<Ts> != 0) && ...),
                "bl::common_float_type_t does not support one of these arithmetic types.");

            static constexpr int rank = max_precision_rank<Ts...>();
            using type =
                std::conditional_t<rank == 5, fqd,
                std::conditional_t<rank == 4, fdd,
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
        FDD,
        FQD,
        COUNT
    };

    [[nodiscard]] constexpr std::string_view to_string(FloatType type) noexcept
    {
        switch (type)
        {
        case FloatType::F32: return "f32";
        case FloatType::F64: return "f64";
        case FloatType::FDD: return "fdd";
        case FloatType::FQD: return "fqd";
        default:             return "unknown";
        }
    }

} // namespace bl

#endif
