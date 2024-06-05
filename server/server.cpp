#include "server.hpp"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "../utils/strutils.hpp"
#include "../utils/utilities.hpp"

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // win xp
#endif
#include <Ws2tcpip.h>
#include <winsock2.h>
#else
// POSIX sockets
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> //close()
#endif

#ifdef _WIN32
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s)   closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s)   close(s)
#define GETSOCKETERRNO() (errno)
#endif

static const char* getSocketError() {
#ifdef _WIN32
    static char message[256];
    message[0] = '\0';
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, WSAGetLastError(), 0, (LPSTR)&message, sizeof(message),
                   NULL);
    char* newline = strrchr(message, '\n');
    if (newline)
        *newline = '\0';
    return message;
#else
    return strerror(errno);
#endif
}

using namespace std;

class NotFoundHandler : public RequestHandler {
public:
    NotFoundHandler(const string& notFoundErrPage = "")
        : notFoundErrPage_(notFoundErrPage) {}
    Response* callback(Request* req) {
        Response* res = new Response(Response::Status::notFound);
        if (!notFoundErrPage_.empty()) {
            res->setHeader("Content-Type", "text/" + utils::getExtension(notFoundErrPage_));
            res->setBody(utils::readFile(notFoundErrPage_));
        }
        return res;
    }

private:
    string notFoundErrPage_;
};

class ServerErrorHandler {
public:
    static Response* callback(const string& msg) {
        Response* res = new Response(Response::Status::internalServerError);
        res->setHeader("Content-Type", "application/json");
        res->setBody("{ \"code\": \"500\", \"message\": \"" + msg + "\" }\n");
        return res;
    }
};

