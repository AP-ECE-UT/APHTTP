#ifndef ROUTE_HPP_INCLUDE
#define ROUTE_HPP_INCLUDE

#include <string>

#include "../utils/include.hpp"
#include "../utils/request.hpp"
#include "../utils/response.hpp"

class RequestHandler;

class Route {
public:
    Route(Request::Method _method, const std::string& _path);
    ~Route();

    void setHandler(RequestHandler* _handler);
    Response* handle(Request* req);
    bool isMatch(Request::Method, const std::string& url);

private:
    Request::Method method;
    std::string path;
    RequestHandler* handler;
};

#endif // ROUTE_HPP_INCLUDE
