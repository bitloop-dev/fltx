#include "tlfloat_support.hpp"

#include <cstdint>

int main()
{
    static_assert(sizeof(tlfloat::Quad) == 16);
    static_assert(sizeof(tlfloat::Octuple) == 32);

    const tlfloat::Quad quad = tlfloat::Quad{ 1.25 } + tlfloat::Quad{ 0.75 };
    const tlfloat::Octuple octuple =
        tlfloat::Octuple{ 2.5 } * tlfloat::Octuple{ 2.0 };

    const bool headers_work =
        static_cast<double>(quad) == 2.0 &&
        static_cast<double>(octuple) == 5.0;
    const bool library_is_linked = tlfloat_version() != std::uint64_t{ 0 };
    return headers_work && library_is_linked ? 0 : 1;
}
