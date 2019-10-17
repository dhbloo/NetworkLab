#ifndef _HEADER_STATICFILEVIEW_
#define _HEADER_STATICFILEVIEW_
#pragma once

#include "view.h"
#include <string>

// 静态文件View
// 将URL解析为静态文件访问
class StaticFileView : public View {
    std::string directory;  // 访问目录在本地的路径

public:
    static std::string DefaultExt;  // 默认扩展名

    StaticFileView(std::string path);

    void handle(Request& request, Response& response) override;
};


#endif