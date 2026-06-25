#include "RuleEngine.hpp"
#include <cstring>
#include <algorithm>

namespace fenrir { namespace rules {

std::optional<std::string> RuleEngine::apply(const CompiledRule& rule,
                                              const std::string& word) {
    if (word.length() > 255) return std::nullopt;
    int len = static_cast<int>(word.length());
    std::memcpy(m_buffer, word.data(), len);

    for (uint32_t packed : rule) {
        if (packed == 0) break;
        OpCode op = static_cast<OpCode>(packed & 0xFF);
        uint8_t p0 = static_cast<uint8_t>((packed >> 8) & 0xFF);
        uint8_t p1 = static_cast<uint8_t>((packed >> 16) & 0xFF);
        int newLen = applyOp(op, p0, p1, m_buffer, len);
        if (newLen < 0) return std::nullopt;
        len = newLen;
        if (len > 255) len = 255;
    }
    return std::string(m_buffer, static_cast<size_t>(len));
}

std::vector<std::string> RuleEngine::applyAll(
    const std::vector<CompiledRule>& rules, const std::string& word) {
    std::vector<std::string> results;
    results.reserve(rules.size());
    for (const auto& r : rules) {
        auto m = apply(r, word);
        if (m) results.push_back(std::move(*m));
    }
    return results;
}

int RuleEngine::applyOp(OpCode op, uint8_t p0, uint8_t p1,
                         char* buf, int len) {
    switch (op) {
        case OpCode::NOP: return len;


        case OpCode::LOWER:
            for (int i=0;i<len;i++) buf[i]=static_cast<char>(std::tolower(static_cast<unsigned char>(buf[i])));
            return len;
        case OpCode::UPPER:
            for (int i=0;i<len;i++) buf[i]=static_cast<char>(std::toupper(static_cast<unsigned char>(buf[i])));
            return len;
        case OpCode::CAPITAL:
            if (len>0) buf[0]=static_cast<char>(std::toupper(static_cast<unsigned char>(buf[0])));
            for (int i=1;i<len;i++) buf[i]=static_cast<char>(std::tolower(static_cast<unsigned char>(buf[i])));
            return len;
        case OpCode::INVERT:
            if (len>0) buf[0]=static_cast<char>(std::tolower(static_cast<unsigned char>(buf[0])));
            for (int i=1;i<len;i++) buf[i]=static_cast<char>(std::toupper(static_cast<unsigned char>(buf[i])));
            return len;
        case OpCode::TOGGLE:
            for (int i=0;i<len;i++) {
                if (std::isupper(static_cast<unsigned char>(buf[i])))
                    buf[i]=static_cast<char>(std::tolower(static_cast<unsigned char>(buf[i])));
                else buf[i]=static_cast<char>(std::toupper(static_cast<unsigned char>(buf[i])));
            }
            return len;
        case OpCode::TOGGLE_AT: {
            int pos=static_cast<int>(p0);
            if (pos>=0&&pos<len) {
                if (std::isupper(static_cast<unsigned char>(buf[pos])))
                    buf[pos]=static_cast<char>(std::tolower(static_cast<unsigned char>(buf[pos])));
                else buf[pos]=static_cast<char>(std::toupper(static_cast<unsigned char>(buf[pos])));
            }
            return len;
        }


        case OpCode::APPEND:
            buf[len]=static_cast<char>(p0);
            return len+1;
        case OpCode::PREPEND:
            std::memmove(buf+1,buf,static_cast<size_t>(len));
            buf[0]=static_cast<char>(p0);
            return len+1;


        case OpCode::INSERT: {
            int pos=static_cast<int>(p0);
            if (pos<0) pos=0; if (pos>len) pos=len;
            std::memmove(buf+pos+1,buf+pos,static_cast<size_t>(len-pos));
            buf[pos]=static_cast<char>(p1);
            return len+1;
        }


        case OpCode::DELETE_: {
            int pos=static_cast<int>(p0);
            if (pos<0||pos>=len) return len;
            std::memmove(buf+pos,buf+pos+1,static_cast<size_t>(len-pos-1));
            return len-1;
        }


        case OpCode::OVERWRITE: {
            int pos=static_cast<int>(p0);
            if (pos>=0&&pos<len) buf[pos]=static_cast<char>(p1);
            return len;
        }


        case OpCode::TRUNCATE: {
            int pos=static_cast<int>(p0);
            if (pos<0) pos=0; if (pos>len) pos=len;
            return pos;
        }


        case OpCode::REPLACE: {
            char from=static_cast<char>(p0), to=static_cast<char>(p1);
            for (int i=0;i<len;i++) if (buf[i]==from) buf[i]=to;
            return len;
        }


        case OpCode::PURGE: {
            char purge=static_cast<char>(p0);
            int w=0;
            for (int r=0;r<len;r++) {
                if (buf[r]!=purge) buf[w++]=buf[r];
            }
            return w;
        }


        case OpCode::REVERSE:
            for (int i=0;i<len/2;i++) std::swap(buf[i],buf[len-1-i]);
            return len;


        case OpCode::DUPLICATE:
            std::memcpy(buf+len,buf,static_cast<size_t>(len));
            return len*2;

        case OpCode::DUPLICATE_N: {
            int times=static_cast<int>(p0);
            if (times<1) times=1; if (times>16) times=16;
            int orig=len;
            for (int t=1;t<times;t++) {
                std::memcpy(buf+orig*t,buf,static_cast<size_t>(orig));
            }
            return orig*times;
        }


        case OpCode::REFLECT:
            for (int i=0;i<len;i++) buf[len+i]=buf[len-1-i];
            return len*2;


        case OpCode::ROTATE_L:
            if (len>1) {
                char first=buf[0];
                std::memmove(buf,buf+1,static_cast<size_t>(len-1));
                buf[len-1]=first;
            }
            return len;


        case OpCode::ROTATE_R:
            if (len>1) {
                char last=buf[len-1];
                std::memmove(buf+1,buf,static_cast<size_t>(len-1));
                buf[0]=last;
            }
            return len;


        case OpCode::SWAP_FIRST:
            if (len>1) std::swap(buf[0],buf[1]);
            return len;


        case OpCode::SWAP_LAST:
            if (len>1) std::swap(buf[len-1],buf[len-2]);
            return len;


        case OpCode::EXTRACT: {
            int pos=static_cast<int>(p0), count=static_cast<int>(p1);
            if (pos<0) pos=0;
            if (pos+count>len) count=len-pos;
            if (count<0) count=0;
            std::memmove(buf,buf+pos,static_cast<size_t>(count));
            return count;
        }


        case OpCode::OMIT: {
            int pos=static_cast<int>(p0), count=static_cast<int>(p1);
            if (pos<0) pos=0;
            if (pos+count>len) count=len-pos;
            if (count<=0) return len;
            std::memmove(buf+pos,buf+pos+count,static_cast<size_t>(len-pos-count));
            return len-count;
        }


        case OpCode::MEMORIZE:
            std::memcpy(m_memory, buf, static_cast<size_t>(len));
            m_memoryLen=len;
            return len;
        case OpCode::APPEND_MEM:
            std::memcpy(buf+len, m_memory, static_cast<size_t>(m_memoryLen));
            return len+m_memoryLen;
        case OpCode::PREPEND_MEM:
            std::memmove(buf+m_memoryLen, buf, static_cast<size_t>(len));
            std::memcpy(buf, m_memory, static_cast<size_t>(m_memoryLen));
            return len+m_memoryLen;


        case OpCode::REJECT_LEN_GT:
            return (len > static_cast<int>(p0)) ? -1 : len;
        case OpCode::REJECT_LEN_LT:
            return (len < static_cast<int>(p0)) ? -1 : len;
        case OpCode::REJECT_CHAR: {
            char ch=static_cast<char>(p0);
            for (int i=0;i<len;i++) if (buf[i]==ch) return -1;
            return len;
        }
        case OpCode::REJECT_UNLESS: {
            char ch=static_cast<char>(p0);
            for (int i=0;i<len;i++) if (buf[i]==ch) return len;
            return -1;
        }


        case OpCode::INC_AT: {
            int pos=static_cast<int>(p0);
            if (pos>=0&&pos<len) buf[pos]++;
            return len;
        }
        case OpCode::DEC_AT: {
            int pos=static_cast<int>(p0);
            if (pos>=0&&pos<len) buf[pos]--;
            return len;
        }

        default: return len;
    }
}

} }
