/**
 * fltx/detail/math_utils.h - Generic math utilities shared by the math headers.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FLTX_DETAIL_MATH_UTILS_INCLUDED
#define FLTX_DETAIL_MATH_UTILS_INCLUDED
#include <cmath>
#include <cstddef>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

#include "fltx/config.h"
#include "fltx/traits.h"

namespace bl::detail::fp
{
    template<class... Ts>
    concept arithmetic_args = (fltx_arithmetic<fltx_expression_value_t<Ts>> && ...);

    // Selection preserves integer-only inputs; floating inputs use the math ladder.
    template<class... Ts>
    struct selection_result : common_float_type<Ts...> {};

    template<class... Ts>
    requires (std::integral<fltx_expression_value_t<Ts>> && ...)
    struct selection_result<Ts...> : std::common_type<fltx_expression_value_t<Ts>...> {};

    template<class... Ts>
    using selection_result_t = typename selection_result<Ts...>::type;

    template<class T>
    concept non_bool_integral =
        std::integral<std::remove_cvref_t<T>> &&
        !std::same_as<std::remove_cvref_t<T>, bool>;

    template<class U>
    requires non_bool_integral<U>
    constexpr std::make_unsigned_t<std::remove_cvref_t<U>> unsigned_abs(U value) noexcept
    {
        using S = std::remove_cvref_t<U>;
        using Uu = std::make_unsigned_t<S>;

        if constexpr (std::signed_integral<S>)
        {
            return (value < 0)
                ? static_cast<Uu>(-(value + 1)) + Uu{ 1 }
            : static_cast<Uu>(value);
        }
        else
        {
            return static_cast<Uu>(value);
        }
    }

    template<class UInt>
    requires std::unsigned_integral<UInt>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool factor_power_of_two_five(
        UInt magnitude,
        int& pow2,
        int& pow5) noexcept
    {
        if (magnitude <= UInt{ 1 })
            return false;

        pow2 = 0;
        pow5 = 0;

        while ((magnitude & UInt{ 1 }) == UInt{ 0 })
        {
            magnitude >>= 1;
            ++pow2;
        }

        while ((magnitude % UInt{ 5 }) == UInt{ 0 })
        {
            magnitude /= UInt{ 5 };
            ++pow5;
        }

        return pow5 != 0 && magnitude == UInt{ 1 };
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr bool checked_exponent_product(
        int factor,
        int exponent,
        int& out) noexcept
    {
        const long long wide = static_cast<long long>(factor) * static_cast<long long>(exponent);
        if (wide < static_cast<long long>(std::numeric_limits<int>::min()) ||
            wide > static_cast<long long>(std::numeric_limits<int>::max()))
        {
            return false;
        }

        out = static_cast<int>(wide);
        return true;
    }

    template<class UInt>
    requires std::unsigned_integral<UInt>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_unsigned_power_of_two_log2(UInt value, int& out) noexcept
    {
        if (value <= UInt{ 1 } || (value & (value - UInt{ 1 })) != UInt{ 0 })
            return false;

        int exponent = 0;
        while (value > UInt{ 1 })
        {
            value >>= 1;
            ++exponent;
        }

        out = exponent;
        return true;
    }

    // Avoid runtime log checks when small integer powers are safely below Dekker split overflow.
    inline constexpr std::uint16_t integral_pow_split_safe_exp_limit_upto_256[] = {
        0, 65535, 996, 628, 498, 428, 385, 354, 332, 314, 299, 287, 277, 269, 261, 254,
        249, 243, 238, 234, 230, 226, 223, 220, 217, 214, 211, 209, 207, 205, 202, 201,
        199, 197, 195, 194, 192, 191, 189, 188, 187, 185, 184, 183, 182, 181, 180, 179,
        178, 177, 176, 175, 174, 173, 173, 172, 171, 170, 170, 169, 168, 167, 167, 166,
        166, 165, 164, 164, 163, 163, 162, 161, 161, 160, 160, 159, 159, 158, 158, 158,
        157, 157, 156, 156, 155, 155, 154, 154, 154, 153, 153, 153, 152, 152, 151, 151,
        151, 150, 150, 150, 149, 149, 149, 148, 148, 148, 148, 147, 147, 147, 146, 146,
        146, 146, 145, 145, 145, 144, 144, 144, 144, 143, 143, 143, 143, 142, 142, 142,
        142, 142, 141, 141, 141, 141, 140, 140, 140, 140, 140, 139, 139, 139, 139, 139,
        138, 138, 138, 138, 138, 137, 137, 137, 137, 137, 137, 136, 136, 136, 136, 136,
        136, 135, 135, 135, 135, 135, 135, 134, 134, 134, 134, 134, 134, 133, 133, 133,
        133, 133, 133, 133, 132, 132, 132, 132, 132, 132, 132, 131, 131, 131, 131, 131,
        131, 131, 131, 130, 130, 130, 130, 130, 130, 130, 130, 129, 129, 129, 129, 129,
        129, 129, 129, 128, 128, 128, 128, 128, 128, 128, 128, 128, 127, 127, 127, 127,
        127, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126,
        125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 124, 124, 124, 124, 124,
        124
    };

    // Avoid reciprocal pow work when small negative integer powers must underflow to binary64 zero.
    inline constexpr std::uint16_t integral_pow_underflow_exp_limit_upto_256[] = {
        0, 65535, 1075, 679, 538, 463, 416, 383, 359, 340, 324, 311, 300, 291, 283, 276,
        269, 263, 258, 254, 249, 245, 242, 238, 235, 232, 229, 227, 224, 222, 220, 217,
        215, 214, 212, 210, 208, 207, 205, 204, 202, 201, 200, 199, 197, 196, 195, 194,
        193, 192, 191, 190, 189, 188, 187, 186, 186, 185, 184, 183, 182, 182, 181, 180,
        180, 179, 178, 178, 177, 176, 176, 175, 175, 174, 174, 173, 173, 172, 172, 171,
        171, 170, 170, 169, 169, 168, 168, 167, 167, 167, 166, 166, 165, 165, 165, 164,
        164, 163, 163, 163, 162, 162, 162, 161, 161, 161, 160, 160, 160, 159, 159, 159,
        158, 158, 158, 158, 157, 157, 157, 156, 156, 156, 156, 155, 155, 155, 155, 154,
        154, 154, 154, 153, 153, 153, 153, 152, 152, 152, 152, 152, 151, 151, 151, 151,
        150, 150, 150, 150, 150, 149, 149, 149, 149, 149, 148, 148, 148, 148, 148, 148,
        147, 147, 147, 147, 147, 146, 146, 146, 146, 146, 146, 145, 145, 145, 145, 145,
        145, 144, 144, 144, 144, 144, 144, 144, 143, 143, 143, 143, 143, 143, 143, 142,
        142, 142, 142, 142, 142, 142, 141, 141, 141, 141, 141, 141, 141, 140, 140, 140,
        140, 140, 140, 140, 140, 139, 139, 139, 139, 139, 139, 139, 139, 139, 138, 138,
        138, 138, 138, 138, 138, 138, 138, 137, 137, 137, 137, 137, 137, 137, 137, 137,
        136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 135, 135, 135, 135, 135, 135,
        135
    };

    template<class Base, class ExpUnsigned>
    requires (non_bool_integral<Base> && std::unsigned_integral<ExpUnsigned>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool integral_pow_split_safe(Base base, ExpUnsigned exp) noexcept
    {
        const std::uintmax_t magnitude_base = static_cast<std::uintmax_t>(unsigned_abs(base));
        if (magnitude_base > 256u)
            return false;

        return static_cast<std::uintmax_t>(exp) <=
            integral_pow_split_safe_exp_limit_upto_256[static_cast<std::size_t>(magnitude_base)];
    }

    template<class Base, class ExpUnsigned>
    requires (non_bool_integral<Base> && std::unsigned_integral<ExpUnsigned>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool integral_pow_reciprocal_underflows_binary64(
        Base base,
        ExpUnsigned exp) noexcept
    {
        const std::uintmax_t original_magnitude_base = static_cast<std::uintmax_t>(unsigned_abs(base));
        if (original_magnitude_base <= 1u)
            return false;

        if (original_magnitude_base <= 256u)
        {
            return static_cast<std::uintmax_t>(exp) >
                integral_pow_underflow_exp_limit_upto_256[static_cast<std::size_t>(original_magnitude_base)];
        }

        std::uintmax_t magnitude_base = original_magnitude_base;
        int floor_log2_base = 0;
        while (magnitude_base > 1u)
        {
            magnitude_base >>= 1u;
            ++floor_log2_base;
        }

        return floor_log2_base > 0 &&
            static_cast<std::uintmax_t>(exp) > (std::uintmax_t{ 1075 } / static_cast<std::uintmax_t>(floor_log2_base));
    }

    template<class Base, class ExpUnsigned>
    requires (non_bool_integral<Base> && std::unsigned_integral<ExpUnsigned>)
    [[nodiscard]] BL_FORCE_INLINE constexpr bool negative_integral_pow_result_is_negative(
        Base base,
        ExpUnsigned exp) noexcept
    {
        if constexpr (std::signed_integral<std::remove_cvref_t<Base>>)
            return base < Base{ 0 } && ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 });
        else
            return false;
    }

    template<class R>
    struct ipow_square_traits
    {
        [[nodiscard]] BL_FORCE_INLINE static constexpr R eval(const R& value)
            noexcept(noexcept(value * value))
        {
            return value * value;
        }
    };

    template<class R>
    [[nodiscard]] BL_FORCE_INLINE constexpr R ipow_square(const R& value)
        noexcept(noexcept(ipow_square_traits<R>::eval(value)))
    {
        return ipow_square_traits<R>::eval(value);
    }

    template<class R>
    requires non_bool_integral<R>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool ipow_integral_mul_overflows(R a, R b) noexcept
    {
        if constexpr (std::unsigned_integral<R>)
        {
            return b != R{ 0 } && a > (std::numeric_limits<R>::max() / b);
        }
        else
        {
            constexpr R min = std::numeric_limits<R>::min();
            constexpr R max = std::numeric_limits<R>::max();

            if (a > R{ 0 })
                return b > R{ 0 } ? a > (max / b) : b < (min / a);
            if (a < R{ 0 })
                return b > R{ 0 } ? a < (min / b) : b < (max / a);
            return false;
        }
    }

    template<class R>
    requires non_bool_integral<R>
    [[nodiscard]] BL_FORCE_INLINE constexpr R ipow_integral_mul(R a, R b)
    {
        BL_CONSTEXPR_DEBUG_ASSERT(
            !ipow_integral_mul_overflows(a, b),
            "bl::ipow integral overflow");

        using U = std::make_unsigned_t<std::remove_cvref_t<R>>;
        const U out = static_cast<U>(a) * static_cast<U>(b);
        return static_cast<R>(out);
    }

    template<class R>
    requires non_bool_integral<R>
    [[nodiscard]] BL_FORCE_INLINE constexpr R ipow_integral_square(R value)
    {
        return ipow_integral_mul(value, value);
    }

    template<class R, class ExpUnsigned>
    BL_FORCE_INLINE constexpr R ipow_nonneg(R base, ExpUnsigned exp)
    {
        R result = R{ 1 };

        while (exp != ExpUnsigned{ 0 })
        {
            if ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 })
                result *= base;

            exp >>= 1;

            if (exp != ExpUnsigned{ 0 })
                base = ipow_square(base);
        }

        return result;
    }

    template<class R, class ExpUnsigned>
    BL_FORCE_INLINE constexpr R ipow_nonneg_fast(R base, ExpUnsigned exp)
    {
        if (exp == ExpUnsigned{ 0 })
            return R{ 1 };
        if (exp == ExpUnsigned{ 1 })
            return base;

        if (exp == ExpUnsigned{ 2 })
            return ipow_square(base);
        if (exp == ExpUnsigned{ 3 })
        {
            const R squared = ipow_square(base);
            return squared * base;
        }
        if (exp == ExpUnsigned{ 4 })
        {
            return ipow_square(ipow_square(base));
        }

        return ipow_nonneg(base, exp);
    }

    template<class R, class ExpUnsigned>
    requires (non_bool_integral<R> && std::unsigned_integral<ExpUnsigned>)
    BL_FORCE_INLINE constexpr R ipow_integral_nonneg(R base, ExpUnsigned exp)
    {
        R result = R{ 1 };

        while (exp != ExpUnsigned{ 0 })
        {
            if ((exp & ExpUnsigned{ 1 }) != ExpUnsigned{ 0 })
                result = ipow_integral_mul(result, base);

            exp >>= 1;

            if (exp != ExpUnsigned{ 0 })
                base = ipow_integral_square(base);
        }

        return result;
    }

    template<class R, class ExpUnsigned>
    requires (non_bool_integral<R> && std::unsigned_integral<ExpUnsigned>)
    BL_FORCE_INLINE constexpr R ipow_integral_nonneg_fast(R base, ExpUnsigned exp)
    {
        if (exp == ExpUnsigned{ 0 })
            return R{ 1 };
        if (exp == ExpUnsigned{ 1 })
            return base;

        if (exp == ExpUnsigned{ 2 })
            return ipow_integral_square(base);
        if (exp == ExpUnsigned{ 3 })
        {
            const R squared = ipow_integral_square(base);
            return ipow_integral_mul(squared, base);
        }
        if (exp == ExpUnsigned{ 4 })
        {
            return ipow_integral_square(ipow_integral_square(base));
        }

        return ipow_integral_nonneg(base, exp);
    }

    template<class Exp>
    requires non_bool_integral<Exp>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool try_int_exponent(Exp value, int& out) noexcept
    {
        using E = std::remove_cvref_t<Exp>;
        constexpr int min_int = std::numeric_limits<int>::min();
        constexpr int max_int = std::numeric_limits<int>::max();

        if constexpr (std::signed_integral<E>)
        {
            const auto wide = static_cast<std::intmax_t>(value);
            if (wide < static_cast<std::intmax_t>(min_int) || wide > static_cast<std::intmax_t>(max_int))
                return false;
        }
        else
        {
            if (static_cast<std::uintmax_t>(value) > static_cast<std::uintmax_t>(max_int))
                return false;
        }

        out = static_cast<int>(value);
        return true;
    }

    template<class Exp>
    requires non_bool_integral<Exp>
    [[nodiscard]] BL_FORCE_INLINE constexpr bool is_odd_integral(Exp value) noexcept
    {
        return (unsigned_abs(value) & std::make_unsigned_t<std::remove_cvref_t<Exp>>{ 1 }) != 0;
    }

    template<class R, class T, class U>
    requires (non_bool_integral<R> && non_bool_integral<T> && non_bool_integral<U>)
    [[nodiscard]] BL_FORCE_INLINE constexpr R ipow_integral_default_result(T x, U y)
    {
        using Exp = std::remove_cvref_t<U>;
        using ExpUnsigned = std::make_unsigned_t<Exp>;

        const R base = static_cast<R>(x);

        if constexpr (std::signed_integral<Exp>)
        {
            if (y < 0)
            {
                if (base == R{ 1 })
                    return R{ 1 };

                if constexpr (std::signed_integral<R>)
                {
                    if (base == R{ -1 })
                        return (y % 2 != 0) ? R{ -1 } : R{ 1 };
                }

                return R{ 0 };
            }
        }

        const ExpUnsigned exp = unsigned_abs(y);
        return ipow_integral_nonneg_fast<R, ExpUnsigned>(base, exp);
    }

    template<class Vector, class Value>
    concept sincos_vector_constructible = requires(const Value& c, const Value& s)
    {
        Vector{ c, s };
    };

    template<class Vector, class Value>
    concept sincos_vector_assignable = requires(Vector& out, const Value& c, const Value& s)
    {
        out = Vector{ c, s };
    };

    template<class Vector, class Value>
    requires sincos_vector_assignable<Vector, Value>
    BL_FORCE_INLINE constexpr void assign_sincos_vector(Vector& out, const Value& s_out, const Value& c_out)
        noexcept(noexcept(out = Vector{ c_out, s_out }))
    {
        out = Vector{ c_out, s_out };
    }

    template<class Value>
    struct sincos_vector_result
    {
        Value c;
        Value s;
        bool ok;

        template<class Vector>
        requires sincos_vector_constructible<Vector, Value>
        [[nodiscard]] BL_FORCE_INLINE constexpr operator Vector() const noexcept(noexcept(Vector{ c, s }))
        {
            return Vector{ c, s };
        }
    };

    template<class Value>
    [[nodiscard]] BL_FORCE_INLINE constexpr sincos_vector_result<Value> make_sincos_result(const Value& s_out, const Value& c_out, bool ok)
    {
        return { c_out, s_out, ok };
    }

} // namespace bl::detail::fp

namespace bl
{
    template<class T>
    [[nodiscard]] constexpr fltx_expression_value_t<T> sqr(T x) noexcept(noexcept(x * x))
    {
        if constexpr (fltx_expression<T>)
            return sqr(fltx_expression_value_t<T>{ x });
        else
            return x * x;
    }

    template<class T>
    requires (!detail::fp::arithmetic_args<T>)
    [[nodiscard]] constexpr T clamp(T x, T low, T high) noexcept(noexcept(x < low) && noexcept(high < x))
    {
        return (x < low) ? low : ((high < x) ? high : x);
    }

    // Comparison-based selection, not the NaN-skipping fmin/fmax policy.
    // Owning results avoid dangling references to converted values or expressions.
    template<class A, class B>
    requires detail::fp::arithmetic_args<A, B>
    [[nodiscard]] constexpr auto min(const A& a, const B& b) noexcept
    {
        using R = detail::fp::selection_result_t<A, B>;
        const R x = static_cast<R>(a), y = static_cast<R>(b);
        // MSVC can otherwise fold the floating select with the wrong zero sign.
        if constexpr (std::floating_point<R>)
            if (x == y) return x;
        return y < x ? y : x;
    }

    template<class A, class B>
    requires detail::fp::arithmetic_args<A, B>
    [[nodiscard]] constexpr auto max(const A& a, const B& b) noexcept
    {
        using R = detail::fp::selection_result_t<A, B>;
        const R x = static_cast<R>(a), y = static_cast<R>(b);
        if constexpr (std::floating_point<R>)
            if (x == y) return x;
        return x < y ? y : x;
    }

    // As with std::min/max, the list must be nonempty; ties keep the first value.
    template<class T>
    requires detail::fp::arithmetic_args<T>
    [[nodiscard]] constexpr auto min(std::initializer_list<T> values) noexcept
    {
        using R = detail::fp::selection_result_t<T>;
        auto it = values.begin();
        R result = static_cast<R>(*it++);
        for (; it != values.end(); ++it)
            result = bl::min(result, *it);
        return result;
    }

    template<class T>
    requires detail::fp::arithmetic_args<T>
    [[nodiscard]] constexpr auto max(std::initializer_list<T> values) noexcept
    {
        using R = detail::fp::selection_result_t<T>;
        auto it = values.begin();
        R result = static_cast<R>(*it++);
        for (; it != values.end(); ++it)
            result = bl::max(result, *it);
        return result;
    }

    template<class A, class B>
    requires detail::fp::arithmetic_args<A, B>
    [[nodiscard]] constexpr auto minmax(const A& a, const B& b) noexcept
    {
        using R = detail::fp::selection_result_t<A, B>;
        const R x = static_cast<R>(a), y = static_cast<R>(b);
        return y < x ? std::pair<R, R>{ y, x } : std::pair<R, R>{ x, y };
    }

    template<class T, class Low, class High>
    requires detail::fp::arithmetic_args<T, Low, High>
    [[nodiscard]] constexpr auto clamp(const T& value, const Low& low, const High& high) noexcept
    {
        using R = detail::fp::selection_result_t<T, Low, High>;
        const R x = static_cast<R>(value), lo = static_cast<R>(low), hi = static_cast<R>(high);
        if constexpr (std::floating_point<R>)
            if (x == lo || x == hi) return x;
        return x < lo ? lo : (hi < x ? hi : x);
    }

    template<class A, class B, class T>
    requires detail::fp::arithmetic_args<A, B, T>
    [[nodiscard]] constexpr auto lerp(const A& a, const B& b, const T& t) noexcept
    {
        using R = common_float_type_t<A, B, T>;
        const R x = static_cast<R>(a), y = static_cast<R>(b), weight = static_cast<R>(t);
        if (weight == R{ 0 }) return x;
        if (weight == R{ 1 }) return y;
        if constexpr (std::floating_point<R>)
            return std::lerp(x, y, weight);
        else
        {
            // Opposite signs require weighted endpoints to avoid overflowing y - x.
            if ((x < R{ 0 } && y > R{ 0 }) || (x > R{ 0 } && y < R{ 0 }))
                return R{ (R{ 1 } - weight) * x + weight * y };
            const R result = x + weight * (y - x);
            // Keep rounding from crossing the endpoint in the wrong direction.
            return (weight > R{ 1 }) == (y > x)
                ? bl::max(result, y) : bl::min(result, y);
        }
    }

    template<class A, class B>
    requires (detail::fp::arithmetic_args<A, B> &&
              !std::same_as<fltx_expression_value_t<A>, bool> &&
              !std::same_as<fltx_expression_value_t<B>, bool>)
    [[nodiscard]] constexpr auto midpoint(const A& a, const B& b) noexcept
    {
        using R = detail::fp::selection_result_t<A, B>;
        const R x = static_cast<R>(a), y = static_cast<R>(b);
        if constexpr (std::is_arithmetic_v<R>)
            return std::midpoint(x, y);
        else
        {
            // Use exact binary scaling; general multiplication can overflow
            // the unchecked Dekker splitter even when the result is finite.
            const R hi = ldexp(std::numeric_limits<R>::max(), -1);
            const R lo = ldexp(std::numeric_limits<R>::min(), 1);
            const R ax = x < R{ 0 } ? R{ -x } : x;
            const R ay = y < R{ 0 } ? R{ -y } : y;
            if (ax <= hi && ay <= hi) return R{ ldexp(R{ x + y }, -1) };
            // Avoid halving a tiny operand when the other must be scaled first.
            if (ax < lo) return R{ x + ldexp(y, -1) };
            if (ay < lo) return R{ ldexp(x, -1) + y };
            // Materialize exact scaling before addition, including at the range limit.
            const R half_x = ldexp(x, -1), half_y = ldexp(y, -1);
            return R{ half_x + half_y };
        }
    }

    template<class T, class Exp>
    requires (
        !std::integral<std::remove_cvref_t<T>> &&
        !fltx_extended_float<fltx_expression_value_t<T>> &&
        detail::fp::non_bool_integral<Exp>)
    [[nodiscard]] constexpr std::remove_cvref_t<T> ipow(T base, Exp exp)
    {
        using R = std::remove_cvref_t<T>;
        using U = std::make_unsigned_t<std::remove_cvref_t<Exp>>;

        const U magnitude = detail::fp::unsigned_abs(exp);
        const R powered = detail::fp::ipow_nonneg_fast<R, U>(static_cast<R>(base), magnitude);

        if constexpr (std::signed_integral<std::remove_cvref_t<Exp>>)
        {
            if (exp < 0)
                return R{ 1 } / powered;
        }

        return powered;
    }

    template<class T, class U>
    requires (detail::fp::non_bool_integral<T> && detail::fp::non_bool_integral<U>)
    [[nodiscard]] constexpr std::remove_cvref_t<T> ipow(T x, U y)
    {
        return detail::fp::ipow_integral_default_result<std::remove_cvref_t<T>>(x, y);
    }

} // namespace bl

#endif
