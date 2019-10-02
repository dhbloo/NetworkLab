#include "routerView.h"
#include "request.h"
#include "response.h"
#include "requestExcept.h"

RouterView::RouterView(const Router& router) : router(router) {}


void RouterView::handle(Request& request, Response& response) {
    beforeRequest(request, response);

    Request subRequest = request;
    subRequest.url = request.urlParams["router"];

    if (ViewPtr viewPtr = router.resolve(subRequest)) {
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
    }
    else {
        response.statusCode = 404;
    }

    afterRequest(request, response);
}
