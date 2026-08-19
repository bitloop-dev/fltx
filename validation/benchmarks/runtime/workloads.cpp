#include "../../support/tlfloat_ops.hpp"
#include "runner.hpp"

#include <fltx/math.h>

#include <array>

namespace fltx::tests::benchmark
{
    namespace
    {
        template<class Float>
        using qd_value = std::conditional_t<std::is_same_v<Float, bl::f128>,
                                            implementations::qd_f128, implementations::qd_f256>;

        template<class Float>
        using boost_value = std::conditional_t<std::is_same_v<Float, bl::f128>,
                                               implementations::cppdd, implementations::mpfr64>;

        template<class Float>
        using tlfloat_value =
            std::conditional_t<std::is_same_v<Float, bl::f128>, implementations::tl_f128,
                               implementations::tl_f256>;

        template<class Value> [[nodiscard]] Value workload_sin(const Value& value)
        {
            using boost::multiprecision::sin;
            return sin(value);
        }

        template<class Value> [[nodiscard]] Value workload_cos(const Value& value)
        {
            using boost::multiprecision::cos;
            return cos(value);
        }

        template<class Value> [[nodiscard]] Value workload_exp(const Value& value)
        {
            using boost::multiprecision::exp;
            return exp(value);
        }

        template<class Value> [[nodiscard]] Value workload_log(const Value& value)
        {
            using boost::multiprecision::log;
            return log(value);
        }

        template<class Value> [[nodiscard]] Value workload_sqrt(const Value& value)
        {
            using boost::multiprecision::sqrt;
            return sqrt(value);
        }

        template<class Value>
        [[nodiscard]] Value workload_atan2(const Value& y, const Value& x)
        {
            using boost::multiprecision::atan2;
            return atan2(y, x);
        }

        template<class Value>
        [[nodiscard]] Value matrix_vector_4x4_trial(std::size_t batch)
        {
            Value x{0.5 + static_cast<double>(batch % 31) * 0x1p-20};
            Value y{-0.75};
            Value z{1.25};
            Value w{1.0};

            const Value m00{1.0009765625};
            const Value m01{-0.00048828125};
            const Value m02{0.000244140625};
            const Value m03{0.0001220703125};
            const Value m10{0.000244140625};
            const Value m11{0.99951171875};
            const Value m12{-0.0001220703125};
            const Value m13{0.000244140625};
            const Value m20{-0.0001220703125};
            const Value m21{0.000244140625};
            const Value m22{1.00048828125};
            const Value m23{-0.00048828125};
            const Value m30{0.00006103515625};
            const Value m31{-0.0001220703125};
            const Value m32{0.000244140625};
            const Value m33{0.999755859375};
            const Value t0{0.00006103515625};
            const Value t1{-0.000030517578125};
            const Value t2{0.0000152587890625};
            const Value t3{-0.00000762939453125};

            for (std::size_t i = 0; i < 64; ++i)
            {
                const Value next_x{m00 * x + m01 * y + m02 * z + m03 * w + t0};
                const Value next_y{m10 * x + m11 * y + m12 * z + m13 * w + t1};
                const Value next_z{m20 * x + m21 * y + m22 * z + m23 * w + t2};
                const Value next_w{m30 * x + m31 * y + m32 * z + m33 * w + t3};
                x = next_x;
                y = next_y;
                z = next_z;
                w = next_w;
            }
            return Value{x + y + z + w};
        }

        template<class Value>
        [[nodiscard]] Value horner_polynomial_trial(std::size_t batch)
        {
            // Expanded coefficients of (x - 1)^12 provide a familiar
            // Horner evaluation with controlled cancellation near x = 1.
            const std::array<Value, 13> coefficients{
                Value{1.0},   Value{-12.0}, Value{66.0},  Value{-220.0}, Value{495.0},
                Value{-792.0}, Value{924.0}, Value{-792.0}, Value{495.0}, Value{-220.0},
                Value{66.0},  Value{-12.0}, Value{1.0}
            };
            Value x{0.875 + static_cast<double>(batch % 31) * 0x1p-20};
            const Value step{0x1p-20};
            Value total{0.0};

            for (std::size_t repetition = 0; repetition < 64; ++repetition)
            {
                Value result{coefficients.front()};
                for (std::size_t i = 1; i < coefficients.size(); ++i)
                    result = result * x + coefficients[i];
                total += result;
                x += step;
            }
            return total;
        }

