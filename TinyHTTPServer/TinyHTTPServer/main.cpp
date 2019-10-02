#include "HttpServer.h"

#include <iostream>

int main() {

    try {
        Router router;
        router.setRoute("/", nullptr, 1);

        HttpServer server(5000, router, std::cout);

        server.run();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}