#include <array>
#include <charconv>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <string_view>
#include <system_error>

#include <fltx/io.h>
#include <fltx/numbers.h>

using namespace bl;
using namespace bl::literals;

const char* errc_name(std::errc ec)
{
    if (ec == std::errc{})
        return "ok";
    if (ec == std::errc::invalid_argument)
        return "invalid_argument";
    if (ec == std::errc::result_out_of_range)
        return "result_out_of_range";
    if (ec == std::errc::value_too_large)
        return "value_too_large";
    return "other";
}

template<std::size_t N>
std::string_view written_view(const std::array<char, N>& buffer, const char* end)
{
    return { buffer.data(), static_cast<std::size_t>(end - buffer.data()) };
}

int main()
{
    constexpr f256 value = std::numbers::pi_v<f256> + 1_qd / 7_qd;

    std::array<char, 160> buffer{};
    const auto out = bl::to_chars(
        buffer.data(),
        buffer.data() + buffer.size(),
        value,
        std::chars_format::fixed,
        48);

    std::cout << "to_chars fixed status: " << errc_name(out.ec) << "\n";
    std::cout << "text: " << written_view(buffer, out.ptr) << "\n\n";

    f256 parsed{};
    const auto in = bl::from_chars(
        buffer.data(),
        out.ptr,
        parsed,
        std::chars_format::fixed);

    std::cout << "from_chars status: " << errc_name(in.ec) << "\n";
    std::cout << std::setprecision(std::numeric_limits<f256>::digits10);
    std::cout << "parsed: " << parsed << "\n\n";

    constexpr std::string_view packet = "6.02214076e23 mol";
    f256 scanned{};
    const auto scan = bl::from_chars(packet.data(), packet.data() + packet.size(), scanned);

    std::cout << "scanned prefix: " << scanned << "\n";
    std::cout << "consumed chars: " << (scan.ptr - packet.data()) << "\n";
    std::cout << "remaining text: "
              << std::string_view{ scan.ptr, static_cast<std::size_t>(packet.data() + packet.size() - scan.ptr) }
              << "\n\n";

    const auto strict = bl::try_parse<f256>(packet);
    std::cout << "try_parse whole packet status: " << errc_name(strict.ec) << "\n";
    std::cout << "try_parse consumed chars: " << strict.consumed << "\n";
}