        template<class Value>
        [[nodiscard]] Value newton_root_trial(std::size_t batch)
        {
            const Value a{0.25 + static_cast<double>(batch % 31) * 0x1p-20};
            const Value three{3.0};
            const Value one{1.0};
            Value x{1.25};

            for (std::size_t i = 0; i < 8; ++i)
            {
                const Value x_squared{x * x};
                const Value numerator{x_squared * x - x - a};
                const Value denominator{three * x_squared - one};
                x -= numerator / denominator;
            }
            return x;
        }

        template<class Value, class Exp, class Log>
        [[nodiscard]] Value log_sum_exp_trial(
            std::size_t batch,
            Exp evaluate_exp,
            Log evaluate_log)
        {
            std::array<Value, 8> values{
                Value{-4.0}, Value{-2.5}, Value{-1.0}, Value{0.25},
                Value{1.5}, Value{3.0}, Value{4.25},
                Value{5.0 + static_cast<double>(batch % 31) * 0x1p-20}
            };
            const Value step{0x1p-18};
            Value total{0.0};

            for (std::size_t repetition = 0; repetition < values.size(); ++repetition)
            {
                Value maximum{values.front()};
                for (std::size_t i = 1; i < values.size(); ++i)
                {
                    if (values[i] > maximum)
                        maximum = values[i];
                }

                Value sum{0.0};
                for (const Value& value : values)
                {
                    const Value shifted{value - maximum};
                    sum += evaluate_exp(shifted);
                }
                total += maximum + evaluate_log(sum);
                values[repetition] += step;
            }
            return total;
        }

        template<class Value, class Sin, class Cos, class Sqrt, class Atan2>
        [[nodiscard]] Value haversine_trial(
            std::size_t batch,
            Sin evaluate_sin,
            Cos evaluate_cos,
            Sqrt evaluate_sqrt,
            Atan2 evaluate_atan2)
        {
            Value latitude1{0.4 + static_cast<double>(batch % 31) * 0x1p-20};
            Value latitude2{0.7};
            Value longitude1{-1.2};
            Value longitude2{0.3};
            const Value half{0.5};
            const Value one{1.0};
            const Value two{2.0};
            const Value step{0x1p-18};
            Value total{0.0};

            for (std::size_t i = 0; i < 8; ++i)
            {
                const Value delta_latitude{latitude2 - latitude1};
                const Value delta_longitude{longitude2 - longitude1};
                const Value half_delta_latitude{delta_latitude * half};
                const Value half_delta_longitude{delta_longitude * half};
                const Value sin_latitude{evaluate_sin(half_delta_latitude)};
                const Value sin_longitude{evaluate_sin(half_delta_longitude)};
                const Value haversine{
                    sin_latitude * sin_latitude +
                    evaluate_cos(latitude1) * evaluate_cos(latitude2) *
                        sin_longitude * sin_longitude
                };
                const Value complement{one - haversine};
                total += two * evaluate_atan2(
                                   evaluate_sqrt(haversine),
                                   evaluate_sqrt(complement));
                latitude1 += step;
                longitude2 -= step;
            }
            return total;
        }

