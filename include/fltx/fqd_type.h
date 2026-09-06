/**
 * fltx/fqd_type.h - qd storage and value type declarations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FQD_TYPE_INCLUDED
#define FQD_TYPE_INCLUDED
#include <type_traits>

#include "fltx/detail/common_fp.h"
#include "fltx/detail/fqd_simd_config.h"

namespace bl {

struct fdd_s;
struct fdd;
struct fqd_s;

namespace detail::_qd // primitives and kernels
{
    using detail::fp::absd;
    using detail::fp::isnan;
    using detail::fp::isinf;
    using detail::fp::isfinite;
    using detail::fp::quick_two_sum_precise;
    using detail::fp::uint64_to_exact_double_pair;
    using detail::fp::int64_to_exact_double_pair;
    using detail::fp::two_prod_precise;
    using detail::fp::two_sum_precise;
    using detail::fp::signbit;
    using detail::fp::fabs;
    using detail::fp::floor;
    using detail::fp::ceil;
    using detail::fp::integer_fits_exact_double;

    BL_FORCE_INLINE constexpr bool qd_runtime_simd_enabled() noexcept
    {
        #if FLTX_FQD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        return !bl::detail::is_constant_evaluated();
        #else
        return false;
        #endif
    }

    BL_FORCE_INLINE constexpr bool qd_runtime_addsub_simd_enabled() noexcept
    {
        #if FLTX_FQD_ENABLE_SIMD && FLTX_HAS_NEON
        return !bl::detail::is_constant_evaluated();
        #else
        return false;
        #endif
    }

    BL_FORCE_INLINE constexpr bool qd_runtime_trig_simd_enabled() noexcept
    {
        #if FLTX_FQD_ENABLE_TRIG_SIMD
        return !bl::detail::is_constant_evaluated();
        #else
        return false;
        #endif
    }

    BL_FORCE_INLINE constexpr bool qd_runtime_product_simd_enabled() noexcept
    {
        #if FLTX_FQD_ENABLE_SIMD && FLTX_HAS_NEON
        return !bl::detail::is_constant_evaluated();
        #elif FLTX_FQD_ENABLE_SIMD && FLTX_HAS_WASM_SIMD
        return !bl::detail::is_constant_evaluated();
        #elif FLTX_FQD_ENABLE_SIMD && FLTX_HAS_SSE2 && (!FLTX_SIMD_USE_FMA_TWO_PROD || FLTX_HAS_X86_FMA)
        return !bl::detail::is_constant_evaluated();
        #else
        return false;
        #endif
    }

    template<class T>
    inline constexpr bool is_integer_scalar_v = detail::fp::is_integer_scalar_v<T>;

    template<class T>
    inline constexpr bool integer_type_fits_exact_double_v = detail::fp::integer_type_fits_exact_double_v<T>;

    // lightweight double-double type to avoid dragging in fltx/fdd.h
    using dd_scalar = detail::fp::double_double;

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr dd_scalar integer_to_double_double(T value) noexcept;

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s integer_to_qd(T value) noexcept;

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s add_dd(const fqd_s& a, dd_scalar b) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_dd(const fqd_s& a, dd_scalar b) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s sub_dd(dd_scalar a, const fqd_s& b) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s mul_dd(const fqd_s& a, dd_scalar b) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_dd(const fqd_s& a, dd_scalar b) noexcept;
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s div_dd(dd_scalar a, const fqd_s& b) noexcept;

#if FLTX_FQD_ENABLE_SIMD
    namespace simd = bl::detail::simd;
#endif

} // namespace detail::_qd

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, const fqd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, const fqd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, const fqd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, const fqd_s& b) noexcept;

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, double b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, double b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, double b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, double b) noexcept;

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, float b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, float b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, float b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, float b) noexcept;

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, const fdd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, const fdd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, const fdd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, const fdd_s& b) noexcept;

[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fdd_s& a, const fqd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fdd_s& a, const fqd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fdd_s& a, const fqd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fdd_s& a, const fqd_s& b) noexcept;

struct fqd_s
{
    double x0, x1, x2, x3; // largest -> smallest

    BL_FORCE_INLINE constexpr fqd_s& operator=(fdd_s x) noexcept;
    BL_FORCE_INLINE constexpr fqd_s& operator=(double x) noexcept { x0 = x; x1 = 0.0; x2 = 0.0; x3 = 0.0; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator=(float x) noexcept { x0 = static_cast<double>(x); x1 = 0.0; x2 = 0.0; x3 = 0.0; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator=(long double x) noexcept
    {
        double limbs[4]{};
        detail::fp::long_double_to_double_expansion(x, limbs);
        x0 = limbs[0]; x1 = limbs[1]; x2 = limbs[2]; x3 = limbs[3];
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator=(uint64_t u) noexcept;
    BL_FORCE_INLINE constexpr fqd_s& operator=(int64_t v) noexcept;

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator=(T v) noexcept
    {
        return (*this = static_cast<int64_t>(v));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator=(T v) noexcept
    {
        return (*this = static_cast<uint64_t>(v));
    }

    BL_FORCE_INLINE constexpr fqd_s& operator+=(fqd_s rhs) noexcept { *this = *this + rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator-=(fqd_s rhs) noexcept { *this = *this - rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator*=(fqd_s rhs) noexcept { *this = *this * rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator/=(fqd_s rhs) noexcept { *this = *this / rhs; return *this; }

    BL_FORCE_INLINE constexpr fqd_s& operator+=(double rhs) noexcept { *this = *this + rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator-=(double rhs) noexcept { *this = *this - rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator*=(double rhs) noexcept { *this = *this * rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator/=(double rhs) noexcept { *this = *this / rhs; return *this; }

    BL_FORCE_INLINE constexpr fqd_s& operator+=(float rhs) noexcept { *this = *this + rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator-=(float rhs) noexcept { *this = *this - rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator*=(float rhs) noexcept { *this = *this * rhs; return *this; }
    BL_FORCE_INLINE constexpr fqd_s& operator/=(float rhs) noexcept { *this = *this / rhs; return *this; }

    BL_FORCE_INLINE constexpr fqd_s& operator+=(const fdd_s& rhs) noexcept;
    BL_FORCE_INLINE constexpr fqd_s& operator-=(const fdd_s& rhs) noexcept;
    BL_FORCE_INLINE constexpr fqd_s& operator*=(const fdd_s& rhs) noexcept;
    BL_FORCE_INLINE constexpr fqd_s& operator/=(const fdd_s& rhs) noexcept;

    BL_FORCE_INLINE constexpr fqd_s& operator+=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this + static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::add_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator-=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this - static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::sub_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator*=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this * static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::mul_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator/=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this / static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::div_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator+=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this + static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::add_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator-=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this - static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::sub_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator*=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this * static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::mul_dd(*this, value);
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& operator/=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this / static_cast<double>(rhs);
        else
        {
            const auto value = detail::_qd::integer_to_double_double(rhs);
            *this = detail::_qd::div_dd(*this, value);
        }
        return *this;
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator+=(T rhs) noexcept
    {
        return (*this += static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator-=(T rhs) noexcept
    {
        return (*this -= static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator*=(T rhs) noexcept
    {
        return (*this *= static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator/=(T rhs) noexcept
    {
        return (*this /= static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator+=(T rhs) noexcept
    {
        return (*this += static_cast<uint64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator-=(T rhs) noexcept
    {
        return (*this -= static_cast<uint64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator*=(T rhs) noexcept
    {
        return (*this *= static_cast<uint64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) <= 8), int> = 0>
    BL_FORCE_INLINE constexpr fqd_s& operator/=(T rhs) noexcept
    {
        return (*this /= static_cast<uint64_t>(rhs));
    }

    [[nodiscard]] explicit constexpr operator fdd() const noexcept;
    [[nodiscard]] explicit constexpr operator fdd_s() const noexcept;

    template<class T, std::enable_if_t<detail::fp::is_native_arithmetic_scalar_v<T>, int> = 0>
    [[nodiscard]] explicit constexpr operator T() const noexcept
    {
        return detail::fp::expansion_to_native<T>(x0, x1, x2, x3);
    }

    [[nodiscard]] constexpr fqd_s operator+() const { return *this; }
    [[nodiscard]] constexpr fqd_s operator-() const noexcept { return fqd_s{ -x0, -x1, -x2, -x3 }; }

    // Spacing above 1.0 in the public nominal 212-bit model.
    [[nodiscard]] static constexpr fqd_s eps() { return { 0x1p-211, 0.0, 0.0, 0.0 }; }
};

namespace detail::_qd_expr
{
    // defined in fltx/detail/fqd_expressions.h
	// fqd needs this hook before its full definition.
    template<class Expr>
    struct is_expr : std::false_type {};

    template<class Expr>
    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s eval_to_qd_s(const Expr& expr) noexcept;

} // namespace detail::_qd_expr

struct fqd : public fqd_s
{
    fqd() = default;
    constexpr fqd(double _x0, double _x1, double _x2, double _x3) noexcept : fqd_s{ _x0, _x1, _x2, _x3 } {}

    template<class T, std::enable_if_t<detail::fp::is_native_arithmetic_scalar_v<T>, int> = 0>
    constexpr fqd(T x) noexcept : fqd_s{}
    {
        static_cast<fqd_s&>(*this) = x;
    }

    constexpr fqd(fdd_s f) noexcept;
    constexpr fqd(const fqd_s& f) noexcept : fqd_s{ f.x0, f.x1, f.x2, f.x3 } {}

    template<class Expr, std::enable_if_t<detail::_qd_expr::is_expr<Expr>::value, int> = 0>
    BL_FORCE_INLINE constexpr fqd(const Expr& expr) noexcept : fqd_s{ detail::_qd_expr::eval_to_qd_s(expr) } {}

    using fqd_s::operator=;

    [[nodiscard]] constexpr fqd operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr fqd operator-() const noexcept { return { -x0, -x1, -x2, -x3 }; }

    template<class Expr, std::enable_if_t<detail::_qd_expr::is_expr<Expr>::value, int> = 0>
    BL_FORCE_INLINE constexpr fqd& operator=(const Expr& expr) noexcept
    {
        static_cast<fqd_s&>(*this) = detail::_qd_expr::eval_to_qd_s(expr);
        return *this;
    }

    [[nodiscard]] static constexpr fqd eps() noexcept { return fqd{ fqd_s::eps() }; }
};

} // namespace bl

#endif
