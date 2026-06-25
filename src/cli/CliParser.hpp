#pragma once

#include "../core/Config.hpp"
#include <CLI/CLI.hpp>

namespace fenrir {
namespace cli {

class CliParser {
public:
    CliParser();


    core::Config parse(int argc, char** argv);


    CLI::App& app() { return m_app; }

private:
    CLI::App m_app;
    void setupOptions();
};

}
}
