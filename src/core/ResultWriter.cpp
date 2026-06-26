#include "ResultWriter.hpp"
#include "../utils/Logger.hpp"

namespace fenrir {
namespace core {

ResultWriter::ResultWriter(const std::string& outputFile, bool append)
{
    auto mode = append ? std::ios::app : std::ios::trunc;
    m_file.open(outputFile, mode);
    if (!m_file.is_open()) {
        utils::Logger::warn("Cannot open output file: " + outputFile);
    }
}

ResultWriter::~ResultWriter() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

void ResultWriter::write(const std::string& hash, const std::string& plaintext) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file << hash << ":" << plaintext << "\n";
        m_file.flush();
    }
    m_count++;
}

void ResultWriter::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file.flush();
    }
}

size_t ResultWriter::count() const {
    return m_count;
}

}
}
