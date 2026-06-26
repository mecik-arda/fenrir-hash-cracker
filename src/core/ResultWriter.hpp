#pragma once

#include <string>
#include <mutex>
#include <fstream>

namespace fenrir {
namespace core {

class ResultWriter {
public:
    explicit ResultWriter(const std::string& outputFile, bool append = false);
    ~ResultWriter();



    void write(const std::string& hash, const std::string& plaintext);


    void flush();


    size_t count() const;

private:
    std::ofstream m_file;
    mutable std::mutex m_mutex;
    size_t m_count = 0;
};

}
}
