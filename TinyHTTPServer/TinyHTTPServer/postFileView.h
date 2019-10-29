#ifndef _HEADER_POSTFILEVIEW_
#define _HEADER_POSTFILEVIEW_
#pragma once

#include "view.h"

#include <string>

// 文件提交View
// 将POST请求的内容保存为文件
class PostFileView : public View
{
    std::string directory;    // 访问目录在本地的路径

public:
    PostFileView(std::string path);

    void handle(Request &request, Response &response) override;
};

#endif