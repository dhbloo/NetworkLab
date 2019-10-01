#ifndef _HEADER_VIEW_
#define _HEADER_VIEW_
#pragma once

class Request;
class Response;

class View {
public:
    virtual ~View() {}
    virtual bool handle(Request& request, Response& response) { return true; }

    virtual bool beforeRequest(Request& request, Response& response) { return true; }
    virtual bool afterRequest(Request& request, Response& response) { return true; }
};

#endif