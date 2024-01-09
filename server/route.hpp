#ifndef ROUTE_HPP_INCLUDE
#define ROUTE_HPP_INCLUDE

#include <string>

#include "../utils/include.hpp"
#include "../utils/request.hpp"
#include "../utils/response.hpp"

class RequestHandler;

class Route {
private:
    Method method;
    std::string path;
    RequestHandler* handler;

public:
    Route(Method _method, std::string _path);
    ~Route();
    bool isMatch(Method, std::string url);
    Response* handle(Request* req);
    void setHandler(RequestHandler* _handler);
};

#endif // ROUTE_HPP_INCLUDE
