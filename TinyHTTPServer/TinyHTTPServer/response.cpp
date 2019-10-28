#include "response.h"

#include <cassert>
#include <sstream>

static std::map<int, std::string> StatusMap = {
    {200, "OK"},

    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},

    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},

    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {505, "HTTP Version Not Supported"},
};

std::string Response::toString() const
{
    std::stringstream ss;

    // ����״̬��
    ss << version << ' ' << statusCode << ' ' << statusInfo() << "\r\n";

    // ����headers
    for (const auto &header : headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }

    // ����body
    ss << "\r\n" << body;

    return ss.str();
}

std::string Response::statusInfo() const
{
    if (StatusMap.find(statusCode) != StatusMap.end())
        return StatusMap[statusCode];
    else
        return "";
}
