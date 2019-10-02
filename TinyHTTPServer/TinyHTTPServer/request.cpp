#include "request.h"
#include "requestExcept.h"
#include <vector>
#include <map>
#include <sstream>

static const std::map<std::string, Request::Method> MethodMap = {
    {"GET", Request::GET},
    {"HEAD", Request::HEAD},
    {"POST", Request::POST},
    {"PUT", Request::PUT},
    {"DELETE", Request::DEL},
    {"OPTIONS", Request::OPTIONS}
};

static const std::vector<std::string> UnImpMethods = {
    "TRACE",
    "CONNECT",
    "LINK",
    "UNLINK"
};

Request Request::parse(std::stringstream& ss) {
    Request request;

    // 解析请求行
    ss >> request.methodStr >> request.url >> request.version;

    // 解析HTTP方法
    auto methodIt = MethodMap.find(request.methodStr);
    if (methodIt != MethodMap.end()) {
        request.method = methodIt->second;
    }
    else if (std::find(UnImpMethods.begin(), UnImpMethods.end(), request.methodStr) != UnImpMethods.end()) {
        request.method = Request::UNSUPPORTED;
        return request;
    }
    else {
        throw Abort(400, "Not a http request");
    }

    // 解析URL中的查询请求
    size_t questMarkPos = request.url.find_first_of('?');
    if (questMarkPos != std::string::npos) {
        std::string queryStr = request.url.substr(questMarkPos + 1);
        request.url = request.url.substr(0, questMarkPos);

        size_t delimPos, lastPos = 0;
        do {
            delimPos = queryStr.find_first_of('&', lastPos);
            std::string kvStr = queryStr.substr(lastPos, delimPos - lastPos);
            lastPos = delimPos + 1;

            size_t equalPos = kvStr.find_first_of('=');
            if (equalPos == std::string::npos) {
                throw Abort(400, "Error when parsing request query url");
            }

            request.querys[kvStr.substr(0, equalPos)] = kvStr.substr(equalPos + 1);

        } while (delimPos != std::string::npos);
    }

    // 解析Headers
    std::string buf = ss.str();
    size_t lineStartPos;
    size_t lineEndPos = buf.find("\r\n");
    do {
        lineStartPos = lineEndPos + 2;
        lineEndPos = buf.find("\r\n", lineStartPos);
        if (lineEndPos > lineStartPos) {
            std::string_view line(buf.data() + lineStartPos, lineEndPos - lineStartPos);
            size_t delimPos = line.find_first_of(':');

            if (delimPos == std::string::npos)
                throw Abort(400, "Error when parsing request header");

            request.headers[std::string(line.substr(0, delimPos))] = line.substr(delimPos + 2);
        }
    } while (lineEndPos > lineStartPos);

    // 剩余的部分为主体
    std::string_view body(buf);
    request.body = buf.substr(lineEndPos + 2);

    return request;
}
