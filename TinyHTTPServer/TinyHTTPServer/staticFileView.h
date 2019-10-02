#ifndef _HEADER_STATICFILEVIEW_
#define _HEADER_STATICFILEVIEW_
#pragma once

#include "view.h"
#include <string>

class StaticFileView : public View {
    std::string directory;

public:
    static std::string DefaultExt;

    StaticFileView(std::string path);

    void handle(Request& request, Response& response) override;
};


#endif