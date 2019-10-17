#ifndef _HEADER_STATICFILEVIEW_
#define _HEADER_STATICFILEVIEW_
#pragma once

#include "view.h"
#include <string>

// ��̬�ļ�View
// ��URL����Ϊ��̬�ļ�����
class StaticFileView : public View {
    std::string directory;  // ����Ŀ¼�ڱ��ص�·��

public:
    static std::string DefaultExt;  // Ĭ����չ��

    StaticFileView(std::string path);

    void handle(Request& request, Response& response) override;
};


#endif