Request* parseRawReq(char* reqData, size_t length) {
    Request* req = nullptr;
    string boundary;
    string lastFieldKey;
    string lastFieldValue;
    string reqDataStr(reqData, reqData + length);
    try {
        size_t endOfHeader = reqDataStr.find("\r\n\r\n");
        string reqHeader = reqDataStr.substr(0, endOfHeader);
        string reqBody = reqDataStr.substr(endOfHeader + 4, reqDataStr.size());
        if (endOfHeader == string::npos) {
            throw Server::Exception("End of request header not found.");
        }
        vector<string> headers = strutils::split(reqHeader, "\r\n");
        if (reqHeader.find('\0') != string::npos) {
            throw Server::Exception("Binary data in header.");
        }
        size_t realBodySize = length - endOfHeader - 4; // string("\r\n\r\n").size();

        vector<string> R = strutils::split(headers[0], ' ');
        if (R.size() != 3) {
            throw Server::Exception("Invalid header (request line)");
        }
        req = new Request(R[0]);
        req->setPath(R[1]);
        size_t pos = req->getPath().find('?');
        if (pos != string::npos && pos != req->getPath().size() - 1) {
            vector<string> Q1 = strutils::split(req->getPath().substr(pos + 1), '&');
            for (vector<string>::size_type q = 0; q < Q1.size(); q++) {
                vector<string> Q2 = strutils::split(Q1[q], '=');
                if (Q2.size() == 2)
                    req->setQueryParam(Q2[0], Q2[1], false);
                else
                    throw Server::Exception("Invalid query");
            }
        }
        req->setPath(req->getPath().substr(0, pos));

        for (size_t headerIndex = 1; headerIndex < headers.size(); headerIndex++) {
            string line = headers[headerIndex];
            vector<string> R = strutils::split(line, ": ");
            if (R.size() != 2)
                throw Server::Exception("Invalid header");
            req->setHeader(R[0], R[1], false);
            if (strutils::tolower(R[0]) == strutils::tolower("Content-Length"))
                if (realBodySize != (size_t)atol(R[1].c_str()))
                    return nullptr;
        }

        string contentType = req->getHeader("Content-Type");
        if (realBodySize != 0 && !contentType.empty()) {
            if (strutils::startsWith(contentType, "application/x-www-form-urlencoded")) {
                vector<string> urlencodedParts = strutils::split(reqBody, "\r\n");
                for (const string& part : urlencodedParts) {
                    vector<string> body = strutils::split(part, '&');
                    for (size_t i = 0; i < body.size(); i++) {
                        vector<string> field = strutils::split(body[i], '=');
                        if (field.size() == 2)
                            req->setBodyParam(field[0], field[1], "application/x-www-form-urlencoded", false);
                        else if (field.size() == 1)
                            req->setBodyParam(field[0], "", "application/x-www-form-urlencoded", false);
                        else
                            throw Server::Exception("Invalid body");
                    }
                }
            }
            else if (strutils::startsWith(contentType, "multipart/form-data")) {
                boundary = contentType.substr(contentType.find("boundary=") + 9);
                size_t firstBoundary = reqBody.find("--" + boundary);
                if (firstBoundary == string::npos) {
                    throw Server::Exception("Boundary data not found.");
                }
                reqBody.erase(reqBody.begin(), reqBody.begin() + firstBoundary + 2 + boundary.size());

                vector<string> boundaries = strutils::split(reqBody, "--" + boundary);
                boundaries.pop_back();

                for (string b : boundaries) {
                    b.pop_back(); // remove "\r\n" from start and end of each boundary
                    b.pop_back();
                    b.erase(b.begin(), b.begin() + 2);
                    string boundaryContentType = "text/plain";

                    size_t endOfBoundaryHeader = b.find("\r\n\r\n") + 4;
                    vector<string> abc = strutils::split(b.substr(0, endOfBoundaryHeader - 4), "\r\n");
                    for (const string& line : abc) {
                        if (line.empty()) {
                            break;
                        }
                        vector<string> R = strutils::split(line, ": ");
                        if (R.size() != 2) throw Server::Exception("Invalid header");
                        if (strutils::tolower(R[0]) == strutils::tolower("Content-Disposition")) {
                            vector<string> A = strutils::split(R[1], "; ");
                            for (size_t i = 0; i < A.size(); i++) {
                                vector<string> attr = strutils::split(A[i], '=');
                                if (attr.size() == 2) {
                                    if (strutils::tolower(attr[0]) == strutils::tolower("name")) {
                                        lastFieldKey = attr[1].substr(1, attr[1].size() - 2);
                                    }
                                }
                                else if (attr.size() != 1) {
                                    throw Server::Exception("Invalid body attribute");
                                }
                            }
                        }
                        else if (strutils::tolower(R[0]) == strutils::tolower("Content-Type")) {
                            boundaryContentType = strutils::tolower(R[1]);
                        }
                    }
                    lastFieldValue = b.substr(endOfBoundaryHeader);
                    req->setBodyParam(lastFieldKey, lastFieldValue, boundaryContentType, false);
                }
            }
            else {
                throw Server::Exception("Unsupported body type: " + contentType);
            }
        }
    }
    catch (const Server::Exception&) {
        throw;
    }
    catch (const std::exception& e) {
        throw Server::Exception("Error on parsing request: " + std::string(e.what()));
    }
    return req;
}

