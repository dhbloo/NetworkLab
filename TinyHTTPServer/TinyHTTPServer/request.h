#ifndef _HEADER_REQUEST_
#define _HEADER_REQUEST_
#pragma once

#include <string>
#include <map>

struct Request {
    enum Method { 
        GET = 1, 
        HEAD = 2, 
        POST = 4, 
        PUT = 8, 
        DEL = 16,
        OPTIONS = 32,
        UNSUPPORTED = 0
    } method;

    std::string methodStr;

    std::string version;

    std::string url;

    std::map<std::string, std::string> headers;

    std::map<std::string, std::string> querys;

    std::map<std::string, std::string> urlParams;

    std::string body;


    static Request parse(std::stringstream& ss);
};


namespace RequestParse {
    Request parse(std::stringstream& ss);
}


#endif