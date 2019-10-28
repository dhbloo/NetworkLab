#include "app.h"

#include "mainFrame.h"

#include <TinyHTTPServer/httpServer.h>
#include <TinyHTTPServer/router.h>

std::unique_ptr<HttpServer> App::GlobalHttpServer;
std::unique_ptr<Router>     App::GlobalRouter;

bool App::OnInit()
{
    GlobalHttpServer = nullptr;
    GlobalRouter     = nullptr;

    MainFrame *frame = new MainFrame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP(App);