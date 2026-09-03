#include <iostream>
#include <iomanip>
#include <limits>

#include <fltx/io.h>
#include <fltx/fqd_math.h>
#include <fltx/numbers.h>

using namespace bl::types;

template<typename T>
bool roundtrip(std::string_view name)
{
    std::cout << "\n----- " << name << " -----\n";

    // ----- try switching format mode -----
    constexpr auto mode = std::ios_base::fixed;
    // constexpr auto mode = std::ios_base::scientific;                         // scientific
    // constexpr auto mode = std::ios_base::fmtflags{};                         // defaultfloat
    // constexpr auto mode = std::ios_base::fixed | std::ios_base::scientific;  // hexfloat

    // ----- try switching base flags -----
    constexpr auto base_flags =
        std::ios_base::showpoint |
        std::ios_base::showpos |
        std::ios_base::uppercase;

    // sets cout stream flags
    constexpr auto flags = mode | base_flags;
    std::cout.setf(flags & std::ios_base::floatfield, std::ios_base::floatfield);
    std::cout.setf(flags & base_flags, base_flags);

    // use enough decimal digits for an exact T text round-trip
    constexpr int digits = std::numeric_limits<T>::max_digits10;
    std::cout << std::setprecision(digits);

    // calculate 10+pi/2
    constexpr T value = 10e30 + std::numbers::pi_v<T> / 2;
    std::cout << "value:\t\t\t" << value << "\n\n";

    // compile-time text round-trip
    {
        constexpr auto txt_static = bl::to_static_string(value, digits, flags);
        std::cout << "txt (static):\t\t" << txt_static << "\n";

        // parse back to T value
        constexpr T parsed_static = bl::parse<T>(txt_static);
        std::cout << "parsed (static):\t" << parsed_static << "\n\n";

        if (parsed_static != value)
            return 1;
    }

    // runtime text round-trip
    {
        auto txt_runtime = bl::to_string(value, digits, flags);
        std::cout << "txt (runtime):\t\t" << txt_runtime << "\n";

        // parse back to T value
        T parsed_runtime = bl::parse<T>(txt_runtime);
        std::cout << "parsed (runtime):\t" << parsed_runtime << "\n\n";

        if (parsed_runtime != value)
            return 1;
    }

    return 0;
}

int main()
{
    roundtrip<f32>("f32");
    roundtrip<f64>("f64");
    roundtrip<fdd>("fdd");
    roundtrip<fqd>("fqd");
}

// ----- Verify every viable stream flag combination round-trips at max_digits10 -----

template<std::ios_base::fmtflags flags>
consteval bool static_roundtrip_ok()
{
    constexpr int digits = std::numeric_limits<fqd>::max_digits10;
    constexpr fqd value = 10 + std::numbers::pi_v<fqd> / 2;
    constexpr auto txt_static = bl::to_static_string(value, digits, flags);
    constexpr fqd parsed_static = bl::parse<fqd>(txt_static);
    return parsed_static == value;
}

template<std::ios_base::fmtflags... flags>
consteval bool all_static_roundtrips_ok()
{
    return (static_roundtrip_ok<flags>() && ...);
}

static_assert(all_static_roundtrips_ok <
    std::ios_base::fmtflags{},
    std::ios_base::showpoint,
    std::ios_base::showpos,
    std::ios_base::uppercase,
    std::ios_base::showpoint | std::ios_base::showpos,
    std::ios_base::showpoint | std::ios_base::uppercase,
    std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::showpoint | std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::fixed,
    std::ios_base::fixed | std::ios_base::showpoint,
    std::ios_base::fixed | std::ios_base::showpos,
    std::ios_base::fixed | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::showpoint | std::ios_base::showpos,
    std::ios_base::fixed | std::ios_base::showpoint | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::showpoint | std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::scientific,
    std::ios_base::scientific | std::ios_base::showpoint,
    std::ios_base::scientific | std::ios_base::showpos,
    std::ios_base::scientific | std::ios_base::uppercase,
    std::ios_base::scientific | std::ios_base::showpoint | std::ios_base::showpos,
    std::ios_base::scientific | std::ios_base::showpoint | std::ios_base::uppercase,
    std::ios_base::scientific | std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::scientific | std::ios_base::showpoint | std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::scientific,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::showpoint,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::showpos,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::showpoint | std::ios_base::showpos,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::showpoint | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::showpos | std::ios_base::uppercase,
    std::ios_base::fixed | std::ios_base::scientific | std::ios_base::showpoint | std::ios_base::showpos | std::ios_base::uppercase > ());
