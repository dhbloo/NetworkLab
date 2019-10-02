#include "routerView.h"
#include "request.h"
#include "response.h"

RouterView::RouterView(const Router& router) : router(router) {}


void RouterView::handle(Request& request, Response& response) {
    beforeRequest(request, response);

    Request subRequest = request;
    subRequest.url = request.urlParams["router"];

    if (ViewPtr viewPtr = router.resolve(subRequest)) {
        viewPtr->handle(request, response);
    }
    else {
        response.statusCode = 404;
    }

    afterRequest(request, response);
}
