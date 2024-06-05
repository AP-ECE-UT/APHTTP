#ifndef ROUTE_HPP_INCLUDE
#define ROUTE_HPP_INCLUDE

#include <string>

#include "../utils/request.hpp"

class Response;
class RequestHandler;

class Route {
public:
    Route(Request::Method method, const std::string& path);
    ~Route();

    void setHandler(RequestHandler* handler);
    Response* handle(Request* req);
    bool isMatch(Request::Method, const std::string& url);

private:
    Request::Method method_;
    std::string path_;
    RequestHandler* handler_;
};

#endif // ROUTE_HPP_INCLUDE
