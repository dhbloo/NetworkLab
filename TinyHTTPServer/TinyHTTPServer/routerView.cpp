#include "routerView.h"
#include "request.h"
#include "response.h"
#include "requestExcept.h"

RouterView::RouterView(const Router& router) : router(router) {}


void RouterView::handle(Request& request, Response& response) {
    beforeRequest(request, response);
    
    auto routerKV = request.urlParams.find("router");
    if (routerKV == request.urlParams.end())
        throw Abort(500, "No router param in url");

    Request subRequest = request;
    subRequest.url = routerKV->second;

    ViewPtr viewPtr = router.resolve(subRequest, response);
    try {
        viewPtr->handle(request, response);
    }
    catch (Abort a) {
        response.statusCode = a.statusCode;

        if (ViewPtr errorView = router.getErrorHandler(a.statusCode))
            errorView->handle(request, response);
        else
            throw;
    }

    afterRequest(request, response);
}
