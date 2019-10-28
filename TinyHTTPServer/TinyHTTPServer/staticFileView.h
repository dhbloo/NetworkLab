#ifndef _HEADER_STATICFILEVIEW_
#define _HEADER_STATICFILEVIEW_
#pragma once

#include "view.h"

#include <string>

// 静态文件View
// 将URL解析为静态文件访问
class StaticFileView : public View
{
    std::string directory;    // 访问目录在本地的路径
    std::string defaultExt;   // 默认扩展名
    std::string defaultFile;  // 默认文件名

public:
    StaticFileView(std::string path,
                   std::string defaultExt  = ".html",
                   std::string defaultFile = "index.html");

    void handle(Request &request, Response &response) override;
};

#endif