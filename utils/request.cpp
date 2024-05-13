#include "request.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "strutils.hpp"

Request::Request(Method method_)
    : method(method_) {}

Request::Request(const std::string& method_) {
    if (method_ == "GET") {
        method = Method::GET;
    }
    else if (method_ == "POST") {
        method = Method::POST;
    }
}

Request::Method Request::getMethod() const {
    return method;
}

std::string Request::getPath() const {
    return path;
}

std::string Request::getHeader(const std::string& key) const {
    auto itr = headers.find(key);
    if (itr == headers.end()) {
        return {};
    }
    return utils::urlDecode(itr->second);
}

std::string Request::getBody() const {
    std::string bs;
    for (auto itr = body.begin(); !body.empty() && itr != body.end(); ++itr) {
        bs += itr->first + "=" + itr->second + "&";
    }
    return bs;
}

std::string Request::getQueryParam(const std::string& key) const {
    auto itr = query.find(key);
    if (itr == query.end()) {
        return {};
    }
    return utils::urlDecode(itr->second);
}

std::string Request::getBodyParam(const std::string& key) const {
    auto bodyType = bodyTypes.find(key);
    auto itr = body.find(key);
    if (bodyType == bodyTypes.end() || itr == body.end()) {
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
    std::vector<std::string> v = utils::split(cookie, ";");
    for (std::string kv : v) {
        trim(kv);
        std::vector<std::string> k = utils::split(kv, "=");
        if (k[0] == "sessionId") {
            return k[1];
        }
    }
    return {};
}

void Request::setPath(const std::string& _path) {
    path = _path;
}

void Request::setHeader(const std::string& key, const std::string& value, bool encode) {
    headers[key] = encode ? utils::urlEncode(value) : value;
}

void Request::setBody(const std::string& _body) {
    body = utils::getCimapFromString(_body);
}

void Request::setQueryParam(const std::string& key, const std::string& value, bool encode) {
    query[key] = encode ? utils::urlEncode(value) : value;
}

void Request::setBodyParam(const std::string& key, const std::string& value, const std::string& contentType, bool encode) {
    body[key] = encode ? utils::urlEncode(value) : value;
    bodyTypes[key] = contentType;
}

void Request::log() {
    const std::string NC = "\033[0;39m";
    const std::string K = "\033[1m";
    const std::string H = "\033[33;1m";

    std::string log;
    log += H + "------- Request --------" + NC + "\n";
    log += K + "Method: " + NC + (method == Method::POST ? "POST" : "GET") + "\n";
    log += K + "Path:   " + NC + path + "\n";
    log += K + "SessionId: " + NC + this->getSessionId() + "\n";

    log += K + "Headers:" + NC + "\n";
    for (auto it = headers.begin(); !headers.empty() && it != headers.end(); it++) {
        log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) + "\n";
    }

    log += K + "Query:" + NC + "\n";
    for (auto it = query.begin(); !query.empty() && it != query.end(); it++) {
        log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) + "\n";
    }

    log += K + "Body:" + NC + "\n";
    for (auto it = body.begin(); !body.empty() && it != body.end(); it++) {
        std::string type = bodyTypes[it->first];
        if (type == "application/x-www-form-urlencoded" || type == "text/plain") {
            log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) + "\n";
        }
        else {
            log += "  " + utils::urlDecode(it->first) + ": <BINARY DATA>\n";
        }
    }
    log += H + "------------------------" + NC + "\n";
    std::clog << log << std::endl;
}
