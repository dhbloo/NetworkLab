#ifndef _HEADER_RESPONSE_
#define _HEADER_RESPONSE_
#pragma once

#include <string>
#include <map>


struct Response {
    static std::map<int, std::string> statusMap;

    std::string version;

    int statusCode;

    std::map<std::string, std::string> headers;

    std::string body;


    std::string toString() const;
    std::string statusInfo() const;
};

#endif