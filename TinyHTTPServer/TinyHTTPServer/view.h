#ifndef _HEADER_VIEW_
#define _HEADER_VIEW_
#pragma once

class Request;
class Response;

class View {

public:
    virtual bool handle(const Request& request, Response& response) = 0;
};

#endif