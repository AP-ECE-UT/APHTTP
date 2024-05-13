#include "route.hpp"

#include "server.hpp"

Route::Route(Request::Method _method, const std::string& _path)
    : method(_method),
      path(_path),
      handler(nullptr) {}

Route::~Route() {
    delete handler;
}

void Route::setHandler(RequestHandler* _handler) {
    handler = _handler;
}

Response* Route::handle(Request* req) {
    return handler->callback(req);
}

bool Route::isMatch(Request::Method _method, const std::string& url) {
    return (_method == method) && (url == path);
}
