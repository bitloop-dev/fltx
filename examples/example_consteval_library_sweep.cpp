#include <iostream>
#include <iomanip>

#include <fltx.h>

using namespace bl;
using namespace bl::literals;

using llong = long long;

consteval f32 f32_test()
{
    constexpr f32 abs_result = bl::abs(-0.123f);
    constexpr f32 fma_result = bl::fma(0.123f, 4.56f, -0.75f);
                                      
    constexpr f32 floor_result     = bl::floor(2.75f);
    constexpr f32 ceil_result      = bl::ceil(2.25f);
    constexpr f32 trunc_result     = bl::trunc(-2.75f);
    constexpr f32 round_result     = bl::round(2.5f);
    constexpr long lround_result   = bl::lround(2.5f);
    constexpr llong llround_result = bl::llround(2.5f);
    constexpr f32 roundeven_result = bl::roundeven(2.5f);
                                      
    constexpr f32 fmod_result      = bl::fmod(5.5f, 2.0f);
    constexpr f32 remainder_result = bl::remainder(5.5f, 2.0f);
    constexpr f32 remquo_result1   = [] { int quo{}; return bl::remquo(5.5f, 2.0f, &quo); }();
    constexpr int remquo_result2   = [] { int quo{}; (void)bl::remquo(5.5f, 2.0f, &quo); return quo; }();
                                      
    constexpr f32 fmin_result     = bl::fmin(-0.25f, 0.5f);
    constexpr f32 fmax_result     = bl::fmax(-0.25f, 0.5f);
    constexpr f32 fdim_result     = bl::fdim(1.25f, 0.75f);
    constexpr f32 copysign_result = bl::copysign(0.125f, -1.0f);
                                      
    constexpr f32 sqrt_result  = bl::sqrt(0.123f);
    constexpr f32 cbrt_result  = bl::cbrt(0.123f);
    constexpr f32 hypot_result = bl::hypot(0.3f, 0.4f);
    constexpr f32 pow_result   = bl::pow(0.123f, 4.56f);
    constexpr f32 pow10_result = bl::ipow(10.0f, 2);
                                      
    constexpr f32 exp_result   = bl::exp(0.123f);
    constexpr f32 exp2_result  = bl::exp2(0.123f);
    constexpr f32 expm1_result = bl::expm1(0.123f);
    constexpr f32 log_result   = bl::log(1.123f);
    constexpr f32 log2_result  = bl::log2(1.123f);
    constexpr f32 log10_result = bl::log10(1.123f);
    constexpr f32 log1p_result = bl::log1p(0.123f);
    constexpr f32 logb_result  = bl::logb(12.5f);
    constexpr int ilogb_result = bl::ilogb(12.5f);
                                      
    constexpr f32 sin_result   = bl::sin(0.123f);
    constexpr f32 cos_result   = bl::cos(0.123f);
    constexpr f32 tan_result   = bl::tan(0.123f);
    constexpr f32 asin_result  = bl::asin(0.123f);
    constexpr f32 acos_result  = bl::acos(0.123f);
    constexpr f32 atan_result  = bl::atan(0.123f);
    constexpr f32 atan2_result = bl::atan2(0.123f, 0.456f);
                                      
    constexpr f32 sinh_result  = bl::sinh(0.123f);
    constexpr f32 cosh_result  = bl::cosh(0.123f);
    constexpr f32 tanh_result  = bl::tanh(0.123f);
    constexpr f32 asinh_result = bl::asinh(0.123f);
    constexpr f32 acosh_result = bl::acosh(1.123f);
    constexpr f32 atanh_result = bl::atanh(0.123f);
                                      
    constexpr f32 erf_result    = bl::erf(0.123f);
    constexpr f32 erfc_result   = bl::erfc(0.123f);
    constexpr f32 lgamma_result = bl::lgamma(1.123f);
    constexpr f32 tgamma_result = bl::tgamma(1.123f);
                                      
    constexpr f32 ldexp_result     = bl::ldexp(0.123f, 5);
    constexpr f32 scalbn_result    = bl::scalbn(0.123f, 5);
    constexpr f32 scalbln_result   = bl::scalbln(0.123f, 5L);
    constexpr f32 frexp_result     = [] { int exp{}; return bl::frexp(12.5f, &exp); }();
    constexpr int frexp_exp_result = [] { int exp{}; (void)bl::frexp(12.5f, &exp); return exp; }();
    constexpr f32 modf_result      = [] { f32 ip{}; return bl::modf(12.5f, &ip); }();
    constexpr f32 modf_int_result  = [] { f32 ip{}; (void)bl::modf(12.5f, &ip); return ip; }();

    constexpr f32 nextafter_result   = bl::nextafter(0.123f, 0.124f);
    constexpr f32 nexttoward_result1 = bl::nexttoward(0.123f, 0.124L);
    constexpr f32 nexttoward_result2 = bl::nexttoward(0.123f, 0.124f);

    constexpr f32 scalar_sum =
        abs_result + fma_result
        + floor_result + ceil_result + trunc_result + round_result
        + static_cast<f32>(lround_result) + static_cast<f32>(llround_result)
        + roundeven_result
        + fmod_result + remainder_result + remquo_result1 + static_cast<f32>(remquo_result2)
        + fmin_result + fmax_result + fdim_result + copysign_result
        + sqrt_result + cbrt_result + hypot_result + pow_result + pow10_result
        + exp_result + exp2_result + expm1_result
        + log_result + log2_result + log10_result + log1p_result + logb_result + static_cast<f32>(ilogb_result)
        + sin_result + cos_result + tan_result + asin_result + acos_result + atan_result + atan2_result
        + sinh_result + cosh_result + tanh_result + asinh_result + acosh_result + atanh_result
        + erf_result + erfc_result + lgamma_result + tgamma_result
        + ldexp_result + scalbn_result + scalbln_result
        + frexp_result + static_cast<f32>(frexp_exp_result) + modf_result + modf_int_result
        + nextafter_result + nexttoward_result1 + nexttoward_result2;

    return scalar_sum;
}

