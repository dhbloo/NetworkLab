#pragma once

class HttpServer;
class Router;

#include "wxcommon.h"

#include <memory>

class App : public wxApp
{
public:
    static std::unique_ptr<HttpServer> GlobalHttpServer;
    static std::unique_ptr<Router>     GlobalRouter;

    virtual bool OnInit() override;
};
