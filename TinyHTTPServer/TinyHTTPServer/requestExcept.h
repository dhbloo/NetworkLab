#ifndef _HEADER_REQUESTEXCEPT_
#define _HEADER_REQUESTEXCEPT_
#pragma once

#include <stdexcept>
#include <string>

class Abort : public std::logic_error {
public:
    int statusCode;

    Abort(int code, const char* message)
        : std::logic_error(message), statusCode(code) {}
    Abort(int code, std::string message)
        : std::logic_error(message), statusCode(code) {}
};


class Redirect : public std::logic_error {
public:
    std::string url;

    Redirect(std::string url, const char* message)
        : std::logic_error(message), url(url) {}
    Redirect(std::string url, std::string message)
        : std::logic_error(message), url(url) {}
};


#endif
