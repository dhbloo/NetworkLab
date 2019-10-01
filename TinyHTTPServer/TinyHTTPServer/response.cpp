#include "response.h"
#include <sstream>
#include <cassert>

std::map<int, std::string> Response::statusMap = {
    {200, "OK"},

    {400, "Bad Request"},

    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {505, "HTTP Version Not Supported"},
};

std::string Response::toString() const {
    std::stringstream ss;

    // ����״̬��
    ss << version << ' ' << statusCode << ' ' << statusInfo() << "\r\n";

    // ����headers
    for (const auto& header : headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }

    // ����body
    ss << "\r\n" << body;

    return ss.str();
}

std::string Response::statusInfo() const {
    assert(statusMap.find(statusCode) != statusMap.end());
    return statusMap[statusCode];
}
