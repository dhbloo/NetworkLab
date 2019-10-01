#include "HttpServer.h"

#include <iostream>

int main() {

    try {
        HttpServer server(5000, std::cout);

        server.run();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}