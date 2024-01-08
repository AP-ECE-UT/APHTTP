#include "response.hpp"

#include <cstring>
#include <iostream>
#include <map>

using namespace std;

map<int, string> getHttpPhrases() {
    map<int, string> httpPhrase;
    httpPhrase[200] = "OK";
    httpPhrase[303] = "See Other";
    httpPhrase[404] = "Not Found";
    return httpPhrase;
}

map<int, string> httpPhrase = getHttpPhrases();

Response::Response(int code) {
    this->code = code;
    this->phrase = httpPhrase[code];
    this->headers["Content-Type"] = "text/plain";
}

int Response::getStatusCode() { return code; }

string Response::getStatusPhrase() { return phrase; }

void Response::setStatus(int _code, string _phrase) {
    phrase = _phrase;
    code = _code;
}

void Response::setStatus(int _code) { setStatus(_code, httpPhrase[_code]); }

string Response::print(int& size) {
    string header = "";
    header += "HTTP/1.0 " + to_string(code) + " " + phrase + "\r\n";
    header += "Server: " + SERVER_NAME + " \r\n";
    header += "Content-Length: " + to_string(body.size()) + "\r\n";
    for (auto it = headers.begin(); !headers.empty() && it != headers.end(); it++)
        header += it->first + ": " + it->second + "\r\n";
    header += "\r\n";
    size = header.size() + body.size();
    return header + body;
}

void Response::log(bool showBody) {
    const string NC = "\033[0;39m";
    const string K = "\033[1m";
    const string H = "\033[34;1m";
    const string G = "\033[32m";
    const string R = "\033[31m";

    string log;
    log += H + "------- Response -------" + NC + "\n";
    log += K + "Status: " + NC + (code == 200 ? G : R) + to_string(code) + " " + phrase + NC + "\n";
    log += K + "Headers:" + NC + "\n";

    for (auto it = headers.begin(); !headers.empty() && it != headers.end(); it++) {
        log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) + "\n";
    }
    if (showBody) {
        log += K + "Body:\n" + NC + body + "\n";
    }
    log += H + "------------------------" + NC + "\n";
    cerr << log << endl;
}

void Response::setHeader(string name, string value) { headers[name] = value; }

void Response::setBody(string _body) { body = _body; }

string Response::getHeader(string name) { return ""; }

void Response::setSessionId(string sessionId) {
    setHeader("set-cookie", "sessionId=" + sessionId + ";");
}

Response* Response::redirect(string url) {
    Response* res = new Response(303);
    res->setHeader("Location", url);
    return res;
}
