#include "request.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "strutils.hpp"

const std::unordered_map<std::string, Request::Method> Request::methodMap_ = {
    {"GET", Request::Method::GET},
    {"POST", Request::Method::POST},
    {"PUT", Request::Method::PUT},
    {"DEL", Request::Method::DEL},
};

Request::Request(Method method)
    : method_(method) {}

Request::Request(const std::string& method)
    : Request(Method::GET) {
    auto itr = methodMap_.find(method);
    if (itr != methodMap_.end()) {
        method_ = itr->second;
    }
}

Request::Method Request::getMethod() const {
    return method_;
}

std::string Request::getPath() const {
    return path_;
}

std::string Request::getHeader(const std::string& key) const {
    auto itr = headers_.find(key);
    if (itr == headers_.end()) {
        return {};
    }
    return utils::urlDecode(itr->second);
}

std::string Request::getBody() const {
    std::string result;
    for (auto itr = body_.begin(); itr != body_.end(); ++itr) {
        result += itr->first + "=" + itr->second + "&";
    }
    return result;
}

std::string Request::getQueryParam(const std::string& key) const {
    auto itr = query_.find(key);
    if (itr == query_.end()) {
        return {};
    }
    return utils::urlDecode(itr->second);
}

std::string Request::getBodyParam(const std::string& key) const {
    auto bodyType = bodyTypes_.find(key);
    auto itr = body_.find(key);
    if (bodyType == bodyTypes_.end() || itr == body_.end()) {
        return {};
    }

    if (bodyType->second == "application/x-www-form-urlencoded") {
        return utils::urlDecode(itr->second);
    }
    return itr->second;
}

static void trim(std::string& s) {
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
}

std::string Request::getSessionId() const {
    std::string cookie = getHeader("cookie");
    if (cookie.empty()) {
        return {};
    }
    trim(cookie);
    std::vector<std::string> v = strutils::split(cookie, ";");
    for (std::string kv : v) {
        trim(kv);
        std::vector<std::string> k = strutils::split(kv, "=");
        if (k[0] == "sessionId") {
            return k[1];
        }
    }
    return {};
}

void Request::setPath(const std::string& path) {
    path_ = path;
}

void Request::setHeader(const std::string& key, const std::string& value, bool encode) {
    headers_[key] = encode ? utils::urlEncode(value) : value;
}

void Request::setBody(const std::string& body) {
    body_ = utils::getCimapFromString(body);
}

void Request::setQueryParam(const std::string& key, const std::string& value, bool encode) {
    query_[key] = encode ? utils::urlEncode(value) : value;
}

void Request::setBodyParam(const std::string& key, const std::string& value, const std::string& contentType, bool encode) {
    body_[key] = encode ? utils::urlEncode(value) : value;
    bodyTypes_[key] = contentType;
}

void Request::log() const {
    const std::string NC = "\033[0;39m";
    const std::string K = "\033[1m";
    const std::string H = "\033[33;1m";

    std::string log;
    log += H + "------- Request --------" + NC + "\n";
    log += K + "Method: " + NC + (method_ == Method::POST ? "POST" : "GET") + "\n";
    log += K + "Path:   " + NC + path_ + "\n";
    log += K + "SessionId: " + NC + this->getSessionId() + "\n";

    log += K + "Headers:" + NC + "\n";
    for (auto itr = headers_.begin(); itr != headers_.end(); itr++) {
        log += "  " + utils::urlDecode(itr->first) + ": " + utils::urlDecode(itr->second) + "\n";
    }

    log += K + "Query:" + NC + "\n";
    for (auto itr = query_.begin(); itr != query_.end(); itr++) {
        log += "  " + utils::urlDecode(itr->first) + ": " + utils::urlDecode(itr->second) + "\n";
    }

    log += K + "Body:" + NC + "\n";
    for (auto itr = body_.begin(); itr != body_.end(); itr++) {
        std::string type = bodyTypes_.find(itr->first)->second;
        if (type == "application/x-www-form-urlencoded" || type == "text/plain") {
            log += "  " + utils::urlDecode(itr->first) + ": " + utils::urlDecode(itr->second) + "\n";
        }
        else {
            log += "  " + utils::urlDecode(itr->first) + ": <BINARY DATA>\n";
        }
    }
    log += H + "------------------------" + NC + "\n";
    std::clog << log << std::endl;
}