consteval f64 f64_test()
{
    constexpr f64 abs_result = bl::abs(-0.123);
    constexpr f64 fma_result = bl::fma(0.123, 4.56, -0.75);
                                      
    constexpr f64 floor_result     = bl::floor(2.75);
    constexpr f64 ceil_result      = bl::ceil(2.25);
    constexpr f64 trunc_result     = bl::trunc(-2.75);
    constexpr f64 round_result     = bl::round(2.5);
    constexpr long lround_result   = bl::lround(2.5);
    constexpr llong llround_result = bl::llround(2.5);
    constexpr f64 roundeven_result = bl::roundeven(2.5);
                                      
    constexpr f64 fmod_result      = bl::fmod(5.5, 2.0);
    constexpr f64 remainder_result = bl::remainder(5.5, 2.0);
    constexpr f64 remquo_result1   = [] { int quo{}; return bl::remquo(5.5, 2.0, &quo); }();
    constexpr int remquo_result2   = [] { int quo{}; (void)bl::remquo(5.5, 2.0, &quo); return quo; }();
                                      
    constexpr f64 fmin_result     = bl::fmin(-0.25, 0.5);
    constexpr f64 fmax_result     = bl::fmax(-0.25, 0.5);
    constexpr f64 fdim_result     = bl::fdim(1.25, 0.75);
    constexpr f64 copysign_result = bl::copysign(0.125, -1.0);
                                      
    constexpr f64 sqrt_result  = bl::sqrt(0.123);
    constexpr f64 cbrt_result  = bl::cbrt(0.123);
    constexpr f64 hypot_result = bl::hypot(0.3, 0.4);
    constexpr f64 pow_result   = bl::pow(0.123, 4.56);
    constexpr f64 pow10_result = bl::pow(10.0, 2);
                                      
    constexpr f64 exp_result   = bl::exp(0.123);
    constexpr f64 exp2_result  = bl::exp2(0.123);
    constexpr f64 expm1_result = bl::expm1(0.123);
    constexpr f64 log_result   = bl::log(1.123);
    constexpr f64 log2_result  = bl::log2(1.123);
    constexpr f64 log10_result = bl::log10(1.123);
    constexpr f64 log1p_result = bl::log1p(0.123);
    constexpr f64 logb_result  = bl::logb(12.5);
    constexpr int ilogb_result = bl::ilogb(12.5);
                                      
    constexpr f64 sin_result   = bl::sin(0.123);
    constexpr f64 cos_result   = bl::cos(0.123);
    constexpr f64 tan_result   = bl::tan(0.123);
    constexpr f64 asin_result  = bl::asin(0.123);
    constexpr f64 acos_result  = bl::acos(0.123);
    constexpr f64 atan_result  = bl::atan(0.123);
    constexpr f64 atan2_result = bl::atan2(0.123, 0.456);
                                      
    constexpr f64 sinh_result  = bl::sinh(0.123);
    constexpr f64 cosh_result  = bl::cosh(0.123);
    constexpr f64 tanh_result  = bl::tanh(0.123);
    constexpr f64 asinh_result = bl::asinh(0.123);
    constexpr f64 acosh_result = bl::acosh(1.123);
    constexpr f64 atanh_result = bl::atanh(0.123);
                                      
    constexpr f64 erf_result    = bl::erf(0.123);
    constexpr f64 erfc_result   = bl::erfc(0.123);
    constexpr f64 lgamma_result = bl::lgamma(1.123);
    constexpr f64 tgamma_result = bl::tgamma(1.123);
                                      
    constexpr f64 ldexp_result     = bl::ldexp(0.123, 5);
    constexpr f64 scalbn_result    = bl::scalbn(0.123, 5);
    constexpr f64 scalbln_result   = bl::scalbln(0.123, 5L);
    constexpr f64 frexp_result     = [] { int exp{}; return bl::frexp(12.5, &exp); }();
    constexpr int frexp_exp_result = [] { int exp{}; (void)bl::frexp(12.5, &exp); return exp; }();
    constexpr f64 modf_result      = [] { f64 ip{}; return bl::modf(12.5, &ip); }();
    constexpr f64 modf_int_result  = [] { f64 ip{}; (void)bl::modf(12.5, &ip); return ip; }();

    constexpr f64 nextafter_result   = bl::nextafter(0.123, 0.124);
    constexpr f64 nexttoward_result1 = bl::nexttoward(0.123, 0.124L);
    constexpr f64 nexttoward_result2 = bl::nexttoward(0.123, 0.124);

    constexpr f64 scalar_sum =
        abs_result + fma_result
        + floor_result + ceil_result + trunc_result + round_result
        + static_cast<f64>(lround_result) + static_cast<f64>(llround_result)
        + roundeven_result
        + fmod_result + remainder_result + remquo_result1 + static_cast<f64>(remquo_result2)
        + fmin_result + fmax_result + fdim_result + copysign_result
        + sqrt_result + cbrt_result + hypot_result + pow_result + pow10_result
        + exp_result + exp2_result + expm1_result
        + log_result + log2_result + log10_result + log1p_result + logb_result + static_cast<f64>(ilogb_result)
        + sin_result + cos_result + tan_result + asin_result + acos_result + atan_result + atan2_result
        + sinh_result + cosh_result + tanh_result + asinh_result + acosh_result + atanh_result
        + erf_result + erfc_result + lgamma_result + tgamma_result
        + ldexp_result + scalbn_result + scalbln_result
        + frexp_result + static_cast<f64>(frexp_exp_result) + modf_result + modf_int_result
        + nextafter_result + nexttoward_result1 + nexttoward_result2;

    return scalar_sum;
}

