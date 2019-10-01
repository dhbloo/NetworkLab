#include "response.h"
#include <sstream>

std::map<int, std::string> Response::statusMap = {
    {200, "OK"},
    {400, "Bad Request"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
};

std::string Response::toString() const {

    std::stringstream ss;

    ss << version << ' ' << statusCode << ' ' << statusMap[statusCode] << "\r\n";


    return ss.str();
}
