#ifndef HANDLERS_HPP_INCLUDE
#define HANDLERS_HPP_INCLUDE

#include <map>
#include <string>

#include "../server/server.hpp"

class RandomNumberHandler : public RequestHandler {
public:
    Response* callback(Request*) override;
};

class LoginHandler : public RequestHandler {
public:
    Response* callback(Request*) override;
};

class UploadHandler : public RequestHandler {
public:
    Response* callback(Request*) override;
};

class ColorHandler : public TemplateHandler {
public:
    ColorHandler(const std::string& filePath);
    std::map<std::string, std::string> handle(Request* req) override;
};

#endif // HANDLERS_HPP_INCLUDE
