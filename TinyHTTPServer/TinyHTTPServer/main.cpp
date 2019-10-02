#include "httpServer.h"
#include "staticFileView.h"
#include "request.h"

#include <iostream>

int main() {

    try {
        Router router;
        ViewPtr sfv = std::make_shared<StaticFileView>(R"(C:\Users\dhb\Desktop\test)");
        router.setRoute("/", Request::GET, sfv);
        router.setRoute("/<path:filepath>", Request::GET, sfv);
        router.setErrorHandler(404, nullptr);

        HttpServer server(5000, router, std::cout);

        server.run();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}