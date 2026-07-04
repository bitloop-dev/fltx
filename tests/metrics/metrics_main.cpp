#include <catch2/catch_config.hpp>
#include <catch2/catch_session.hpp>

#include "metrics_case_output.h"

int main(int argc, char* argv[])
{
    Catch::Session session;

    Catch::ConfigData config;
    config.runOrder = Catch::TestRunOrder::LexicographicallySorted;
    session.useConfigData(config);

    bl::test::metrics::start_metrics_console_html_capture(Catch::cout());
    const int result = session.run(argc, argv);
    bl::test::metrics::write_and_stop_metrics_console_html_capture(Catch::cout());
    return result;
}
