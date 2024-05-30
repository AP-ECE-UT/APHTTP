#include "response.hpp"

#include <iostream>

#include "include.hpp"

const std::unordered_map<Response::Status, std::string> Response::httpPhraseMap = {
    {Status::ok, "OK"},
    {Status::created, "Created"},

    {Status::movedPermanently, "Moved Permanently"},
    {Status::seeOther, "See Other"},

    {Status::badRequest, "Bad Request"},
    {Status::unauthorized, "Unauthorized"},
    {Status::forbidden, "Forbidden"},
    {Status::notFound, "Not Found"},
    {Status::methodNotAllowed, "Method Not Allowed"},
    {Status::conflict, "Conflict"},
    {Status::teapot, "I'm a teapot"},

    {Status::internalServerError, "Internal Server Error"},
    {Status::notImplemented, "Not Implemented"},
};

Response::Response(Status code_)
    : code(static_cast<int>(code_)),
      phrase(httpPhraseMap.find(code_)->second) {
    headers["Content-Type"] = "text/plain";
}

Response::Response(int _code, const std::string& _phrase)
    : code(_code),
      phrase(_phrase) {
    headers["Content-Type"] = "text/plain";
}

void Response::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void Response::setBody(const std::string& _body) {
    body = _body;
}

void Response::setSessionId(const std::string& sessionId) {
    setHeader("set-cookie", "sessionId=" + sessionId + ";");
}

std::string Response::print() {
    std::string header;
    header += "HTTP/1.0 " + std::to_string(code) + " " + phrase + "\r\n";
    header += "Server: " + SERVER_NAME + " \r\n";
    header += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    for (auto itr = headers.begin(); itr != headers.end(); ++itr) {
        header += itr->first + ": " + itr->second + "\r\n";
    }
    header += "\r\n";
    return header + body;
}

void Response::log(bool showBody) {
    const std::string NC = "\033[0;39m";
    const std::string K = "\033[1m";
    const std::string H = "\033[34;1m";
    const std::string G = "\033[32m";
    const std::string R = "\033[31m";

    std::string log;
    log += H + "------- Response -------" + NC + "\n";
    log += K + "Status: " + NC + (code == 200 ? G : R) + std::to_string(code) + " " + phrase + NC + "\n";
    log += K + "Headers:" + NC + "\n";

    for (auto itr = headers.begin(); itr != headers.end(); ++itr) {
        log += "  " + utils::urlDecode(itr->first) + ": " + utils::urlDecode(itr->second) + "\n";
    }
    if (showBody) {
        log += K + "Body:\n" + NC + body + "\n";
    }
    log += H + "------------------------" + NC + "\n";
    std::clog << log << std::endl;
}

Response* Response::redirect(const std::string& url) {
    Response* res = new Response(Status::seeOther);
    res->setHeader("Location", url);
    return res;
}
