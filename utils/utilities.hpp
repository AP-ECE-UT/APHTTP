#ifndef UTILITIES_HPP_INCLUDE
#define UTILITIES_HPP_INCLUDE

#include <map>
#include <string>

namespace utils {

struct StringInsensitiveComp {
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

using CiMap = std::map<std::string, std::string, StringInsensitiveComp>;

std::string readFile(const std::string& filename);
bool writeToFile(const std::string& str, const std::string& filename);
std::string getExtension(const std::string& filename);

std::string urlEncode(const std::string& url);
std::string urlDecode(const std::string& url);

CiMap getCimapFromString(const std::string& str);
int readMapFromFile(const std::string& filename, std::map<std::string, std::string>& m);

} // namespace utils

#endif // UTILITIES_HPP_INCLUDE
