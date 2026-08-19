#ifndef FLTX_TESTS_BENCHMARK_INTEGER_OBSERVER_INCLUDED
#define FLTX_TESTS_BENCHMARK_INTEGER_OBSERVER_INCLUDED

#include <array>
#include <bit>
#include <cstdint>

namespace fltx::tests::benchmark
{
    class integer_observer
    {
    public:
        void add(const std::array<double, 4>& components) noexcept
        {
            lane0_ += std::bit_cast<std::uint64_t>(components[0]);
            lane1_ += std::bit_cast<std::uint64_t>(components[1]);
            lane2_ += std::bit_cast<std::uint64_t>(components[2]);
            lane3_ += std::bit_cast<std::uint64_t>(components[3]);
        }

        [[nodiscard]] std::uint64_t value() const noexcept
        {
            return lane0_ ^
                   std::rotl(lane1_, 13) ^
                   std::rotl(lane2_, 29) ^
                   std::rotl(lane3_, 47);
        }

    private:
        std::uint64_t lane0_ = 0;
        std::uint64_t lane1_ = 0;
        std::uint64_t lane2_ = 0;
        std::uint64_t lane3_ = 0;
    };
}

#endif
