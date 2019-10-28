#ifndef _HEADER_VIEW_
#define _HEADER_VIEW_
#pragma once

class Request;
class Response;

// View 是具体的请求处理逻辑
class View
{
public:
    virtual ~View() {}

    // View的请求处理函数
    // 当请求被导向View时,调用该函数处理请求
    virtual void handle(Request &request, Response &response) = 0;
};

#endif