#include "strutils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace strutils {

void trimLeft(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }));
}

void trimRight(std::string& str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }).base(),
              str.end());
}

void trim(std::string& str) {
    trimRight(str);
    trimLeft(str);
}

std::vector<std::string> split(const std::string& str, char delim) {
    std::istringstream sstr(str);
    std::string item;
    std::vector<std::string> result;
    while (std::getline(sstr, item, delim)) {
        result.push_back(std::move(item));
    }
    return result;
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
    std::string::size_type startPos = 0;
    std::string::size_type endPos;

    std::vector<std::string> result;
    while ((endPos = str.find(delim, startPos)) != std::string::npos) {
        result.push_back(str.substr(startPos, endPos - startPos));
        startPos = endPos + delim.size();
    }
    result.push_back(str.substr(startPos));

    return result;
}

std::string toupper(const std::string& str) {
    std::string res(str);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return res;
}

std::string tolower(const std::string& str) {
    std::string res(str);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return res;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

bool startsWith(const std::string& str, const std::string& s) {
    return str.size() >= s.size() &&
           std::equal(s.begin(), s.end(), str.begin());
}

} // namespace strutils
