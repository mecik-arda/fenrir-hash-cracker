#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <optional>
#include <cstdint>

namespace fenrir {
namespace utils {

class FileReader {
public:
    explicit FileReader(const std::string& path);
    ~FileReader();


    std::optional<std::string> nextLine();


    std::vector<std::string> readLines(size_t count);


    uint64_t currentOffset() const;


    void seekTo(uint64_t offset);


    uint64_t fileSize() const;


    bool isOpen() const;

private:
    std::ifstream m_file;
    uint64_t m_fileSize = 0;
};

}
}
