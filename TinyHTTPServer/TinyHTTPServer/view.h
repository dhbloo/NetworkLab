#ifndef _HEADER_VIEW_
#define _HEADER_VIEW_
#pragma once

class Request;
class Response;

class View {
public:
    virtual ~View() {}

    virtual void handle(Request& request, Response& response) {}
};

#endif