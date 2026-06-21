#ifndef FLTX_TESTS_METRICS_SAMPLES_INCLUDED
#define FLTX_TESTS_METRICS_SAMPLES_INCLUDED

#include <fltx/detail/common_fp.h>

#include <cmath>
#include <limits>
#include <string_view>
#include <vector>

#include "metrics_types.h"

namespace bl::test::metrics
{
    struct value_sample
    {
        std::string_view label;
        double hi = 0.0;
        double lo = 0.0;
        double x2 = 0.0;
        double x3 = 0.0;
    };

    struct unary_sample
    {
        std::string_view label;
        value_sample x;
    };

    struct binary_sample
    {
        std::string_view label;
        value_sample x;
        value_sample y;
    };

    struct ternary_sample
    {
        std::string_view label;
        value_sample x;
        value_sample y;
        value_sample z;
    };

    struct unary_int_sample
    {
        std::string_view label;
        value_sample x;
        int n = 0;
    };

    template<class T>
    struct binary_value
    {
        T x;
        T y;
    };

    template<class T>
    struct ternary_value
    {
        T x;
        T y;
        T z;
    };

    template<class T>
    struct unary_int_value
    {
        T x;
        int n = 0;
    };

    [[nodiscard]] inline bool sample_value_has_inf(const value_sample& value) noexcept
    {
        return detail::fp::isinf(value.hi) || detail::fp::isinf(value.lo) ||
               detail::fp::isinf(value.x2) || detail::fp::isinf(value.x3);
    }

    [[nodiscard]] inline bool sample_value_has_nan(const value_sample& value) noexcept
    {
        return detail::fp::isnan(value.hi) || detail::fp::isnan(value.lo) ||
               detail::fp::isnan(value.x2) || detail::fp::isnan(value.x3);
    }

    [[nodiscard]] inline bool sample_has_inf(const unary_sample& sample) noexcept
    {
        return sample_value_has_inf(sample.x);
    }

    [[nodiscard]] inline bool sample_has_nan(const unary_sample& sample) noexcept
    {
        return sample_value_has_nan(sample.x);
    }

    [[nodiscard]] inline bool sample_has_inf(const binary_sample& sample) noexcept
    {
        return sample_value_has_inf(sample.x) || sample_value_has_inf(sample.y);
    }

    [[nodiscard]] inline bool sample_has_nan(const binary_sample& sample) noexcept
    {
        return sample_value_has_nan(sample.x) || sample_value_has_nan(sample.y);
    }

    [[nodiscard]] inline bool sample_has_inf(const ternary_sample& sample) noexcept
    {
        return sample_value_has_inf(sample.x) || sample_value_has_inf(sample.y) ||
               sample_value_has_inf(sample.z);
    }

    [[nodiscard]] inline bool sample_has_nan(const ternary_sample& sample) noexcept
    {
        return sample_value_has_nan(sample.x) || sample_value_has_nan(sample.y) ||
               sample_value_has_nan(sample.z);
    }

    [[nodiscard]] inline bool sample_has_inf(const unary_int_sample& sample) noexcept
    {
        return sample_value_has_inf(sample.x);
    }

    [[nodiscard]] inline bool sample_has_nan(const unary_int_sample& sample) noexcept
    {
        return sample_value_has_nan(sample.x);
    }

    template<class T>
    struct frexp_value
    {
        T fraction;
        int exponent = 0;
    };

    template<class Sample>
    struct sample_set
    {
        operation_id operation;
        domain_id domain;
        std::vector<Sample> values;
    };

    [[nodiscard]] inline value_sample special_nan_sample(std::string_view label = "nan") noexcept
    {
        return { label, std::numeric_limits<double>::quiet_NaN(), 0.0 };
    }

    [[nodiscard]] inline value_sample special_pos_inf_sample(std::string_view label = "+inf") noexcept
    {
        return { label, std::numeric_limits<double>::infinity(), 0.0 };
    }

    [[nodiscard]] inline value_sample special_neg_inf_sample(std::string_view label = "-inf") noexcept
    {
        return { label, -std::numeric_limits<double>::infinity(), 0.0 };
    }

    [[nodiscard]] inline value_sample special_pos_zero_sample(std::string_view label = "+0") noexcept
    {
        return { label, 0.0, 0.0 };
    }

    [[nodiscard]] inline value_sample special_neg_zero_sample(std::string_view label = "-0") noexcept
    {
        return { label, std::copysign(0.0, -1.0), 0.0 };
    }

    [[nodiscard]] inline value_sample special_one_sample(std::string_view label = "1") noexcept
    {
        return { label, 1.0, 0.0 };
    }

    [[nodiscard]] inline value_sample special_neg_one_sample(std::string_view label = "-1") noexcept
    {
        return { label, -1.0, 0.0 };
    }

    [[nodiscard]] inline std::vector<unary_sample> make_special_unary_samples()
    {
        return {
            { "nan", special_nan_sample() },
            { "+inf", special_pos_inf_sample() },
            { "-inf", special_neg_inf_sample() },
            { "+0", special_pos_zero_sample() },
            { "-0", special_neg_zero_sample() }
        };
    }

    [[nodiscard]] inline std::vector<binary_sample> make_special_binary_samples()
    {
        const value_sample nan = special_nan_sample();
        const value_sample pinf = special_pos_inf_sample();
        const value_sample ninf = special_neg_inf_sample();
        const value_sample zero = special_pos_zero_sample();
        const value_sample neg_zero = special_neg_zero_sample();
        const value_sample one = special_one_sample();
        const value_sample neg_one = special_neg_one_sample();

        return {
            { "nan left", nan, one },
            { "nan right", one, nan },
            { "both nan", nan, nan },
            { "+inf left", pinf, one },
            { "-inf left", ninf, one },
            { "+inf right", one, pinf },
            { "-inf right", one, ninf },
            { "opposite infinities", pinf, ninf },
            { "matching infinities", pinf, pinf },
            { "zero divisor", one, zero },
            { "zero zero", zero, zero },
            { "signed zero", neg_zero, one },
            { "negative finite", neg_one, pinf }
        };
    }

    [[nodiscard]] inline std::vector<ternary_sample> make_special_ternary_samples()
    {
        const value_sample nan = special_nan_sample();
        const value_sample pinf = special_pos_inf_sample();
        const value_sample ninf = special_neg_inf_sample();
        const value_sample zero = special_pos_zero_sample();
        const value_sample one = special_one_sample();

        return {
            { "nan x", nan, one, one },
            { "nan y", one, nan, one },
            { "nan z", one, one, nan },
            { "inf times zero", pinf, zero, one },
            { "inf plus opposite inf", pinf, one, ninf },
            { "negative inf plus inf", ninf, one, pinf },
            { "inf finite", pinf, one, one }
        };
    }

    [[nodiscard]] inline std::vector<unary_int_sample> make_special_unary_int_samples()
    {
        return {
            { "nan", special_nan_sample(), 4 },
            { "+inf", special_pos_inf_sample(), 4 },
            { "-inf", special_neg_inf_sample(), -4 },
            { "+0", special_pos_zero_sample(), 4 },
            { "-0", special_neg_zero_sample(), -4 }
        };
    }
}

#endif
