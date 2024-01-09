#ifndef STRUTILS_HPP_INCLUDE
#define STRUTILS_HPP_INCLUDE

#include <string>
#include <vector>

namespace utils {

void trimLeft(std::string& str);
void trimRight(std::string& str);
void trim(std::string& str);

std::vector<std::string> split(const std::string& str, char delim);
std::vector<std::string> split(const std::string& str, const std::string& delim);

std::string toupper(const std::string& str);
std::string tolower(const std::string& str);

void replaceAll(std::string& str, const std::string& from, const std::string& to);
bool startsWith(const std::string& str, const std::string& s);

} // namespace utils

#endif // STRUTILS_HPP_INCLUDE
