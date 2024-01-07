#include <iostream>
#include <string>

#include "../server/server.hpp"
#include "handlers.hpp"

void mapServerPaths(Server& server) {
    server.setNotFoundErrPage("static/404.html");
    server.get("/login", new ShowPage("static/logincss.html"));
    server.post("/login", new LoginHandler());
    server.get("/up", new ShowPage("static/upload_form.html"));
    server.post("/up", new UploadHandler());
    server.get("/rand", new RandomNumberHandler());
    server.get("/home.png", new ShowImage("static/home.png"));
    server.get("/", new ShowPage("static/home.html"));
    server.get("/colors", new ColorHandler("template/colors.html"));
}

int main(int argc, char** argv) {
    try {
        int port = argc > 1 ? std::stoi(argv[1]) : 5000;
        Server server(port);
        mapServerPaths(server);
        std::cout << "Server running on port: " << port << std::endl;
        server.run();
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
    }
    catch (const Server::Exception& e) {
        std::cerr << e.getMessage() << std::endl;
    }
    return 0;
}
