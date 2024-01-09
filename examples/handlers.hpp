#ifndef HANDLERS_HPP_INCLUDE
#define HANDLERS_HPP_INCLUDE

#include <cstdlib> // for rand and srand
#include <ctime>   // for time
#include <iostream>

#include "../server/server.hpp"

class RandomNumberHandler : public RequestHandler {
public:
    Response* callback(Request*);
};

class LoginHandler : public RequestHandler {
public:
    Response* callback(Request*);
};

class UploadHandler : public RequestHandler {
public:
    Response* callback(Request*);
};

class ColorHandler : public TemplateHandler {
public:
    ColorHandler(std::string filePath);
    std::map<std::string, std::string> handle(Request* req);
};

#endif // HANDLERS_HPP_INCLUDE
