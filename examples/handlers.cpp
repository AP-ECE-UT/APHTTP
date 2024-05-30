#include "handlers.hpp"

#include <cstdlib>
#include <iostream>

Response* RandomNumberHandler::callback(Request* req) {
    Response* res = new Response();
    res->setHeader("Content-Type", "text/html");
    std::string body;
    body += "<!DOCTYPE html>";
    body += "<html>";
    body += "<body style=\"text-align: center;\">";
    body += "<h1>AP HTTP</h1>";
    body += "<p>";
    body += "a random number in [1, 10] is: ";
    body += std::to_string(std::rand() % 10 + 1);
    body += "</p>";
    body += "<p>";
    body += "SessionId: ";
    body += req->getSessionId();
    body += "</p>";
    body += "</body>";
    body += "</html>";
    res->setBody(body);
    return res;
}

Response* LoginHandler::callback(Request* req) {
    std::string username = req->getBodyParam("username");
    std::string password = req->getBodyParam("password");
    if (username == "root") {
        throw Server::Exception("Remote root access has been disabled.");
    }
    std::cout << "username: " << username << ",\tpassword: " << password << std::endl;
    Response* res = Response::redirect("/rand");
    res->setSessionId("SID");
    return res;
}

Response* UploadHandler::callback(Request* req) {
    std::string name = req->getBodyParam("file_name");
    std::string file = req->getBodyParam("file");
    utils::writeToFile(file, name);
    Response* res = Response::redirect("/");
    return res;
}

ColorHandler::ColorHandler(const std::string& filePath) : TemplateHandler(filePath) {}

std::map<std::string, std::string> ColorHandler::handle(Request* req) {
    std::string newName = "I am " + req->getQueryParam("name");
    std::map<std::string, std::string> context;
    context["name"] = newName;
    context["color"] = req->getQueryParam("color");
    return context;
}
