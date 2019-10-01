#include "HttpServer.h"

#include <iostream>

#include "router.h"
#include "request.h"

int main() {

    Router router;
    router.setRoute("/", 100);
    router.setRoute("/ids/<id>/name/<name>/<g>/<h>/<path:t>", 101);

    Request request;
    request.url = "/";
    std::cout << router.resolve(request) << std::endl;

    request.url = "/ids/36/name/John/gender/male/2233/4455/6677";
    std::cout << router.resolve(request) << std::endl;
    for (auto p : request.urlParams) {
        std::cout << p.first << " : " << p.second << std::endl;
    }

    return 0;

    try {
        HttpServer server(5000, std::cout);

        server.run();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}