#include "CliParser.hpp"
#include "../core/Constants.hpp"

namespace fenrir {
namespace cli {

CliParser::CliParser()
    : m_app(core::DESCRIPTION)
{
    m_app.set_version_flag("--version,-V", core::VERSION_STRING);
    setupOptions();
}

void CliParser::setupOptions() {

}

core::Config CliParser::parse(int argc, char** argv) {
    try {
        m_app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        throw;
    }

    return core::Config{};
}

}
}
