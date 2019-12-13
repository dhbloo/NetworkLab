#ifndef _HEADER_FUNCVIEW_
#define _HEADER_FUNCVIEW_
#pragma once

#include "request.h"
#include "response.h"
#include "view.h"
#include <memory>

// 函数View
// 以任意的lambda函数处理请求
template<typename Func> class FuncView : public View
{
    Func func;

public:
    FuncView(Func func) : func(func) {}

    void handle(Request &request, Response &response) override { func(request, response); }
};

template<typename Func> inline auto MakeFuncView(Func func)
{
    return std::make_shared<FuncView<Func>>(func);
}

#endif