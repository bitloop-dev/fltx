/**
 * fltx/detail/math_promotion.h - shared argument-promotion utilities for math functions.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_MATH_PROMOTION_INCLUDED
#define FLTX_DETAIL_MATH_PROMOTION_INCLUDED

#include "fltx/config.h"
#include "fltx/detail/math_utils.h"
#include "fltx/traits.h"

#include <concepts>
#include <type_traits>

namespace bl::detail::math
{
    template<class T>
    using clean_t = std::remove_cvref_t<T>;

    template<class T>
    concept native_math_float =
        std::same_as<clean_t<T>, f32> ||
        std::same_as<clean_t<T>, f64>;

    template<class T>
    concept promoted_math_arg =
        fltx_arithmetic<value_t<T>> &&
        fltx_precision_rank_v<T> != 0 &&
        !std::same_as<value_t<T>, long double>;

    template<class T>
    struct promoted_call_type
    {
        using type = T;
    };

    template<> struct promoted_call_type<fdd> { using type = fdd_s; };
    template<> struct promoted_call_type<fqd> { using type = fqd_s; };

    template<class... Args>
    using promoted_t = typename promoted_call_type<common_float_type_t<Args...>>::type;

    template<class P, class Exp>
    struct promoted_pow_exponent_type
    {
        using type = P;
    };

    template<class P, detail::fp::non_bool_integral Exp>
    struct promoted_pow_exponent_type<P, Exp>
    {
        using type = clean_t<Exp>;
    };

    template<class Exp>
    requires (!detail::fp::non_bool_integral<Exp> && fltx_precision_rank_v<Exp> <= 2)
    struct promoted_pow_exponent_type<fdd_s, Exp>
    {
        using type = f64;
    };

    template<class Exp>
    requires (!detail::fp::non_bool_integral<Exp> && fltx_precision_rank_v<Exp> <= 2)
    struct promoted_pow_exponent_type<fqd_s, Exp>
    {
        using type = f64;
    };

    template<class Exp>
    requires fltx_fdd<Exp>
    struct promoted_pow_exponent_type<fqd_s, Exp>
    {
        using type = fdd_s;
    };

    template<class P, class Exp>
    using promoted_pow_exponent_t = typename promoted_pow_exponent_type<P, Exp>::type;

    template<class... Args> concept promoted_math_args = (promoted_math_arg<Args> && ...);

    template<class... Args> concept f64_promoted_math_args =
        promoted_math_args<Args...> && ::bl::detail::traits::max_precision_rank<Args...>() == 2;
    template<class... Args> concept dd_promoted_math_args =
        promoted_math_args<Args...> && ::bl::detail::traits::max_precision_rank<Args...>() == 4;
    template<class... Args> concept qd_promoted_math_args =
        promoted_math_args<Args...> && ::bl::detail::traits::max_precision_rank<Args...>() == 5;

    template<class To, class From>
    [[nodiscard]] BL_FORCE_INLINE constexpr To promoted_cast(const From& value)
    {
        if constexpr (std::same_as<To, clean_t<From>>)
        {
            return value;
        }
        else
        {
            To out{};
            out = value;
            return out;
        }
    }

    template<class To>
    concept native_nexttoward_target =
        std::is_integral_v<clean_t<To>> ||
        native_math_float<To> ||
        std::same_as<clean_t<To>, long double>;

    template<class From, class To>
    concept f64_nexttoward_args =
        promoted_math_arg<From> &&
        fltx_precision_rank_v<From> == 2 &&
        native_nexttoward_target<To>;

    template<class To>
    concept dd_nexttoward_target =
        std::is_integral_v<clean_t<To>> ||
        native_math_float<To> ||
        fltx_fdd<To> ||
        std::same_as<clean_t<To>, long double>;

    template<class From, class To>
    concept dd_nexttoward_args =
        promoted_math_arg<From> &&
        fltx_precision_rank_v<From> == 4 &&
        dd_nexttoward_target<To>;

    template<class To>
    concept qd_nexttoward_target =
        promoted_math_arg<To> ||
        std::same_as<clean_t<To>, long double>;

    template<class From, class To>
    concept qd_nexttoward_args =
        promoted_math_arg<From> &&
        fltx_precision_rank_v<From> == 5 &&
        qd_nexttoward_target<To>;

} // namespace bl::detail::math

#endif
