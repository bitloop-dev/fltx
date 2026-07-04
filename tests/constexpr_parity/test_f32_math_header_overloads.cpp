#include <type_traits>

#include <fltx/f32_math.h>

namespace
{
    static_assert(std::is_same_v<decltype(bl::sqrt(9)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::sin(1)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::isnan(1)), bool>);
    static_assert(std::is_same_v<decltype(bl::hypot(3, 4)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::hypot(3.0f, 4.0)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::atan2(1.0f, 2.0)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::fmin(1.0f, 2.0)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::fma(1.0f, 2.0, 3.0)), bl::f64>);
    static_assert(std::is_same_v<decltype(bl::nexttoward(1.0f, 2.0)), bl::f32>);
}
