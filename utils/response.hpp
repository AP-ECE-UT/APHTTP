#ifndef __RESPONSE__
#define __RESPONSE__
#include "../include.hpp"
#include <map>
#include <string>

#define BUFSIZE 8096

const std::string SERVER_NAME = "AP HTTP Server";

class Response {
public:
  Response();
  char *print();
  void log();
  void setHeader(std::string name, std::string value);
  void setBody(std::string _body);
  void setStatus(int code, std::string phrase);
  int getStatusCode();
  std::string getStatusPhrase();
  std::string getHeader(std::string name);

private:
  int code;
  std::string phrase;
  std::string body;
  std::map<std::string, std::string> headers;
};

#endif
