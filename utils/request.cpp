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

string Request::getBodyParam(string key) { return utils::urlDecode(body[key]); }

string Request::getHeader(string key) { return utils::urlDecode(headers[key]); }

string Request::getPath() { return path; }

void Request::setPath(string _path) { path = _path; }

Method Request::getMethod() { return method; }

void Request::setMethod(Method _method) { method = _method; }

void Request::setQueryParam(string key, string value, bool encode) {
    query[key] = encode ? utils::urlEncode(value) : value;
}

void Request::setBodyParam(string key, string value, bool encode) {
    body[key] = encode ? utils::urlEncode(value) : value;
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
    string log = "";
    log += H + string("------- Request --------") + NC + string("\n");
    log +=
        K + string("Method:\t") + NC + (method ? "POST" : "GET") + string("\n");
    log += K + string("Path:\t") + NC + path + string("\n");
    log += K + string("Headers:") + NC + string("\n");
    for (auto it = headers.begin(); !headers.empty() && it != headers.end(); it++)
        log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) +
               string("\n");
    log += "[ " + K + string("SessionId:\t") + NC + this->getSessionId() + " ]" +
           string("\n");
    log += K + string("Query:") + NC + string("\n");
    for (auto it = query.begin(); !query.empty() && it != query.end(); it++)
        log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) +
               string("\n");
    log += K + string("Body:") + NC + string("\n");
    for (auto it = body.begin(); !body.empty() && it != body.end(); it++)
        log += "  " + utils::urlDecode(it->first) + ": " + utils::urlDecode(it->second) +
               string("\n");
    log += H + string("------------------------") + NC + string("\n");
    cerr << log << endl;
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
