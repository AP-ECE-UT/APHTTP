#ifndef REQUEST_HPP_INCLUDE
#define REQUEST_HPP_INCLUDE

#include <string>
#include <unordered_map>

#include "utilities.hpp"

class Request {
public:
    enum class Method {
        GET,
        POST,
        PUT,
        DEL,
    };

    Request(Method method);
    Request(const std::string& method);

    void setPath(const std::string& path);
    void setHeader(const std::string& key, const std::string& value, bool encode = true);
    void setBody(const std::string& body);
    void setQueryParam(const std::string& key, const std::string& value, bool encode = true);
    void setBodyParam(const std::string& key, const std::string& value,
                      const std::string& contentType = "text/plain", bool encode = true);

    Method getMethod() const;
    std::string getPath() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    std::string getQueryParam(const std::string& key) const;
    std::string getBodyParam(const std::string& key) const;
    std::string getSessionId() const;

    void log() const;

private:
    Method method_;
    std::string path_;
    utils::CiMap headers_;
    utils::CiMap query_;
    utils::CiMap body_;
    utils::CiMap bodyTypes_;

    static const std::unordered_map<std::string, Method> methodMap_;
};

#endif // REQUEST_HPP_INCLUDE
