#ifndef SERVER_HPP_INCLUDE
#define SERVER_HPP_INCLUDE

#include <exception>
#include <string>
#include <vector>

#include "../utils/include.hpp"
#include "../utils/request.hpp"
#include "../utils/response.hpp"
#include "../utils/template_parser.hpp"
#include "route.hpp"

#ifdef _WIN32
typedef unsigned SOCKET;
#else
typedef int SOCKET;
#endif

class TemplateParser;

class RequestHandler {
public:
    virtual ~RequestHandler();
    virtual Response* callback(Request* req) = 0;
};

class ShowFile : public RequestHandler {
public:
    ShowFile(const std::string& filePath, const std::string& fileType);
    Response* callback(Request* req) override;

private:
    std::string filePath_;
    std::string fileType_;
};

class ShowPage : public ShowFile {
public:
    ShowPage(const std::string& filePath);
};

class ShowImage : public ShowFile {
public:
    ShowImage(const std::string& filePath);
};

class TemplateHandler : public RequestHandler {
public:
    TemplateHandler(const std::string& filePath);
    ~TemplateHandler();

    Response* callback(Request* req) override;
    virtual std::map<std::string, std::string> handle(Request* req);

private:
    std::string filePath_;
    TemplateParser* parser_;
};

class Server {
public:
    Server(int port = 5000);
    ~Server();

    void run();

    void get(const std::string& path, RequestHandler* handler);
    void post(const std::string& path, RequestHandler* handler);
    void put(const std::string& path, RequestHandler* handler);
    void del(const std::string& path, RequestHandler* handler);
    void setNotFoundErrPage(const std::string& notFoundErrPage);

    class Exception : public std::exception {
    public:
        Exception() = default;
        Exception(const std::string message);
        std::string getMessage() const;

    private:
        std::string message_;
    };

private:
    SOCKET sc_;
    int port_;
    std::vector<Route*> routes_;
    RequestHandler* notFoundHandler_;

    void mapRequest(const std::string& path, RequestHandler* handler, Request::Method method);
};

#endif // SERVER_HPP_INCLUDE
