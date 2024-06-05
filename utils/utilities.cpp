#include "utilities.hpp"

#include <fstream>
#include <vector>

#include "include.hpp"
#include "strutils.hpp"

namespace utils {

bool StringInsensitiveComp::operator()(const std::string& lhs, const std::string& rhs) const {
    return strutils::tolower(lhs) < strutils::tolower(rhs);
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios_base::binary);
    if (!file.is_open()) return {};

    file.seekg(0, file.end);
    size_t length = file.tellg();
    file.seekg(0, file.beg);

    if (length > BUFSIZE) {
        length = BUFSIZE;
    }
    std::string buffer(length, '\0');
    file.read(&buffer[0], length);
    return buffer;
}

bool writeToFile(const std::string& str, const std::string& filename) {
    std::ofstream file(filename, std::ios_base::binary);
    if (!file.is_open()) return false;
    file.write(str.c_str(), str.size());
    return true;
}

std::string getExtension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    return filename.substr(pos != std::string::npos ? pos + 1 : filename.size());
}

std::string urlEncode(const std::string& url) {
    // http://www.blooberry.com/indexdot/html/topics/urlencoding.htm
    char encode_buf[4];
    encode_buf[0] = '%';
    std::string result;
    result.reserve(url.size());

    for (size_t pos = 0; pos < url.size(); ++pos) {
        switch (url[pos]) {
        default:
            if (url[pos] >= 32 && url[pos] < 127) {
                result += url[pos];
                break;
            }
        case '$':
        case '&':
        case '+':
        case ',':
        case '/':
        case ':':
        case ';':
        case '=':
        case '?':
        case '@':
        case '"':
        case '<':
        case '>':
        case '#':
        case '%':
        case '{':
        case '}':
        case '|':
        case '\\':
        case '^':
        case '~':
        case '[':
        case ']':
        case '`':
            sprintf(encode_buf + 1, "%02X", url[pos]);
            result += encode_buf;
            break;
        }
    };
    return result;
}

std::string urlDecode(const std::string& url) {
    char decode_buf[3];
    std::string result;
    result.reserve(url.size());

    for (size_t pos = 0; pos < url.size(); ++pos) {
        switch (url[pos]) {
        case '+':
            result += ' ';
            break;
        case '%':
            if (pos + 2 < url.size()) {
                decode_buf[0] = url[++pos];
                decode_buf[1] = url[++pos];
                decode_buf[2] = '\0';
                result += static_cast<char>(strtol(decode_buf, nullptr, 16));
            }
            else {
                result += '%';
            }
            break;
        default:
            result += url[pos];
        }
    }
    return result;
}

CiMap getCimapFromString(const std::string& str) {
    CiMap m;
    std::vector<std::string> tokenized = strutils::split(str, '&');
    for (const std::string& token : tokenized) {
        std::vector<std::string> keyValue = strutils::split(token, '=');
        if (keyValue.size() != 2) continue;
        m[keyValue[0]] = keyValue[1];
    }
    return m;
}

int readMapFromFile(const std::string& filename, std::map<std::string, std::string>& m) {
    std::ifstream infile(filename);
    if (!infile.is_open()) return -1;

    std::string line;
    while (std::getline(infile, line)) {
        auto tokens = strutils::split(line, '=');
        m[tokens[0]] = tokens[(tokens.size() < 2) ? 0 : 1];
    }

    return m.size();
}

} // namespace utils
