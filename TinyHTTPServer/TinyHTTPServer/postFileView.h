#ifndef _HEADER_POSTFILEVIEW_
#define _HEADER_POSTFILEVIEW_
#pragma once

#include "view.h"

#include <string>

// �ļ��ύView
// ��POST��������ݱ���Ϊ�ļ�
class PostFileView : public View
{
    std::string directory;    // ����Ŀ¼�ڱ��ص�·��

public:
    PostFileView(std::string path);

    void handle(Request &request, Response &response) override;
};

#endif