Server::Server(int port) : port_(port) {
#ifdef _WIN32
    WSADATA wsa_data;
    int initializeResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (initializeResult != 0) {
        throw Exception("Error: WinSock WSAStartup failed: " +
                        string(getSocketError()));
    }
#endif

    notFoundHandler_ = new NotFoundHandler();

    sc_ = socket(AF_INET, SOCK_STREAM, 0);
    int sc_option = 1;

#ifdef _WIN32
    setsockopt(sc_, SOL_SOCKET, SO_REUSEADDR, (char*)&sc_option, sizeof(sc_option));
#else
    setsockopt(sc_, SOL_SOCKET, SO_REUSEADDR, &sc_option, sizeof(sc_option));
#endif
    if (!ISVALIDSOCKET(sc_)) {
        throw Exception("Error on opening socket: " + string(getSocketError()));
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_);

    if (::bind(sc_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        throw Exception("Error on binding: " + string(getSocketError()));
    }
}

void Server::mapRequest(const string& path, RequestHandler* handler, Request::Method method) {
    Route* route = new Route(method, path);
    route->setHandler(handler);
    routes_.push_back(route);
}

void Server::get(const std::string& path, RequestHandler* handler) {
    mapRequest(path, handler, Request::Method::GET);
}

void Server::post(const std::string& path, RequestHandler* handler) {
    mapRequest(path, handler, Request::Method::POST);
}

void Server::put(const std::string& path, RequestHandler* handler) {
    mapRequest(path, handler, Request::Method::PUT);
}

void Server::del(const std::string& path, RequestHandler* handler) {
    mapRequest(path, handler, Request::Method::DEL);
}

void Server::run() {
    ::listen(sc_, 10);

    struct sockaddr_in cli_addr;
    socklen_t clilen;
    clilen = sizeof(cli_addr);
    SOCKET newsc;

    while (true) {
        newsc = ::accept(sc_, (struct sockaddr*)&cli_addr, &clilen);
        if (!ISVALIDSOCKET(newsc))
            throw Exception("Error on accept: " + string(getSocketError()));
        Response* res = nullptr;
        try {
            char* data = new char[BUFSIZE + 1];
            size_t recv_len, recv_total_len = 0;
            Request* req = nullptr;
            while (!req) {
                recv_len = recv(newsc, data + recv_total_len, BUFSIZE - recv_total_len, 0);
                if (recv_len > 0) {
                    recv_total_len += recv_len;
                    data[recv_total_len >= 0 ? recv_total_len : 0] = 0;
                    req = parseRawReq(data, recv_total_len);
                }
                else
                    break;
            }
            delete[] data;
            if (!recv_total_len) {
                CLOSESOCKET(newsc);
                continue;
            }
            req->log();
            size_t i = 0;
            for (; i < routes_.size(); i++) {
                if (routes_[i]->isMatch(req->getMethod(), req->getPath())) {
                    res = routes_[i]->handle(req);
                    break;
                }
            }
            if (i == routes_.size() && notFoundHandler_) {
                res = notFoundHandler_->callback(req);
            }
            delete req;
        }
        catch (const Exception& exc) {
            delete res;
            res = ServerErrorHandler::callback(exc.getMessage());
        }
        res->log();
        string res_data = res->getResponse();
        int si = res_data.size();
        delete res;
        int wr = send(newsc, res_data.c_str(), si, 0);
        if (wr != si)
            throw Exception("Send error: " + string(getSocketError()));
        CLOSESOCKET(newsc);
    }
}

Server::~Server() {
    if (sc_ >= 0) {
        CLOSESOCKET(sc_);
    }
    delete notFoundHandler_;
    for (size_t i = 0; i < routes_.size(); ++i) {
        delete routes_[i];
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

Server::Exception::Exception(const string message) : message_(message) {}

string Server::Exception::getMessage() const { return message_; }

ShowFile::ShowFile(const string& filePath, const string& fileType)
    : filePath_(filePath),
      fileType_(fileType) {}

Response* ShowFile::callback(Request* req) {
    Response* res = new Response();
    res->setHeader("Content-Type", fileType_);
    res->setBody(utils::readFile(filePath_));
    return res;
}

ShowPage::ShowPage(const string& filePath)
    : ShowFile(filePath, "text/" + utils::getExtension(filePath)) {}

ShowImage::ShowImage(const string& filePath)
    : ShowFile(filePath, "image/" + utils::getExtension(filePath)) {}

void Server::setNotFoundErrPage(const std::string& notFoundErrPage) {
    delete notFoundHandler_;
    notFoundHandler_ = new NotFoundHandler(notFoundErrPage);
}

RequestHandler::~RequestHandler() {}

TemplateHandler::TemplateHandler(const string& filePath)
    : filePath_(filePath),
      parser_(new TemplateParser(filePath)) {}

TemplateHandler::~TemplateHandler() {
    delete parser_;
}

Response* TemplateHandler::callback(Request* req) {
    map<string, string> context;
    context = this->handle(req);
    Response* res = new Response();
    res->setHeader("Content-Type", "text/html");
    res->setBody(parser_->getHtml(context));
    return res;
}

map<string, string> TemplateHandler::handle(Request* req) {
    map<string, string> context;
    return context;
}
