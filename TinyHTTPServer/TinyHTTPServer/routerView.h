#ifndef _HEADER_ROUTERVIEW_
#define _HEADER_ROUTERVIEW_
#pragma once

#include "router.h"
#include "view.h"

class RouterView : public View {
    Router router;

public:
    RouterView(const Router& router);

    bool handle(Request& request, Response& response) override;
    bool beforeRequest(Request& request, Response& response) override;
    bool afterRequest(Request& request, Response& response) override;
};

#endif