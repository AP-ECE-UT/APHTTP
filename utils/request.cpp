#include "request.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "../utils/strutils.hpp"
#include "../utils/utilities.hpp"

using namespace std;

Request::Request(string method_) {
    if (method_ == "GET")
        method = GET;
    if (method_ == "POST")
        method = POST;
}

string Request::getQueryParam(string key) { return utils::urlDecode(query[key]); }

string Request::getBodyParam(string key) {
    if (bodyTypes[key] == "application/x-www-form-urlencoded") {
        return utils::urlDecode(body[key]);
    }
    else return body[key];
}

string Request::getHeader(string key) { return utils::urlDecode(headers[key]); }

string Request::getPath() { return path; }

void Request::setPath(string _path) { path = _path; }

Method Request::getMethod() { return method; }

void Request::setMethod(Method _method) { method = _method; }

void Request::setQueryParam(string key, string value, bool encode) {
    query[key] = encode ? utils::urlEncode(value) : value;
}

void Request::setBodyParam(string key, string value, string contentType, bool encode) {
    body[key] = encode ? utils::urlEncode(value) : value;
    bodyTypes[key] = contentType;
}

void Request::setHeader(string key, string value, bool encode) {
    headers[key] = encode ? utils::urlEncode(value) : value;
}

string Request::getBody() {
    string bs = "";
    for (auto it = body.begin(); !body.empty() && it != body.end(); it++)
        bs += it->first + "=" + it->second + "&";
    return bs;
}

static void trim(string& s) {
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
}

string Request::getSessionId() {
    string cookie = getHeader("cookie");
    if (cookie == "")
        return "";
    trim(cookie);
    vector<string> v = utils::split(cookie, ";");
    for (string kv : v) {
        trim(kv);
        vector<string> k = utils::split(kv, "=");
        if (k[0] == "sessionId")
            return k[1];
    }
    return "";
}

void Request::log() {
    const string NC = "\033[0;39m";
    const string K = "\033[1m";
    const string H = "\033[33;1m";

    string log;
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
        string type = bodyTypes[it->first];
        if (type == "application/x-www-form-urlencoded" || type == "text/plain") {
            log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) + "\n";
        }
        else {
            log += "  " + utils::urlDecode(it->first) + ": <BINARY DATA>\n";
        }
    }
    log += H + "------------------------" + NC + "\n";
    clog << log << endl;
}

utils::CiMap Request::getHeaders() {
    vector<string> res;
    for (map<string, string>::iterator i = headers.begin();
         !headers.empty() && i != headers.end(); i++) {
        res.push_back(i->first);
        res.push_back(i->second);
    }
    return headers;
}

string Request::getQueryString() {
    if (query.empty())
        return "";
    string res = "?";
    for (map<string, string>::iterator i = query.begin();
         !query.empty() && i != query.end(); i++) {
        res += i->first;
        res += "=";
        res += i->second;
        res += "&";
    }
    return res;
}

string Request::getHeadersString() {
    string headerString = "";
    for (auto it = headers.begin(); !headers.empty() && it != headers.end(); it++)
        headerString += it->first + "=" + it->second + "&";
    return headerString;
}

void Request::setHeaders(string _headers) {
    headers = utils::getCimapFromString(_headers);
}

void Request::setQuery(std::string _query) {
    _query = _query.substr(1);
    query = utils::getCimapFromString(_query);
}

void Request::setBody(std::string _body) { body = utils::getCimapFromString(_body); }

void Request::serializeToFile(Request* req, string filePath) {
    string reqString = to_string(req->getMethod());
    reqString += "\n";
    reqString += req->getPath();
    reqString += "\n";
    reqString += req->getHeadersString();
    reqString += "\n";
    reqString += req->getBody();
    reqString += "\n";
    reqString += req->getQueryString();
    utils::writeToFile(reqString, filePath);
}

void Request::deserializeFromFile(Request* req, string filePath) {
    vector<string> fields = utils::split(utils::readFile(filePath), '\n');
    switch (fields.size()) {
    case 5:
        req->setQuery(fields[4]);
    case 4:
        req->setBody(fields[3]);
    case 3:
        req->setHeaders(fields[2]);
    case 2:
        req->setPath(fields[1]);
    case 1:
        req->setMethod(stoi(fields[0]) == GET ? GET : POST);
    }
}
