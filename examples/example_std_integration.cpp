#include <compare>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <string>
#include <unordered_map>

#include <fltx.h>
#include <fltx/format.h>

using namespace bl;
using namespace bl::literals;

int main()
{
    const fqd pi = std::numbers::pi_v<fqd>;

    std::cout
        << "sizeof(fdd): " << sizeof(fdd) << "\n"
        << "sizeof(fqd): " << sizeof(fqd) << "\n"
        << "fdd digits10: " << std::numeric_limits<fdd>::digits10 << "\n"
        << "fqd digits10: " << std::numeric_limits<fqd>::digits10 << "\n\n";

    std::cout
        << std::setprecision(std::numeric_limits<fqd>::digits10)
        << "std::numbers::pi_v<fqd>: " << pi << "\n\n";

    std::unordered_map<fdd, std::string> labels;
    labels.emplace(0.1_dd, "one tenth");
    labels.emplace(0.25_dd, "one quarter");

    const auto found = labels.find(0.1_dd);
    if (found != labels.end())
        std::cout << "unordered_map lookup for 0.1: " << found->second << "\n\n";

    const fqd nan = std::numeric_limits<fqd>::quiet_NaN();
    const auto ordering = nan <=> pi;
    std::cout << std::boolalpha
              << "NaN comparison is unordered: "
              << (ordering == std::partial_ordering::unordered) << "\n\n";

#if FLTX_HAS_STD_FORMAT
    std::cout << std::format("std::format fixed: {:.40f}\n", pi);
    std::cout << std::format("std::format padded: {:>72.60g}\n", pi);
#else
    std::cout << "std::format is not available in this standard library.\n";
#endif
}
