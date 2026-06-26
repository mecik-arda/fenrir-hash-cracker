#pragma once

#include "TargetHash.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace fenrir {
namespace core {

class IHashEngine {
public:
    virtual ~IHashEngine() = default;


    virtual HashType type() const = 0;


    virtual std::string name() const = 0;


    virtual std::size_t hashSize() const = 0;


    virtual bool supportsSalt() const = 0;



    virtual bool isSlowHash() const = 0;





    virtual std::vector<uint8_t> hash(
        const std::string& input,
        const std::vector<uint8_t>& salt = {}) const = 0;





    virtual void hashBatch(
        const std::vector<std::string>& inputs,
        std::vector<std::vector<uint8_t>>& outputs,
        const std::vector<uint8_t>& salt = {}) const = 0;



    virtual std::string kernelSourcePath() const = 0;


    virtual std::string kernelFunctionName() const = 0;




    virtual double estimatedHashPerSecond(int gpuComputeUnits) const = 0;
};

}
}
