#ifndef _HEADER_VIEW_
#define _HEADER_VIEW_
#pragma once

class Request;
class Response;

// View �Ǿ�����������߼�
class View
{
public:
    virtual ~View() {}

    // View����������
    // �����󱻵���Viewʱ,���øú�����������
    virtual void handle(Request &request, Response &response) = 0;
};

#endif