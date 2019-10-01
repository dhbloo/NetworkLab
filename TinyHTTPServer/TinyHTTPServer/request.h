#ifndef _HEADER_REQUEST_
#define _HEADER_REQUEST_
#pragma once

#include <sstream>
#include <string>
#include <map>

struct Request {
    enum Method { GET, HEAD, UNSUPPORTED } method;

    std::string methodStr;

    std::string version;

    std::string url;

    std::map<std::string, std::string> headers;

    std::string body;


    static Request parse(std::stringstream& ss);
};


namespace RequestParse {
    Request parse(std::stringstream& ss);
}


#endif