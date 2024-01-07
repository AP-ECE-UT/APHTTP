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
    string notFoundErrPage;

public:
    NotFoundHandler(string notFoundErrPage = "")
        : notFoundErrPage(notFoundErrPage) {}
    Response* callback(Request* req) {
        Response* res = new Response(404);
        if (!notFoundErrPage.empty()) {
            res->setHeader("Content-Type", "text/" + utils::getExtension(notFoundErrPage));
            res->setBody(utils::readFile(notFoundErrPage.c_str()));
        }
        return res;
    }
};

class ServerErrorHandler {
public:
    static Response* callback(string msg) {
        Response* res = new Response(500);
        res->setHeader("Content-Type", "application/json");
        res->setBody("{ \"code\": \"500\", \"message\": \"" + msg + "\" }\n");
        return res;
    }
};

Request* parseRawReq(char* headersRaw, size_t length) {
    Request* req = nullptr;
    string boundary;
    string lastFieldKey;
    string lastFieldValue;
    bool shouldBeEmpty;
    try {
        enum State {
            REQ,
            HEADER,
            BODY,
            BODY_HEADER,
            BODY_BODY,
        };
        State state = REQ;
        vector<string> headers = utils::split(string(headersRaw), "\r\n");
        for (size_t i = 0; i < length; i++) {
            if (!headersRaw[i])
                throw Server::Exception("Unsupported binary data in request.");
        }
        size_t realBodySize =
            string(headersRaw).size() -
            utils::split(string(headersRaw), "\r\n\r\n")[0].size() -
            string("\r\n\r\n").size();
        for (size_t headerIndex = 0; headerIndex < headers.size(); headerIndex++) {
            string line = headers[headerIndex];
            switch (state) {
            case REQ: {
                vector<string> R = utils::split(line, ' ');
                if (R.size() != 3) {
                    throw Server::Exception("Invalid header (request line)");
                }
                req = new Request(R[0]);
                req->setPath(R[1]);
                size_t pos = req->getPath().find('?');
                if (pos != string::npos && pos != req->getPath().size() - 1) {
                    vector<string> Q1 = utils::split(req->getPath().substr(pos + 1), '&');
                    for (vector<string>::size_type q = 0; q < Q1.size(); q++) {
                        vector<string> Q2 = utils::split(Q1[q], '=');
                        if (Q2.size() == 2)
                            req->setQueryParam(Q2[0], Q2[1], false);
                        else
                            throw Server::Exception("Invalid query");
                    }
                }
                req->setPath(req->getPath().substr(0, pos));
                state = HEADER;
            } break;
            case HEADER: {
                if (line == "") {
                    state = BODY;
                    if (req->getHeader("Content-Type")
                            .substr(0, string("multipart/form-data").size()) ==
                        "multipart/form-data") {
                        boundary =
                            req->getHeader("Content-Type")
                                .substr(req->getHeader("Content-Type").find("boundary=") +
                                        string("boundary=").size());
                    }
                    break;
                }
                vector<string> R = utils::split(line, ": ");
                if (R.size() != 2)
                    throw Server::Exception("Invalid header");
                req->setHeader(R[0], R[1], false);
                if (utils::tolower(R[0]) == utils::tolower("Content-Length"))
                    if (realBodySize != (size_t)atol(R[1].c_str()))
                        return NULL;
            } break;
            case BODY: {
                if (req->getHeader("Content-Type") == "") {
                }
                else if (req->getHeader("Content-Type") ==
                         "application/x-www-form-urlencoded") {
                    vector<string> body = utils::split(line, '&');
                    for (size_t i = 0; i < body.size(); i++) {
                        vector<string> field = utils::split(body[i], '=');
                        if (field.size() == 2)
                            req->setBodyParam(field[0], field[1], false);
                        else if (field.size() == 1)
                            req->setBodyParam(field[0], "", false);
                        else
                            throw Server::Exception("Invalid body");
                    }
                }
                else if (req->getHeader("Content-Type")
                             .substr(0, string("multipart/form-data").size()) ==
                         "multipart/form-data") {
                    if (line == "--" + boundary || line == "--" + boundary + "--") {
                        lastFieldKey = "";
                        lastFieldValue = "";
                        shouldBeEmpty = false;
                        state = BODY_HEADER;
                    }
                }
                else {
                    throw Server::Exception("Unsupported body type: " +
                                            req->getHeader("Content-Type"));
                }
            } break;
            case BODY_HEADER: {
                if (line == "") {
                    state = BODY_BODY;
                    break;
                }
                vector<string> R = utils::split(line, ": ");
                if (R.size() != 2)
                    throw Server::Exception("Invalid header");
                if (utils::tolower(R[0]) == utils::tolower("Content-Disposition")) {
                    vector<string> A = utils::split(R[1], "; ");
                    for (size_t i = 0; i < A.size(); i++) {
                        vector<string> attr = utils::split(A[i], '=');
                        if (attr.size() == 2) {
                            if (utils::tolower(attr[0]) == utils::tolower("name")) {
                                lastFieldKey = attr[1].substr(1, attr[1].size() - 2);
                            }
                        }
                        else if (attr.size() == 1) {
                        }
                        else
                            throw Server::Exception("Invalid body attribute");
                    }
                }
                else if (utils::tolower(R[0]) == utils::tolower("Content-Type")) {
                    if (utils::tolower(R[1]) == utils::tolower("application/octet-stream"))
                        shouldBeEmpty = true;
                    else if (utils::tolower(R[1].substr(0, R[1].find("/"))) !=
                             utils::tolower("text"))
                        throw Server::Exception("Unsupported file type: " + R[1]);
                }
            } break;
            case BODY_BODY: {
                if (line == "--" + boundary || line == "--" + boundary + "--") {
                    req->setBodyParam(lastFieldKey,
                                      lastFieldValue.substr(string("\r\n").size()),
                                      false);
                    lastFieldKey = "";
                    lastFieldValue = "";
                    state = BODY_HEADER;
                    shouldBeEmpty = false;
                }
                else if (shouldBeEmpty && !line.empty())
                    throw Server::Exception("Unsupported file type: " +
                                            string("application/octet-stream"));
                else
                    lastFieldValue += "\r\n" + line;
            } break;
            }
        }
    }
    catch (const Server::Exception&) {
        throw;
    }
    catch (...) {
        throw Server::Exception("Error on parsing request");
    }
    return req;
}

