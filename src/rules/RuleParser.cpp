#include "RuleParser.hpp"
#include "../utils/FileReader.hpp"
#include <cctype>

namespace fenrir { namespace rules {

static uint8_t hexVal(char c) {
    if (c>='0'&&c<='9') return c-'0';
    if (c>='a'&&c<='f') return c-'a'+10;
    if (c>='A'&&c<='F') return c-'A'+10;
    return 0;
}

static uint8_t decodePos(char c) {
    if (c>='0'&&c<='9') return c-'0';
    if (c>='A'&&c<='Z') return c-'A'+10;
    if (c>='a'&&c<='z') return c-'a'+10;
    return 0;
}

static CompiledRule parseRule(const std::string& line) {
    CompiledRule rule;
    size_t i = 0;

    while (i < line.size()) {
        char c = line[i];


        if (c == ' ') { i++; continue; }

        if (c == '#') break;


        if (c == ':' || c == 'l' || c == 'u' || c == 'c' || c == 'C' ||
            c == 't' || c == 'r' || c == 'd' || c == 'f' || c == '{' ||
            c == '}' || c == 'k' || c == 'K' || c == 'M' || c == '4' ||
            c == '6') {
            rule.push_back(packOp(static_cast<OpCode>(c)));
            i++;
            continue;
        }


        if ((c == '$' || c == '^' || c == '@') && i + 1 < line.size()) {
            rule.push_back(packOp(static_cast<OpCode>(c),
                                   static_cast<uint8_t>(line[i+1]), 0));
            i += 2;
            continue;
        }


        if ((c == '!' || c == '/') && i + 1 < line.size()) {
            rule.push_back(packOp(static_cast<OpCode>(c),
                                   static_cast<uint8_t>(line[i+1]), 0));
            i += 2;
            continue;
        }


        if ((c == 'D' || c == 'T' || c == '+' || c == '-' || c == 'p') &&
            i + 1 < line.size()) {
            uint8_t pos = decodePos(line[i+1]);
            rule.push_back(packOp(static_cast<OpCode>(c), pos, 0));
            i += 2;
            continue;
        }


        if ((c == '<' || c == '>') && i + 1 < line.size()) {
            uint8_t n = decodePos(line[i+1]);
            rule.push_back(packOp(static_cast<OpCode>(c), n, 0));
            i += 2;
            continue;
        }


        if ((c == 'i' || c == 'o' || c == 's') && i + 2 < line.size()) {
            uint8_t p0 = decodePos(line[i+1]);
            uint8_t p1 = static_cast<uint8_t>(line[i+2]);
            if (c == 's') {

                p0 = static_cast<uint8_t>(line[i+1]);
            }
            rule.push_back(packOp(static_cast<OpCode>(c), p0, p1));
            i += 3;
            continue;
        }


        if ((c == 'x' || c == 'O') && i + 2 < line.size()) {
            uint8_t p0 = decodePos(line[i+1]);
            uint8_t p1 = decodePos(line[i+2]);
            rule.push_back(packOp(static_cast<OpCode>(c), p0, p1));
            i += 3;
            continue;
        }


        if (c == '\'' && i + 1 < line.size()) {
            uint8_t pos = decodePos(line[i+1]);
            rule.push_back(packOp(OpCode::TRUNCATE, pos, 0));
            i += 2;
            continue;
        }


        i++;
    }


    rule.push_back(0);
    return rule;
}

std::vector<CompiledRule> RuleParser::parseFile(const std::string& path) {
    std::vector<CompiledRule> rules;
    utils::FileReader reader(path);
    while (auto line = reader.nextLine()) {

        std::string s = *line;
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        s = s.substr(start);

        if (s.empty() || s[0] == '#') continue;
        auto rule = parseRule(s);
        if (!rule.empty()) {

            if (rule.size() > 1) rules.push_back(rule);
        }
    }
    return rules;
}

CompiledRule RuleParser::parseSingle(const std::string& ruleStr) {
    return parseRule(ruleStr);
}

} }
