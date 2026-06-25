#pragma once

#include "IAttackMode.hpp"
#include <memory>
#include <string>

namespace fenrir {
namespace attack {

class AttackRegistry {
public:

    static std::unique_ptr<IAttackMode> create(const std::string& name);
};

}
}