Server::Server(int _port) : port(_port) {
#ifdef _WIN32
    WSADATA wsa_data;
    int initializeResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (initializeResult != 0) {
        throw Exception("Error: WinSock WSAStartup failed: " +
                        string(getSocketError()));
    }
#endif

    notFoundHandler = new NotFoundHandler();

    sc = socket(AF_INET, SOCK_STREAM, 0);
    int sc_option = 1;

#ifdef _WIN32
    setsockopt(sc, SOL_SOCKET, SO_REUSEADDR, (char*)&sc_option,
               sizeof(sc_option));
#else
    setsockopt(sc, SOL_SOCKET, SO_REUSEADDR, &sc_option, sizeof(sc_option));
#endif
    if (!ISVALIDSOCKET(sc))
        throw Exception("Error on opening socket: " + string(getSocketError()));

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (::bind(sc, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        throw Exception("Error on binding: " + string(getSocketError()));
    }
}

void Server::get(string path, RequestHandler* handler) {
    Route* route = new Route(GET, path);
    route->setHandler(handler);
    routes.push_back(route);
}

void Server::post(string path, RequestHandler* handler) {
    Route* route = new Route(POST, path);
    route->setHandler(handler);
    routes.push_back(route);
}

void Server::run() {
    ::listen(sc, 10);

    struct sockaddr_in cli_addr;
    socklen_t clilen;
    clilen = sizeof(cli_addr);
    SOCKET newsc;

    while (true) {
        newsc = ::accept(sc, (struct sockaddr*)&cli_addr, &clilen);
        if (!ISVALIDSOCKET(newsc))
            throw Exception("Error on accept: " + string(getSocketError()));
        Response* res = NULL;
        try {
            char* data = new char[BUFSIZE + 1];
            size_t recv_len, recv_total_len = 0;
            Request* req = NULL;
            while (!req) {
                recv_len =
                    recv(newsc, data + recv_total_len, BUFSIZE - recv_total_len, 0);
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
            for (; i < routes.size(); i++) {
                if (routes[i]->isMatch(req->getMethod(), req->getPath())) {
                    res = routes[i]->handle(req);
                    break;
                }
            }
            if (i == routes.size() && notFoundHandler) {
                res = notFoundHandler->callback(req);
            }
            delete req;
        }
        catch (const Exception& exc) {
            delete res;
            res = ServerErrorHandler::callback(exc.getMessage());
        }
        int si;
        res->log();
        string res_data = res->print(si);
        delete res;
        int wr = send(newsc, res_data.c_str(), si, 0);
        if (wr != si)
            throw Exception("Send error: " + string(getSocketError()));
        CLOSESOCKET(newsc);
    }
}

Server::~Server() {
    if (sc >= 0)
        CLOSESOCKET(sc);
    delete notFoundHandler;
    for (size_t i = 0; i < routes.size(); ++i)
        delete routes[i];

#ifdef _WIN32
    WSACleanup();
#endif
}

Server::Exception::Exception(const string msg) { message = msg; }

string Server::Exception::getMessage() const { return message; }

ShowFile::ShowFile(string _filePath, string _fileType) {
    filePath = _filePath;
    fileType = _fileType;
}

Response* ShowFile::callback(Request* req) {
    Response* res = new Response;
    res->setHeader("Content-Type", fileType);
    res->setBody(utils::readFile(filePath.c_str()));
    return res;
}

ShowPage::ShowPage(string filePath)
    : ShowFile(filePath, "text/" + utils::getExtension(filePath)) {}

ShowImage::ShowImage(string filePath)
    : ShowFile(filePath, "image/" + utils::getExtension(filePath)) {}

void Server::setNotFoundErrPage(std::string notFoundErrPage) {
    delete notFoundHandler;
    notFoundHandler = new NotFoundHandler(notFoundErrPage);
}

RequestHandler::~RequestHandler() {}

TemplateHandler::TemplateHandler(string _filePath) {
    filePath = _filePath;
    parser = new TemplateParser(filePath);
}

Response* TemplateHandler::callback(Request* req) {
    map<string, string> context;
    context = this->handle(req);
    Response* res = new Response;
    res->setHeader("Content-Type", "text/html");
    res->setBody(parser->getHtml(context));
    return res;
}

map<string, string> TemplateHandler::handle(Request* req) {
    map<string, string> context;
    return context;
}