consteval fdd fdd_test()
{
    constexpr fdd abs_result = bl::abs(-0.123_dd);
    constexpr fdd fma_result = bl::fma(0.123_dd, 4.56_dd, -0.75_dd);
                                      
    constexpr fdd floor_result     = bl::floor(2.75_dd);
    constexpr fdd ceil_result      = bl::ceil(2.25_dd);
    constexpr fdd trunc_result     = bl::trunc(-2.75_dd);
    constexpr fdd round_result     = bl::round(2.5_dd);
    constexpr long lround_result    = bl::lround(2.5_dd);
    constexpr llong llround_result  = bl::llround(2.5_dd);
    constexpr fdd roundeven_result = bl::roundeven(2.5_dd);
                                      
    constexpr fdd fmod_result      = bl::fmod(5.5_dd, 2.0_dd);
    constexpr fdd remainder_result = bl::remainder(5.5_dd, 2.0_dd);
    constexpr fdd remquo_result1   = [] { int quo{}; return bl::remquo(5.5_dd, 2.0_dd, &quo); }();
    constexpr int remquo_result2    = [] { int quo{}; (void)bl::remquo(5.5_dd, 2.0_dd, &quo); return quo; }();
                                      
    constexpr fdd fmin_result     = bl::fmin(-0.25_dd, 0.5_dd);
    constexpr fdd fmax_result     = bl::fmax(-0.25_dd, 0.5_dd);
    constexpr fdd fdim_result     = bl::fdim(1.25_dd, 0.75_dd);
    constexpr fdd copysign_result = bl::copysign(0.125_dd, -1.0_dd);
                                      
    constexpr fdd sqrt_result  = bl::sqrt(0.123_dd);
    constexpr fdd cbrt_result  = bl::cbrt(0.123_dd);
    constexpr fdd hypot_result = bl::hypot(0.3_dd, 0.4_dd);
    constexpr fdd pow_result   = bl::pow(0.123_dd, 4.56_dd);
    constexpr fdd pow10_result = bl::pow(10.0_dd, 2);
                                      
    constexpr fdd exp_result   = bl::exp(0.123_dd);
    constexpr fdd exp2_result  = bl::exp2(0.123_dd);
    constexpr fdd expm1_result = bl::expm1(0.123_dd);
    constexpr fdd log_result   = bl::log(1.123_dd);
    constexpr fdd log2_result  = bl::log2(1.123_dd);
    constexpr fdd log10_result = bl::log10(1.123_dd);
    constexpr fdd log1p_result = bl::log1p(0.123_dd);
    constexpr fdd logb_result  = bl::logb(12.5_dd);
    constexpr int ilogb_result  = bl::ilogb(12.5_dd);
                                      
    constexpr fdd sin_result   = bl::sin(0.123_dd);
    constexpr fdd cos_result   = bl::cos(0.123_dd);
    constexpr fdd tan_result   = bl::tan(0.123_dd);
    constexpr fdd asin_result  = bl::asin(0.123_dd);
    constexpr fdd acos_result  = bl::acos(0.123_dd);
    constexpr fdd atan_result  = bl::atan(0.123_dd);
    constexpr fdd atan2_result = bl::atan2(0.123_dd, 0.456_dd);
                                      
    constexpr fdd sinh_result  = bl::sinh(0.123_dd);
    constexpr fdd cosh_result  = bl::cosh(0.123_dd);
    constexpr fdd tanh_result  = bl::tanh(0.123_dd);
    constexpr fdd asinh_result = bl::asinh(0.123_dd);
    constexpr fdd acosh_result = bl::acosh(1.123_dd);
    constexpr fdd atanh_result = bl::atanh(0.123_dd);
                                      
    constexpr fdd erf_result    = bl::erf(0.123_dd);
    constexpr fdd erfc_result   = bl::erfc(0.123_dd);
    constexpr fdd lgamma_result = bl::lgamma(1.123_dd);
    constexpr fdd tgamma_result = bl::tgamma(1.123_dd);
                                      
    constexpr fdd ldexp_result    = bl::ldexp(0.123_dd, 5);
    constexpr fdd scalbn_result   = bl::scalbn(0.123_dd, 5);
    constexpr fdd scalbln_result  = bl::scalbln(0.123_dd, 5L);
    constexpr fdd frexp_result    = [] { int exp{}; return bl::frexp(12.5_dd, &exp); }();
    constexpr int frexp_exp_result = [] { int exp{}; (void)bl::frexp(12.5_dd, &exp); return exp; }();
    constexpr fdd modf_result     = [] { fdd ip{}; return bl::modf(12.5_dd, &ip); }();
    constexpr fdd modf_int_result = [] { fdd ip{}; (void)bl::modf(12.5_dd, &ip); return ip; }();

    constexpr fdd nextafter_result   = bl::nextafter(0.123_dd, 0.124_dd);
    constexpr fdd nexttoward_result1 = bl::nexttoward(0.123_dd, 0.124L);
    constexpr fdd nexttoward_result2 = bl::nexttoward(0.123_dd, 0.124_dd);

    constexpr fdd scalar_sum =
        abs_result + fma_result
        + floor_result + ceil_result + trunc_result + round_result
        + fdd{ static_cast<double>(lround_result) } + fdd{ static_cast<double>(llround_result) }
        + roundeven_result
        + fmod_result + remainder_result + remquo_result1 + fdd{ static_cast<double>(remquo_result2) }
        + fmin_result + fmax_result + fdim_result + copysign_result
        + sqrt_result + cbrt_result + hypot_result + pow_result + pow10_result
        + exp_result + exp2_result + expm1_result
        + log_result + log2_result + log10_result + log1p_result + logb_result + fdd{ static_cast<double>(ilogb_result) }
        + sin_result + cos_result + tan_result + asin_result + acos_result + atan_result + atan2_result
        + sinh_result + cosh_result + tanh_result + asinh_result + acosh_result + atanh_result
        + erf_result + erfc_result + lgamma_result + tgamma_result
        + ldexp_result + scalbn_result + scalbln_result
        + frexp_result + fdd{ static_cast<double>(frexp_exp_result) } + modf_result + modf_int_result
        + nextafter_result + nexttoward_result1 + nexttoward_result2;

    return scalar_sum;
}

