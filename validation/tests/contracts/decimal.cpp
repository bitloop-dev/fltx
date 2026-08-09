#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

#include <fltx/detail/common_decimal.h>

namespace
{
    using bl::detail::exact_decimal::biguint;

    void check_reciprocal_power5_table(int significand_bits)
    {
        using namespace bl::detail::exact_decimal;

        for (int exponent = 1;
             exponent <= bl::detail::pow_tables::reciprocal_power5_max_exponent;
             ++exponent)
        {
            const biguint denominator = pow5_big(exponent);
            const auto& cached =
                bl::detail::pow_tables::reciprocal_power5_table[exponent - 1];
            const biguint reciprocal = from_words(
                cached.words,
                bl::detail::pow_tables::reciprocal_power5_word_count);

            biguint reciprocal_predecessor = reciprocal;
            reciprocal_predecessor.sub_small(1);
            biguint binary_scale{ 1 };
            binary_scale.shl_bits(cached.binary_shift);

            const biguint rounded_product = mul_big(reciprocal, denominator);
            const biguint predecessor_product =
                mul_big(reciprocal_predecessor, denominator);

            CAPTURE(exponent, cached.binary_shift);
            CHECK(rounded_product.compare(binary_scale) >= 0);
            CHECK(predecessor_product.compare(binary_scale) < 0);

            biguint denominator_minus_one = denominator;
            denominator_minus_one.sub_small(1);
            biguint denominator_plus_one = denominator;
            denominator_plus_one.add_small(1);

            biguint binary_boundary{ 1 };
            binary_boundary.shl_bits(1 + (exponent * 29) % 211);
            biguint binary_boundary_minus_one = binary_boundary;
            binary_boundary_minus_one.sub_small(1);
            biguint binary_boundary_plus_one = binary_boundary;
            binary_boundary_plus_one.add_small(1);

            biguint wide{ UINT64_C(0xfedcba9876543211) };
            wide.shl_bits((exponent * 17) % 157);
            wide.add_small(static_cast<std::uint32_t>(exponent));

            const std::array numerators{
                biguint{ 1 },
                denominator_minus_one,
                denominator,
                denominator_plus_one,
                binary_boundary_minus_one,
                binary_boundary,
                binary_boundary_plus_one,
                wide
            };

            int reciprocal_cases = 0;
            for (std::size_t sample = 0; sample < numerators.size(); ++sample)
            {
                const biguint& numerator = numerators[sample];
                const int ratio_exp = floor_log2_ratio(numerator, denominator);

                biguint reciprocal_result;
                const bool used_reciprocal =
                    try_extract_rounded_significand_reciprocal(
                        numerator,
                        denominator,
                        exponent,
                        ratio_exp,
                        significand_bits,
                        reciprocal_result);

                biguint exact_numerator = numerator;
                biguint exact_denominator = denominator;
                const biguint exact_result = extract_rounded_significand_chunks(
                    exact_numerator,
                    exact_denominator,
                    ratio_exp,
                    significand_bits);

                CAPTURE(exponent, significand_bits, sample);
                if (used_reciprocal)
                {
                    ++reciprocal_cases;
                    CHECK(reciprocal_result.compare(exact_result) == 0);
                }
            }

            CAPTURE(exponent, significand_bits, reciprocal_cases);
            CHECK(reciprocal_cases >= 4);
        }
    }
}

TEST_CASE("cached reciprocal powers of five match exact quotient extraction",
          "[contracts][io][decimal]")
{
    check_reciprocal_power5_table(159);
    check_reciprocal_power5_table(265);
}
