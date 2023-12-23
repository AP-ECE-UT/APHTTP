#ifndef MY_SERVER_HPP_INCLUDE
#define MY_SERVER_HPP_INCLUDE

#include "../server/server.hpp"

class MyServer : public Server {
public:
    MyServer(int port = 5000);
};

#endif // MY_SERVER_HPP_INCLUDE
