/**
 * fltx/detail/interop.h - Cross-type conversions and arithmetic for fltx types.
 *
 * Copyright (c) 2026 William Hemsworth
 *
 * This software is released under the MIT License.
 * See LICENSE for details.
 */

// This header is intentionally multi-pass. fltx/fdd.h and fltx/fqd.h both include it
// at the end; the interop bodies are emitted only after both types are complete.

#if defined(FDD_INCLUDED) && defined(FQD_INCLUDED) && !defined(FLTX_INTEROP_INCLUDED)
#define FLTX_INTEROP_INCLUDED

namespace bl
{
    BL_FORCE_INLINE constexpr fqd_s::operator fdd_s() const noexcept
    {
        if (x0 == 0.0 && x1 == 0.0 && x2 == 0.0 && x3 == 0.0)
            return fdd_s{ x0, 0.0 };
        return fdd_s{ x0, x1 } + fdd_s{ x2, x3 };
    }

    BL_FORCE_INLINE constexpr fqd_s::operator fdd() const noexcept
    {
        return static_cast<fdd_s>(*this);
    }

    BL_FORCE_INLINE constexpr fdd_s::operator fqd_s() const noexcept { return fqd_s{ hi, lo }; }

    BL_FORCE_INLINE constexpr fdd::operator fqd_s() const noexcept { return fqd_s{ hi, lo }; }
    BL_FORCE_INLINE constexpr fdd::operator fqd() const noexcept { return fqd_s{ hi, lo }; }

    BL_FORCE_INLINE constexpr fqd::fqd(fdd_s x) noexcept
    {
        x0 = x.hi; x1 = x.lo; x2 = 0.0; x3 = 0.0;
    }

    BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator=(fdd_s x) noexcept
    {
        x0 = x.hi; x1 = x.lo; x2 = 0.0; x3 = 0.0;
        return *this;
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fqd_s& a, const fdd_s& b) noexcept
    {
        return detail::_qd::add_dd(a, detail::_qd::dd_scalar{ b.hi, b.lo });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fqd_s& a, const fdd_s& b) noexcept
    {
        return detail::_qd::sub_dd(a, detail::_qd::dd_scalar{ b.hi, b.lo });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fqd_s& a, const fdd_s& b) noexcept
    {
        return detail::_qd::mul_dd(a, detail::_qd::dd_scalar{ b.hi, b.lo });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fqd_s& a, const fdd_s& b) noexcept
    {
        return detail::_qd::div_dd(a, detail::_qd::dd_scalar{ b.hi, b.lo });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator+(const fdd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::add_dd(b, detail::_qd::dd_scalar{ a.hi, a.lo });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator-(const fdd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::sub_dd(detail::_qd::dd_scalar{ a.hi, a.lo }, b);
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator*(const fdd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::mul_dd(b, detail::_qd::dd_scalar{ a.hi, a.lo });
    }

    [[nodiscard]] BL_FORCE_INLINE constexpr fqd_s operator/(const fdd_s& a, const fqd_s& b) noexcept
    {
        return detail::_qd::div_dd(detail::_qd::dd_scalar{ a.hi, a.lo }, b);
    }

    BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator+=(const fdd_s& rhs) noexcept
    {
        *this = *this + rhs;
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator-=(const fdd_s& rhs) noexcept
    {
        *this = *this - rhs;
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator*=(const fdd_s& rhs) noexcept
    {
        *this = *this * rhs;
        return *this;
    }

    BL_FORCE_INLINE constexpr fqd_s& fqd_s::operator/=(const fdd_s& rhs) noexcept
    {
        *this = *this / rhs;
        return *this;
    }

} // namespace bl

#endif
