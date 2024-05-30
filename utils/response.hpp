#ifndef RESPONSE_HPP_INCLUDE
#define RESPONSE_HPP_INCLUDE

#include <string>
#include <unordered_map>

#include "../utils/utilities.hpp"

class Response {
public:
    enum class Status {
        ok = 200,
        created = 201,

        movedPermanently = 301,
        seeOther = 303,

        badRequest = 400,
        unauthorized = 401,
        forbidden = 403,
        notFound = 404,
        methodNotAllowed = 405,
        conflict = 409,
        teapot = 418,

        internalServerError = 500,
        notImplemented = 501,
    };

    Response(Status code = Status::ok);
    Response(int code, const std::string& phrase);

    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& _body);
    void setSessionId(const std::string& sessionId);

    std::string print();
    void log(bool showBody = false);

    static Response* redirect(const std::string& url);

private:
    int code;
    std::string phrase;
    std::string body;
    utils::CiMap headers;

    static const std::unordered_map<Status, std::string> httpPhraseMap;
};

#endif // RESPONSE_HPP_INCLUDE