        template<class Float> void run_workloads(csv_writer& output, const options& settings)
        {
            runner<Float> run(output, settings);
            using qd = qd_value<Float>;
            using boost_type = boost_value<Float>;
            using tl_type = tlfloat_value<Float>;
            constexpr std::size_t repetitions = 64;

            run.workload(
                "chained_arithmetic", repetitions,
                [](std::size_t batch) {
                    Float x{1.125 + static_cast<double>(batch % 31) * 0x1p-20};
                    const Float a{0.75};
                    const Float b{1.0009765625};
                    for (std::size_t i = 0; i < repetitions; ++i)
                        x = ((x * b + a) / (b + 0.5)) - a * 0.25;
                    return x;
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         qd x{1.125 + static_cast<double>(batch % 31) * 0x1p-20};
                         const qd a{0.75};
                         const qd b{1.0009765625};
                         for (std::size_t i = 0; i < repetitions; ++i)
                             x = ((x * b + a) / (b + 0.5)) - a * 0.25;
                         return x;
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         boost_type x{1.125 + static_cast<double>(batch % 31) * 0x1p-20};
                         const boost_type a{0.75};
                         const boost_type b{1.0009765625};
                         for (std::size_t i = 0; i < repetitions; ++i)
                             x = ((x * b + a) / (b + 0.5)) - a * 0.25;
                         return x;
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         tl_type x{1.125 + static_cast<double>(batch % 31) * 0x1p-20};
                         const tl_type a{0.75};
                         const tl_type b{1.0009765625};
                         for (std::size_t i = 0; i < repetitions; ++i)
                             x = ((x * b + a) / (b + 0.5)) - a * 0.25;
                         return x;
                     }));

            run.workload(
                "affine_trig", repetitions,
                [](std::size_t batch) {
                    Float x{0.125 + static_cast<double>(batch % 31) * 0x1p-20};
                    for (std::size_t i = 0; i < repetitions; ++i)
                        x = bl::sin(x * 1.001 + 0.125) + bl::cos(x - 0.25);
                    return x;
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         qd x{0.125 + static_cast<double>(batch % 31) * 0x1p-20};
                         for (std::size_t i = 0; i < repetitions; ++i)
                             x = ::sin(x * 1.001 + 0.125) + ::cos(x - 0.25);
                         return x;
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         boost_type x{0.125 + static_cast<double>(batch % 31) * 0x1p-20};
                         for (std::size_t i = 0; i < repetitions; ++i)
                         {
                             x = workload_sin(x * 1.001 + 0.125) + workload_cos(x - 0.25);
                         }
                         return x;
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         tl_type x{0.125 + static_cast<double>(batch % 31) * 0x1p-20};
                         for (std::size_t i = 0; i < repetitions; ++i)
                         {
                             x = tlfloat_ops::sin(x * 1.001 + 0.125) + tlfloat_ops::cos(x - 0.25);
                         }
                         return x;
                     }));

