#ifndef _HEADER_ROUTERVIEW_
#define _HEADER_ROUTERVIEW_
#pragma once

#include "router.h"
#include "view.h"

// Router View 可以用来在View嵌套Router
class RouterView : public View
{
    Router router;

public:
    RouterView(const Router &router);

    void handle(Request &request, Response &response) override;

    virtual void beforeRequest(Request &request, Response &response) {}
    virtual void afterRequest(Request &request, Response &response) {}
};

#endif