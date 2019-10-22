#include "httpServer.h"
#include "staticFileView.h"
#include "funcView.h"
#include "requestExcept.h"
#include "request.h"
#include "util.h"

#include <iostream>

int main() {

    try {
        Router router;
        router.setRoute("/time", Request::GET, MakeFuncView(
            [](auto& req, auto& res) { res.body = Rfc1123DateTimeNow(); }));
        router.setRoute("/factorial/<n>", Request::GET, MakeFuncView(
            [](auto& req, auto& res) {
                try {
                    uint64_t n = std::stoi(req.urlParams["n"]);
                    for (uint64_t i = 2, m = n; i < m; i++) n *= i;
                    res.body = std::to_string(n);
                }
                catch (std::invalid_argument e) {
                    throw Abort(400, "Argument is not a number");
                }
            }));
        router.setRoute("/<path:filepath>", Request::GET,
            std::make_shared<StaticFileView>(R"(C:\Users\dhb\Desktop\TestWeb)"));
        router.setErrorHandler(0, MakeFuncView(
            [](auto& req, auto& res) {
                res.body = R"(<h1 style="text-align:center;">)"
                    + std::to_string(res.statusCode) + " "
                    + res.statusInfo() + "!</h1>";
                res.headers["Content-Type"] = "text/html;";
            }));

        HttpServer server(nullptr, 80, router, std::cout);

        server.run();
    }
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}