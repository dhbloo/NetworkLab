#ifndef _HEADER_STATICFILEVIEW_
#define _HEADER_STATICFILEVIEW_
#pragma once

#include "view.h"

#include <string>

// ��̬�ļ�View
// ��URL����Ϊ��̬�ļ�����
class StaticFileView : public View
{
    std::string directory;    // ����Ŀ¼�ڱ��ص�·��
    std::string defaultExt;   // Ĭ����չ��
    std::string defaultFile;  // Ĭ���ļ���

public:
    StaticFileView(std::string path,
                   std::string defaultExt  = ".html",
                   std::string defaultFile = "index.html");

    void handle(Request &request, Response &response) override;
};

#endif