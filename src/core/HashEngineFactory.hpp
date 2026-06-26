#pragma once
#include "IHashEngine.hpp"
#include "TargetHash.hpp"
#include <memory>
#include <string>

namespace fenrir { namespace core {

class HashEngineFactory {
public:
    static std::unique_ptr<IHashEngine> create(HashType type);
    static std::unique_ptr<IHashEngine> create(const std::string& name, bool useSIMD = false);
    static HashType typeFromName(const std::string& name);
};

} }