consteval fqd fqd_test()
{
    constexpr fqd abs_result = bl::abs(-0.123_qd);
    constexpr fqd fma_result = bl::fma(0.123_qd, 4.56_qd, -0.75_qd);

    constexpr fqd floor_result     = bl::floor(2.75_qd);
    constexpr fqd ceil_result      = bl::ceil(2.25_qd);
    constexpr fqd trunc_result     = bl::trunc(-2.75_qd);
    constexpr fqd round_result     = bl::round(2.5_qd);
    constexpr long lround_result    = bl::lround(2.5_qd);
    constexpr llong llround_result  = bl::llround(2.5_qd);
    constexpr fqd roundeven_result = bl::roundeven(2.5_qd);

    constexpr fqd fmod_result      = bl::fmod(5.5_qd, 2.0_qd);
    constexpr fqd remainder_result = bl::remainder(5.5_qd, 2.0_qd);
    constexpr fqd remquo_result1   = [] { int quo{}; return bl::remquo(5.5_qd, 2.0_qd, &quo); }();
    constexpr int remquo_result2    = [] { int quo{}; (void)bl::remquo(5.5_qd, 2.0_qd, &quo); return quo; }();

    constexpr fqd fmin_result     = bl::fmin(-0.25_qd, 0.5_qd);
    constexpr fqd fmax_result     = bl::fmax(-0.25_qd, 0.5_qd);
    constexpr fqd fdim_result     = bl::fdim(1.25_qd, 0.75_qd);
    constexpr fqd copysign_result = bl::copysign(0.125_qd, -1.0_qd);
                                      
    constexpr fqd sqrt_result  = bl::sqrt(0.123_qd);
    constexpr fqd cbrt_result  = bl::cbrt(0.123_qd);
    constexpr fqd hypot_result = bl::hypot(0.3_qd, 0.4_qd);
    constexpr fqd pow_result   = bl::pow(0.123_qd, 4.56_qd);
    constexpr fqd pow10_result = bl::pow(10.0_qd, 2);
                                      
    constexpr fqd exp_result   = bl::exp(0.123_qd);
    constexpr fqd exp2_result  = bl::exp2(0.123_qd);
    constexpr fqd expm1_result = bl::expm1(0.123_qd);
    constexpr fqd log_result   = bl::log(1.123_qd);
    constexpr fqd log2_result  = bl::log2(1.123_qd);
    constexpr fqd log10_result = bl::log10(1.123_qd);
    constexpr fqd log1p_result = bl::log1p(0.123_qd);
    constexpr fqd logb_result  = bl::logb(12.5_qd);
    constexpr int ilogb_result  = bl::ilogb(12.5_qd);
                                           
    constexpr fqd sin_result   = bl::sin(0.123_qd);
    constexpr fqd cos_result   = bl::cos(0.123_qd);
    constexpr fqd tan_result   = bl::tan(0.123_qd);
    constexpr fqd asin_result  = bl::asin(0.123_qd);
    constexpr fqd acos_result  = bl::acos(0.123_qd);
    constexpr fqd atan_result  = bl::atan(0.123_qd);
    constexpr fqd atan2_result = bl::atan2(0.123_qd, 0.456_qd);
                                           
    constexpr fqd sinh_result  = bl::sinh(0.123_qd);
    constexpr fqd cosh_result  = bl::cosh(0.123_qd);
    constexpr fqd tanh_result  = bl::tanh(0.123_qd);
    constexpr fqd asinh_result = bl::asinh(0.123_qd);
    constexpr fqd acosh_result = bl::acosh(1.123_qd);
    constexpr fqd atanh_result = bl::atanh(0.123_qd);
                                      
    constexpr fqd erf_result    = bl::erf(0.123_qd);
    constexpr fqd erfc_result   = bl::erfc(0.123_qd);
    constexpr fqd lgamma_result = bl::lgamma(1.123_qd);
    constexpr fqd tgamma_result = bl::tgamma(1.123_qd);
                                      
    constexpr fqd ldexp_result    = bl::ldexp(0.123_qd, 5);
    constexpr fqd scalbn_result   = bl::scalbn(0.123_qd, 5);
    constexpr fqd scalbln_result  = bl::scalbln(0.123_qd, 5L);
    constexpr fqd frexp_result    = [] { int exp{}; return bl::frexp(12.5_qd, &exp); }();
    constexpr int frexp_exp_result = [] { int exp{}; (void)bl::frexp(12.5_qd, &exp); return exp; }();
    constexpr fqd modf_result     = [] { fqd ip{}; return bl::modf(12.5_qd, &ip); }();
    constexpr fqd modf_int_result = [] { fqd ip{}; (void)bl::modf(12.5_qd, &ip); return ip; }();

    constexpr fqd nextafter_result   = bl::nextafter(0.123_qd, 0.124_qd);
    constexpr fqd nexttoward_result1 = bl::nexttoward(0.123_qd, 0.124L);
    constexpr fqd nexttoward_result2 = bl::nexttoward(0.123_qd, 0.124_qd);

    constexpr fqd scalar_sum =
        abs_result + fma_result
        + floor_result + ceil_result + trunc_result + round_result
        + fqd{ static_cast<double>(lround_result) } + fqd{ static_cast<double>(llround_result) }
        + roundeven_result
        + fmod_result + remainder_result + remquo_result1 + fqd{ static_cast<double>(remquo_result2) }
        + fmin_result + fmax_result + fdim_result + copysign_result
        + sqrt_result + cbrt_result + hypot_result + pow_result + pow10_result
        + exp_result + exp2_result + expm1_result
        + log_result + log2_result + log10_result + log1p_result + logb_result + fqd{ static_cast<double>(ilogb_result) }
        + sin_result + cos_result + tan_result + asin_result + acos_result + atan_result + atan2_result
        + sinh_result + cosh_result + tanh_result + asinh_result + acosh_result + atanh_result
        + erf_result + erfc_result + lgamma_result + tgamma_result
        + ldexp_result + scalbn_result + scalbln_result
        + frexp_result + fqd{ static_cast<double>(frexp_exp_result) } + modf_result + modf_int_result
        + nextafter_result + nexttoward_result1 + nexttoward_result2;

    return scalar_sum;
}

int main()
{
    constexpr f32 result_f32 = f32_test();
    constexpr f64 result_f64 = f64_test();
    constexpr fdd result_fdd = fdd_test();
    constexpr fqd result_fqd = fqd_test();

    constexpr bool match =
        bl::approx_eq(result_f32, static_cast<f32>(result_f64)) &&
        bl::approx_eq(result_f64, static_cast<f64>(result_fdd)) &&
        bl::approx_eq(result_fdd, static_cast<fdd>(result_fqd));

    std::cout
        << std::fixed
        << std::setprecision(std::numeric_limits<fqd>::digits10)
        << result_f32 << "\n"
        << result_f64 << "\n"
        << result_fdd << "\n"
        << result_fqd;

    return match ? 0 : 1;
}
