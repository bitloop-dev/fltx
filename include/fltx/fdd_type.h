/**
 * fltx/fdd_type.h - dd storage and value type declarations.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

#ifndef FDD_TYPE_INCLUDED
#define FDD_TYPE_INCLUDED
#include "fltx/detail/common_fp.h"
#include "fltx/detail/simd.h"

#if !defined(FLTX_FDD_ENABLE_SIMD)
#  if FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD
#    define FLTX_FDD_ENABLE_SIMD 1
#  else
#    define FLTX_FDD_ENABLE_SIMD 0
#  endif
#endif

#if FLTX_FDD_ENABLE_SIMD && !(FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
#  error "FLTX_FDD_ENABLE_SIMD requires AArch64 NEON or wasm128 SIMD support."
#endif

namespace bl {

struct fdd_s;
struct fqd_s;
struct fqd;

namespace detail::_dd // primitives and kernels
{
    using detail::fp::absd;
    using detail::fp::isnan;
    using detail::fp::isinf;
    using detail::fp::isfinite;
    using detail::fp::uint64_to_exact_double_pair;
    using detail::fp::int64_to_exact_double_pair;
    using detail::fp::two_prod_precise;
    using detail::fp::two_sum_precise;

    using detail::fp::signbit;
    using detail::fp::fabs;
    using detail::fp::floor;
    using detail::fp::ceil;
    using detail::fp::integer_fits_exact_double;

    BL_FORCE_INLINE constexpr bool dd_runtime_product_pair_simd_enabled() noexcept
    {
        #if FLTX_FDD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
        return !bl::detail::is_constant_evaluated();
        #else
        return false;
        #endif
    }

    #if FLTX_FDD_ENABLE_SIMD && (FLTX_HAS_NEON || FLTX_HAS_WASM_SIMD)
    namespace simd = bl::detail::simd;
    #endif

    template<class T>
    inline constexpr bool is_integer_scalar_v = detail::fp::is_integer_scalar_v<T>;

    template<class T>
    inline constexpr bool integer_type_fits_exact_double_v = detail::fp::integer_type_fits_exact_double_v<T>;

    template<class T>
    [[nodiscard]] BL_FORCE_INLINE constexpr fdd_s integer_to_dd(T value) noexcept;

} // namespace detail::_dd

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, const fdd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, const fdd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, const fdd_s& b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, const fdd_s& b) noexcept;

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, double b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, double b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, double b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, double b) noexcept;

[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator+(const fdd_s& a, float b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator-(const fdd_s& a, float b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator*(const fdd_s& a, float b) noexcept;
[[nodiscard]] BL_FORCE_INLINE constexpr fdd_s operator/(const fdd_s& a, float b) noexcept;

struct fdd_s
{
    double hi, lo;

    BL_FORCE_INLINE constexpr fdd_s& operator=(double x) noexcept { hi = x; lo = 0.0; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator=(float x) noexcept { hi = static_cast<double>(x); lo = 0.0; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator=(long double x) noexcept
    {
        double limbs[2]{};
        detail::fp::long_double_to_double_expansion(x, limbs);
        hi = limbs[0]; lo = limbs[1];
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator=(uint64_t u) noexcept;
    BL_FORCE_INLINE constexpr fdd_s& operator=(int64_t v) noexcept;

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator=(T v) noexcept
    {
        return (*this = static_cast<int64_t>(v));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator=(T v) noexcept
    {
        return (*this = static_cast<uint64_t>(v));
    }

    BL_FORCE_INLINE constexpr fdd_s& operator+=(fdd_s rhs) noexcept { *this = *this + rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator-=(fdd_s rhs) noexcept { *this = *this - rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator*=(fdd_s rhs) noexcept { *this = *this * rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator/=(fdd_s rhs) noexcept { *this = *this / rhs; return *this; }

    BL_FORCE_INLINE constexpr fdd_s& operator+=(double rhs) noexcept { *this = *this + rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator-=(double rhs) noexcept { *this = *this - rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator*=(double rhs) noexcept { *this = *this * rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator/=(double rhs) noexcept { *this = *this / rhs; return *this; }

    BL_FORCE_INLINE constexpr fdd_s& operator+=(float rhs) noexcept { *this = *this + rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator-=(float rhs) noexcept { *this = *this - rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator*=(float rhs) noexcept { *this = *this * rhs; return *this; }
    BL_FORCE_INLINE constexpr fdd_s& operator/=(float rhs) noexcept { *this = *this / rhs; return *this; }

    BL_FORCE_INLINE constexpr fdd_s& operator+=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this + static_cast<double>(rhs);
        else {
            const fdd_s value = detail::_dd::integer_to_dd(rhs);
            *this = *this + value;
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator-=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this - static_cast<double>(rhs);
        else {
            const fdd_s value = detail::_dd::integer_to_dd(rhs);
            *this = *this - value;
        }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator*=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this * static_cast<double>(rhs);
        else { const fdd_s value = detail::_dd::integer_to_dd(rhs); *this = *this * value; }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator/=(int64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this / static_cast<double>(rhs);
        else { const fdd_s value = detail::_dd::integer_to_dd(rhs); *this = *this / value; }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator+=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this + static_cast<double>(rhs);
        else { const fdd_s value = detail::_dd::integer_to_dd(rhs); *this = *this + value; }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator-=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this - static_cast<double>(rhs);
        else { const fdd_s value = detail::_dd::integer_to_dd(rhs); *this = *this - value; }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator*=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this * static_cast<double>(rhs);
        else { const fdd_s value = detail::_dd::integer_to_dd(rhs); *this = *this * value; }
        return *this;
    }

    BL_FORCE_INLINE constexpr fdd_s& operator/=(uint64_t rhs) noexcept
    {
        if (detail::fp::integer_fits_exact_double(rhs))
            *this = *this / static_cast<double>(rhs);
        else { const fdd_s value = detail::_dd::integer_to_dd(rhs); *this = *this / value; }
        return *this;
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator+=(T rhs) noexcept
    {
        return (*this += static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator-=(T rhs) noexcept
    {
        return (*this -= static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator*=(T rhs) noexcept
    {
        return (*this *= static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_signed_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator/=(T rhs) noexcept
    {
        return (*this /= static_cast<int64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator+=(T rhs) noexcept
    {
        return (*this += static_cast<uint64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator-=(T rhs) noexcept
    {
        return (*this -= static_cast<uint64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator*=(T rhs) noexcept
    {
        return (*this *= static_cast<uint64_t>(rhs));
    }

    template<class T, std::enable_if_t<std::is_integral_v<T>&& std::is_unsigned_v<T> && (sizeof(T) < 8), int> = 0>
    BL_FORCE_INLINE constexpr fdd_s& operator/=(T rhs) noexcept
    {
        return (*this /= static_cast<uint64_t>(rhs));
    }

    [[nodiscard]] constexpr operator fqd_s() const noexcept;

    template<class T, std::enable_if_t<detail::fp::is_native_arithmetic_scalar_v<T>, int> = 0>
    [[nodiscard]] explicit constexpr operator T() const noexcept
    {
        return detail::fp::expansion_to_native<T>(hi, lo);
    }

    [[nodiscard]] constexpr fdd_s operator+() const { return *this; }
    [[nodiscard]] constexpr fdd_s operator-() const noexcept { return fdd_s{ -hi, -lo }; }

    // Spacing above 1.0 in the public nominal 106-bit model.
    [[nodiscard]] static constexpr fdd_s eps() { return { 0x1p-105, 0.0 }; }
};

struct fdd : public fdd_s
{
    fdd() = default;
    constexpr fdd(double _hi, double _lo) noexcept : fdd_s{ _hi, _lo } {}

    template<class T, std::enable_if_t<detail::fp::is_native_arithmetic_scalar_v<T>, int> = 0>
    constexpr fdd(T x) noexcept : fdd_s{}
    {
        static_cast<fdd_s&>(*this) = x;
    }

    BL_FORCE_INLINE constexpr fdd(const fdd_s& f) noexcept : fdd_s{ f.hi, f.lo } {}

    using fdd_s::operator=;

    [[nodiscard]] constexpr fdd operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr fdd operator-() const noexcept { return { -hi, -lo }; }

    [[nodiscard]] constexpr operator fqd_s() const noexcept;
    [[nodiscard]] constexpr operator fqd() const noexcept;

    [[nodiscard]] static constexpr fdd eps() noexcept { return fdd{ fdd_s::eps() }; }
};

} // namespace bl

#endif
