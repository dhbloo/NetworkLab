#include "routerView.h"

#include "request.h"
#include "requestExcept.h"
#include "response.h"

RouterView::RouterView(const Router &router) : router(router) {}

void RouterView::handle(Request &request, Response &response)
{
    beforeRequest(request, response);

    auto routerKV = request.urlParams.find("router");
    if (routerKV == request.urlParams.end())
        throw Abort(500, "No router param in url");

    Request subRequest = request;
    if (routerKV->second.empty() || routerKV->second.front() != '/')
        subRequest.url = "/" + routerKV->second;
    else
        subRequest.url = routerKV->second;

    ViewPtr viewPtr = router.resolve(subRequest, response);
    try {
        viewPtr->handle(subRequest, response);
    }
    catch (Abort a) {
        response.statusCode = a.statusCode;
        request.headers["error"] = a.what();

        // 在Router中寻找是否有局部/全局错误处理View
        ViewPtr errorView = router.getErrorHandler(response.statusCode);
        if (errorView || (errorView = router.getErrorHandler(0)))
            errorView->handle(subRequest, response);
        else
            throw;
    }

    afterRequest(request, response);
}
