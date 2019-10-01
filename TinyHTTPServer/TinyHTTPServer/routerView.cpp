#include "routerView.h"
#include "request.h"
#include "response.h"

RouterView::RouterView(const Router& router) : router(router) {}

#define HANDLER(func) \
bool RouterView::func(Request& request, Response& response) { \
    Request subRequest = request; \
    subRequest.url = request.urlParams["router"]; \
    ViewPtr viewPtr = router.resolve(subRequest); \
    if (viewPtr) { return viewPtr->func(request, response); } \
    else { response.statusCode = 404; return false; } \
}

HANDLER(handle);
HANDLER(beforeRequest);
HANDLER(afterRequest);