#include "FileReader.hpp"

#include <stdexcept>

namespace fenrir {
namespace utils {

FileReader::FileReader(const std::string& path) {
    m_file.open(path, std::ios::binary);
    if (!m_file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    m_file.seekg(0, std::ios::end);
    m_fileSize = static_cast<uint64_t>(m_file.tellg());
    m_file.seekg(0, std::ios::beg);
}

FileReader::~FileReader() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

std::optional<std::string> FileReader::nextLine() {
    std::string line;
    if (std::getline(m_file, line)) {

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return line;
    }
    return std::nullopt;
}

std::vector<std::string> FileReader::readLines(size_t count) {
    std::vector<std::string> lines;
    lines.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        auto line = nextLine();
        if (!line) break;
        lines.push_back(std::move(*line));
    }
    return lines;
}

uint64_t FileReader::currentOffset() const {
    return static_cast<uint64_t>(const_cast<std::ifstream&>(m_file).tellg());
}

void FileReader::seekTo(uint64_t offset) {
    m_file.clear();
    m_file.seekg(static_cast<std::streamoff>(offset));
}

uint64_t FileReader::fileSize() const {
    return m_fileSize;
}

bool FileReader::isOpen() const {
    return m_file.is_open();
}

}
}