            run.workload(
                "mandelbrot", repetitions,
                [](std::size_t batch) {
                    Float x{0.0};
                    Float y{0.0};
                    const Float cx{-0.743643887037151 + static_cast<double>(batch % 31) * 0x1p-24};
                    const Float cy{0.131825904205330};
                    for (std::size_t i = 0; i < repetitions; ++i)
                    {
                        const Float next_x{x * x - y * y + cx};
                        y = 2.0 * x * y + cy;
                        x = next_x;
                    }
                    return Float{x + y};
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         qd x{0.0};
                         qd y{0.0};
                         const qd cx{-0.743643887037151 +
                                     static_cast<double>(batch % 31) * 0x1p-24};
                         const qd cy{0.131825904205330};
                         for (std::size_t i = 0; i < repetitions; ++i)
                         {
                             const qd next_x = x * x - y * y + cx;
                             y = qd{2} * x * y + cy;
                             x = next_x;
                         }
                         return qd{x + y};
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         boost_type x{0.0};
                         boost_type y{0.0};
                         const boost_type cx{-0.743643887037151 +
                                             static_cast<double>(batch % 31) * 0x1p-24};
                         const boost_type cy{0.131825904205330};
                         for (std::size_t i = 0; i < repetitions; ++i)
                         {
                             const boost_type next_x = x * x - y * y + cx;
                             y = boost_type{2} * x * y + cy;
                             x = next_x;
                         }
                         return boost_type{x + y};
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         tl_type x{0.0};
                         tl_type y{0.0};
                         const tl_type cx{-0.743643887037151 +
                                          static_cast<double>(batch % 31) * 0x1p-24};
                         const tl_type cy{0.131825904205330};
                         for (std::size_t i = 0; i < repetitions; ++i)
                         {
                             const tl_type next_x = x * x - y * y + cx;
                             y = tl_type{2} * x * y + cy;
                             x = next_x;
                         }
                         return tl_type{x + y};
                     }));

            run.workload(
                "matrix_vector_4x4", repetitions,
                [](std::size_t batch) {
                    return matrix_vector_4x4_trial<Float>(batch);
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return matrix_vector_4x4_trial<qd>(batch);
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return matrix_vector_4x4_trial<boost_type>(batch);
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return matrix_vector_4x4_trial<tl_type>(batch);
                     }));

            run.workload(
                "horner_polynomial", repetitions,
                [](std::size_t batch) {
                    return horner_polynomial_trial<Float>(batch);
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return horner_polynomial_trial<qd>(batch);
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return horner_polynomial_trial<boost_type>(batch);
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return horner_polynomial_trial<tl_type>(batch);
                     }));

            constexpr std::size_t newton_iterations = 8;
            run.workload(
                "newton_root", newton_iterations,
                [](std::size_t batch) {
                    return newton_root_trial<Float>(batch);
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return newton_root_trial<qd>(batch);
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return newton_root_trial<boost_type>(batch);
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return newton_root_trial<tl_type>(batch);
                     }));

            constexpr std::size_t formula_repetitions = 8;
            run.workload(
                "log_sum_exp", formula_repetitions,
                [](std::size_t batch) {
                    return log_sum_exp_trial<Float>(
                        batch,
                        [](const auto& value) { return bl::exp(value); },
                        [](const auto& value) { return bl::log(value); });
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return log_sum_exp_trial<qd>(
                             batch,
                             [](const auto& value) { return ::exp(value); },
                             [](const auto& value) { return ::log(value); });
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return log_sum_exp_trial<boost_type>(
                             batch,
                             [](const auto& value) { return workload_exp(value); },
                             [](const auto& value) { return workload_log(value); });
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return log_sum_exp_trial<tl_type>(
                             batch,
                             [](const auto& value) { return tlfloat_ops::exp(value); },
                             [](const auto& value) { return tlfloat_ops::log(value); });
                     }));

            run.workload(
                "haversine", formula_repetitions,
                [](std::size_t batch) {
                    return haversine_trial<Float>(
                        batch,
                        [](const auto& value) { return bl::sin(value); },
                        [](const auto& value) { return bl::cos(value); },
                        [](const auto& value) { return bl::sqrt(value); },
                        [](const auto& y, const auto& x) { return bl::atan2(y, x); });
                },
                task(implementations::qdpp_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return haversine_trial<qd>(
                             batch,
                             [](const auto& value) { return ::sin(value); },
                             [](const auto& value) { return ::cos(value); },
                             [](const auto& value) { return ::sqrt(value); },
                             [](const auto& y, const auto& x) { return ::atan2(y, x); });
                     }),
                task(implementations::boost_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return haversine_trial<boost_type>(
                             batch,
                             [](const auto& value) { return workload_sin(value); },
                             [](const auto& value) { return workload_cos(value); },
                             [](const auto& value) { return workload_sqrt(value); },
                             [](const auto& y, const auto& x) {
                                 return workload_atan2(y, x);
                             });
                     }),
                task(implementations::tlfloat_identity<Float>, "same workload",
                     [](std::size_t batch) {
                         return haversine_trial<tl_type>(
                             batch,
                             [](const auto& value) { return tlfloat_ops::sin(value); },
                             [](const auto& value) { return tlfloat_ops::cos(value); },
                             [](const auto& value) { return tlfloat_ops::sqrt(value); },
                             [](const auto& y, const auto& x) {
                                 return tlfloat_ops::atan2(y, x);
                             });
                     }));
        }
    } // namespace

    void run_workloads_f128(csv_writer& output, const options& settings)
    {
        run_workloads<bl::f128>(output, settings);
    }

    void run_workloads_f256(csv_writer& output, const options& settings)
    {
        run_workloads<bl::f256>(output, settings);
    }
} // namespace fltx::tests::benchmark
