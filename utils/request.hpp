#ifndef REQUEST_HPP_INCLUDE
#define REQUEST_HPP_INCLUDE

#include <string>

#include "../utils/include.hpp"
#include "../utils/utilities.hpp"

class Request {
public:
    Request(std::string method = "GET");
    std::string getPath();
    void setPath(std::string);
    Method getMethod();
    void setMethod(Method);
    std::string getQueryParam(std::string key);
    void setQueryParam(std::string key, std::string value, bool encode = true);
    std::string getBodyParam(std::string key);
    void setBodyParam(std::string key, std::string value, std::string contentType = "text/plain", bool encode = true);
    std::string getHeader(std::string key);
    void setHeader(std::string key, std::string value, bool encode = true);
    std::string getBody();
    std::string getSessionId();
    void setSessionId(std::string);
    std::string getQueryString();
    utils::CiMap getHeaders();
    std::string getHeadersString();
    void setHeaders(std::string headers);
    void setQuery(std::string query);
    void setBody(std::string body);
    void log();
    static void serializeToFile(Request* req, std::string filePath);
    static void deserializeFromFile(Request* req, std::string filePath);

private:
    std::string path;
    Method method;
    utils::CiMap headers;
    utils::CiMap query;
    utils::CiMap body;
    utils::CiMap bodyTypes;
};

#endif // REQUEST_HPP_INCLUDE
