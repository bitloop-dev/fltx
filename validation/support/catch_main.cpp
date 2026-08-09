#include <catch2/catch_session.hpp>

#include "config_banner.hpp"

#ifndef FLTX_TESTS_NAME
#define FLTX_TESTS_NAME "tests"
#endif

int main(int argc, char* argv[])
{
    fltx::tests::support::print_config_banner(FLTX_TESTS_NAME);
    return Catch::Session{}.run(argc, argv);
}
