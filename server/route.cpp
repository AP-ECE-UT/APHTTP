#include "route.hpp"

#include "server.hpp"

Route::Route(Request::Method method, const std::string& path)
    : method_(method),
      path_(path),
      handler_(nullptr) {}

Route::~Route() {
    delete handler_;
}

void Route::setHandler(RequestHandler* handler) {
    handler_ = handler;
}

Response* Route::handle(Request* req) {
    return handler_->callback(req);
}

bool Route::isMatch(Request::Method method, const std::string& url) {
    return (method_ == method) && (url == path_);
}
