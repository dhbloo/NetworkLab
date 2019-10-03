#include "httpServer.h"
#include "staticFileView.h"
#include "funcView.h"
#include "request.h"

#include <iostream>

int main() {

    try {
        Router router;
        router.setRoute("/<path:filepath>", Request::GET,
            std::make_shared<StaticFileView>(R"(C:\Users\dhb\Desktop\test)"));
        router.setErrorHandler(0, MakeFuncView(
            [](auto& req, auto& res) {
                res.body = R"(<h1 style="text-align:center;">)"
                         + std::to_string(res.statusCode) + " "
                         + res.statusInfo() + "!</h1>";
            }));

        HttpServer server(5000, router, std::cout);

        server.run();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}